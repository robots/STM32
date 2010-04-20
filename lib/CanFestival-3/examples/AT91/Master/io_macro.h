/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
ARM Port: Peter CHRISTEN

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

#ifndef _IO_MACRO
#define _IO_MACRO

// Macros for set and clear bits in I/O registers
#define setbit(address,bit) (address |= (1<<bit))
#define clearbit(address,bit) (address &= ~(1<<bit))
#define togglebit(address,bit) (address ^= (1<<bit))

// Macro for testing of a single bit in an I/O location
#define checkbit(address,bit) (address & (1<<bit))

#endif
