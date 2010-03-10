/*
 * nRF24 driver for STM32 family processors
 *
 * 2009-2010 Michal Demin
 *
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "stm32f10x.h"
#include "platform.h"

#include "spi.h"

#include "nRF24.h"

SPI_InitTypeDef nRFSPIConf;

void nRF_Init() {

	nRFSPIConf.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	nRFSPIConf.SPI_Mode = SPI_Mode_Master;
	nRFSPIConf.SPI_DataSize = SPI_DataSize_8b;
	nRFSPIConf.SPI_CPOL = SPI_CPOL_Low;
	nRFSPIConf.SPI_CPHA = SPI_CPHA_1Edge;
	nRFSPIConf.SPI_NSS = SPI_NSS_Soft;
	nRFSPIConf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	nRFSPIConf.SPI_FirstBit = SPI_FirstBit_MSB;
	nRFSPIConf.SPI_CRCPolynomial = 7;
	
	SPIx_Init(SPI1);

}

void nRF_ReadStatus() {
	uint8_t tmp;

	return nRF_ReadCmd(NRF_NOP, &tmp, 0);
}

uint8_t nRF_ReadCmd(uint8_t cmd, uint8_t *buf, uint8_t size) {
	uint8_t in[33]; // status + 32 max
	uint8_t out[33]; // cmd + 32 nop
	
	SPIx_Lock(SPI1);
	
	/* write the cmd */
	memset(out, 0, 33);
	out[0] = cmd;

	NRF_CS_WRITE(Bit_RESET);
	SPIx_TranscieveBuffer(SPI1, &nRFSPIConf, &out[0], size+1, &in[0], size+1);
	NRF_CS_WRITE(Bit_SET);
	/* 
	 * in[0] - status register content
	 * in[1] - reg content
	 */
	SPIx_Unlock(SPI1);

	memcpy(buf, &in[1], size);

	return in[0];

}

uint8_t nRF_WriteCmd(uint8_t cmd, uint8_t *buf, uint8_t size) {
	uint8_t in[1];
	uint8_t out[33]; // cmd + 32 max
	
	SPIx_Lock(SPI1);
	
	/* write the cmd */
	out[0] = cmd;
	memcpy(&out[1], buf, size);

	NRF_CS_WRITE(Bit_RESET);
	SPIx_TranscieveBuffer(SPI1, &nRFSPIConf, &out[0], size+1, &in[0], 1);
	NRF_CS_WRITE(Bit_SET);
	/* 
	 * in[0] - status register content
	 */
	SPIx_Unlock(SPI1);

	return in[0];
}


