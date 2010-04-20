/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

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

/** @defgroup nmtslave NMT Slave
 *  @brief The NMT Slave methods are called automatically when a NMT message from Master are received. 
 *  @ingroup networkmanagement
 */
 
#ifndef __nmtSlave_h__
#define __nmtSlave_h__

#include <applicfg.h>
#include "data.h"

/** 
 * @brief Threat the reception of a NMT message from the master.
 * @param *d Pointer to the CAN data structure
 * @param *m Pointer to the message received
 * @return 
 *  -  0 if OK 
 *  - -1 if the slave is not allowed, by its state, to receive the message
 */
void proceedNMTstateChange (CO_Data* d, Message * m);

/** 
 * @brief Transmit the boot-Up frame when the slave is moving from initialization
 * state to pre_operational state.
 * @param *d Pointer on the CAN data structure
 * @return canSend(bus_id,&m)
 */
UNS8 slaveSendBootUp (CO_Data* d);


#endif /* __nmtSlave_h__ */
