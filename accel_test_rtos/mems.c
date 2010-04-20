

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include "platform.h"
#include "rtc_bkp.h"
#include "input.h"
#include "lcd.h"
#include "lisXXX.h"

#include "ADIS1625x.h"
#include "mems_pos.h"

#define INPUT_PRIORITY     ( tskIDLE_PRIORITY + 2 )

struct mems_sensors_t mems_sensors;
struct position_t position;

uint16_t mems_go = 0;

static void taskMems(void *pvParameters) {
	const portTickType xDelay = 100 / portTICK_RATE_MS;
	struct accel_raw_t accel_raw;
	struct gyro_raw_t gyro_raw;
	uint8_t count;
	
	LisXXX_Init();
	ADIS_Init();

	while (1) {
		vTaskDelay(xDelay);

		while (mems_go = 1) {
			vTaskDelay(xDelay);


			while((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5)) != Bit_SET);
			// get accel samples
			LisXXX_GetData(&accel_raw);

			// get gyro samples
			gyro_raw.rate = ADIS_ReadReg(GYRO_OUT) & ADIS_DATA;
			gyro_raw.temp = ADIS_ReadReg(TEMP_OUT) & ADIS_DATA;
			gyro_raw.angle = ADIS_ReadReg(ANGL_OUT) & ADIS_DATA;


			// apply normalization data
			accel_process(&mems_sensors.accel, &accel_raw);

			// apply normalization on gyro
			gyro_process(&mems_sensors.gyro, &gyro_raw);

			// calculate position
			position_process_measurement(&position, &mems_sensors);
			position_update(&position);

			// display every 5th measurement on display
			if (count == 0) {
				
			}
			count ++;
			count %= 5;
		}
	}
}

void Mems_Init() {
	xTaskCreate( taskMems, ( signed char * ) "MEMS", configMINIMAL_STACK_SIZE, NULL, INPUT_PRIORITY, NULL );
}


