/*
Copyright (C): Giuseppe Massimo BERTANI
gmbertani@users.sourceforge.net


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

/**
 * @file slave.h
 * @author GMB
 * @date 17/1/08
 *
 * This file is part of SillySlave demo for CANfestival
 * open source CANopen stack.
 */ 



#include "data.h"

INTEGER8 InitCANdevice( UNS8 bus, UNS32 baudrate, UNS8 node );

void SillySlave_heartbeatError(CO_Data* d, UNS8);

UNS8 SillySlave_canSend(Message *);

void SillySlave_initialisation(CO_Data* d);
void SillySlave_preOperational(CO_Data* d);
void SillySlave_operational(CO_Data* d);
void SillySlave_stopped(CO_Data* d);

void SillySlave_post_sync(CO_Data* d);
void SillySlave_post_TPDO(CO_Data* d);
void SillySlave_storeODSubIndex(CO_Data* d, UNS16 wIndex, UNS8 bSubindex);
void SillySlave_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg);

