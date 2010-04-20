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

#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/errno.h>

#include "timer.h"
#include "applicfg.h"

static spinlock_t lock = SPIN_LOCK_UNLOCKED;

static struct timer_list timer;

static TIMEVAL last_time_read,
	last_occured_alarm,
	last_alarm_set;

void TimerInit(void)
{
	/* only used in realtime apps */
}

void TimerCleanup(void)
{
	/* only used in realtime apps */
}

void EnterMutex(void)
{
	spin_lock (&lock);
}

void LeaveMutex(void)
{
	spin_unlock (&lock);
}

void timer_notify(unsigned long data)
{
	last_occured_alarm = last_alarm_set;

	EnterMutex();
	TimeDispatch();
	LeaveMutex();
}

void StartTimerLoop(TimerCallback_t init_callback)
{
	getElapsedTime();
	last_alarm_set = last_time_read;
	last_occured_alarm = last_alarm_set;

	init_timer(&timer);
	timer.function = timer_notify;

	EnterMutex();
	// At first, TimeDispatch will call init_callback.
	SetAlarm(NULL, 0, init_callback, 0, 0);
	LeaveMutex();
}

void StopTimerLoop(TimerCallback_t exitfunction)
{
	EnterMutex();
	del_timer (&timer);
	exitfunction(NULL,0);
	LeaveMutex();
}

void setTimer(TIMEVAL value)
{
	if (value == TIMEVAL_MAX)
		return;

	last_alarm_set = last_time_read + value;
	mod_timer (&timer, last_alarm_set);
}

TIMEVAL getElapsedTime(void)
{
	last_time_read = jiffies;

	return (long)last_time_read - (long)last_occured_alarm;
}

void CreateReceiveTask(CAN_PORT port, TASK_HANDLE *Thread, void* ReceiveLoopPtr)
{
	*Thread = kthread_run(ReceiveLoopPtr, port, "canReceiveLoop");
}

void WaitReceiveTaskEnd(TASK_HANDLE *Thread)
{
	force_sig (SIGTERM, *Thread);
}
