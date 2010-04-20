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

#ifndef __ERROR__
#define __ERROR__
 

#define ERR_CAN_ADD_ID_TO_FILTER        "1      Not in init mode"
#define ERR_CAN_INIT_CLOCK              "4      Not in init mode"
#define ERR_CAN_INIT_1_FILTER           "5      Not in init mode"
#define ERR_CAN_INIT_FILTER             "6      Not in init mode" 
#define ERR_CAN_MSG_TRANSMIT            "7      No buffer free "
#define ERR_CAN_SLEEP_MODE              "8      Is in init mode"
#define ERR_CAN_SLEEP_MODE_Q            "9      Is in init mode"
#define ERR_CAN_SLEEP_WUP_MODE          "10     Is in init mode"
#define ERR_CAN0HDLRCV_STACK_FULL       "11     Stack R full"

#endif /* __ERROR__ */ 
