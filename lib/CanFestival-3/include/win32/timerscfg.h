/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
Win32 port by Leonid Tochinski

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

#include <windows.h>

// Time unit : 100 ns
#define TIMEVAL unsigned long long
#define TIMEVAL_MAX ~(TIMEVAL)0

#define MS_TO_TIMEVAL(ms) ms*10000
#define US_TO_TIMEVAL(us) us*10

#define TASK_HANDLE HANDLE

#endif
