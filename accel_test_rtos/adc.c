/*
 * ADC driver
 *
 * 2009-2010 Michal Demin
 *
 */
#include "stm32f10x.h"

#include "adc.h"



void ADC_Setup(void) {

	ADC_InitTypeDef   ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	ADC_DeInit(ADC1);

	// ADC Structure Initialization
	ADC_StructInit(&ADC_InitStructure);

	// Preinit
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	// Enable the ADC
	ADC_Cmd(ADC1, ENABLE);
}

uint16_t ADC_GetChannel(uint8_t ch) {

	// Configure chanel
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_55Cycles5);

	// Start the conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	// Wait until conversion completion
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

	// Get the conversion value
	return ADC_GetConversionValue(ADC1);

}

