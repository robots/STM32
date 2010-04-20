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

/** @defgroup statemachine State Machine
 *  @ingroup userapi
 */
 
#ifndef __states_h__
#define __states_h__

#include <applicfg.h>

/* The nodes states 
 * -----------------
 * values are choosen so, that they can be sent directly
 * for heartbeat messages...
 * Must be coded on 7 bits only
 * */
/* Should not be modified */
enum enum_nodeState {
  Initialisation  = 0x00, 
  Disconnected    = 0x01,
  Connecting      = 0x02,
  Preparing       = 0x02,
  Stopped         = 0x04,
  Operational     = 0x05,
  Pre_operational = 0x7F,
  Unknown_state   = 0x0F
};

typedef enum enum_nodeState e_nodeState;

typedef struct
{
	INTEGER8 csBoot_Up;
	INTEGER8 csSDO;
	INTEGER8 csEmergency;
	INTEGER8 csSYNC;
	INTEGER8 csHeartbeat;
	INTEGER8 csPDO;
	INTEGER8 csLSS;
} s_state_communication;

/** 
 * @brief Function that user app can overload
 * @ingroup statemachine
 */
typedef void (*initialisation_t)(CO_Data*);
typedef void (*preOperational_t)(CO_Data*);
typedef void (*operational_t)(CO_Data*);
typedef void (*stopped_t)(CO_Data*);

/** 
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _initialisation(CO_Data* d);

/** 
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _preOperational(CO_Data* d);

/**
 * @ingroup statemachine 
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _operational(CO_Data* d);

/** 
 * @ingroup statemachine
 * @brief Function that user app can overload
 * @param *d Pointer on a CAN object data structure
 */
void _stopped(CO_Data* d);

#include "data.h"

/************************* prototypes ******************************/

/** 
 * @brief Called by driver/app when receiving messages
 * @param *d Pointer on a CAN object data structure
 * @param *m Pointer on a CAN message structure
 */
void canDispatch(CO_Data* d, Message *m);

/** 
 * @ingroup statemachine
 * @brief Returns the state of the node
 * @param *d Pointer on a CAN object data structure
 * @return The node state
 */
e_nodeState getState (CO_Data* d);

/** 
 * @ingroup statemachine
 * @brief Change the state of the node 
 * @param *d Pointer on a CAN object data structure
 * @param newState The state to assign
 * @return 
 */
UNS8 setState (CO_Data* d, e_nodeState newState);

/**
 * @ingroup statemachine 
 * @brief Returns the nodId 
 * @param *d Pointer on a CAN object data structure
 * @return
 */
UNS8 getNodeId (CO_Data* d);

/** 
 * @ingroup statemachine
 * @brief Define the node ID. Initialize the object dictionary
 * @param *d Pointer on a CAN object data structure
 * @param nodeId The node ID to assign
 */
void setNodeId (CO_Data* d, UNS8 nodeId);

/** 
 * @brief Some stuff to do when the node enter in pre-operational mode
 * @param *d Pointer on a CAN object data structure
 */
void initPreOperationalMode (CO_Data* d);

#endif
