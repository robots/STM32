/**********************************************************************************/
/*                                                                                */
/*    Copyright (C) 2005 OLIMEX  LTD.                                             */
/*                                                                                */
/*    Module Name    :  driver for EEPROM                                         */
/*    File   Name    :  drv_24xx.h                                                */
/*    Revision       :  01.00                                                     */
/*    Date           :  2005/07/04 initial version                                */
/*                                                                                */
/**********************************************************************************/

#ifndef drv_24xx
#define drv_24xx

#include "i2c.h"

/****************************************/
/*          definition                  */
/****************************************/
#define I2C_TIME_OUT    10000
#define PHYS_ADDR_LIS  	0x3A
#define WRITE           0
#define READ            1

typedef enum {
 DRV_OK = 0,
 DRV_ERR = 1
}DRV_STATUS;


/****************************************************************************/
/*  Wait for acknowledge                                                    */
/*  Function : WriteByte                                                    */
/*      Parameters                                                          */
/*          Input   :  address and character to write                       */
/*          Output  :  error status                                         */
/****************************************************************************/
DRV_STATUS Wait_AKN(unsigned char addr);

/****************************************************************************/
/*  Write byte to REGISTER                                                  */
/*  Function : WriteReg                                                     */
/*      Parameters                                                          */
/*          Input   :  register and character to write                      */
/*          Output  :  error status                                         */
/****************************************************************************/
DRV_STATUS WriteReg(unsigned char byte, unsigned int addr);

/****************************************************************************/
/*  Read REGISTER	                                                    */
/*  Function : ReadReg                                                      */
/*      Parameters                                                          */
/*          Input   :  address and character to write                       */
/*          Output  :  error status                                         */
/****************************************************************************/
DRV_STATUS ReadReg(unsigned char* byte, unsigned int addr);

#endif

