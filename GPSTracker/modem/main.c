/*
 *
 * 2009-2010 Michal Demin
 *
 */
#include <stdio.h>

#include "stm32f10x.h"
#include "platform.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"


#include "adc.h"
#include "rfm12.h"
#include "modem.h"

/*
void bli(void) {
	if((GPIOC->IDR)&(0x00001000)) {
		// clear
		LED_WRITE(Bit_RESET);
	} else {
		// set
		LED_WRITE(Bit_SET);
	}
}*/

#ifdef VECT_TAB_RAM
/* vector-offset (TBLOFF) from bottom of SRAM. defined in linker script */
extern uint32_t _isr_vectorsram_offs;
inline void NVIC_Configuration(void)
{
	/* Set the Vector Table base location at 0x20000000+_isr_vectorsram_offs */
	NVIC_SetVectorTable(NVIC_VectTab_RAM, (uint32_t)&_isr_vectorsram_offs);
}
#else
extern uint32_t _isr_vectorsflash_offs;
inline void NVIC_Configuration(void)
{
	/* Set the Vector Table base location at 0x08000000+_isr_vectorsflash_offs */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)&_isr_vectorsflash_offs);
}
#endif /* VECT_TAB_RAM */

inline void RCC_Configuration(void)
{
	SystemInit();
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

	// enable usb 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

	// Enable GPIO modules 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1 | RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC | RCC_AHBPeriph_DMA1, ENABLE);
	 	
}

inline void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {
		.GPIO_Speed = GPIO_Speed_50MHz,
	};

	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	AFIO->MAPR |= 0x01000000; 

	// sPI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// PA0 - RSSI in 
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
/*
	// PA3 - RFM reset, PA4 - RFM Chip Select 
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
*/
	//  RFM - IRQn
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //innput pullup 
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// RFM int
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // input floating
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// LEDs in Open-Drain mode 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init (GPIOB, &GPIO_InitStructure);

	// Configure USB pull-up pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_4 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	LED_GREEN(LED_OFF);
	LED_RED(LED_OFF);
	LED_YELLOW(LED_OFF);
}

void main(void) __attribute__ ((noreturn));
void main(void)
{
	NVIC_InitTypeDef NVIC_InitStructure = {
		.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn,
		.NVIC_IRQChannelPreemptionPriority = 0,
		.NVIC_IRQChannelSubPriority = 0,
		.NVIC_IRQChannelCmd = ENABLE
	};

	/* System Clocks Configuration */
	RCC_Configuration();

	/* NVIC configuration */
	NVIC_Configuration();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	// enable usb interrupt
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the GPIO ports */
	GPIO_Configuration();

	// Enable SysTick - 50usec
	if (SysTick_Config(SystemFrequency / 2000)) {
		while (1);
	}

	RFM_RST(Bit_RESET);
	RFM_RST(Bit_SET);

	RFM_Init();
	ADC_init();

	//USB_Init(&Device_Table, &Device_Property, &User_Standard_Requests, NULL, NULL);
	USB_Init();

	while (1); /* _WFI() */
}

void SysTick_Handler(void)
{
	Mdm_Worker();
}


