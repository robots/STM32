#include "platform.h"
#include "stm32f10x.h"


#include <string.h>

#include "modem.h"
#include "adc.h"
#include "rfm12.h"
#include "flash.h"


static uint8_t Mdm_state = MDM_STATE_CMD;
uint16_t Mdm_CMDTimeout = 1000; // 50ms
uint16_t Mdm_RXTimeout = 500; // 25ms
uint16_t Mdm_SniffTimeout = 5; // 250us


uint16_t timer = 0;


static const struct mdm_cfg_t Mdm_CfgDefault = {
};

struct mdm_cfg_t Mdm_Cfg = {
};

void Mdm_Init()
{
	// init configuration
	if (FLSH_ReadStorage(&Mdm_Cfg, sizeof(struct mdm_cfg_t))) {
		// just copy defaults if configuration does not exist
		memcpy(&Mdm_Cfg, &Mdm_Cfg_Default, sizeof(struct mdm_cfg_t));
	}

	// setup rfmodule as configured
	RFM_SetFreq(96);
}

void Mdm_ProcessCmd(uint8_t *buf, uint16_t len)
{
	uint8_t cmd_data_len; // only data
	uint8_t *cmd_data;

	// check ID
	if (memcmp(&Mdm_Cfg.ID, buf, 2) != 0) {
		// not us.
		return; 
	}

	if ((len > 3) && (buf[2] > 1)) {
		cmd_len = buf[2] - 1;
		cmd_data = &buf[4];

		switch (buf[3]) {
			case 'e': // erase
				if ((cmd_len == 4) && (memcmp(&Mdm_Cfg.EraseMagic, cmd_data, 4) == 0)) {
					//Spiflasherasechip
				}
				break;
			case 'c': // configure
				if (cmd_len == sizeof(struct mdm_cfg_t)) {
					//FLSH_UpdateStorage(buf, sizeof(struct mdm_cfg_t));
				}
				break;
			case 'r': // reboot
				// send reply ? 
				NVIC_SystemReset();
				break;
			case 'd': // download data
				if (cmd_len == 3) {
					// spiflashreadbuffer( addr, count) 
				}
				break;
			case 'w': // get record count
				if (cmd_len == 0) {
					// return record count
				}
				break;
			case 's': // poll state
				if (cmd_len == 0) {
					// return state
				}
		}

	}
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

void Mdm_RfmTxDoneCb()
{
	timer = Mdm_RXTimeout; 	

	LED_RED(LED_OFF);
	LED_GREEN(LED_ON);
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

