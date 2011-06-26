/*
 *
 * 2009-2010 Michal Demin
 *
 */
#include <stdio.h>

#include "stm32f10x.h"
#include "platform.h"

#include "uart.h"
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
void NVIC_Configuration(void)
{
	/* Set the Vector Table base location at 0x20000000+_isr_vectorsram_offs */
	NVIC_SetVectorTable(NVIC_VectTab_RAM, (uint32_t)&_isr_vectorsram_offs);
}
#else
extern uint32_t _isr_vectorsflash_offs;
void NVIC_Configuration(void)
{
	/* Set the Vector Table base location at 0x08000000+_isr_vectorsflash_offs */
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)&_isr_vectorsflash_offs);
}
#endif /* VECT_TAB_RAM */

inline void RCC_Configuration(void)
{
	SystemInit();
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

	// Enable GPIO modules 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1 | RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC | RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  
	 	
}

inline void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = {
		.GPIO_Speed = GPIO_Speed_50MHz,
	};

	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	AFIO->MAPR |= 0x01000000; 

	// sPI, Uart TX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// uart RX
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  

	// PA0 - Analog input 
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//  RFM - IRQn
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //innput pullup 
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// LEDs in Open-Drain mode 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init (GPIOA, &GPIO_InitStructure);

	// RFM - RST, CS, GPSEN
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// FLASH CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RFM_CS(Bit_SET);
	FLASH_CS(Bit_SET);
	GPS_EN(Bit_RESET);
	LED_GREEN(LED_OFF);
	LED_RED(LED_OFF);
	LED_YELLOW(LED_OFF);
}

inline void wdt_enable()
{
  /* IWDG timeout equal to 280 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: 40KHz(LSI) / 32 = 1.25 KHz */
  IWDG_SetPrescaler(IWDG_Prescaler_32);

  /* Set counter reload value to 349 */
  IWDG_SetReload(349);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

void main(void) __attribute__ ((noreturn));
void main(void)
{
	/* System Clocks Configuration */
	RCC_Configuration();

	/* NVIC configuration */
	NVIC_Configuration();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);


	/* Configure the GPIO ports */
	GPIO_Configuration();

	// Enable SysTick - 50usec
	if (SysTick_Config(SystemFrequency / 2000)) {
		while (1);
	}

	RFM_RST(Bit_RESET);
	RFM_RST(Bit_SET);

//	RFM_Init();

	UART_Init();
	ADC_init();
	Log_Init();

	GPS_EN(Bit_SET); // enable gps

	// wdt_enable();	
	while (1); /* _WFI() */
}

void SysTick_Handler(void)
{
  //IWDG_ReloadCounter();
}


