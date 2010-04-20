/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
AT91 Port: Peter CHRISTEN

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

#include "can_AT91.h"
#include "canfestival.h"

void can_irq_handler(void);

unsigned char canInit(unsigned int bitrate)
/******************************************************************************
Initialize the hardware to receive CAN messages and start the timer for the
CANopen stack.
INPUT	
OUTPUT	
******************************************************************************/
{
  unsigned char i;
  AT91S_CAN_MB *mb_ptr = AT91C_BASE_CAN_MB0;

  // Enable CAN PIOs
  AT91F_CAN_CfgPIO();
  // Enable CAN Clock
  AT91F_CAN_CfgPMC();

  // Enable CAN Transceiver
  AT91F_PIOA_CfgPMC();

  // Init CAN Interrupt Source Level
  AT91F_AIC_ConfigureIt(AT91C_BASE_AIC,				// CAN base address
                        AT91C_ID_CAN,				// CAN ID
                        AT91C_AIC_PRIOR_HIGHEST,		// Max priority
                        AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL,	// Level sensitive
                        can_irq_handler);			// C Handler

  AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_CAN);

  if (bitrate <= 500)
  {
    // CANopen 10..500 kbit with 16 tq, sample point is at 14 tq
    // all values are added to 1 by hardware
    // Resynchronisation jump width (SJW)	= 1 tq
    // Propagation Time Segment (PRS)		= 5 tq
    // Phase Segment 1 (PHS1)			= 8 tq
    // Phase Segment 2 (PHS2)			= 2 tq
    // Total					= 16 tq
    AT91F_CAN_CfgBaudrateReg(AT91C_BASE_CAN,
                             (AT91C_MASTER_CLOCK/16/1000/bitrate - 1) << 16 | 0x0471);
  }
  else
    return 0;

  // Enable CAN and Wait for WakeUp Interrupt
//  AT91F_CAN_EnableIt(AT91C_BASE_CAN, AT91C_CAN_WAKEUP);
  AT91F_CAN_CfgModeReg(AT91C_BASE_CAN, AT91C_CAN_CANEN);

  // Reset all mailsboxes (MBs), filters are zero (accept all) by clear all MB
  // Set the lower MBs as rx buffer
  for (i = 0; i < NB_RX_MB; i++, mb_ptr++)
  // Configure receive MBs as receive buffer, last as receive overwrite
    AT91F_InitMailboxRegisters(mb_ptr,
				((i < (NB_RX_MB - 1)) ? AT91C_CAN_MOT_RX : AT91C_CAN_MOT_RXOVERWRITE)
				| AT91C_CAN_PRIOR,	// Mailbox Mode Reg
				0x00000000,		// Mailbox Acceptance Mask Reg
				0x00000000,		// Mailbox ID Reg
				0x00000000,		// Mailbox Data Low Reg
				0x00000000,		// Mailbox Data High Reg
				0x00000000);		// Mailbox Control Reg
  for (   ; i < NB_MB; i++, mb_ptr++)
  // Configure transmit MBs
    AT91F_InitMailboxRegisters(mb_ptr,
				AT91C_CAN_MOT_TX
				| AT91C_CAN_PRIOR,	// Mailbox Mode Reg
				0x00000000,		// Mailbox Acceptance Mask Reg
				0x00000000,		// Mailbox ID Reg
				0x00000000,		// Mailbox Data Low Reg
				0x00000000,		// Mailbox Data High Reg
				0x00000000);		// Mailbox Control Reg
  // Enable Reception on all receive Mailboxes
  AT91F_CAN_InitTransferRequest(AT91C_BASE_CAN, RX_INT_MSK);
  // Enable all receive interrupts
  AT91F_CAN_EnableIt(AT91C_BASE_CAN, RX_INT_MSK);
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
  unsigned int mask;
  AT91S_CAN_MB *mb_ptr = AT91C_BASE_CAN_MB0 + START_TX_MB;

  if ((AT91F_CAN_GetStatus(AT91C_BASE_CAN) & TX_INT_MSK) == 0)
    return 0;			// No free MB for sending

  for (mask = 1 << START_TX_MB;
       (mask & TX_INT_MSK) && !(AT91F_CAN_GetStatus(AT91C_BASE_CAN) & mask);
	mask <<= 1, mb_ptr++)	// Search the first free MB
  {
  }
  AT91F_CAN_CfgMessageIDReg(mb_ptr, m->cob_id, 0);	// Set cob id
  // Mailbox Control Register, set remote transmission request and data lenght code
  AT91F_CAN_CfgMessageCtrlReg(mb_ptr, m->rtr ? AT91C_CAN_MRTR : 0 | (m->len << 16));	
  AT91F_CAN_CfgMessageDataLow(mb_ptr, *(UNS32*)(&m->data[0]));// Mailbox Data Low Reg
  AT91F_CAN_CfgMessageDataHigh(mb_ptr, *(UNS32*)(&m->data[4]));// Mailbox Data High Reg
  // Start sending by writing the MB configuration register to transmit
  AT91F_CAN_InitTransferRequest(AT91C_BASE_CAN, mask);
  return 1;	// successful
}

unsigned char canReceive(Message *m)
/******************************************************************************
The driver passes a received CAN message to the stack
INPUT	Message *m pointer to received CAN message
OUTPUT	1 if a message received
******************************************************************************/
{
  unsigned int mask;
  AT91S_CAN_MB *mb_ptr = AT91C_BASE_CAN_MB0;

  if ((AT91F_CAN_GetStatus(AT91C_BASE_CAN) & RX_INT_MSK) == 0)
    return 0;		// Nothing received

  for (mask = 1;
       (mask & RX_INT_MSK) && !(AT91F_CAN_GetStatus(AT91C_BASE_CAN) & mask);
	mask <<= 1, mb_ptr++)	// Search the first MB received
  {
  }
  m->cob_id = AT91F_CAN_GetFamilyID(mb_ptr);
  m->len = (AT91F_CAN_GetMessageStatus(mb_ptr) & AT91C_CAN_MDLC) >> 16;
  m->rtr = (AT91F_CAN_GetMessageStatus(mb_ptr) & AT91C_CAN_MRTR) ? 1 : 0;
  *(UNS32*)(&m->data[0]) = AT91F_CAN_GetMessageDataLow(mb_ptr);
  *(UNS32*)(&m->data[4]) = AT91F_CAN_GetMessageDataHigh(mb_ptr);
  // Enable Reception on Mailbox
  AT91F_CAN_CfgMessageModeReg(mb_ptr, AT91C_CAN_MOT_RX | AT91C_CAN_PRIOR);
  AT91F_CAN_InitTransferRequest(AT91C_BASE_CAN, mask);
  return 1;		// message received
}

/******************************************************************************

 ******************************* CAN INTERRUPT  *******************************/

void can_irq_handler(void)
/******************************************************************************
CAN Interrupt
******************************************************************************/
{
  volatile unsigned int status;
  static Message m = Message_Initializer;		// contain a CAN message
		
  status = AT91F_CAN_GetStatus(AT91C_BASE_CAN) & AT91F_CAN_GetInterruptMaskStatus(AT91C_BASE_CAN);

  if(status & RX_INT_MSK)
  {	// Rx Interrupt
    if (canReceive(&m))			// a message received
      canDispatch(&ObjDict_Data, &m);         // process it
  }
}
