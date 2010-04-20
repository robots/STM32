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
/* m68hc11/ports.h -- Definition of 68HC11 ports
   Copyright 1999, 2000 Free Software Foundation, Inc.
   Written by Stephane Carrez (stcarrez@worldnet.fr)

This file is part of GDB, GAS, and the GNU binutils.

GDB, GAS, and the GNU binutils are free software; you can redistribute
them and/or modify them under the terms of the GNU General Public
License as published by the Free Software Foundation; either version
1, or (at your option) any later version.

GDB, GAS, and the GNU binutils are distributed in the hope that they
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this file; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */


/* Revised francis.dupin@inrets.fr 2003-07 */

#ifndef _M68HC11_PORTS_H
#define _M68HC11_PORTS_H



#include <asm-m68hc12/ports_def.h>

/* Prototypes */
                 unsigned short get_timer_counter (void);
void             set_timer_counter (unsigned short value);
void             cop_reset (void);


/* Must me adapted to hc12
void             timer_acknowledge (void);
void             timer_initialize_rate (unsigned char divisor);
void             set_bus_expanded (void);
void             set_bus_single_chip (void);
unsigned short * get_input_capture_1 (void);
void             set_input_capture_1 (unsigned short value);
unsigned short * get_input_capture_2 (void);
void             set_input_capture_2 (unsigned short value);
unsigned short * get_input_capture_3 (void);
void             set_input_capture_3 (unsigned short value);
unsigned short * get_output_compare_1 (void);
void             set_output_compare_1 (unsigned short value);
unsigned short * get_output_compare_2 (void);
void             set_output_compare_2 (unsigned short value);
unsigned short * get_output_compare_3 (void);
void             set_output_compare_3 (unsigned short value);
unsigned short * get_output_compare_4 (void);
void             set_output_compare_4 (unsigned short value);
unsigned short * get_output_compare_5 (void);
void             set_output_compare_5 (unsigned short value);
*/


extern inline unsigned short
get_timer_counter (void)
{
  return ((unsigned volatile short*) &_io_ports[TCNTH])[0];
}

extern inline void
set_timer_counter (unsigned short value)
{
  ((unsigned volatile short*) &_io_ports[TCNTH])[0] = value;
}


/* Reset the COP.  */
extern inline void
cop_reset (void)
{
  _io_ports[ARMCOP] = 0x55;
  _io_ports[ARMCOP] = 0xAA;
}


#if 0
/* Acknowledge the timer interrupt.  */
extern inline void
timer_acknowledge (void)
{
  _io_ports[TFLG2] = 0x80;
}

/* Initialize the timer.  */
extern inline void
timer_initialize_rate (unsigned char divisor)
{
  _io_ports[M6811_TMSK2] = M6811_RTII | divisor;
}

extern inline void
cop_optional_reset (void)
{
#if defined(M6811_USE_COP) && M6811_USE_COP == 1
  cop_reset ();
#endif
}

/* Set the board in the expanded mode to get access to external bus.  */
extern inline void
set_bus_expanded (void)
{
  _io_ports[M6811_HPRIO] |= M6811_MDA;
}


/* Set the board in single chip mode.  */
extern inline void
set_bus_single_chip (void)
{
  _io_ports[M6811_HPRIO] &= ~M6811_MDA;
}

extern inline unsigned short
get_input_capture_1 (void)
{
  return ((unsigned volatile short*) &_io_ports[M6811_TIC1_H])[0];
}

extern inline void
set_input_capture_1 (unsigned short value)
{
  ((unsigned volatile short*) &_io_ports[M6811_TIC1_H])[0] = value;
}

extern inline unsigned short
get_input_capture_2 (void)
{
  return ((unsigned volatile short*) &_io_ports[M6811_TIC2_H])[0];
}

extern inline void
set_input_capture_2 (unsigned short value)
{
  ((unsigned volatile short*) &_io_ports[M6811_TIC2_H])[0] = value;
}

extern inline unsigned short
get_input_capture_3 (void)
{
  return ((unsigned volatile short*) &_io_ports[M6811_TIC3_H])[0];
}

extern inline void
set_input_capture_3 (unsigned short value)
{
  ((unsigned volatile short*) &_io_ports[M6811_TIC3_H])[0] = value;
}

/* Get output compare 16-bit register.  */
extern inline unsigned short
get_output_compare_1 (void)
{
  return ((unsigned volatile short*) &_io_ports[M6811_TOC1_H])[0];
}

extern inline void
set_output_compare_1 (unsigned short value)
{
  ((unsigned volatile short*) &_io_ports[M6811_TOC1_H])[0] = value;
}

extern inline unsigned short
get_output_compare_2 (void)
{
  return ((unsigned volatile short*) &_io_ports[M6811_TOC2_H])[0];
}

extern inline void
set_output_compare_2 (unsigned short value)
{
  ((unsigned volatile short*) &_io_ports[M6811_TOC2_H])[0] = value;
}

extern inline unsigned short
get_output_compare_3 (void)
{
  return ((unsigned volatile short*) &_io_ports[M6811_TOC3_H])[0];
}

extern inline void
set_output_compare_3 (unsigned short value)
{
  ((unsigned volatile short*) &_io_ports[M6811_TOC3_H])[0] = value;
}

extern inline unsigned short
get_output_compare_4 (void)
{
  return ((unsigned volatile short*) &_io_ports[M6811_TOC4_H])[0];
}

extern inline void
set_output_compare_4 (unsigned short value)
{
  ((unsigned volatile short*) &_io_ports[M6811_TOC4_H])[0] = value;
}

extern inline unsigned short
get_output_compare_5 (void)
{
  return ((unsigned volatile short*) &_io_ports[M6811_TOC5_H])[0];
}

extern inline void
set_output_compare_5 (unsigned short value)
{
  ((unsigned volatile short*) &_io_ports[M6811_TOC5_H])[0] = value;
}

#endif




#endif /* _M68HC11_PORTS_H */

