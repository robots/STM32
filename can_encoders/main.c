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

uint32_t DEBUG_ON = 0;


static Message m = Message_Initializer;

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
	// System Clocks Configuration
	RCC_Configuration();

	// NVIC configuration
	NVIC_Configuration();
	//NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	// Configure the GPIO ports
	GPIO_Configuration();

	/* Enable SysTick - 500 usec */
	if (SysTick_Config(SystemFrequency / 500)) {
		while (1);
	}

	CANController_Init();

	setNodeId(&STM32Encoders_Data, node_id);
	setState(&STM32Encoders_Data, Initialisation);  

	while (1) {
		if (CANController_Received > 0) {
			canReceive(&m);
			canDispatch(&m);
		}
	}

}

void SysTick_Handler(void)
{
	static uint16_t cnt=0;
	static uint16_t cnt1=0;
	static uint8_t flip=0;

	if( cnt++ >= 1000 ) {
		cnt = 0;
		if ( flip ) {
			LED_GREEN(Bit_RESET);
		} else {
			LED_GREEN(Bit_SET);
		}
		flip = !flip;
	}

	/* process encoders every 500usec */
	Enc1_Count();	

	if (cnt1++ >= 1) {
		cnt1 = 0;
		TimeDispatch();
	}
}

/* CanFestival Hooks */
void setTimer(TIMEVAL value) {
	// bla bla :)
}

/* the system timer is set to 500usec */
TIMEVAL getElapsedTime(void) {
	return 500;
}

