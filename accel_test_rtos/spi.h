/*
 * SPI driver for STM32 family processors
 *
 * 2009-2010 Michal Demin
 *
 */

#ifndef SPI_H_
#define SPI_H_


void SPIx_Init(SPI_TypeDef *);

/* lock before doing anything more */
void SPIx_Lock(SPI_TypeDef *);
void SPIx_Unlock(SPI_TypeDef *);

/* use dma to transfer the buffer */
void SPIx_TranscieveBuffer(SPI_TypeDef *SPI, SPI_InitTypeDef *SPIConf, void *inBuf, uint16_t inSize, void *outBuff, uint16_t outSize);

/* transmit byte */
uint8_t SPIx_TransmitByte(SPI_TypeDef *SPI, SPI_InitTypeDef *SPIConf, uint8_t data);

#endif

