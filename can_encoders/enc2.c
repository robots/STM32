#include "stm32f10x.h"
#include "platform.h"

#include "enc2.h"

uint16_t Enc2_Interrupted = 0;
uint32_t Enc2_TotalCount = 0;

void Enc2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_Int;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	NVIC_Int.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_Int.NVIC_IRQChannelSubPriority = 0;
	NVIC_Int.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Int.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_Init(&NVIC_Int);

	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

}

void Enc2_SetCount(uint32_t in)
{
	TIM_Cmd(TIM3, DISABLE);
	Enc2_TotalCount = in;
	TIM2->CNT = 0;
	TIM_Cmd(TIM3, ENABLE);
}

uint32_t Enc2_GetCount(void)
{
	uint32_t count = 0;
	uint32_t tmpTotalCount = 0;
	uint32_t tmpEncCount = 0;

	do {
			Enc2_Interrupted = 1;
			tmpTotalCount = Enc2_TotalCount;
			tmpEncCount = TIM3->CNT;
	} while ( Enc2_Interrupted == 0 );

	count = tmpTotalCount + tmpEncCount;

	return count;
}

void TIM3_IRQHandler(void)
{
	uint16_t cnt = TIM3->CNT;

	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

	if ( cnt < 0x7FFF ) {
		Enc2_TotalCount += (uint32_t)0x10000;
	} else {
		Enc2_TotalCount -= (uint32_t)0x10000;
	}
	Enc2_Interrupted = 0;
}

