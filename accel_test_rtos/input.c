/*
 * Input driver
 *
 * 2009-2010 Michal Demin
 *
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x.h"
#include "platform.h"
#include "adc.h"

#include "input.h"

#define INPUT_PRIORITY     ( tskIDLE_PRIORITY + 1 )
#define INPUT_QUEUE_SIZE   (3)

xQueueHandle xInputQueue;

static uint8_t Input_Get();

uint8_t Input_GetEvent() {
	uint8_t ret = KEY_NONE;

	xQueueReceive( xInputQueue, &ret, 0);

	return ret;
}

static void taskInput( void *pvParameters ) {
	const portTickType xDelay = 100 / portTICK_RATE_MS;
	uint8_t eventCur;
	uint8_t eventLast;
	char buf[14];

	ADC_Setup();

	while (1) {
		vTaskDelay(xDelay);

		eventCur = Input_Get();
		if (eventCur != KEY_NONE) {
			if (eventCur != eventLast) {
	/*			siprintf(buf, "Ahoj %i %i", eventCur, eventLast);
				LCD_Str(1, buf, 1);
				LCD_Update();*/
				xQueueSend(xInputQueue, &eventCur, portMAX_DELAY);
			}
			eventLast = eventCur;
		} else {
			eventLast = KEY_NONE;
		}
	}
}

void Input_Init() {
	xInputQueue = xQueueCreate( INPUT_QUEUE_SIZE, sizeof(uint8_t) );
	xTaskCreate( taskInput, ( signed char * ) "Joy", configMINIMAL_STACK_SIZE, NULL, INPUT_PRIORITY, NULL );
}

#define VALUE_NEAR(x, y) ((x>(y-DIVERSION))&&(x<(y+DIVERSION)))
static uint8_t Input_Get() {
	uint16_t ADCValue;
	ADCValue = ADC_GetChannel(ADC_Channel_15);
	if (VALUE_NEAR(ADCValue, UP_VALUE))
		return KEY_UP; 

	if (VALUE_NEAR(ADCValue, DOWN_VALUE))
		return KEY_DOWN; 

	if (VALUE_NEAR(ADCValue, LEFT_VALUE))
		return KEY_LEFT;

	if (VALUE_NEAR(ADCValue, RIGHT_VALUE))
		return KEY_RIGHT;

	if (JOY_CENTER_READ == Bit_SET) 
		return KEY_CENTER;
/*
	if (TAMP_READ == Bit_SET)
		return KEY_BACK;
*/
	return KEY_NONE;
}
