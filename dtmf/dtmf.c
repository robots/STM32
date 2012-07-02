
/*
 * Signal generator inspired by AVR314 appnote
 */

#include "stm32f10x.h"
#include "dtmf.h"

uint8_t dtmf_active = 0;
uint8_t dtmf_low = 0;
uint8_t dtmf_high = 0;

const uint8_t dtmf_code_high[] = {0, 79, 87, 96, 107};
const uint8_t dtmf_code_low[] = {0, 46, 50, 56, 61};

const uint8_t dtmf_sine[128] = {
64,67,70,73,76,79,82,85,88,91,94,96,99,102,104,106,109,111,113,115,117,118,
120,121,123,124,125,126,126,127,127,127,127,127,127,127,126,126,125,124,123,121,
120,118,117,115,113,111,109,106,104,102,99,96,94,91,88,85,82,79,76,73,70,67,
64,60,57,54,51,48,45,42,39,36,33,31,28,25,23,21,18,16,14,12,10,9,7,6,4,3,2,1,1,0,0,0,0,0,
0,0,1,1,2,3,4,6,7,9,10,12,14,16,18,21,23,25,28,31,33,36,39,42,45,48,51,54,57,60};


void dtmf_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStruct;
  NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_DeInit(TIM1);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Prescaler = 17;
	TIM_TimeBaseStructure.TIM_Period = 0xFF;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	TIM_ARRPreloadConfig(TIM1, ENABLE);

	TIM_OCStructInit(&TIM_OCInitStruct);
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStruct.TIM_Pulse = 0;
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OC1Init(TIM1, &TIM_OCInitStruct);

  TIM_CtrlPWMOutputs(TIM1, ENABLE);

	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	// amp enable
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_RESET);
}

uint8_t dtmf_sending(void)
{
	return dtmf_active;
}

void dtmf_deinit(void)
{
	// amp stby
	GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_SET);
}

void dtmf_set(uint8_t code)
{
	if (code == 0) {
		TIM_Cmd(TIM1, DISABLE);
		dtmf_active = 0;
		return;
	}

	dtmf_low = dtmf_code_low[code & 0x0F];
	dtmf_high = dtmf_code_high[(code >> 4) & 0x0F];

	TIM_Cmd(TIM1, ENABLE);
	dtmf_active = 1;
}

void TIM1_UP_IRQHandler(void)
{
	static uint16_t low_val = 0;
	static uint16_t high_val = 0;

	uint8_t low_ptr;
	uint8_t high_ptr;

	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);

	low_val += dtmf_low;
	high_val += dtmf_high;

	low_ptr = ((low_val + 4) >> 3) & (0x007F);
	high_ptr = ((high_val + 4) >> 3) & (0x007F);

	// high + 3/4 low
	TIM1->CCR1 = dtmf_sine[high_ptr] + (dtmf_sine[low_ptr] - (dtmf_sine[low_ptr] >> 2));
}
