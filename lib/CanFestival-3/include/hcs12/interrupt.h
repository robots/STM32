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

#ifndef __INTERRUPT__
#define __INTERRUPT__

 
/**
Disable interrupts
*/
void lock (void);

/**
Enable interrupts
*/
void unlock (void);

/**
Timer overflow
*/
void __attribute__((interrupt)) timerOvflHdl (void);

/**
Message transmitted on MSCAN 0
*/
void __attribute__((interrupt)) can0HdlTra (void);

/**
Message received on MSCAN 0
*/
void __attribute__((interrupt)) can0HdlRcv (void);

/**
Message error on MSCAN 0

*/
void __attribute__((interrupt)) can0HdlWup (void);

/**
Message error on MSCAN 0
*/
void __attribute__((interrupt)) can0HdlErr (void);

/**
Message transmited on MSCAN 1
*/
void __attribute__((interrupt)) can1HdlTra (void);

/**
Message received on MSCAN 1
*/
void __attribute__((interrupt)) can1HdlRcv (void);

/**
Message error on MSCAN 1
*/
void __attribute__((interrupt)) can1HdlWup (void);

/**
Message error on MSCAN 1
*/
void __attribute__((interrupt)) can1HdlErr (void);

/**
Message transmited on MSCAN 2
*/
void __attribute__((interrupt)) can2HdlTra (void);

/**
Message received on MSCAN 2
*/
void __attribute__((interrupt)) can2HdlRcv (void);

/*
Message error on MSCAN 2
*/
void __attribute__((interrupt)) can2HdlWup (void);

/**
Message error on MSCAN 2
*/
void __attribute__((interrupt)) can2HdlErr (void);

/**
Message transmited on MSCAN 3
*/
void __attribute__((interrupt)) can3HdlTra (void);

/**
Message received on MSCAN 3
*/
void __attribute__((interrupt)) can3HdlRcv (void);

/**
Message error on MSCAN 3
*/
void __attribute__((interrupt)) can3HdlWup (void);

/**
Message error on MSCAN 3
*/
void __attribute__((interrupt)) can3HdlErr (void);

/**
Message error on MSCAN 4
*/
void __attribute__((interrupt)) can4HdlTra (void);

/**
Message received on MSCAN 4
*/
void __attribute__((interrupt)) can4HdlRcv (void);

/*
Message error on MSCAN 4
*/
void __attribute__((interrupt)) can4HdlWup (void);

/**
Message error on MSCAN 4
*/
void __attribute__((interrupt)) can4HdlErr (void);


#endif /* __INTERRUPT__  */
