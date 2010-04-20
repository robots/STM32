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

/* Regs - Software registers used by GCC
   Copyright (C) 2000 Free Software Foundation, Inc.
   Written by Stephane Carrez (stcarrez@worldnet.fr)	

This file is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file with other programs, and to distribute
those programs without any restriction coming from the use of this
file.  (The General Public License restrictions do apply in other
respects; for example, they cover modification of the file, and
distribution when not linked into another program.)

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef _M68HC11_REGS_H
#define _M68HC11_REGS_H

/* Include this file if you want to have access to the soft registers
   used by GCC.  The soft registers are memory location.  They have
   special names that cannot conflict with C or C++ variables.
   The extern definition below map those special names to
   some usable names in C and C++.  */
  
extern unsigned short __tmp    __asm__("_.tmp");
extern unsigned short __z      __asm__("_.z");
extern unsigned short __xy     __asm__("_.xy");
extern unsigned short __frame  __asm__("_.frame");
extern unsigned short __d1     __asm__("_.d1");
extern unsigned short __d2     __asm__("_.d2");
extern unsigned short __d3     __asm__("_.d3");
extern unsigned short __d4     __asm__("_.d4");
extern unsigned short __d5     __asm__("_.d5");
extern unsigned short __d6     __asm__("_.d6");
extern unsigned short __d7     __asm__("_.d7");
extern unsigned short __d8     __asm__("_.d8");
extern unsigned short __d9     __asm__("_.d9");
extern unsigned short __d10    __asm__("_.d10");
extern unsigned short __d11    __asm__("_.d11");
extern unsigned short __d12    __asm__("_.d12");
extern unsigned short __d13    __asm__("_.d13");
extern unsigned short __d14    __asm__("_.d14");
extern unsigned short __d15    __asm__("_.d15");
extern unsigned short __d16    __asm__("_.d16");
extern unsigned short __d17    __asm__("_.d17");
extern unsigned short __d18    __asm__("_.d18");
extern unsigned short __d19    __asm__("_.d19");
extern unsigned short __d20    __asm__("_.d20");
extern unsigned short __d21    __asm__("_.d21");
extern unsigned short __d22    __asm__("_.d22");
extern unsigned short __d23    __asm__("_.d23");
extern unsigned short __d24    __asm__("_.d24");
extern unsigned short __d25    __asm__("_.d25");
extern unsigned short __d26    __asm__("_.d26");
extern unsigned short __d27    __asm__("_.d27");
extern unsigned short __d28    __asm__("_.d28");
extern unsigned short __d29    __asm__("_.d29");
extern unsigned short __d30    __asm__("_.d30");
extern unsigned short __d31    __asm__("_.d31");
extern unsigned short __d32    __asm__("_.d32");

#endif
