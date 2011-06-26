#include "platform.h"
#include "stm32f10x.h"


#include "modem.h"
#include "log.h"
#include "adc.h"

uint16_t AD_Data;
/* debouncing timer */
static uint8_t ADC_timeout = 0;
uint8_t ADC_nextstate;

void ADC_init()
{
	NVIC_InitTypeDef NVIC_InitStructure = {
		.NVIC_IRQChannel = ADC1_2_IRQn,
		.NVIC_IRQChannelPreemptionPriority = 1,
		.NVIC_IRQChannelSubPriority = 0,
		.NVIC_IRQChannelCmd = ENABLE
	};

	ADC_InitTypeDef ADC_InitStructure = {
		.ADC_Mode = ADC_Mode_Independent,
		.ADC_ScanConvMode = DISABLE,
		.ADC_ContinuousConvMode = ENABLE,
		.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None,
		.ADC_DataAlign = ADC_DataAlign_Right,
		.ADC_NbrOfChannel = 1
	};

	DMA_InitTypeDef ADC_DMA_Init = {
		.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR,
		.DMA_MemoryBaseAddr = (uint32_t)&AD_Data,
		.DMA_BufferSize = 1,
		.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
		.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord,
		.DMA_DIR = DMA_DIR_PeripheralSRC,
		.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
		.DMA_MemoryInc = DMA_MemoryInc_Disable,
		.DMA_Mode = DMA_Mode_Circular,
		.DMA_Priority = DMA_Priority_High,
		.DMA_M2M = DMA_M2M_Disable
	};

	DMA_DeInit(DMA1_Channel1);
	DMA_Init(DMA1_Channel1, &ADC_DMA_Init);
	DMA_Cmd(DMA1_Channel1, ENABLE);

	NVIC_Init(&NVIC_InitStructure);

	ADC_DeInit(ADC1);
	ADC_Init(ADC1, &ADC_InitStructure);
  ADC_DMACmd(ADC1, ENABLE);

	/* Setup regular channel configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);

	ADC_Cmd(ADC1, ENABLE);

	// do the calibration
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void ADC_watch(uint8_t a)
{
	ADC_ITConfig(ADC1, ADC_IT_AWD, DISABLE);
	ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_None);

	if (a == AD_DIS) {
		return;
	} else if (a == AD_LOW) {
		ADC_AnalogWatchdogThresholdsConfig(ADC1, 2048, 4096);
	} else {
		ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x0000, 2048);

	}
	ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_AllRegEnable);
	ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
	ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);
}

void ADC_Timer()
{
	if (ADC_timeout > 0)
		ADC_timeout--;

	if (ADC_timeout == 1) {
		if (ADC_nextstate == AD_LOW) {
			Log_Stop();
		} else {
			Log_Start();
		}
	}
}

void ADC1_2_IRQHandler(void)
{
	ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);

	ADC_timeout = 3;
	if (AD_Data > 2048) {
		ADC_nextstate = AD_HIGH;
	} else {
		ADC_nextstate = AD_LOW;
	}
}

