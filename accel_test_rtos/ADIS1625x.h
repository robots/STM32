/*
 * ADIS gyro driver
 *
 * 2009-2010 Michal Demin
 *
 */
#ifndef ADIS1625X_H_
#define ADIS1625X_H_

#define ADIS_READ     0x0000
#define ADIS_WRITE    0x8000

#define ADIS_REG_MASK 0x3f

/* flags valid for data out regs */
#define ADIS_ND_FLAG  0x8000
#define ADIS_EA_FLAG  0x4000
#define ADIS_DATA     0x3FFF

/* DATA OUT regs */
#define ENDURANCE     0x00
#define SUPPLY_OUT    0x02
#define GYRO_OUT      0x04
#define AUX_ADC       0x0a
#define TEMP_OUT      0x0c
#define ANGL_OUT      0x0e

/* CONFIG REGS */

/* gyro cal data */
#define GYRO_OFF      0x14
#define GYRO_SCALE    0x16

/* alarm settings */
#define ALM_MAG1      0x20
#define ALM_MAG2      0x22
#define ALM_SMPL1     0x24
#define ALM_SMPL2     0x26
#define ALM_CTRL      0x28

/* dac */
#define AUX_DAC       0x30

#define GPIO_CTRL     0x32
#define MSC_CTRL      0x34

/* ADC setting */
#define SMPL_PRD      0x36
#define SENS_AVG      0x38

/* sleep setting */
#define SLP_CNT       0x3a

/* controll registers */
#define STATUS        0x3c
#define COMMAND       0x3e

void ADIS_Init();
void ADIS_Reset();
uint16_t ADIS_ReadReg(uint8_t reg);
void ADIS_WriteReg(uint8_t reg, uint16_t data);


#endif

