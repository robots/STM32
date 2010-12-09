/*
 * SPI driver for STM32 family processors
 *
 * 2009-2010 Michal Demin
 *
 */
#include "platform.h"
#include "stm32f10x.h"

#include "spi.h"

void SPI_init(void)
{
	SPI_InitTypeDef SPIConf;

	SPIConf.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPIConf.SPI_Mode = SPI_Mode_Master;
	SPIConf.SPI_DataSize = SPI_DataSize_16b;
	SPIConf.SPI_CPOL = SPI_CPOL_Low;
	SPIConf.SPI_CPHA = SPI_CPHA_1Edge;
	SPIConf.SPI_NSS = SPI_NSS_Soft;
	SPIConf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPIConf.SPI_FirstBit = SPI_FirstBit_MSB;
	SPIConf.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPIConf);
	SPI_Cmd(SPI1, ENABLE);
}

/* Simple Byte transmit */
uint16_t SPI_Xfer(uint16_t data)
{

	while ((SPI1->SR & SPI_I2S_FLAG_TXE) == RESET);

	// Send byte through the SPI1 peripheral
	SPI1->DR = data;

	// Wait to receive a byte
	while ((SPI1->SR & SPI_I2S_FLAG_RXNE) == RESET);

	return SPI1->DR;
}

