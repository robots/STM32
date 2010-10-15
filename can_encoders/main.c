/*
 * Main file
 *
 * 2010 Michal Demin
 *
 */

#include "stm32f10x.h"

#include "canfestival.h"

#include "platform.h"
#include "can.h"
#include "enc1.h"
#include "enc2.h"
#include "STM32Encoders.h"

/* fixme - to be configurable */
#define NODE_ID 0x18

static Message m = Message_Initializer;
uint32_t timerCnt = 0;
uint32_t DEBUG_ON = 0;

static void TestAlarm(CO_Data* d, UNS32 id);
static void RedLedOffAlarm(CO_Data* d, UNS32 id);

#ifdef VECT_TAB_RAM
/* vector-offset (TBLOFF) from bottom of SRAM. defined in linker script */
extern uint32_t _isr_vectorsram_offs;
void NVIC_Configuration(void)
{
	// Set the Vector Table base location at 0x20000000+_isr_vectorsram_offs
	NVIC_SetVectorTable(NVIC_VectTab_RAM, (uint32_t)&_isr_vectorsram_offs);
}
#else
extern uint32_t _isr_vectorsflash_offs;
void NVIC_Configuration(void)
{
	// Set the Vector Table base location at 0x08000000+_isr_vectorsflash_offs
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)&_isr_vectorsflash_offs);
}
#endif /* VECT_TAB_RAM */

void RCC_Configuration(void)
{
	SystemInit();

	// Enable GPIO for led
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// disable JTAG !!!
	if (DEBUG_ON == 0) {
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	}
/*
	// set PA[0-4] as analog inputs
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
*/

	// set LED open drain
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// pins to HiZ state
	LED_RED(Bit_SET);
	LED_GREEN(Bit_SET);
	LED_YELLOW(Bit_RESET);
}

int main(void)
{
	// TODO read from somewhere 
	uint8_t nodeId = NODE_ID;

	// System Clocks Configuration
	RCC_Configuration();

	// NVIC configuration
	NVIC_Configuration();
	//NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	// Configure the GPIO ports
	GPIO_Configuration();

	// Enable SysTick - 500 usec 
	if (SysTick_Config(SystemFrequency / 2000)) {
		while (1);
	}

	// initialize encoders
	Enc1_Init();
	Enc2_Init();

	Enc1_SetCount(0);
	Enc2_SetCount(0);

	// initialize can engine
	canInit("1M");

	// clear filter store (this does not apply the change)
	canFilterClear();

	// CanFestival initialize state
	setNodeId(&STM32Encoders_Data, nodeId);
	setState(&STM32Encoders_Data, Initialisation);  

	// filters should have been added by Canfestival, let's apply changes
	canFilterApply();

	// blink led, using timer framework in canfestival
	SetAlarm(0, 0, &TestAlarm, MS_TO_TIMEVAL(500), MS_TO_TIMEVAL(500));

	while (1) {
		if (canReceive(&m)) {
			static uint8_t flip=0;
			if ( flip ) {
				LED_GREEN(Bit_SET);
			} else {
				LED_GREEN(Bit_RESET);
			}
			flip = !flip;

			canDispatch(&STM32Encoders_Data, &m);
		}

		if (CAN_Error) {
			CAN_Error = 0;
			LED_RED(Bit_RESET);
			SetAlarm(0, 0, &RedLedOffAlarm, MS_TO_TIMEVAL(2500), 0);
			EMCY_setError(&STM32Encoders_Data, 0x5100, 0x08, 0x0000);
			EMCY_errorRecovered(&STM32Encoders_Data, 0x5100);
		}
	}

}

void SysTick_Handler(void)
{
	uint32_t tmp;

	// process encoders every TIMER_PERIOD usec, put value into ODict
	Position_Value = Enc1_GetCount();
	tmp = Enc2_GetCount();

	Position_Value_for_Multi_Sensor_Device[0] = (int32_t)Position_Value;
	Position_Value_for_Multi_Sensor_Device[1] = (int32_t)tmp;

	// CanFestival hook
	TimeDispatch();
}

/* CanFestival Hooks */
// the system timer is set to 500  usec
TIMEVAL getTimerPeriod(void) {
	return 500;
}

static void TestAlarm(CO_Data* d, UNS32 id)
{
	(void)d;
	(void)id;
	static uint8_t flip=0;
	if ( flip ) {
		LED_YELLOW(Bit_SET);
	} else {
		LED_YELLOW(Bit_RESET);
	}
	flip = !flip;
}

static void RedLedOffAlarm(CO_Data* d, UNS32 id)
{
	(void)d;
	(void)id;
	LED_RED(Bit_SET);
}

