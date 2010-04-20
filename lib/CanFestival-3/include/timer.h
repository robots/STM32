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

#ifndef __timer_h__
#define __timer_h__

#include <timerscfg.h>
#include <applicfg.h>

#define TIMER_HANDLE INTEGER16

#include "data.h"

/* --------- types and constants definitions --------- */
#define TIMER_FREE 0
#define TIMER_ARMED 1
#define TIMER_TRIG 2
#define TIMER_TRIG_PERIOD 3

#define TIMER_NONE -1

typedef void (*TimerCallback_t)(CO_Data* d, UNS32 id);

struct struct_s_timer_entry {
	UNS8 state;
	CO_Data* d;
	TimerCallback_t callback; /* The callback func. */
	UNS32 id; /* The callback func. */
	TIMEVAL val;
	TIMEVAL interval; /* Periodicity */
};

typedef struct struct_s_timer_entry s_timer_entry;

/* ---------  prototypes --------- */
/*#define SetAlarm(d, id, callback, value, period) printf("%s, %d, SetAlarm(%s, %s, %s, %s, %s)\n",__FILE__, __LINE__, #d, #id, #callback, #value, #period); _SetAlarm(d, id, callback, value, period)*/
/**
 * @ingroup timer
 * @brief Set an alarm to execute a callback function when expired.
 * @param *d Pointer to a CAN object data structure
 * @param id The alarm Id
 * @param callback A callback function
 * @param value Call the callback function at current time + value
 * @param period Call periodically the callback function
 * @return handle The timer handle
 */
TIMER_HANDLE SetAlarm(CO_Data* d, UNS32 id, TimerCallback_t callback, TIMEVAL value, TIMEVAL period);

/**
 * @ingroup timer
 * @brief Delete an alarm before expiring.
 * @param handle A timer handle
 * @return The timer handle
 */
TIMER_HANDLE DelAlarm(TIMER_HANDLE handle);

void TimeDispatch(void);

/**
 * @ingroup timer
 * @brief Set a timerfor a given time.
 * @param value The time value.
 */
void setTimer(TIMEVAL value);

/**
 * @ingroup timer
 * @brief Get the time elapsed since latest timer occurence.
 * @return time elapsed since latest timer occurence
 */
TIMEVAL getElapsedTime(void);

#endif /* #define __timer_h__ */
