
#include "stm32f10x.h"
#include "rtc.h"

#include "log.h"

void RTC_IRQHandler(void)
{
	ADC_Timer();
	Log_Timer();
	RTC_ClearFlag(RTC_IT_SEC);
}

void RTCInit(void) {
	NVIC_InitTypeDef NVIC_InitStructure = {
		.NVIC_IRQChannel = RTC_IRQn,
		.NVIC_IRQChannelPreemptionPriority = 1,
		.NVIC_IRQChannelSubPriority = 0,
		.NVIC_IRQChannelCmd = ENABLE
	};
	

	// Allow access to BKP Domain
	PWR_BackupAccessCmd(ENABLE);

	// Reset Backup Domain
	BKP_DeInit();

	// Disable the LSI OSC
	RCC_LSICmd(ENABLE);

	// Select the RTC Clock Source
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI); 

	// Enable the RTC Clock
	RCC_RTCCLKCmd(ENABLE);

	// Wait for RTC registers synchronization
	RTC_WaitForSynchro();

	// Wait until last write operation on RTC registers has finished
	RTC_WaitForLastTask();

	// Enable the RTC overflow interrupt
	RTC_ITConfig(RTC_IT_SEC, ENABLE);

	//Set 40000 prescaler - for one second interupt
	RTC_SetPrescaler(0x9C3F);

	NVIC_Init(&NVIC_InitStructure);

}

