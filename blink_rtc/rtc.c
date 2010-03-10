
#include "stm32f10x.h"
#include "rtc.h"

INTFUNC void RTC_IRQHandler(void)
{
	// Blinking
	if((GPIOC->IDR)&(0x00001000)) {
		// clear
		GPIOC->BRR |= 0x00001000;
	} else {
		// set
		GPIOC->BSRR |= 0x00001000;
	}

	RTC_ClearFlag(RTC_IT_SEC);
}

void RTCInit(void) {
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  

	// Allow access to BKP Domain
	PWR_BackupAccessCmd(ENABLE);

	// Reset Backup Domain
	BKP_DeInit();

	// Enable the LSE OSC
	RCC_LSEConfig(RCC_LSE_ON);

	// Disable the LSI OSC
	RCC_LSICmd(DISABLE);

	// Select the RTC Clock Source
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	// Enable the RTC Clock
	RCC_RTCCLKCmd(ENABLE);

	// Wait for RTC registers synchronization
	RTC_WaitForSynchro();

	// Wait until last write operation on RTC registers has finished
	RTC_WaitForLastTask();

	// Enable the RTC overflow interrupt
	RTC_ITConfig(RTC_IT_SEC, ENABLE);

	//Set 32768 prescaler - for one second interupt
	RTC_SetPrescaler(0x7FFF);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}



