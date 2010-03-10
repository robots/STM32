/*
 * Driver for LisXXX accelerometer
 *
 * 2009-2010 Michal Demin
 *
 */

#include "i2c.h"

#include "lisXXX.h"

void LisXXX_Init() {
	uint8_t val;	
	Hrd_I2C_Init();
	
	ReadReg(&val, WHO_AM_I);    // have to be 0x3A
	
	// Set CTRL_REG1
	WriteReg(0xC7, CTRL_REG1);
	
	// Set CTRL_REG2
	WriteReg(0x55, CTRL_REG2);
	
	// Set CTRL_REG3
	WriteReg(0x00, CTRL_REG3);

}

void LisXXX_GetData(struct lisxxx_data_t *data) {
	uint8_t val_h;
	uint8_t val_l;

	// X
	ReadReg(&val_l, OUTX_L);
	ReadReg(&val_h, OUTX_H);
	data->ax = (val_h<<8)|val_l;

	// y
	ReadReg(&val_l, OUTY_L);
	ReadReg(&val_h, OUTY_H);
	data->ay = (val_h<<8)|val_l;

	// z
	ReadReg(&val_l, OUTZ_L);
	ReadReg(&val_h, OUTZ_H);
	data->az = (val_h<<8)|val_l;
	
}

