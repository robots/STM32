/*
 * SPI driver for STM32 family processors
 *
 * 2009-2010 Michal Demin
 *
 */

#ifndef SPI_H_
#define SPI_H_


void SPI_init();

uint16_t SPI_Xfer(uint16_t data);

#endif

