
#include "stm32f10x.h"
#include "rtc.h"

INTFUNC void RTC_IRQHandler(void)
{
	static int i = 0;

	if (RTC_GetITStatus(RTC_IT_SEC) != RESET) {
		/* Clear the RTC Second interrupt */
		RTC_ClearITPendingBit(RTC_IT_SEC);

		// Blinking
		if (i == 0) {
			GPIO_WriteBit(GPIOC, GPIO_Pin_12, Bit_RESET);
			i = 1;
		} else {
			GPIO_WriteBit(GPIOC, GPIO_Pin_12, Bit_SET);
			i = 0;
		}
	}
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
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

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
	RTC_SetPrescaler(32767);

	RTC_WaitForLastTask();

	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}



