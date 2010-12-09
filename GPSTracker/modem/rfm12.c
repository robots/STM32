#include "platform.h"
#include "stm32f10x.h"

#include "spi.h"
#include "buf.h"

#include "modem.h"
#include "rfm12.h"

NVIC_InitTypeDef EXT_Int = {
	.NVIC_IRQChannelPreemptionPriority = 15,
	.NVIC_IRQChannelSubPriority = 0,
	.NVIC_IRQChannelCmd = ENABLE,
	.NVIC_IRQChannel = EXTI1_IRQn
};

static uint8_t state = RFM_STATE_SLEEP; 
static uint8_t RFM_GroupID = 0x48;
static uint8_t RFM_Buffer[512];
static uint8_t RFM_Idx;
static uint8_t RFM_Len;

uint16_t RFM_xfer(uint16_t d)
{
	uint16_t ret;
	RFM_CS(Bit_RESET);

	ret = SPI_Xfer(d);

	RFM_CS(Bit_SET);
	return ret;
}

void RFM_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure = {
		.EXTI_Mode = EXTI_Mode_Interrupt,
		.EXTI_Trigger = EXTI_Trigger_Falling,
		.EXTI_LineCmd = ENABLE,
		.EXTI_Line = EXTI_Line1
	};

	SPI_init();

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
	
	EXTI_Init(&EXTI_InitStructure);

	// enable interrupts
	NVIC_Init(&EXT_Int);

	RFM_xfer(0x0000);

	RFM_xfer(RFM_SLEEP_MODE);

	RFM_xfer(RFM_TX_REG);
	while (RFM_IRQ_READ() == Bit_RESET)
		RFM_xfer(0x0000);
	
	RFM_SetBand(RFM_BAND_868MHZ);
	RFM_SetFreq(0x640); // 868mhz = 860 + x * 0.0050 
	RFM_SetDataRate(6); // ~ 49.xx kbps
	// RFM_xfer(0xC606); // ~ 49.xx kbps

	//RFM_xfer(0x94A2); // VDI,FAST,134kHz,0dBm,-91dBm
	RFM_SetRXWidth(5);
	RFM_xfer(0xC2AC); // AL,!ml,DIG,DQD4

	RFM_SetFIFO(RFM_GroupID); // special ID
	
	RFM_xfer(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN 
	RFM_SetTXDeviation(5);
	//RFM_xfer(0x9850); // !mp,90kHz,MAX OUT 
	RFM_xfer(0xCC77);
	RFM_xfer(0xE000); // NOT USE 
	RFM_xfer(0xC800); // NOT USE 
	RFM_xfer(0xC049); // 1.66MHz,3.1V 
				 
	RFM_IdleMode(0);
}

void RFM_SetDataRate(uint8_t r)
{
	if (r == 0)
		r = 1;
	RFM_xfer(0xC600 | r);
}

void RFM_SetTXDeviation(uint8_t d)
{
	d &= 0x0f;
	RFM_xfer(0x9800 | (d << 4) ); // !mp,90kHz,MAX OUT 
}

void RFM_SetRXWidth(uint8_t w)
{
	w &= 0x07;
	RFM_xfer(0x9402 | (w << 5));
}

void RFM_SetGroup(uint8_t g)
{
	RFM_GroupID = g;
	RFM_SetFIFO(g);
}

// g = 0 - receive all packets starting 2D.
//   > 0 - receive all packets starting 2D gg.
void RFM_SetFIFO(uint8_t g)
{
	if (g == 0) {
		RFM_xfer(0xca8b); // fifo level 8, 1byte sync, !ff, dr
		RFM_xfer(0xce2d); 
	} else {
		RFM_xfer(0xca83); // fifo level 8, 2byte sync, !ff, dr
		RFM_xfer(0xce00 | g); 
	}
}

// set band to (433, 868, 913)
void RFM_SetBand(uint8_t b)
{
	b &= 3;
	if (b)
		RFM_xfer(0x80C7 | (b << 4));
}

// set frequency to BAND(430, 860, 900) + b * STEP(0.0025, 0.0050, 0.0075)Mhz
void RFM_SetFreq(uint16_t f)
{
	if (f >= 96 && f <= 3903)
		RFM_xfer(0xA000 | f);

	if (state == RFM_STATE_SCAN) {
		RFM_xfer(0xca81);
		RFM_xfer(0xca83);
	}
}

void RFM_IdleMode()
{
	RFM_SetFIFO(RFM_GroupID); // special ID

	// enter idle mode
	RFM_xfer(RFM_IDLE_MODE);

	state = RFM_STATE_IDLE;
}

void RFM_SniffMode(uint8_t a)
{
	if (a == 'a') {
		// alwais fill fifo
		RFM_xfer(0xca87);
	}

	// enter recv mode
	RFM_xfer(RFM_RECV_MODE);

	RFM_Idx = 0;

	state = RFM_STATE_SNIFF;
}

void RFM_ScanMode()
{
	// alwais fill fifo
	RFM_xfer(0xca87);

	// enable recv mode
	RFM_xfer(RFM_RECV_MODE);

	state = RFM_STATE_SCAN;
}

void RFM_RecvMode()
{
	// enable recv mode
	RFM_xfer(RFM_RECV_MODE);
	
	state = RFM_STATE_RX;
}

uint8_t RFM_IsIdle()
{
	if (state == RFM_STATE_SCAN || state == RFM_STATE_TX) {
		return 0;
	}

	return 1;
}

uint8_t RFM_ReadFifo()
{
	return RFM_xfer(RFM_RX_REG) & 0xff;
}


uint8_t RFM_Send(uint16_t id, uint8_t *data, uint8_t len)
{
	uint8_t i;
	uint8_t chksum = 0;

	if (RFM_IsIdle() == 0)
		return 1;

	
	// enable transmitter
	RFM_xfer(RFM_XMIT_MODE);

	// dummy
	RFM_Buffer[0] = 0xaa;
	RFM_Buffer[1] = 0xaa;

	// fifo enable
	RFM_Buffer[2] = 0x2d;
	RFM_Buffer[3] = RFM_GroupID; //0xA8; // ID

	// data here
	RFM_Buffer[4] = id & 0xff;
	RFM_Buffer[5] = id >> 8;
	RFM_Buffer[6] = len;


	// add data
	for(i = 0; i < len; i++) {
		RFM_Buffer[2+5 + i] = data[i];
	}

	for(i = 0; i < 3+len; i++) {
		chksum += RFM_Buffer[4+i];
	}

	RFM_Buffer[2+5+len] = ~(chksum & 0xFF);
	RFM_Buffer[2+5+len+1] = 0xAA; // dummy byte

	RFM_Len = 2+5+len+2;
	RFM_Idx = 0;

	// add data to fifo
	RFM_xfer(RFM_TX_REG | 0xaa);

	state = RFM_STATE_TX;
	return 0;
}


void EXTI1_IRQHandler(void)
{
	static uint8_t chksum;
	static uint16_t status;
	static uint8_t data;

	EXTI_ClearITPendingBit(EXTI_Line1);

	status = RFM_xfer(0x0000);

	// ignore LBD, EXT, WKUP, POR, FFOV 
	if (status & 0x8000) {
		if ((state == RFM_STATE_SCAN) || (state == RFM_STATE_IDLE)) {
			data = RFM_xfer(RFM_RX_REG);
		} else if (state == RFM_STATE_SNIFF) {
			RFM_Buffer[RFM_Idx] = RFM_xfer(RFM_RX_REG);
			Buf_PushByte(RFM_Buffer[RFM_Idx]);
			Buf_Send();

			RFM_Idx ++;
			if ((RFM_Idx > 2) && (3 + RFM_Buffer[2] + 1 == RFM_Idx)) {
				// restart receive
				RFM_xfer(RFM_IDLE_MODE);
				RFM_xfer(RFM_RECV_MODE);
				RFM_Idx = 0;
			}
		} else if (state == RFM_STATE_RX) {
			RFM_Buffer[RFM_Idx] = RFM_xfer(RFM_RX_REG);
			chksum += RFM_Buffer[RFM_Idx];
			RFM_Idx ++;

			if (RFM_Idx == 2) {
				// get rssi
				ADC_SoftwareStartConvCmd(ADC1, ENABLE);
			}
			if ((RFM_Idx > 2) && (3 + RFM_Buffer[2] + 1 == RFM_Idx)) {
				uint8_t c = ~chksum;
				if (c == RFM_Buffer[RFM_Idx]) {
					// RX done 
					RFM_IdleMode();

					// send to host without groupid
					Buf_PushString(&RFM_Buffer[4], 3 + RFM_Buffer[2]);
					Buf_Send();

					Mdm_RfmRxDoneCb();
				}	
			}
		} else if (state == RFM_STATE_TX) {
			if (RFM_Idx == RFM_Len) {
				RFM_xfer(RFM_RECV_MODE);
				state = RFM_STATE_RX;
				RFM_Idx = 0;
				chksum = 0;

				Mdm_RfmTxDoneCb();
				return;
			}

			RFM_xfer(RFM_TX_REG | RFM_Buffer[RFM_Idx]);
			RFM_Idx++;
		} 
	}
}

