#include "stm32f10x.h"
#include "platform.h"

#include "enc1.h"

uint16_t Enc1_Interrupted = 0;
uint32_t Enc1_TotalCount = 0;

void Enc1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

}

void Enc1_SetCount(uint32_t in)
{
	TIM_Cmd(TIM2, DISABLE);
	Enc1_TotalCount = in;
	TIM2->CNT = 0;
	TIM_Cmd(TIM2, ENABLE);
}

uint32_t Enc1_Count(void)
{
	uint32_t count = 0;
	uint32_t tmpTotalCount = 0;
	uint32_t tmpEncCount = 0;

	do {
			Enc1_Interrupted = 1;
			tmpTotalCount = Enc1_TotalCount;
			tmpEncCount = TIM2->CNT;
	} while ( Enc1_Interrupted == 0 );

	count = tmpTotalCount + tmpEncCount;

	return count;
}

void TIM2_IRQHandler(void)
{
	uint16_t cnt = TIM2->CNT;

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

	if ( cnt < 0x7FFF ) {
		Enc1_TotalCount += (uint32_t)0x10000;
	} else {
		End1_TotalCount -= (uint32_t)0x10000;
	}
	Enc1_Interrupted = 0;
}

