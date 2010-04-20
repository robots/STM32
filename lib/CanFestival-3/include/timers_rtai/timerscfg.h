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

#ifndef __TIMERSCFG_H__
#define __TIMERSCFG_H__

#include <pthread.h>
#include <rtai_lxrt.h>

// Time unit : RTAI's timers count, 64bit signed 
#define TIMEVAL RTIME
#define TIMEVAL_MAX ((long long)(~0ULL>>1))

#define MS_TO_TIMEVAL(ms) nano2count((RTIME)ms*1000000)
#define US_TO_TIMEVAL(us) nano2count((RTIME)us*1000)

#define TASK_HANDLE pthread_t

#endif
