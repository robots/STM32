/*
 * SPI driver for STM32 family processors
 *
 * 2009-2010 Michal Demin
 *
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "platform.h"
#include "stm32f10x.h"

#include "spi.h"

xSemaphoreHandle xSPI1_Sem = NULL;
xSemaphoreHandle xSPI1_Mutex = NULL;
xSemaphoreHandle xSPI2_Sem = NULL;
xSemaphoreHandle xSPI2_Mutex = NULL;

void SPIx_Init(SPI_TypeDef *SPI) {
	xSemaphoreHandle xSPI_Mtx = NULL;
	
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	DMA_Channel_TypeDef* DMA_Tx;
	DMA_Channel_TypeDef* DMA_Rx;

	if (SPI == SPI1 && xSPI1_Mutex != NULL) return;
	if (SPI == SPI2 && xSPI2_Mutex != NULL) return;
	if (SPI == SPI3) return;

	/* initialize */
	if (SPI == SPI1) {
		xSPI1_Mutex = xSemaphoreCreateMutex();
		vSemaphoreCreateBinary( xSPI1_Sem );
		
		DMA_Rx = DMA1_Channel2;
		DMA_Tx = DMA1_Channel3;
		
		xSPI_Mtx = xSPI1_Mutex;
	} else {
		xSPI2_Mutex = xSemaphoreCreateMutex();
		vSemaphoreCreateBinary( xSPI2_Sem );

		DMA_Rx = DMA1_Channel4;
		DMA_Tx = DMA1_Channel5;
		
		xSPI_Mtx = xSPI2_Mutex;
	}

	xSemaphoreTake(xSPI_Mtx, portMAX_DELAY);

	// SPI module enable
	if (SPI == SPI1) {
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	} else {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	}
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* Init DMA structure */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI->DR;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	/* Init DMA Channel (MEM->SPI) */
	DMA_Init(DMA_Tx, &DMA_InitStructure);
	DMA_ITConfig(DMA_Tx, DMA_IT_TC, ENABLE);

	/* Init DMA Channel (SPI->MEM) */
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;

	DMA_Init(DMA_Rx, &DMA_InitStructure);
	DMA_ITConfig(DMA_Rx, DMA_IT_TC, ENABLE);


	if (SPI == SPI1) {
		// Configure SPI1 pins: SCK, MISO and MOSI
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // alternate function
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	} else {
		// Configure SPI2 pins: SCK, MISO and MOSI
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // alternate function
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	}

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	if (SPI == SPI1) {
		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
		NVIC_Init(&NVIC_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
		NVIC_Init(&NVIC_InitStructure);
	} else {
		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
		NVIC_Init(&NVIC_InitStructure);
		
		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
		NVIC_Init(&NVIC_InitStructure);
	}

	xSemaphoreGive(xSPI_Mtx);
}

#define DMA_DATA_MASK   (0x00000F00)
#define SPI_DMA_8BIT    (DMA_PeripheralDataSize_Byte | DMA_MemoryDataSize_Byte)
#define SPI_DMA_16BIT   (DMA_PeripheralDataSize_HalfWord | DMA_MemoryDataSize_HalfWord)

void SPIx_TranscieveBuffer(SPI_TypeDef *SPI, SPI_InitTypeDef* SPIConf, void *inBuf, uint16_t inSize, void *outBuf, uint16_t outSize) {
	xSemaphoreHandle xSPI_Sem = NULL;
	DMA_Channel_TypeDef* DMA_Tx;
	DMA_Channel_TypeDef* DMA_Rx;
	uint32_t DMA_DataSize = SPI_DMA_8BIT;

	if (SPI == SPI1) {
		DMA_Rx = DMA1_Channel2;
		DMA_Tx = DMA1_Channel3;
		xSPI_Sem = xSPI1_Sem;
	} else if (SPI == SPI2) {
		DMA_Rx = DMA1_Channel4;
		DMA_Tx = DMA1_Channel5;
		xSPI_Sem = xSPI2_Sem;
	} else {
		return;
	}

	if (SPIConf->SPI_DataSize != SPI_DataSize_8b) {
		DMA_DataSize = SPI_DMA_16BIT;
	}

	SPI_Init(SPI, SPIConf);

	if (outBuf != NULL) {
		/* tell DMA where to receive */
		DMA_Rx->CMAR = (uint32_t)outBuf;
		DMA_Rx->CNDTR = outSize;

		/* change the Datasize according the SPI */
		DMA_Rx->CCR &= ~DMA_DATA_MASK;
		DMA_Rx->CCR |= DMA_DataSize;

		/* enable Rx DMA channel and requests */
		DMA_Cmd(DMA_Rx, ENABLE);
		SPI_I2S_DMACmd(SPI, SPI_I2S_DMAReq_Rx, ENABLE);
	}

	/* tell DMA what to transfer */
	DMA_Tx->CMAR = (uint32_t)inBuf;
	DMA_Tx->CNDTR = inSize;

	/* change the Datasize according the SPI */
	DMA_Tx->CCR &= ~DMA_DATA_MASK;
	DMA_Tx->CCR |= DMA_DataSize;

	/* enable Tx DMA channel */
	DMA_Cmd(DMA_Tx, ENABLE);

	/* tell SPI1 to ask DMA for data */
	SPI_I2S_DMACmd(SPI, SPI_I2S_DMAReq_Tx, ENABLE);
	SPI_Cmd(SPI, ENABLE);

	/* we wait until transfer is completed */
	xSemaphoreTake(xSPI_Sem, portMAX_DELAY);

	/* disable Tx DMA channel request */
	SPI_I2S_DMACmd(SPI, SPI_I2S_DMAReq_Tx, DISABLE);

	if (outBuf != NULL) {
		/* disable Rx DMA channel request */ 
		SPI_I2S_DMACmd(SPI, SPI_I2S_DMAReq_Rx, ENABLE);
	}

	/* we are done, disable SPI */
	SPI_Cmd(SPI, DISABLE);
}

/* Simple Byte transmit */
uint8_t SPIx_TransmitByte(SPI_TypeDef *SPI, SPI_InitTypeDef *SPIConf, uint8_t data) {
	uint8_t ret;

	SPI_Init(SPI, SPIConf);
	SPI_Cmd(SPI, ENABLE);

	while(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_TXE) == RESET);

	// Send byte through the SPI1 peripheral
	SPI_I2S_SendData(SPI, data);	

	// Wait to receive a byte
	while(SPI_I2S_GetFlagStatus(SPI, SPI_I2S_FLAG_RXNE) == RESET);

	ret = SPI_I2S_ReceiveData(SPI);

	SPI_Cmd(SPI, DISABLE);

	return ret;
}

void SPIx_Lock(SPI_TypeDef *SPI) {
	if (SPI == SPI1) {
		xSemaphoreTake(xSPI1_Mutex, portMAX_DELAY);
	} else if (SPI == SPI2) {
		xSemaphoreTake(xSPI2_Mutex, portMAX_DELAY);
	}
}

void SPIx_Unlock(SPI_TypeDef *SPI) {
	if (SPI == SPI1) {
		xSemaphoreGive(xSPI1_Mutex);
	} else if (SPI == SPI2) {
		xSemaphoreGive(xSPI2_Mutex);
	}
}

/** ISR for DMA1 Channel2 
 * This DMA channel has higher priority, so it *should* end sooner
 * than Channel3, therefore there is no need for semaphores,
 * Just disable Channel2 and wait for Channel3 to finish
 */
void DMA1_Channel2_IRQHandler(void) {
	if (DMA_GetITStatus(DMA1_IT_TC2)) {
		/* clear int pending bit */
		DMA_ClearITPendingBit(DMA1_IT_GL2);
		/* disable DMA Channel */
		DMA_Cmd(DMA1_Channel2, DISABLE);
	}
}

/* ISR for DMA1 Channel3 */
void DMA1_Channel3_IRQHandler(void) {
	static portBASE_TYPE xHigherPriorityTaskWoken; 

	xHigherPriorityTaskWoken = pdFALSE;

	if (DMA_GetITStatus(DMA1_IT_TC3)) {
		/* clear int pending bit */
		DMA_ClearITPendingBit(DMA1_IT_GL3);
		/* disable the DMA Channel */
		DMA_Cmd(DMA1_Channel3, DISABLE);
		/* wake up the sleeping task */
		xSemaphoreGiveFromISR(xSPI1_Sem, &xHigherPriorityTaskWoken );
	}

	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

/** ISR for DMA1 Channel4 
 * This DMA channel has higher priority, so it *should* end sooner
 * than Channel5, therefore there is no need for semaphores,
 * Just disable Channel4 and wait for Channel5 to finish
 */
void DMA1_Channel4_IRQHandler(void) {
	if (DMA_GetITStatus(DMA1_IT_TC4)) {
		/* clear int pending bit */
		DMA_ClearITPendingBit(DMA1_IT_GL4);
		/* disable DMA Channel */
		DMA_Cmd(DMA1_Channel4, DISABLE);
	}
}

/* ISR for DMA1 Channel5 */
void DMA1_Channel5_IRQHandler(void) {
	static portBASE_TYPE xHigherPriorityTaskWoken; 

	xHigherPriorityTaskWoken = pdFALSE;

	if (DMA_GetITStatus(DMA1_IT_TC5)) {
		/* clear int pending bit */
		DMA_ClearITPendingBit(DMA1_IT_GL5);
		/* disable the DMA Channel */
		DMA_Cmd(DMA1_Channel5, DISABLE);
		/* wake up the sleeping task */
		xSemaphoreGiveFromISR(xSPI2_Sem, &xHigherPriorityTaskWoken );
	}

	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

