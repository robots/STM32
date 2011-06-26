/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Version            : V3.1.1
* Date               : 04/07/2010
* Description        : Device Firmware Upgrade(DFU) demo main file
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_istr.h"
//#include "dfu_mal.h"
#include "hw_config.h"
#include "platform.h"

/* Private typedef -----------------------------------------------------------*/
typedef  void (*pFunction)(void);

/* Private define ------------------------------------------------------------*/
#define MAGIC_ADDR 0x20002000
#define MAGIC_KEY  0xDEADBEEF
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
uint8_t DeviceState = STATE_dfuERROR;
uint8_t DeviceStatus[6] = {
	[0] = STATUS_ERRFIRMWARE,
	[4] = STATE_dfuERROR,
};

pFunction Jump_To_Application;
uint32_t JumpAddress;

const NVIC_InitTypeDef NVIC_Usb = {
	.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn,
	.NVIC_IRQChannelPreemptionPriority = 0,
	.NVIC_IRQChannelSubPriority = 0,
	.NVIC_IRQChannelCmd = ENABLE
};

GPIO_InitTypeDef GPIO_InitStructure = {
	.GPIO_Speed = GPIO_Speed_50MHz,
	.GPIO_Pin = GPIO_Pin_4,
	.GPIO_Mode = GPIO_Mode_IPU
};


extern uint32_t _isr_vectorsflash_offs;

inline void RCC_Configuration(void)
{
	SystemInit();
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	// enable usb 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
}

inline void GPIO_Configuration(void)
{
	// Configure USB pull-up pin
	GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);

	// Configure USB pull-up pin
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_0);
	GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
}

/*******************************************************************************
* Function Name  : main.
* Description    : main routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void main(void) __attribute__ ((noreturn));
void main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO, ENABLE);
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	AFIO->MAPR |= 0x01000000; 

	// Button w/ pullup .. using default value, compiled in
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Check if the Key push-button on STM3210x-EVAL Board is pressed */
	if ((*(volatile uint32_t*)MAGIC_ADDR) != MAGIC_KEY) {
		if (GPIOB->IDR & GPIO_Pin_4) {
			/* Test if user code is programmed starting from address 0x8003000 */
			if (((*(volatile uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000) {
				/* Jump to user application */
				JumpAddress = *(volatile uint32_t*) (ApplicationAddress + 4);
				Jump_To_Application = (pFunction) JumpAddress;
				/* Initialize user application's Stack Pointer */
				__set_MSP(*(volatile uint32_t*) ApplicationAddress);
				Jump_To_Application();
			}
		}
	}
	/* Otherwise enters DFU mode to allow user to program his application */
	// reset magic word
	*(volatile uint32_t*)MAGIC_ADDR = 0xFFFFFFFF;

	RCC_Configuration();

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)&_isr_vectorsflash_offs);
	NVIC_Init((NVIC_InitTypeDef*)&NVIC_Usb); 

	GPIO_Configuration();

	/* light up led */
	GPIO_ResetBits(GPIOB, GPIO_Pin_0);

	USB_Init();

	//USB_Init(&Device_Table, &Device_Property, &User_Standard_Requests, NULL, NULL);

	while (1);
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
