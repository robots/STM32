/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
ARM Port: Peter CHRISTEN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Include Standard LIB  files
#include "AT91SAM7X-EK.h"
#include <string.h>
#include "config.h"
#include "io_macro.h"
/************************* CANopen includes **********************************/
#include "canfestival.h"
#include "objdict.h"
#include "can_AT91.h"

//   Waiting time between AT91B_LED1 and AT91B_LED2
#define     WAIT_TIME       AT91B_MCK

#define PIO_INTERRUPT_LEVEL     6
#define SOFT_INTERRUPT_LEVEL	2
#define FIQ_INTERRUPT_LEVEL     7  // Always high

// External Function Prototype
extern void timer_init (unsigned int time);
void sys_init();

// Global variable
extern int timer0_interrupt;

unsigned int leds = 0;
unsigned int keys,keys_old,keys_edge;

unsigned char string_down[] = "1234567890x";
unsigned char string_up[20];
unsigned char val_down = 1;
unsigned char val_up;
unsigned char val_pdo = 1;

// nodes
#define MEMORY_MODULE		0
#define OUTPUT_MODULE		1
#define IO_MODULE		2

// node ids
#define ID_MEMORY_MODULE	0x20
#define ID_OUTPUT_MODULE	0x21
#define ID_IO_MODULE		0x22

//----------------------------------------------------------------------------
// Function Name       : main
// Object              : Main interrupt function
// Input Parameters    : none
// Output Parameters   : TRUE
//----------------------------------------------------------------------------
int main(void)
// Begin
{
  sys_init();                                   // Initialize system
  timer_init(10);
  canInit(CAN_BAUDRATE);         		// Initialize the CANopen bus
  initTimer();                                 	// Start timer for the CANopen stack
  __enable_interrupt();
  setState(&ObjDict_Data, Initialisation);	// Init the state
  setNodeId (&ObjDict_Data, 0x7F);
  setState(&ObjDict_Data, Operational);		// Put the master in operational mode
	

  for (;;)
  {
    if (timer0_interrupt)
    {
      timer0_interrupt = 0;
      AT91F_PIO_SetOutput(AT91C_BASE_PIOB, 0x01);

      keys = ~AT91F_PIO_GetInput(AT91D_BASE_PIO_SW)>>21 & 0x1F;

      keys_edge = keys & ~keys_old;	// edge detection
      keys_old = keys;

//      static Message m = Message_Initializer;		// contain a CAN message

//      if (canReceive(&m))			// a message received
//        canDispatch(&ObjDict_Data, &m);         // process it

      if (checkbit(keys_edge,0))	// edge on key 0
      {
	masterSendNMTstateChange (&ObjDict_Data, 0x00, NMT_Start_Node);
//	startSYNC(&ObjDict_Data);
      }

      if (checkbit(keys_edge,1))	// edge on key 1
      {
	masterSendNMTstateChange (&ObjDict_Data, 0x00, NMT_Reset_Node);
//	stopSYNC(&ObjDict_Data);
      }

      if (checkbit(keys,2))	// edge on key 2
      {
	if (!(DO1 <<= 1))					// generate running light
	  DO1 = 1;
	sendPDOevent (&ObjDict_Data);
      }

      if (checkbit(keys_edge,3))	// edge on key 3
      {
      }


      leds = DI1;

      AT91F_PIO_ClearOutput(AT91C_BASE_PIOB, (leds << 19));
      AT91F_PIO_SetOutput(AT91C_BASE_PIOB, ~(leds << 19) & AT91B_LED_MASK);

      AT91F_PIO_ClearOutput(AT91C_BASE_PIOB, 0x01);
    }
  }
}

void sys_init()
/******************************************************************************
Initialize the relays, the main states and the modbus protocol stack.
INPUT	LOCK_STATES *lock_states
OUTPUT	void
******************************************************************************/
{
  // Enable User Reset and set its minimal assertion to 960 us
  AT91C_BASE_RSTC->RSTC_RMR = AT91C_RSTC_URSTEN | (0x4<<8) | (unsigned int)(0xA5<<24);

  // First, enable the clock of the PIOs
  AT91F_PMC_EnablePeriphClock (AT91C_BASE_PMC, 1 << AT91C_ID_PIOA) ;
  AT91F_PMC_EnablePeriphClock (AT91C_BASE_PMC, 1 << AT91C_ID_PIOB) ;

  // then, we configure the PIO Lines corresponding to switches
  // to be inputs.
  AT91F_PIO_CfgInput(AT91C_BASE_PIOA, AT91B_SW_MASK) ;
  // then, we configure the PIO Lines corresponding to AT91B_LEDx
  // to be outputs. No need to set these pins to be driven by the PIO because it is GPIO pins only.
  AT91F_PIO_CfgOutput(AT91C_BASE_PIOB, AT91B_LED_MASK | 0xFF) ;
  // Clear the AT91B_LED's. On the EK we must apply a "1" to turn off AT91B_LEDs
  AT91F_PIO_SetOutput(AT91C_BASE_PIOB, AT91B_LED_MASK) ;

  AT91F_PIO_CfgOutput(AT91C_BASE_PIOA,AT91B_CAN_TRANSCEIVER_RS) ;
  // Clear PA2 <=> Enable Transceiver Normal Mode (versus Standby mode)
  AT91F_PIO_ClearOutput(AT91C_BASE_PIOA,AT91B_CAN_TRANSCEIVER_RS) ;

}
