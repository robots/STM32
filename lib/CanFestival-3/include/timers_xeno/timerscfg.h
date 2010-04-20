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

#include <native/task.h>
#include <native/timer.h>
#include <native/mutex.h>
#include <native/cond.h>
#include <native/sem.h>
#include <native/alarm.h>

// Time unit : ns
// Time resolution : 64bit (~584 years)
#define TIMEVAL RTIME
#define TIMEVAL_MAX ~(RTIME)0
#define MS_TO_TIMEVAL(ms)  rt_timer_ns2ticks((RTIME)ms*1000000)
#define US_TO_TIMEVAL(us)  rt_timer_ns2ticks((RTIME)us*1000)

#define TASK_HANDLE RT_TASK

#endif
