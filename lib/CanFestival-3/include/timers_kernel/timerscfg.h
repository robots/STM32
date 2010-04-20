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

#include <linux/param.h>
#include <linux/kthread.h>

/* Time unit : clock ticks (jiffies) */
/* Time resolution : ticks per second (architecture-dependent value 'HZ' defined in linux/param.h) */

#define TIMEVAL unsigned long
#define TIMEVAL_MAX (~(TIMEVAL)0) >> 1
#define MS_TO_TIMEVAL(ms) ( (ms * HZ) / 1000 )
#define US_TO_TIMEVAL(us) ( (us * HZ) / 1000000)

#define TASK_HANDLE struct task_struct*

#endif
