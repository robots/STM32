/*
 * ADIS gyro driver
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

#include "ADIS1625x.h"

SPI_InitTypeDef ADISSPIConf;

void ADIS_Init() {

	ADISSPIConf.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	ADISSPIConf.SPI_Mode = SPI_Mode_Master;
	ADISSPIConf.SPI_DataSize = SPI_DataSize_8b;
	ADISSPIConf.SPI_CPOL = SPI_CPOL_High;
	ADISSPIConf.SPI_CPHA = SPI_CPHA_2Edge;
	ADISSPIConf.SPI_NSS = SPI_NSS_Soft;
	ADISSPIConf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	ADISSPIConf.SPI_FirstBit = SPI_FirstBit_MSB;
	ADISSPIConf.SPI_CRCPolynomial = 7;
	
	SPIx_Init(SPI2);

	ADIS_Reset();
}

void ADIS_Reset() {
	ADIS_RESET_WRITE(Bit_RESET);
	vTaskDelay(300 / portTICK_RATE_MS);
	ADIS_RESET_WRITE(Bit_SET);
}

uint16_t ADIS_ReadReg(uint8_t reg) {
	uint8_t in[4];
	uint8_t out[4];
	
	SPIx_Lock(SPI2);
	
	/* write the reg address */
	out[0] = (ADIS_READ | (reg & ADIS_REG_MASK));
	out[1] = 0;
	out[2] = 0;
	out[3] = 0;

	ADIS_CS_WRITE(Bit_RESET);
	SPIx_TranscieveBuffer(SPI2, &ADISSPIConf, &out[0], 2, &in, 2);
	ADIS_CS_WRITE(Bit_SET);
	
	vTaskDelay(1 / portTICK_RATE_MS);
	
	ADIS_CS_WRITE(Bit_RESET);
	SPIx_TranscieveBuffer(SPI2, &ADISSPIConf, &out[2], 2, &in, 2);
	ADIS_CS_WRITE(Bit_SET);

	SPIx_Unlock(SPI2);

	return in[0] << 8 | in[1];
}

void ADIS_WriteReg(uint8_t reg, uint16_t data) {
/*	uint8_t out[4];
	
	SPIx_Lock(SPI2);
	
	// First the MSBs of the reg 
	out[0] = (ADIS_WRITE | ((reg+1) & ADIS_REG_MASK));
	out[1] = (data >> 8) & 0xFF;
	// then the LSBs of the reg 
	out[2] = (ADIS_WRITE | (reg & ADIS_REG_MASK));
	out[3] = data & 0xFF;

	ADIS_CS_WRITE(Bit_RESET);
	SPIx_TranscieveBuffer(SPI2, &ADISSPIConf, &out[0], 2, NULL, 0);
	ADIS_CS_WRITE(Bit_SET);
	
	vTaskDelay(1 / portTICK_RATE_MS);
	
	ADIS_CS_WRITE(Bit_RESET);
	SPIx_TranscieveBuffer(SPI2, &ADISSPIConf, &out[2], 2, NULL, 0);
	ADIS_CS_WRITE(Bit_SET);

	SPIx_Unlock(SPI2);
*/
}

