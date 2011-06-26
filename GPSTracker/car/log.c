
#include "platform.h"
#include "stm32f10x.h"


#include <string.h>

#include "log.h"
#include "spi_flash.h"

// devide info -- to be struct
uint16_t Log_Voltage = 0;
uint8_t Log_State = LOG_STOP; // to be enum

// flash info
uint32_t Log_FlashAddress = 0;
uint32_t Log_RecordCount = 0;
uint32_t Log_PointCount = 0;

void Log_Init()
{
	// update record counters
	Log_CheckFlash();

	Log_Stop();
}


void Log_CheckFlash()
{
	uint8_t ret = 0;
	uint32_t addr = 0;
	uint8_t data;

	Log_FlashAddress = 0;
	Log_RecordCount = 0;
	Log_PointCount = 0;

	while (1) {
		sFLASH_ReadBuffer(&data, Log_FlashAddress, 1);
		if (data == 'a') {
			Log_FlashAddress += sizeof(struct rec_full_t);
			Log_RecordCount ++;
			Log_PointCount ++;
		} else if (data == 'd') {
			Log_FlashAddress += sizeof(struct rec_delta_t);
			Log_RecordCount ++;
			Log_PointCount += REC_DELTA_POINTS;
		} else if (data == 's') {
			Log_FlashAddress += sizeof(struct rec_status_t);
			Log_RecordCount ++;
		} else if (data == 0xff) {
			break;
		} else {
			ret = 1;
			break;
		}
	}

	if (ret == 1) {
		// error
		Log_Device.state = LOG_STATE_FLASH_ERROR;
		Log_WriteError();
	}
}

void Log_Start()
{
	Log_Stop = LOG_RUNNING;

	/* enable GPS */
	GPS_EN(Bit_SET);


}

void Log_Stop()
{
	Log_State = LOG_STOP;

	/* disable GPS */
	GPS_EN(Bit_RESET);

	/* write unwritten data */
}

void Log_WriteError()
{

}

void Log_Timer()
{
	if (Log_State != LOG_RUNNING)
		return;

}

