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

/** @defgroup timer Timer Management
 *  @ingroup userapi
 */

#ifndef __timer_driver_h__
#define __timer_driver_h__

#include "timerscfg.h"
#include "timer.h"

// For use from CAN driver


/**
 * @ingroup timer
 * @brief Acquire mutex
 */
void EnterMutex(void);

/**
 * @ingroup timer
 * @brief Release mutex
 */
void LeaveMutex(void);

void WaitReceiveTaskEnd(TASK_HANDLE*);

/**
 * @ingroup timer
 * @brief Initialize Timer
 */
void TimerInit(void);

/**
 * @ingroup timer
 * @brief Cleanup Timer  
 */
void TimerCleanup(void);

/**
 * @ingroup timer
 * @brief Start the timer task
 * @param Callback A callback function
 */
void StartTimerLoop(TimerCallback_t Callback);

/**
 * @ingroup timer
 * @brief Stop the timer task
 * @param Callback A callback function
 */
void StopTimerLoop(TimerCallback_t Callback);

/**
 * @brief Stop the timer task
 * @param port CanFestival file descriptor
 * @param *handle handle of receive loop thread
 * @param *ReceiveLoopPtr Pointer on the receive loop function
 */
void CreateReceiveTask(CAN_PORT port, TASK_HANDLE* handle, void* ReceiveLoopPtr);

#endif
