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

#ifndef __CAN_AT91__
#define __CAN_AT91__

#include "config.h"

// Canfestivals includes
#include "can.h"

// Number of receive MB
#define NB_MB				8
#define NB_RX_MB			4
// Number of transmit MB
#define NB_TX_MB			(NB_MB - NB_RX_MB)

#if (NB_TX_MB < 1)
#error define less RX MBs, you must have at least 1 TX MB!
#elif (NB_RX_MB < 1)
#error define at least 1 RX MBs!
#endif

#define START_TX_MB			NB_RX_MB
#define TX_INT_MSK			((0xFF << (NB_MB - NB_TX_MB)) & 0xFF)
#define RX_INT_MSK			(0xFF >> (NB_MB - NB_RX_MB))

/************************* To be called by user app ***************************/

unsigned char canInit(unsigned int bitrate);
unsigned char canSend(CAN_PORT notused, Message *m);
unsigned char canReceive(Message *m);
#endif
