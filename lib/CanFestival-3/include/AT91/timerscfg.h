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

#ifndef __TIMERSCFG_H__
#define __TIMERSCFG_H__

// Whatever your microcontroller, the timer wont work if
// TIMEVAL is not at least on 32 bits
#define TIMEVAL UNS32

// The timer of the AT91 counts from 0000 to 0xFFFF
#define TIMEVAL_MAX 0xFFFF

// The timer is incrementing every 2.66 us.
#define MS_TO_TIMEVAL(ms) ((ms) * 375)
#define US_TO_TIMEVAL(us) ((us) * 3 / 8)

#endif
