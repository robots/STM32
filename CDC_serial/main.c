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
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"


extern uint32_t count_in;
extern __IO uint32_t count_out;
extern uint8_t buffer_in[VIRTUAL_COM_PORT_DATA_SIZE];
extern uint8_t buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];



void bli(void) {
	if((GPIOC->IDR)&(0x00001000)) {
		// clear
		LED_WRITE(Bit_RESET);
	} else {
		// set
		LED_WRITE(Bit_SET);
	}
}

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

void RCC_Configuration(void)
{
	SystemInit();
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

	// enable usb 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

	// Enable GPIO modules 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC| RCC_APB2Periph_AFIO, ENABLE);

	// RTC clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// PA1 - MIC
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = 0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // analog input
	GPIO_Init (GPIOA, &GPIO_InitStructure);

	// PA0 - WAKEUP button
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// PA3 - ADIS reset, PA4 - nRF Chip Select 
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	// PB2 - LCD_DC, PB10 - ADIS_CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // output push-pull
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// PB5 - SENS_INT
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // input pull-down
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// PC12 - LED, PC7 - LCD_RES, PC8 - nRF CE, PC10 - LCD_E
	GPIO_WriteBit(GPIOC,GPIO_Pin_12,Bit_SET);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // output push-pull
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// PC13 - TAMP button, PC6 -Joy Center, PC9 - nRF IRQ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_6 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // input floating
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// PC5 - Joystick
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = 0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // analog input
	GPIO_Init (GPIOC, &GPIO_InitStructure);

	// Configure USB pull-up pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	NRF_CS_WRITE(Bit_SET);
	NRF_CE_WRITE(Bit_RESET);

	ADIS_CS_WRITE(Bit_SET);
	ADIS_RESET_WRITE(Bit_SET);
}

int main(void)
{
	uint32_t count = 0;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* System Clocks Configuration */
	RCC_Configuration();

	/* NVIC configuration */
	NVIC_Configuration();

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure the GPIO ports */
	GPIO_Configuration();

	USB_Init();

	while (1) {
		if ((count_out != 0) && (bDeviceState == CONFIGURED)) {
			uint8_t i;
			bli();

//			USB_SIL_Write(EP1_IN, buffer_out, count_out);
			for (i = 0; i<count_out; i++) {
				buffer_in[(count_in+i)%64] = buffer_out[i];
			}
			count_in += count_out;
			count_out = 0;
			count = 0;
		}

		count++;
		count%=10000000/2;
		if (count == 0 && count_in > 0) {
			bli();
			USB_SIL_Write(EP1_IN, buffer_in, count_in);
			SetEPTxValid(ENDP1);
		}
	}

	while (1);
}


void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}

