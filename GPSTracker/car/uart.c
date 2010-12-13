#include "platform.h"
#include "stm32f10x.h"

#include "nmea.h"
#include "uart.h"

#define UART_GPSSIZE 128
uint8_t UART_GpsBuf[UART_GPSSIZE];
uint8_t UART_GpsIdx = 0;

void UART_Init() {
	USART_InitTypeDef USART_InitStructure = {
		.USART_BaudRate = 9200; //921600; //115200; //230400; // 921600
		.USART_WordLength = USART_WordLength_8b;
		.USART_StopBits = USART_StopBits_1;
		.USART_Parity = USART_Parity_No;
		.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	};

  NVIC_InitTypeDef NVIC_InitStructure = {
		.NVIC_IRQChannel = USART1_IRQn,
		.NVIC_IRQChannelPreemptionPriority = 1,
		.NVIC_IRQChannelSubPriority = 0,
		.NVIC_IRQChannelCmd = ENABLE
	};

	DMA_InitTypeDef DMA_InitStructure = {
		.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR,
		.DMA_MemoryBaseAddr = (uint32_t)&AD_Data[0],
		.DMA_BufferSize = AD_BUFFER,
		.DMA_DIR = DMA_DIR_PeripheralDST,
		.DMA_PeripheralInc = DMA_PeripheralInc_Disable,
		.DMA_MemoryInc = DMA_MemoryInc_Enable,
		.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
		.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte,
		.DMA_Mode = DMA_Mode_Normal,
		.DMA_Priority = DMA_Priority_VeryHigh,
		.DMA_M2M = DMA_M2M_Disable
  };

	/* DMA_Channel (triggered by USART Tx event) */
//	DMA_DeInit(DMA1_Channel4);
//	DMA_Init(DMA1_Channel4, &DMA_InitStructure);
//
//	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);


	/* Enable the USARTy Interrupt */
	NVIC_Init(&NVIC_InitStructure);

///	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
///	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
///	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
///	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
///	NVIC_Init(&NVIC_InitStructure);

	/* Configure USART1 */
	USART_Init(USART1, &USART_InitStructure);

	/* Enable RX interrupt */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	/* DMA event on TX */
	//USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

	/* Enable USART1 */
	USART_Cmd(USART1, ENABLE);
}

static void UART_ProcessGPS(char ch) {
	if ((UART_GpsIdx == UART_GPSSIZE-1) && (ch == '$')) {
		UART_GpsIdx = 0;
	}

	UART_GpsBuf[UART_GpsIdx] = ch;
	UART_GpsIdx++;

	if ((UART_GpsIdx > 6) && (UART_GpsBuf[0] == '$') && (ch == 0x0a)) {
		NMEA_Parse(UART_GpsBuf, UART_GpsIdx);
		UART_GpsIdx = 0;
	}
}

void DMA1_Channel4_IRQHandler() {
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA_ClearITPendingBit(DMA1_IT_TC4);
}

/* USART1 ISR routine */
void USART1_IRQHandler() {
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		/* Read one byte from the receive data register */
		UART_ProcessGPS(USART1->DR & 0xff);
	}
}

