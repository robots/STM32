#include "stm32f10x.h"

#include "rtc.h"
#include "dtmf.h"
#include "dtmf_det.h"


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

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
}

void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);

	// STBY - 3, LED - 12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOC, GPIO_Pin_12, Bit_SET);
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_SET);

	// PWMOUT - 8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}

int main(void)
{
	uint32_t dly;

	/* System Clocks Configuration */
	RCC_Configuration();

	/* NVIC configuration */
	NVIC_Configuration();

	/* Configure the GPIO ports */
	GPIO_Configuration();


	// led blink
	RTCInit();

	dtmf_init();

	// Enable SysTick - 1ms
	if (SysTick_Config(SystemFrequency / 1000)) {
		while (1);
	}

//	TEST1:
//	dtmf_set(DTMF_1477);

	dtmf_det_init();

	while (1);
}

/*
	// TEST2:
#define DTMF_SIZE (sizeof(dtmf_seq)/sizeof(uint8_t))
uint8_t dtmf_seq[] = { DTMF_8};
uint8_t dtmf_pos = 0;
*/

void SysTick_Handler(void)
{
/*
	static uint32_t time = 0;

	time ++;

	if (time % 500 == 0) {
		time = 0;

		if (dtmf_sending()) {
			dtmf_set(0);
		} else {
			if (dtmf_pos < DTMF_SIZE) {
				dtmf_set(dtmf_seq[dtmf_pos]);
				dtmf_pos++;
			}
		}
	}
*/
}
