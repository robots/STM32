/**********************************************************************************/
/*                                                                                */
/*    Copyright (C) 2005 OLIMEX  LTD.                                             */
/*                                                                                */
/*    Module Name    :  driver for EEPROM                                         */
/*    File   Name    :  drv_24xx.c                                                */
/*    Revision       :  01.00                                                     */
/*    Date           :  2005/07/04 initial version                                */
/*                                                                                */
/**********************************************************************************/

#include "drv_lisxxx.h"


/****************************************************************************/
/*  Wait for acknowledge                                                    */
/*  Function : WriteByte                                                    */
/*      Parameters                                                          */
/*          Input   :  address and character to write                       */
/*          Output  :  error status                                         */
/****************************************************************************/
DRV_STATUS Wait_AKN(unsigned char addr) {
  unsigned int i = I2C_TIME_OUT;
  do
  {
    Hrd_I2C_StartCond();
    if(--i == 0) return DRV_ERR;
  }while(Hrd_I2C_WriteByte(addr+WRITE) != I2C_AKN);
  return DRV_OK;
}


/****************************************************************************/
/*  Write byte to REGISTER                                                  */
/*  Function : WriteReg                                                     */
/*      Parameters                                                          */
/*          Input   :  register and character to write                      */
/*          Output  :  error status                                         */
/****************************************************************************/
DRV_STATUS WriteReg(unsigned char byte, unsigned int addr) {
  if(Wait_AKN(PHYS_ADDR_LIS) == DRV_ERR) return DRV_ERR;
  if(Hrd_I2C_WriteByte(addr)!=I2C_AKN) return DRV_ERR;
  if(Hrd_I2C_WriteByte(byte)!=I2C_AKN) return DRV_ERR;
  Hrd_I2C_StopCond();
  return DRV_OK;
}

/****************************************************************************/
/*  Read REGISTER	                                                    */
/*  Function : ReadReg                                                      */
/*      Parameters                                                          */
/*          Input   :  address and character to write                       */
/*          Output  :  error status                                         */
/****************************************************************************/
DRV_STATUS ReadReg(unsigned char* byte, unsigned int addr) {
  if(Wait_AKN(PHYS_ADDR_LIS) == DRV_ERR) return DRV_ERR;
  if(Hrd_I2C_WriteByte(addr)!=I2C_AKN) return DRV_ERR;
  Hrd_I2C_StartCond();
  Hrd_I2C_WriteByte((unsigned char)(PHYS_ADDR_LIS+READ));
  *byte = Hrd_I2C_ReadByte(I2C_NAK);
  Hrd_I2C_StopCond();
  return DRV_OK;
}



