#include "platform.h"
#include "stm32f10x.h"


#include <string.h>

#include "modem.h"
#include "adc.h"
#include "rfm12.h"
#include "buf.h"


#define MAGIC_ADDR 0x200027FC
#define MAGIC_KEY  0xDEADBEEF

static uint8_t Mdm_state = MDM_STATE_CMD;
uint16_t Mdm_CMDTimeout = 1000; // 50ms
uint16_t Mdm_RXTimeout = 500; // 25ms
uint16_t Mdm_SniffTimeout = 5; // 250us

// analyzer variables
uint16_t a_freq;
uint16_t a_delta;
uint16_t a_start;
uint16_t a_stop;

uint8_t cmd_data[512];
uint16_t cmd_len = 0;

uint16_t timer = 0;

uint8_t Mdm_ProcessCmd();
void Mdm_EnterDFU();
void Mdm_AnalyzerStart(uint16_t s, uint16_t e, uint16_t d);
void Mdm_AnalyzerStop();
void Mdm_SniffStart(uint8_t a);
void Mdm_SniffStop();

// command syntax
// to send over air:
// 's' ii ii ss dd dd dd ..  - sends data to ID 'iiii' Data size 'ss' and data 'dd'
// 'a' ss ss ee ee dd dd - start scanning at a_freq 'ssss', end at 'eeee', delta 'dddd'*0.005khz, channel width 'ww'khz
// 'r' xx - start sniffing mode - xx = 'a'
// 'c' bb cc cc rr tx rx gg  - band, channel, rate, txdev, rxwidth, groupid
// 'w' - get rssi
// 'e' - echo
// 'd' - enter dfu

void Mdm_Process(uint8_t *buf, uint8_t len)
{
	memcpy(cmd_data + cmd_len, buf, len);
	cmd_len += len;
	
	timer = Mdm_CMDTimeout;

	if (Mdm_ProcessCmd()) {
		cmd_len = 0;
	}
}

uint8_t Mdm_ProcessCmd()
{

	if (Mdm_state == MDM_STATE_ANALYZER) {
		Mdm_state = MDM_STATE_CMD;
		Mdm_AnalyzerStop();
		return 1;
	}
	if (Mdm_state == MDM_STATE_SNIFF) {
		Mdm_state = MDM_STATE_CMD;
		Mdm_SniffStop();
		return 1;
	}

	if (Mdm_state == MDM_STATE_CMD) {
		if (cmd_len != 0) {
			switch (cmd_data[0]) {
				case 'a':
					if (cmd_len > 6) {
						uint16_t start = cmd_data[1] | cmd_data[2] << 8;
						uint16_t stop = cmd_data[3] | cmd_data[4] << 8;
						uint16_t delta = cmd_data[5] | cmd_data[6] << 8;
						Mdm_AnalyzerStart(start, stop, delta);
						Mdm_state = MDM_STATE_ANALYZER;
						return 1;
					}
					break;
				case 's':
					if ((cmd_len > 4) && (cmd_len >= cmd_data[3] + 4)) {
						LED_RED(LED_ON);
						LED_GREEN(LED_OFF);
						timer = Mdm_RXTimeout;
						RFM_Send(cmd_data[1] | cmd_data[2] << 8, &cmd_data[4], cmd_data[3]);
						Mdm_state = MDM_STATE_COMM;
						return 1;
					}
					break;
				case 'r':
					if (cmd_len > 1) {
						Mdm_SniffStart(cmd_data[1]);
						Mdm_state = MDM_STATE_SNIFF;
						return 1;
					}
					break;
				case 'c':
					if (cmd_len > 7) {
						RFM_SetBand(cmd_data[1]);
						RFM_SetFreq(cmd_data[2] | cmd_data[3] << 8);
						RFM_SetDataRate(cmd_data[4]);
						RFM_SetTXDeviation(cmd_data[5]);
						RFM_SetRXWidth(cmd_data[6]);
						RFM_SetGroup(cmd_data[7]);
						return 1; 
					}
					break;
				case 'e':
					if (cmd_len > 1) {
						Buf_PushString(cmd_data, cmd_len);
						Buf_Send();
						return 1;
					}
					break;
				case 'w':
					Buf_PushWord(AD_Data);
					Buf_Send();
					return 1;
					break;
				case 't':
					if (cmd_len > 6) {
						Mdm_CMDTimeout = cmd_data[1] | cmd_data[2] << 8;
						Mdm_RXTimeout = cmd_data[3] | cmd_data[4] << 8;
						Mdm_SniffTimeout = cmd_data[5] | cmd_data[6] << 8;
						return 1;
					}
					break;
				case 'w':
					Mdm_EnterDFU();
					return 1;
					break;
			}
		}
	}
	return 0;
}

// to be called from systick
void Mdm_Worker()
{
	if (timer != 0) {
		timer --;
	}

	if (timer == 1) {
		if (Mdm_state == MDM_STATE_CMD) {
			LED_GREEN(LED_OFF);
			LED_YELLOW(LED_OFF);
			LED_RED(LED_OFF);

			// cmd timeout
			cmd_len = 0;
		} else if (Mdm_state == MDM_STATE_COMM) {
			// timeouted :(
			LED_YELLOW(LED_ON);

			// turn off receiver
			RFM_IdleMode();

			// turn off led in 'timeout'.
			timer = Mdm_CMDTimeout;
			Mdm_state = MDM_STATE_CMD;
		} else if (Mdm_state == MDM_STATE_ANALYZER) {
			// start sample
			ADC_start();
		}
	}
}

void Mdm_EnterDFU()
{
	USB_Cable_Config(DISABLE);
	*(volatile uint32_t*)MAGIC_ADDR = MAGIC_KEY;
	NVIC_SystemReset();
}

void Mdm_RfmTxDoneCb()
{
	timer = Mdm_RXTimeout; 	

	LED_RED(LED_OFF);
	LED_GREEN(LED_ON);
}

void Mdm_RfmRxDoneCb()
{
	timer = Mdm_CMDTimeout; // just for led
	Mdm_state = MDM_STATE_CMD;

	LED_GREEN(LED_OFF);
}

void Mdm_AnalyzerStart(uint16_t s, uint16_t e, uint16_t d)
{
	a_delta = 1;
	a_freq = 96;
	a_stop = 3903;
	a_start = 96;

	if ((s >= 96) && (s < 3903)) {
		a_freq = s;
		a_start = s;
	}

	if ((e > 96) && (s <= 3903)) {
		a_stop = e;
	}

	a_delta = d;
	
	// set lowest possible frequency
	RFM_SetFreq(a_freq);

	// enable scan mode
	RFM_ScanMode();

	// enable ADC
	ADC_scan(1);

	timer = Mdm_SniffTimeout;

	LED_GREEN(LED_ON);
	LED_RED(LED_ON);
}

void Mdm_AnalyzerStop()
{
	// disable ADC
	ADC_scan(0);

	// disable scan mode
	RFM_IdleMode();

	LED_GREEN(LED_OFF);
	LED_RED(LED_OFF);
}

void Mdm_SniffStart(uint8_t a)
{
	// enable sniff mode
	RFM_SniffMode(a);

	LED_GREEN(LED_ON);
	LED_YELLOW(LED_ON);
}

void Mdm_SniffStop()
{
	// disable sniff mode
	RFM_IdleMode();

	LED_GREEN(LED_OFF);
	LED_YELLOW(LED_OFF);
}

