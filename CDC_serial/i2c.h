/**********************************************************************************/
/*                                                                                */
/*    Copyright (C) 2005 OLIMEX  LTD.                                             */
/*                                                                                */
/*    Module Name    :  i2c module                                                */
/*    File   Name    :  i2c.h                                                     */
/*    Revision       :  01.00                                                     */
/*    Date           :  2005/07/04 initial version                                */
/*                                                                                */
/**********************************************************************************/

#ifndef I2C_def
#define I2C_def

#include "stm32f10x.h"


#define I2C_TIME_OUT    10000
#define PHYS_ADDR_LIS  	0x3A
#define WRITE           0
#define READ            1

typedef enum {
 DRV_OK = 0,
 DRV_ERR = 1
}DRV_STATUS;


typedef enum
{
  I2C_NAK = 0,
  I2C_AKN = 1,
}I2C_AKN_DEF;

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

/****************************************************************************/
/*  Initialize I2C interface                                                */
/*  Function : Hrd_I2C_Init                                                 */
/*      Parameters                                                          */
/*          Input   :   Nothing                                             */
/*          Output  :   Nothing                                             */
/****************************************************************************/
void Hrd_I2C_Init();


/****************************************************************************/
/*  Start Conditional for I2C                                               */
/*  Function : Hrd_I2C_StartCond                                            */
/*      Parameters                                                          */
/*          Input   :   Nothing                                             */
/*          Output  :   Nothing                                             */
/****************************************************************************/
void Hrd_I2C_StartCond();


/****************************************************************************/
/*  Stop Conditional for I2C                                                */
/*  Function : Hrd_I2C_StopCond                                             */
/*      Parameters                                                          */
/*          Input   :   Nothing                                             */
/*          Output  :   Nothing                                             */
/****************************************************************************/
void Hrd_I2C_StopCond();


/****************************************************************************/
/*  Write Byte to I2C                                                       */
/*  Function : Hrd_I2C_WriteByte                                            */
/*      Parameters                                                          */
/*          Input   :   character to write                                  */
/*          Output  :   acknowledge                                         */
/****************************************************************************/
I2C_AKN_DEF Hrd_I2C_WriteByte  (unsigned char ch);


/****************************************************************************/
/*  Read Byte from I2C                                                      */
/*  Function : Hrd_I2C_ReadByte                                             */
/*      Parameters                                                          */
/*          Input   :   need acknowledge                                    */
/*          Output  :   read character                                      */
/****************************************************************************/
unsigned char Hrd_I2C_ReadByte (I2C_AKN_DEF  Akn);

#endif
