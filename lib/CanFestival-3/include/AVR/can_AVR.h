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

#ifndef __CAN_AVR__
#define __CAN_AVR__

// AVR implementation of the CANopen driver includes
// Hardware related macros and ATMEL lib can_drv
#include "config.h"
#include "can_drv.h"

// Canfestivals includes
#include "can.h"

// Number of receive MOb
#define NB_RX_MOB                       13	// minimal 8
// Number of transmit MOb
#define NB_TX_MOB                       (NB_MOB - NB_RX_MOB)

#if (NB_TX_MOB < 1)
#error define less RX Mobs, you must have at least 1 TX MOb!
#elif (NB_RX_MOB < 8)
#error define at least 8 RX MObs!
#endif

#define START_TX_MOB                    NB_RX_MOB
#define TX_INT_MSK			((0x7F << (7 - NB_TX_MOB)) & 0x7F)

/************************* To be called by user app ***************************/

unsigned char canInit(unsigned int bitrate);
unsigned char canSend(CAN_PORT notused, Message *m);
unsigned char canReceive(Message *m);
unsigned char canChangeBaudRate_driver( CAN_HANDLE fd, char* baud);
#endif
