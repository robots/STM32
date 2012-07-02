/* note:
 *
 *
 *
 * TODO:
 * - fix freq input
 */
#include "stm32f10x.h"
#include "ad.h"

// 1635 is the center on my board
#define AD_CENTER 1635

ad_callback_t ad_cb = NULL;

static void ADC_Enable(ADC_TypeDef* ADCx)
{
	ADC_Cmd(ADCx, ENABLE);

	/* do calibration */
	ADC_ResetCalibration(ADCx);
	while(ADC_GetResetCalibrationStatus(ADCx));
	ADC_StartCalibration(ADCx);
	while(ADC_GetCalibrationStatus(ADCx));
}

static void AD_Reset()
{
	ADC_InitTypeDef ADC_InitStructure;

	ADC_DeInit(ADC1);

	/* ADC configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_RegInjecSimult; //ADC_Mode_RegSimult;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;

	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_ExternalTrigConvCmd(ADC1, ENABLE);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);

	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

	/* enable and calibrate ADCs */
	ADC_Enable(ADC1);

}

/*
	// timer period = 72000000 hz /90/100 = 8000hz
	AD_SetTimer(100, 90);
*/
void AD_SetTimer(uint16_t prescaler, uint16_t period)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {
		.TIM_ClockDivision = 0x0,
		.TIM_CounterMode = TIM_CounterMode_Up,
  	.TIM_RepetitionCounter = 0x0000
	};

	TIM_DeInit(TIM3);

	if ((period <= 3) || (prescaler <= 1)) {
		return;
	}

	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Period = period - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = prescaler - 1;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);
}

void ADC1_2_IRQHandler(void)
{
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

	int16_t data = ADC1->DR - AD_CENTER;

	ad_cb(data);
}

void AD_Init(ad_callback_t callback)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* init ad converters */
	AD_Reset();

	ad_cb = callback;
}

void AD_Start()
{
	if (UNLIKELY(ad_cb == NULL)) {
		return;
	}

	/* TIM3 counter enable */
	TIM_Cmd(TIM3, ENABLE);
}

void AD_Stop()
{
	/* TIM3 counter disable */
	TIM_Cmd(TIM3, DISABLE);

	AD_Reset();
}


