/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : dfu_mal.c
* Author             : MCD Application Team
* Version            : V3.1.1
* Date               : 04/07/2010
* Description        : Generic media access Layer
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "dfu_mal.h"
#include "flash_if.h"
#include "usb_lib.h"
#include "usb_type.h"
#include "usb_desc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*nt16_t (*pMAL_Init) (void);
uint16_t (*pMAL_Erase) (uint32_t SectorAddress);
uint16_t (*pMAL_Write) (uint32_t SectorAddress, uint32_t DataLength);
uint8_t  *(*pMAL_Read)  (uint32_t SectorAddress, uint32_t DataLength);*/
uint8_t  MAL_Buffer[wTransferSize]; /* RAM Buffer for Downloaded Data */
extern ONE_DESCRIPTOR DFU_String_Descriptor[7];


static const uint16_t  TimingTable[5][2] =
  {
//    { 3000 ,  20 }, /* SPI Flash */
//    { 1000 ,  25 }, /* NOR Flash M29W128F */
    {  100 , 104 }, /* Internal Flash */
//    { 1000 ,  25 }, /* NOR Flash M29W128G */
//    { 1000 ,  45 }  /* NOR Flash S29GL128 */
  };

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Init(void)
{

//  FLASH_If_Init(); /* Internal Flash */

  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Erase
* Description    : Erase sector
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*uint16_t MAL_Erase(uint32_t SectorAddress)
{
	if ((SectorAddress & MAL_MASK) != INTERNAL_FLASH_BASE)
		return MAL_FAIL;
	
	return FLASH_If_Erase(SectorAddress);

	switch (SectorAddress & MAL_MASK)
  {
    case INTERNAL_FLASH_BASE:
      pMAL_Erase = FLASH_If_Erase;
      break;
    default:
      return MAL_FAIL;
  }
  return pMAL_Erase(SectorAddress);
}*/

/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Write (uint32_t SectorAddress, uint32_t DataLength)
{
	if ((SectorAddress & MAL_MASK) != INTERNAL_FLASH_BASE)
		return MAL_FAIL;
	
	return FLASH_If_Write(SectorAddress, DataLength);
/*
  switch (SectorAddress & MAL_MASK)
  {
    case INTERNAL_FLASH_BASE:
      pMAL_Write = FLASH_If_Write;
      break;
    default:
      return MAL_FAIL;
  }
  return pMAL_Write(SectorAddress, DataLength);*/
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
/*nt8_t *MAL_Read (uint32_t SectorAddress, uint32_t DataLength)
{
	if ((SectorAddress & MAL_MASK) != INTERNAL_FLASH_BASE)
		return 0;
	
	return FLASH_If_Read(SectorAddress, DataLength);
  switch (SectorAddress & MAL_MASK)
  {
    case INTERNAL_FLASH_BASE:
      pMAL_Read = FLASH_If_Read;
      break;
    default:
      return 0;
  }
  return pMAL_Read (SectorAddress, DataLength);
}*/

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
uint16_t MAL_GetStatus(uint32_t SectorAddress , uint8_t Cmd, uint8_t *buffer)
{
//	uint8_t x = (SectorAddress  >> 26) & 0x03 ; /* 0x000000000 --> 0 */
//  /* 0x640000000 --> 1 */
//  /* 0x080000000 --> 2 */
//
  uint8_t y = Cmd & 0x01;
//
//  if ((x == 1) && (NOR_ID.Device_Code2 == NOR_M29W128G)&& (NOR_ID.Manufacturer_Code == 0x20))
//  {
//    x = 3 ;
//  }
//  else if((x == 1) && (NOR_ID.Device_Code2 == NOR_S29GL128) && (NOR_ID.Manufacturer_Code == 0x01))
//  {
//    x = 4 ;
//  }  
//
//  SET_POLLING_TIMING(TimingTable[x][y]);  /* x: Erase/Write Timing */
  SET_POLLING_TIMING(TimingTable[0][y]);  /* x: Erase/Write Timing */
  /* y: Media              */
  return MAL_OK;
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
