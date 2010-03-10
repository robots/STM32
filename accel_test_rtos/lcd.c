/**********************************************************************************/
/*                                                                                */
/*    Copyright (C) 2005 OLIMEX  LTD.                                             */
/*                                                                                */
/*    Module Name    :  LCD module                                                */
/*    File   Name    :  lcd.h                                                     */
/*    Revision       :  01.00                                                     */
/*    Date           :  2006/01/26 initial version                                */
/*                                                                                */
/**********************************************************************************/

/*
 *
 * Modified to use my SPI driver + Code clean-up
 * 2009-2010 Michal Demin
 *
 */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "stm32f10x.h"
#include "platform.h"

#include "spi.h"

#include "lcd.h"

#define LCD_PRIORITY       ( tskIDLE_PRIORITY + 1 )

// LCD memory index
unsigned int  LcdMemIdx = 0;

xSemaphoreHandle xLCDGo;
xSemaphoreHandle xLCDMutex;

SPI_InitTypeDef LcdSPIConf;

// represent LCD matrix
unsigned char  LcdMemory[LCD_FB_SIZE];

unsigned int LcdCmdIdx = 0;
unsigned char  LcdCmd[LCD_CMD_SIZE];

static void taskLCD( void *pvParameters );


void LCD_Init() {
	xLCDMutex = xSemaphoreCreateMutex();
	vSemaphoreCreateBinary(xLCDGo);
	xTaskCreate( taskLCD, ( signed char * ) "LCD", configMINIMAL_STACK_SIZE, NULL, LCD_PRIORITY, NULL );

}

static void taskLCD( void *pvParameters ) {

	const portTickType xDelay = 50 / portTICK_RATE_MS;

	LcdSPIConf.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	LcdSPIConf.SPI_Mode = SPI_Mode_Master;
	LcdSPIConf.SPI_DataSize = SPI_DataSize_8b;
	LcdSPIConf.SPI_CPOL = SPI_CPOL_High;
	LcdSPIConf.SPI_CPHA = SPI_CPHA_2Edge;
	LcdSPIConf.SPI_NSS = SPI_NSS_Soft;
	LcdSPIConf.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	LcdSPIConf.SPI_FirstBit = SPI_FirstBit_MSB;
	LcdSPIConf.SPI_CRCPolynomial = 7;
	
	SPIx_Init(SPI1);

  // D/C high
  LCD_DC_WRITE(Bit_SET);

  // LCD_E - disable
  LCD_E_WRITE(Bit_SET);

	// Set Reset pin (active low)
	LCD_RESET_WRITE(Bit_SET);

  // Toggle display reset pin.
  LCD_RESET_WRITE(Bit_RESET);
  vTaskDelay(xDelay);
	LCD_RESET_WRITE(Bit_SET);
  vTaskDelay(xDelay);

  // Send sequence of command
  LCD_CMD_Add(0x21);  // LCD Extended Commands.
  LCD_CMD_Add(0xC8);  // Set LCD Vop (Contrast).
  LCD_CMD_Add(0x06);  // Set Temp coefficent.
  LCD_CMD_Add(0x13);  // LCD bias mode 1:48.
  LCD_CMD_Add(0x20);  // LCD Standard Commands, Horizontal addressing mode.
  LCD_CMD_Add(0x08);  // LCD blank
  LCD_CMD_Add(0x0C);  // LCD in normal mode.

	LCD_Send(SEND_CMD);

	LCD_Contrast(0x45);

  // Clear and Update
  LCD_Clear();

	LCD_Str(0, "hello FREERTOS", 1);
	LCD_Str(1, "hello FREERTOS", 0);
	LCD_Str(2, "<-  :)        ", 0);
	LCD_Str(3, "hello FREERTOS", 1);
	LCD_Str(4, "hello FREERTOS", 0);
	LCD_Str(5, "<-  :)        ", 0);

	
	while (1) {
		LCD_CMD_Add(0x80);
		LCD_CMD_Add(0x40);
		LCD_Send(SEND_CMD);

		LCD_Send(SEND_FB);
		xSemaphoreTake(xLCDGo, portMAX_DELAY);
	}

}

/** Adds command to cmd_list
 */
void LCD_CMD_Add(unsigned char cmd) {
	xSemaphoreTake(xLCDMutex, portMAX_DELAY);

	LcdCmd[LcdCmdIdx] = cmd;
	LcdCmdIdx ++;
	
	xSemaphoreGive(xLCDMutex);
}


/****************************************************************************/
/*  Send to LCD                                                             */
/*  Function : LCDSend                                                      */
/*      Parameters                                                          */
/*          Input   :  data and  SEND_CHR or SEND_CMD                       */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCD_Send(uint8_t type) {

	xSemaphoreTake(xLCDMutex, portMAX_DELAY);

	SPIx_Lock(SPI1);
	LCD_E_WRITE(Bit_RESET);

	if (type == SEND_FB) {
		LCD_DC_WRITE(Bit_SET);
		SPIx_TranscieveBuffer(SPI1, &LcdSPIConf, LcdMemory, LCD_FB_SIZE, NULL, 0);
	} else if (LcdCmdIdx != 0) {
		LCD_DC_WRITE(Bit_RESET);
		SPIx_TranscieveBuffer(SPI1, &LcdSPIConf, LcdCmd, LcdCmdIdx, NULL, 0);
		LcdCmdIdx = 0;
	}

	LCD_E_WRITE(Bit_SET);
	SPIx_Unlock(SPI1);
	
	xSemaphoreGive(xLCDMutex);
}

/****************************************************************************/
/*  Update LCD memory                                                       */
/*  Function : LCDUpdate                                                    */
/*      Parameters                                                          */
/*          Input   :  Nothing                                              */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCD_Update ( void )
{
	xSemaphoreGive(xLCDGo);
}

/****************************************************************************/
/*  Clear LCD                                                               */
/*  Function : LCDClear                                                     */
/*      Parameters                                                          */
/*          Input   :  Nothing                                              */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCD_Clear(void) {

	xSemaphoreTake(xLCDMutex, portMAX_DELAY);

  // loop all cashe array
  for (int i=0; i<LCD_FB_SIZE; i++)
  {
     LcdMemory[i] = 0x0;
  }

	xSemaphoreGive(xLCDMutex);
}




/****************************************************************************/
/*  Change LCD Pixel mode                                                   */
/*  Function : LcdContrast                                                  */
/*      Parameters                                                          */
/*          Input   :  contrast                                             */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCD_Pixel (unsigned char x, unsigned char y, unsigned char mode )
{
    unsigned int    index   = 0;
    unsigned char   offset  = 0;
    unsigned char   data    = 0;

    // check for out off range
    if ( x > LCD_X_RES ) return;
    if ( y > LCD_Y_RES ) return;

    index = ((y / 8) * 84) + x;
    offset  = y - ((y / 8) * 8);


		xSemaphoreTake(xLCDMutex, portMAX_DELAY);
    
		data = LcdMemory[index];

    if ( mode == PIXEL_OFF ) {
        data &= (~(0x01 << offset));
    } else if ( mode == PIXEL_ON ) {
        data |= (0x01 << offset);
    } else if ( mode  == PIXEL_XOR ) {
        data ^= (0x01 << offset);
    }

    LcdMemory[index] = data;
		xSemaphoreGive(xLCDMutex);

}

/****************************************************************************/
/*  Write char at x position on y row                                       */
/*  Function : LCDChrXY                                                     */
/*      Parameters                                                          */
/*          Input   :  pos, row, char                                       */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCD_ChrXY (unsigned char x, unsigned char y, unsigned char ch )
{
    unsigned int    index   = 0;
    unsigned int    i       = 0;

    // check for out off range
    if ( x > LCD_X_RES ) return;
    if ( y > LCD_Y_RES ) return;

    index = (x*48 + y*48*14)/8 ;

		xSemaphoreTake(xLCDMutex, portMAX_DELAY);

    for ( i = 0; i < 5; i++ ) {
      LcdMemory[index] = FontLookup[ch - 32][i] << 1;
      index++;
    }
		xSemaphoreGive(xLCDMutex);

}


/****************************************************************************/
/*  Write char at x position on y row - inverse                             */
/*  Function : LCDChrXY                                                     */
/*      Parameters                                                          */
/*          Input   :  pos, row, char                                       */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCD_ChrXYInverse (unsigned char x, unsigned char y, unsigned char ch )
{
    unsigned int    index   = 0;
    unsigned int    i       = 0;

    // check for out off range
    if ( x > LCD_X_RES ) return;
    if ( y > LCD_Y_RES ) return;

    index = (x*48 + y*48*14)/8 ;

		xSemaphoreTake(xLCDMutex, portMAX_DELAY);

    for ( i = 0; i < 5; i++ ) {
      LcdMemory[index] = ~(FontLookup[ch - 32][i]);
      index++;

      if(i==4)
        LcdMemory[index] = 0xFF;
    }
		xSemaphoreGive(xLCDMutex);

}


/****************************************************************************/
/*  Set LCD Contrast                                                        */
/*  Function : LcdContrast                                                  */
/*      Parameters                                                          */
/*          Input   :  contrast                                             */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCD_Contrast(unsigned char contrast) {

    //  LCD Extended Commands.
    LCD_CMD_Add(0x21);

    // Set LCD Vop (Contrast).
    LCD_CMD_Add(0x80 | contrast);

    //  LCD Standard Commands, horizontal addressing mode.
    LCD_CMD_Add(0x20);
		LCD_Send(SEND_CMD);
}


/****************************************************************************/
/*  Send string to LCD                                                      */
/*  Function : LCDStr                                                       */
/*      Parameters                                                          */
/*          Input   :  row, text, inversion                                 */
/*          Output  :  Nothing                                              */
/****************************************************************************/
void LCD_Str(unsigned char row, const char *dataPtr, unsigned char inv ) {

  // variable for X coordinate
  unsigned char x = 0;

  // loop to the and of string
  while ( *dataPtr ) {

    if(inv) {
      LCD_ChrXYInverse(x, row, (*dataPtr));
    } else {
      LCD_ChrXY( x, row, (*dataPtr));
    }
    x++;
    dataPtr++;
  }
}



