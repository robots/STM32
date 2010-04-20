/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
AVR Port: Andreas GLAUSER and Peter CHRISTEN

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

//#define DEBUG_WAR_CONSOLE_ON
//#define DEBUG_ERR_CONSOLE_ON

#include "can_AVR.h"
#include "canfestival.h"

volatile unsigned char msg_received = 0;

unsigned char canInit(unsigned int bitrate)
/******************************************************************************
Initialize the hardware to receive CAN messages and start the timer for the
CANopen stack.
INPUT	bitrate		bitrate in kilobit
OUTPUT	1 if successful	
******************************************************************************/
{
  unsigned char i,k;
    //- Pull-up on TxCAN & RxCAN one by one to use bit-addressing
  CAN_PORT_DIR &= ~(1<<CAN_INPUT_PIN );
  CAN_PORT_DIR &= ~(1<<CAN_OUTPUT_PIN);
  CAN_PORT_OUT |=  (1<<CAN_INPUT_PIN );
  CAN_PORT_OUT |=  (1<<CAN_OUTPUT_PIN);

  Can_reset();				// Reset the CAN controller

  if (bitrate <= 500)
  {
    // CANopen 10..500 kbit with 16 tq, sample point is at 14 tq
    // all values are added to 1 by hardware
    // Resynchronisation jump width (SJW)	= 1 tq 
    // Propagation Time Segment (PRS)		= 5 tq
    // Phase Segment 1 (PHS1)			= 8 tq 
    // Phase Segment 2 (PHS2)			= 2 tq
    // Total					= 16 tq
    CANBT1 = ((F_CPU/16/1000/bitrate-1) << BRP);	// set bitrate
    CANBT2 = ((1-1) << SJW) |((5-1) << PRS);	// set SJW, PRS
    CANBT3 = (((2-1) << PHS2) | ((8-1) << PHS1) | (1<<SMP)); // set PHS1, PHS2, 3 sample points
  }
  else 
    return 0;

  // Reset all mailsboxes (MObs), filters are zero (accept all) by clear all MOb
  // Set the lower MObs as rx buffer
  for (i = 0; i < NB_MOB; i++)
  {
    Can_set_mob(i);		// Change to MOb with the received message
    Can_clear_mob();		// All MOb Registers=0
    for (k = 0; k < NB_DATA_MAX; k++)
      CANMSG = 0;		// MOb data FIFO
    if (i < NB_RX_MOB)		// Is receive MOb
      Can_config_rx_buffer();	// configure as receive buffer
  }
  // The tx MOb is still disabled, it will be set to tx mode when the first message will be sent
  // Enable the general CAN interrupts
  CANGIE = (1 << ENIT) | (1 << ENRX) | (1 << ENTX) | (0 << ENERR) | (0 << ENERG) | (0 << ENOVRT);
  CANIE1 = 0x7F;	// Enable the interrupts of all MObs (0..14)
  CANIE2 = 0xFF;   
  Can_enable();                                 // Enable the CAN bus controller
  return 1;
}

unsigned char canSend(CAN_PORT notused, Message *m)
/******************************************************************************
The driver send a CAN message passed from the CANopen stack
INPUT	CAN_PORT is not used (only 1 avaiable)
	Message *m pointer to message to send
OUTPUT	1 if  hardware -> CAN frame
******************************************************************************/
{
  unsigned char i;

  for (i = START_TX_MOB; i < NB_MOB; i++)	// Search the first free MOb
  {
    Can_set_mob(i);			// Change to MOb
    if ((CANCDMOB & CONMOB_MSK) == 0)	// MOb disabled = free
    {
      break;
    }
  }
  if (i < NB_MOB)			// free MOb found
  {
    Can_set_mob(i);			// Switch to the sending messagebox
    Can_set_std_id(m->cob_id);		// Set cob id
    if (m->rtr)				// Set remote transmission request
      Can_set_rtr();
    Can_set_dlc(m->len);		// Set data lenght code

    for (i= 0; i < (m->len); i++)	// Add data bytes to the MOb
      CANMSG = m->data[i];
  // Start sending by writing the MB configuration register to transmit
    Can_config_tx();		// Set the last MOb to transmit mode
    return 1;	// succesful
  }
  else
    return 0;	// not succesful
}

unsigned char canReceive(Message *m)
/******************************************************************************
The driver pass a received CAN message to the stack
INPUT	Message *m pointer to received CAN message
OUTPUT	1 if a message received
******************************************************************************/
{
  unsigned char i;

  if (msg_received == 0)
    return 0;		// Nothing received

  for (i = 0; i < NB_RX_MOB; i++)	// Search the first MOb received
  {
    Can_set_mob(i);			// Change to MOb
    if ((CANCDMOB & CONMOB_MSK) == 0)	// MOb disabled = received
    {
      msg_received--;
      break;
    }
  }
  if (i < NB_RX_MOB)			// message found
  {
    Can_get_std_id(m->cob_id);		// Get cob id
    m->rtr = Can_get_rtr();		// Get remote transmission request
    m->len = Can_get_dlc();		// Get data lenght code
    for (i= 0; i < (m->len); i++)	// get data bytes from the MOb
      m->data[i] = CANMSG;
    Can_config_rx_buffer();		// reset the MOb for receive
    return 1;                  		// message received
  }
  else					// no message found
  {
    msg_received = 0;			// reset counter
    return 0;                  		// no message received
  }
}

/***************************************************************************/
unsigned char canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
{

	return 0;
}

#ifdef  __IAR_SYSTEMS_ICC__
#pragma type_attribute = __interrupt
#pragma vector=CANIT_vect
void CANIT_interrupt(void)
#else	// GCC
ISR(CANIT_vect)
#endif	// GCC
/******************************************************************************
CAN Interrupt
******************************************************************************/
{
  unsigned char saved_page = CANPAGE;
  unsigned char i;

  if (CANGIT & (1 << CANIT))	// is a messagebox interrupt
  {
    if ((CANSIT1 & TX_INT_MSK) == 0)	// is a Rx interrupt
    {
      for (i = 0; (i < NB_RX_MOB) && (CANGIT & (1 << CANIT)); i++)	// Search the first MOb received
      {
        Can_set_mob(i);			// Change to MOb
        if (CANSTMOB & MOB_RX_COMPLETED)	// receive ok
        {
          Can_clear_status_mob();	// Clear status register
	  Can_mob_abort();		// disable the MOb = received
	  msg_received++;
        }
        else if (CANSTMOB & ~MOB_RX_COMPLETED)	// error
        {
          Can_clear_status_mob();	// Clear status register
	  Can_config_rx_buffer();	// reconfigure as receive buffer
        }
      }
    }
    else				// is a Tx interrupt	 
    {
      for (i = NB_RX_MOB; i < NB_MOB; i++)	// Search the first MOb transmitted
      {
        Can_set_mob(i);			// change to MOb
        if (CANSTMOB)			// transmission ok or error
        {
          Can_clear_status_mob();	// clear status register
	  CANCDMOB = 0;			// disable the MOb
	  break;
        }
      }
    }
  }

  CANPAGE = saved_page;

  // Bus Off Interrupt Flag
  if (CANGIT & (1 << BOFFIT))    // Finaly clear the interrupt status register
  {
    CANGIT |= (1 << BOFFIT);                    // Clear the interrupt flag
  }
  else
    CANGIT |= (1 << BXOK) | (1 << SERG) | (1 << CERG) | (1 << FERG) | (1 << AERG);// Finaly clear other interrupts
}

#ifdef  __IAR_SYSTEMS_ICC__
#pragma type_attribute = __interrupt
#pragma vector=OVRIT_vect
void OVRIT_interrupt(void)
#else	// GCC
ISR(OVRIT_vect)
#endif	// GCC
/******************************************************************************
CAN Timer Interrupt
******************************************************************************/
{
  CANGIT |= (1 << OVRTIM);
}

