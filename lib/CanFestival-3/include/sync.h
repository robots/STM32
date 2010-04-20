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

/** @defgroup synco Synchronisation Object
 *  SYNC object is a CANopen message forcing the receiving nodes to sample the inputs mapped into synchronous TPDOS.
 *  Receiving this message cause the node to set the outputs to values received in the previous synchronous RPDO.
 *  @ingroup comobj
 */
					 
#ifndef __SYNC_h__
#define __SYNC_h__

void startSYNC(CO_Data* d);

void stopSYNC(CO_Data* d);

typedef void (*post_sync_t)(CO_Data*);
void _post_sync(CO_Data* d);

typedef void (*post_TPDO_t)(CO_Data*);
void _post_TPDO(CO_Data* d);

/** 
 * @brief Transmit a SYNC message and trigger sync TPDOs
 * @param *d Pointer on a CAN object data structure
 * @return
 */
UNS8 sendSYNC (CO_Data* d);

/** 
 * @brief Transmit a SYNC message on CAN bus
 * @param *d Pointer on a CAN object data structure
 * @return
 */
UNS8 sendSYNCMessage(CO_Data* d);

/** 
 * @brief This function is called when the node is receiving a SYNC message (cob-id = 0x80).
 *  - Check if the node is in OERATIONAL mode. (other mode : return 0 but does nothing).
 *  - Get the SYNC cobId by reading the dictionary index 1005, check it does correspond to the received cobId
 *  - Trigger sync TPDO emission 
 * @param *d Pointer on a CAN object data structure
 * @return 0 if OK, 0xFF if error 
 */
UNS8 proceedSYNC (CO_Data* d);

#endif
