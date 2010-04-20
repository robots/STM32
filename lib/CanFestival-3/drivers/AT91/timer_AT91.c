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

// Includes for the Canfestival driver
#include "canfestival.h"
#include "timer.h"

// Define the timer registers
#define AT91C_BASE_TC	AT91C_BASE_TC2
#define AT91C_ID_TC	AT91C_ID_TC2
#define TimerAlarm      AT91C_BASE_TC2->TC_RC
#define TimerCounter    AT91C_BASE_TC2->TC_CV

#define TIMER_INTERRUPT_LEVEL		1

void timer_can_irq_handler(void);


/************************** Modul variables **********************************/
// Store the last timer value to calculate the elapsed time
static TIMEVAL last_time_set = TIMEVAL_MAX;

void initTimer(void)
/******************************************************************************
Initializes the timer, turn on the interrupt and put the interrupt time to zero
INPUT	void
OUTPUT	void
******************************************************************************/
{
  unsigned int dummy;

  // First, enable the clock of the TIMER
  AT91F_PMC_EnablePeriphClock (AT91C_BASE_PMC, 1 << AT91C_ID_TC);
  // Disable the clock and the interrupts
  AT91C_BASE_TC->TC_CCR = AT91C_TC_CLKDIS ;
  AT91C_BASE_TC->TC_IDR = 0xFFFFFFFF ;
  // Clear status bit
  dummy = AT91C_BASE_TC->TC_SR;
  // Suppress warning variable "dummy" was set but never used
  dummy = dummy;

  // Set the Mode of the Timer Counter (MCK / 128)
  AT91C_BASE_TC->TC_CMR = AT91C_TC_CLKS_TIMER_DIV4_CLOCK;

  // Enable the clock
  AT91C_BASE_TC->TC_CCR = AT91C_TC_CLKEN ;

  // Open Timer interrupt
  AT91F_AIC_ConfigureIt (AT91C_BASE_AIC, AT91C_ID_TC, TIMER_INTERRUPT_LEVEL,
			 AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE, timer_can_irq_handler);

  AT91C_BASE_TC->TC_IER = AT91C_TC_CPCS;  //  IRQ enable CPC
  AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC2);

  // Start Timer
  AT91C_BASE_TC->TC_CCR = AT91C_TC_SWTRG ;
}

void setTimer(TIMEVAL value)
/******************************************************************************
Set the timer for the next alarm.
INPUT	value TIMEVAL (unsigned long)
OUTPUT	void
******************************************************************************/
{
  TimerAlarm += value;	// Add the desired time to timer interrupt time
}

TIMEVAL getElapsedTime(void)
/******************************************************************************
Return the elapsed time to tell the stack how much time is spent since last call.
INPUT	void
OUTPUT	value TIMEVAL (unsigned long) the elapsed time
******************************************************************************/
{
  unsigned int timer = TimerCounter;	// Copy the value of the running timer
  // Calculate the time difference
  return timer > last_time_set ? timer - last_time_set : last_time_set - timer;
}


//*----------------------------------------------------------------------------
//* Function Name       : timer_can_irq_handler
//* Object              : C handler interrupt function by the interrupts
//*                       assembling routine
//* Output Parameters   : calls TimeDispatch
//*----------------------------------------------------------------------------
void timer_can_irq_handler(void)
{
  AT91PS_TC TC_pt = AT91C_BASE_TC;
  unsigned int dummy;
  // AcknowAT91B_LEDge interrupt status
  dummy = TC_pt->TC_SR;
  // Suppress warning variable "dummy" was set but never used
  dummy = dummy;
  last_time_set = TimerCounter;
  TimeDispatch();	// Call the time handler of the stack to adapt the elapsed time
}

