/*
 * bxCAN driver for STM32 family processors
 *
 * 2010 Michal Demin
 *
 */

#include "platform.h"
#include "stm32f10x.h"
#include <string.h>
#include "can.h"

// TODO: add others
//"1M" "800K" "500K" "250K" "125K" "100K" "50K" "20K" "10K"
#define TIMINGS_NUM (sizeof(CAN_Timings)/sizeof(struct can_timing_t))
static const struct can_timing_t CAN_Timings[] = {
	{"1M", 0x03, 0x115 } // 1Mbit CAN speed @ 72mHz internal clock
};

#define FILTER_NUM	14
static CAN_FilterInitTypeDef CAN_Filters[FILTER_NUM] = {
	// Id_H,  Id_L    MskIdH  MskIdL  FIFO Filt# Mode                   Scale                  Active 
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0,   0,    CAN_FilterMode_IdMask, CAN_FilterScale_32bit, ENABLE },
};


struct can_timing_t *CAN_Timing;
static NVIC_InitTypeDef CAN_Int;

uint8_t canChangeBaudRate( CAN_HANDLE fd, char* baud)
{
	(void)fd;
	uint8_t i;
	for (i = 0; i < TIMINGS_NUM; i++){
		if (strcmp(CAN_Timings[i].baud, baud) == 0) {
			CAN_Timing = &CAN_Timings[i];
			return 0;
		}
	}
	return 1;
}

void canInit(char* baud)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);

	// Configure CAN pin: RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Configure CAN pin: TX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	CAN_Int.NVIC_IRQChannelPreemptionPriority = 14;
	CAN_Int.NVIC_IRQChannelSubPriority = 0;

	CAN_Timing = &CAN_Timings[0];
	canChangeBaudRate(0, baud);

	canHWReinit();
}

void canHWReinit()
{
	CAN_Int.NVIC_IRQChannelCmd = DISABLE;
	CAN_Int.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
	NVIC_Init(&CAN_Int);
	CAN_Int.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_Init(&CAN_Int);
	CAN_Int.NVIC_IRQChannel = CAN1_SCE_IRQn;
	NVIC_Init(&CAN_Int);

	// Reset CAN1 - clears the error state
	CAN_DeInit(CAN1);

	canFilterApply();

	// enable intertrupts
	//CAN1->IER = 0x00008F13; // enable all interrupts (except FIFOx full/overrun, sleep/wakeup)
	CAN1->IER = CAN_IER_TMEIE ;//| CAN_IER_FMPIE0;
	CAN1->IER |= CAN_IER_EWGIE | CAN_IER_EPVIE | CAN_IER_BOFIE | CAN_IER_LECIE;
//	CAN1->IER |= CAN_IER_ERRIE; // FIXME 

	// enter the init mode
	CAN1->MCR &= ~CAN_MCR_SLEEP;
	CAN1->MCR |= CAN_MCR_INRQ;

	// wait for it !
	while ((CAN1->MSR & CAN_MSR_INAK) != CAN_MSR_INAK);

	/* setup timing */
	CAN1->BTR = (CAN_Timing->ts << 16) | CAN_Timing->brp;

	/* finish bxCAN setup */
	CAN1->MCR |= CAN_MCR_ABOM;
	CAN1->MCR |= CAN_MCR_TXFP | CAN_MCR_RFLM | CAN_MCR_AWUM; // automatic wakeup, tx round-robin mode
	CAN1->MCR &= ~(CAN_MCR_SLEEP | 0x10000); // we don't support sleep, no debug-freeze
	CAN1->MCR &= ~CAN_MCR_INRQ; // leave init mode

	CAN_Int.NVIC_IRQChannelCmd = ENABLE;
	CAN_Int.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
	NVIC_Init(&CAN_Int);
	CAN_Int.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_Init(&CAN_Int);
	CAN_Int.NVIC_IRQChannel = CAN1_SCE_IRQn;
	NVIC_Init(&CAN_Int);

}

void canFilterClear()
{
	int i;
	// setup can filters
	for (i = 0; i < FILTER_NUM; i++) {
		CAN_Filters[i].CAN_FilterActivation = DISABLE;
	}
}

void canFilterApply()
{
	int i;
	// setup can filters
	for (i = 0; i < FILTER_NUM; i++) {
		if (CAN_Filters[i].CAN_FilterActivation == DISABLE) 
			break;
		CAN_FilterInit(&CAN_Filters[i]);
	}
}

void canFilterAddMask(uint16_t cobid, uint16_t cobid_mask, uint8_t prio)
{
	uint8_t i = 0;

	for (i = 0; i < FILTER_NUM; i++) {
		if (CAN_Filters[i].CAN_FilterActivation == DISABLE)
			break;
	}

	// check limit
	if (i >= FILTER_NUM) 
		return;

	CAN_Filters[i].CAN_FilterActivation = ENABLE;
	CAN_Filters[i].CAN_FilterNumber = i;
	CAN_Filters[i].CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_Filters[i].CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_Filters[i].CAN_FilterFIFOAssignment = (prio > 0)? 1: 0;
  CAN_Filters[i].CAN_FilterIdHigh = cobid << 5;
	CAN_Filters[i].CAN_FilterIdLow = 0x0000;
	CAN_Filters[i].CAN_FilterMaskIdHigh = cobid_mask << 5;
  CAN_Filters[i].CAN_FilterMaskIdLow = 0x0004;
}

/* TODO: priority scheduling */
uint8_t canSend(CAN_PORT notused, Message *m)
{
	(void)notused;
	uint16_t mailbox = 4;
	if (CAN1->TSR & (CAN_TSR_TME0 | CAN_TSR_TME1 | CAN_TSR_TME2)) {
		mailbox = (CAN1->TSR & CAN_TSR_CODE) >> 24;
	} else {
		// nothing empty ? :( this should not happen !
		// TODO: Error ?
		return 0xFF;
	}

	// clear message
	CAN1->sTxMailBox[mailbox].TIR &= CAN_TI0R_TXRQ;

	// add IDE and RTR fields
	CAN1->sTxMailBox[mailbox].TIR |= m->rtr ? 0x02 : 0;

	// add msg ID (cob_id)
	CAN1->sTxMailBox[mailbox].TIR |= m->cob_id << 21;

	// setup the DLC field (lenght)
	CAN1->sTxMailBox[mailbox].TDTR &= 0xFFFFFFF0;
	CAN1->sTxMailBox[mailbox].TDTR |= m->len & CAN_MSG_SIZE;

	// Set up the data fields
	CAN1->sTxMailBox[mailbox].TDLR = (((uint32_t)m->data[3] << 24) | ((uint32_t)m->data[2] << 16) | ((uint32_t)m->data[1] << 8) | ((uint32_t)m->data[0]));
	CAN1->sTxMailBox[mailbox].TDHR = (((uint32_t)m->data[7] << 24) | ((uint32_t)m->data[6] << 16) | ((uint32_t)m->data[5] << 8) | ((uint32_t)m->data[4]));

	// mark message for transmission
	CAN1->sTxMailBox[mailbox].TIR |= CAN_TI0R_TXRQ;

	return 0;
}


/* TX interrupt */
void USB_HP_CAN1_TX_IRQHandler(void) {
	if (CAN1->TSR & CAN_TSR_RQCP0) {
		CAN1->TSR |= CAN_TSR_RQCP0;
	}
	if (CAN1->TSR & CAN_TSR_RQCP1) {
		CAN1->TSR |= CAN_TSR_RQCP1;
	}
	if (CAN1->TSR & CAN_TSR_RQCP2) {
/*		CANController_Status |= (CAN1->TSR & CAN_TSR_ALST2)?CAN_STAT_ALST:0;
		CANController_Status |= (CAN1->TSR & CAN_TSR_TERR2)?CAN_STAT_TERR:0;
		CANController_Status |= (CAN1->TSR & CAN_TSR_TXOK2)?CAN_STAT_TXOK:0;*/
		CAN1->TSR |= CAN_TSR_RQCP2;
	}

}

/* RX0 fifo interrupt
 * We cannot eat the whole FIFO, instead we let NVIC process higher prio
 * interrupt and return here later.
 * This is necessary for SPI_Slave to work !!!
 */
/*void USB_LP_CAN1_RX0_IRQHandler(void)*/
uint8_t canReceive(Message *m)
{
	uint8_t fifo = 0;

	if (CAN1->RF1R&0x03) {
		fifo = 1;
	} else if (CAN1->RF0R&0x03) {
		fifo = 0;
	} else {
		return 0;
	}


	if (CAN1->sFIFOMailBox[fifo].RIR & 0x04) {
		// extended id we do not support
		// release fifo
		CAN1->RF0R = CAN_RF0R_RFOM0;
		return 0;
	}

	m->rtr = CAN1->sFIFOMailBox[fifo].RIR & 0x02 ? 1 : 0;
	m->len = CAN1->sFIFOMailBox[fifo].RDTR & CAN_MSG_SIZE;

	// standard id
	m->cob_id = (uint32_t)0x000007FF & (CAN1->sFIFOMailBox[0].RIR >> 21);

	// copy data bytes
	m->data[0] = (uint8_t)0xFF & (CAN1->sFIFOMailBox[fifo].RDLR);
	m->data[1] = (uint8_t)0xFF & (CAN1->sFIFOMailBox[fifo].RDLR >> 8);
	m->data[2] = (uint8_t)0xFF & (CAN1->sFIFOMailBox[fifo].RDLR >> 16);
	m->data[3] = (uint8_t)0xFF & (CAN1->sFIFOMailBox[fifo].RDLR >> 24);
	m->data[4] = (uint8_t)0xFF & (CAN1->sFIFOMailBox[fifo].RDHR);
	m->data[5] = (uint8_t)0xFF & (CAN1->sFIFOMailBox[fifo].RDHR >> 8);
	m->data[6] = (uint8_t)0xFF & (CAN1->sFIFOMailBox[fifo].RDHR >> 16);
	m->data[7] = (uint8_t)0xFF & (CAN1->sFIFOMailBox[fifo].RDHR >> 24);
	
	CAN1->RF0R = CAN_RF0R_RFOM0;

	return fifo+1;
}

/* status change and error
 * This ISR is called periodicaly while error condition persists !
 *
 * FIXME: This is very much broken (HW side) !!!
 */
uint32_t CAN_Error = 0;

void CAN1_SCE_IRQHandler(void) {
	static uint32_t _CAN_Error = 0;
	static uint32_t _CAN_Error_Last = 0;
	static uint16_t count = 0;

	if (CAN1->ESR & (CAN_ESR_EWGF | CAN_ESR_EPVF | CAN_ESR_BOFF)) {
		// if error happened, copy the state
		_CAN_Error = CAN1->ESR;

		// abort msg transmission on Bus-Off
		if (CAN1->ESR & CAN_ESR_BOFF) {
			CAN1->TSR |= (CAN_TSR_ABRQ0 | CAN_TSR_ABRQ1 | CAN_TSR_ABRQ2);
		}
		// clean flag - not working at all :(
		CAN1->ESR &= ~ (CAN_ESR_EWGF | CAN_ESR_EPVF | CAN_ESR_BOFF);

		// clear last error code
		CAN1->ESR |= CAN_ESR_LEC;

		// clear interrupt flag
		CAN1->MSR &= ~CAN_MSR_ERRI;

		// work around the bug in HW
		// notify only on "new" error, otherwise reset can controller
		if (_CAN_Error ^ _CAN_Error_Last) {
			count = 0;
			// notify CAN stack - emcy event
			CAN_Error = 1;
		} else {
			count ++;
			if (count > 10) {
				count = 0;
				canHWReinit();
			}
		}
		_CAN_Error_Last = _CAN_Error;
	}
}

