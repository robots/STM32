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


#ifndef __CANOPENDRIVER__
#define __CANOPENDRIVER__



// ---------  to be called by user app ---------

void initTimer(void);
void resetTimer(void);
void TimerLoop(void);

/** 
Returns 0 if no message received, 0xFF if the receiving stack is not empty.
May be call in polling.
You can also call canDispatch(...) in void __attribute__((interrupt)) can0HdlRcv (void)
(see include/hcs12/canOpenDriver.c)
 */
UNS8 f_can_receive(UNS8 notused, Message *m);
UNS8 canChangeBaudRate_driver( CAN_HANDLE fd, char* baud);

#endif
