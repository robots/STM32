/*
  This file is part of CanFestival, a library implementing CanOpen
  Stack.

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
  USA
*/
/*!
** @file   nmtMaster.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 08:47:18 2007
**
** @brief
**
**
*/
#include "nmtMaster.h"
#include "canfestival.h"
#include "sysdep.h"

/*!
**
**
** @param d
** @param Node_ID
** @param cs
**
** @return
**/
UNS8 masterSendNMTstateChange(CO_Data* d, UNS8 Node_ID, UNS8 cs)
{
  Message m;

  MSG_WAR(0x3501, "Send_NMT cs : ", cs);
  MSG_WAR(0x3502, "    to node : ", Node_ID);
  /* message configuration */
  m.cob_id = 0x0000; /*(NMT) << 7*/
  m.rtr = NOT_A_REQUEST;
  m.len = 2;
  m.data[0] = cs;
  m.data[1] = Node_ID;

  return canSend(d->canHandle,&m);
}


/*!
**
**
** @param d
** @param nodeId
**
** @return
**/
UNS8 masterSendNMTnodeguard(CO_Data* d, UNS8 nodeId)
{
  Message m;

  /* message configuration */
  UNS16 tmp = nodeId | (NODE_GUARD << 7); 
  m.cob_id = UNS16_LE(tmp);
  m.rtr = REQUEST;
  m.len = 0;

  MSG_WAR(0x3503, "Send_NODE_GUARD to node : ", nodeId);

  return canSend(d->canHandle,&m);
}

/*!
**
**
** @param d
** @param nodeId
**/
void masterRequestNodeState(CO_Data* d, UNS8 nodeId)
{
  /* FIXME: should warn for bad toggle bit. */

  /* NMTable configuration to indicate that the master is waiting
    for a Node_Guard frame from the slave whose node_id is ID
  */
  d->NMTable[nodeId] = Unknown_state; /* A state that does not exist
                                       */

  if (nodeId == 0) { /* NMT broadcast */
    UNS8 i = 0;
    for (i = 0 ; i < NMT_MAX_NODE_ID ; i++) {
      d->NMTable[i] = Unknown_state;
    }
  }
  masterSendNMTnodeguard(d,nodeId);
}

