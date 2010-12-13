/*
 * SPI driver for STM32 family processors
 *
 * 2009-2010 Michal Demin
 *
 */
#include "platform.h"
#include "stm32f10x.h"

#include "spi.h"

static SPI_InitTypeDef *SPI_ConfCur = NULL;

void SPI_init(void)
{
/*	SPI_InitTypeDef SPIConf = {
		.SPI_Direction = SPI_Direction_2Lines_FullDuplex,
		.SPI_Mode = SPI_Mode_Master,
		.SPI_DataSize = SPI_DataSize_16b,
		.SPI_CPOL = SPI_CPOL_Low,
		.SPI_CPHA = SPI_CPHA_1Edge,
		.SPI_NSS = SPI_NSS_Soft,
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32,
		.SPI_FirstBit = SPI_FirstBit_MSB,
		.SPI_CRCPolynomial = 7
	};


	SPI_Init(SPI1, &SPIConf);
	SPI_Cmd(SPI1, ENABLE);*/
}

/* Simple Byte transmit */
uint16_t SPI_Xfer(SPI_InitTypeDef *c, uint16_t data)
{
	if (SPI_ConfCur != c) {
		SPI_Cmd(SPI1, DISABLE);
		SPI_Init(SPI1, &SPIConf);
		SPI_Cmd(SPI1, ENABLE);
	}

	while ((SPI1->SR & SPI_I2S_FLAG_TXE) == RESET);

	// Send byte through the SPI1 peripheral
	SPI1->DR = data;

	// Wait to receive a byte
	while ((SPI1->SR & SPI_I2S_FLAG_RXNE) == RESET);

	return SPI1->DR;
}

