/*
 * ADIS gyro driver
 *
 * 2009-2010 Michal Demin
 *
 */
#include "stm32f10x.h"
#include "platform.h"

#include "ADIS1625x.h"


void ADIS_Init() {
	SPI_InitTypeDef ADISSPIConf;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// SPI2 module enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	ADISSPIConf.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	ADISSPIConf.SPI_Mode = SPI_Mode_Master;
	ADISSPIConf.SPI_DataSize = SPI_DataSize_8b;
	ADISSPIConf.SPI_CPOL = SPI_CPOL_High;
	ADISSPIConf.SPI_CPHA = SPI_CPHA_2Edge;
	ADISSPIConf.SPI_NSS = SPI_NSS_Soft;
	ADISSPIConf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	ADISSPIConf.SPI_FirstBit = SPI_FirstBit_MSB;
	ADISSPIConf.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &ADISSPIConf);
	
	// Configure SPI2 pins: SCK, MISO and MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // alternate function
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Enable SPI2
	SPI_Cmd(SPI2, ENABLE);

	ADIS_Reset();
}

void ADIS_Reset() {
	uint32_t a = 100000;
	ADIS_RESET_WRITE(Bit_RESET);
	while (a) a--;
	ADIS_RESET_WRITE(Bit_SET);
}

void ADIS_SendBuf(uint8_t *out, uint8_t *in, uint8_t c) {
	uint8_t cnt = 0;
	uint8_t dummy;

	for (cnt = 0; cnt < c; cnt ++) {
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

		// Send byte through the SPI2 peripheral
		SPI2->DR = out[cnt];

		// Wait to receive a byte
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

		// Return the byte read from the SPI bus
		if (in != 0) 
			in[cnt] = SPI2->DR;
		else 
			dummy = SPI2->DR;
	}
}

uint16_t ADIS_ReadReg(uint8_t reg) {
	uint32_t a;
	uint8_t in[4];
	uint8_t out[4];
	
	/* write the reg address */
	out[0] = (ADIS_READ | (reg & ADIS_REG_MASK));
	out[1] = 0;
	out[2] = 0;
	out[3] = 0;

	ADIS_CS_WRITE(Bit_RESET);
	ADIS_SendBuf(&out[0], &in[0], 2);
	ADIS_CS_WRITE(Bit_SET);

	a = 500;
	while (a) a--;
	
	ADIS_CS_WRITE(Bit_RESET);
	ADIS_SendBuf(&out[0], &in[0], 2);
	ADIS_CS_WRITE(Bit_SET);

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

