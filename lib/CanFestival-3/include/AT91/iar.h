/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
Copyright (C): AVR Port Andreas GLAUSER and Peter CHRISTEN

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

#ifndef _IAR_H_
#define _IAR_H_

#ifdef  __IAR_SYSTEMS_ICC__	// IAR Compiler

/*
#define ISR(vect)	\
  	_Pragma("vector="vect) \
	__interrupt void Interrupt_##vect (void)
*/
#define sei()		__enable_interrupt()
#define cli()		__disable_interrupt()
#define sleep_enable()	SMCR = 1 << SE
#define sleep_cpu()	__sleep()
#define wdt_reset()	__watchdog_reset()
#define wdt_enable(val)	{WDTCSR = 1 << WDCE  | 1 << WDE;	\
			WDTCSR = 1 << WDE | (val);}

#else
#error Not an IAR Compiler!
#endif				// IAR Compiler

#endif // _IAR_H_
