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
/* param.h - 32K Board specific parameters
   Copyright (C) 2002 Free Software Foundation, Inc.
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

#ifndef _M68HC11_ARCH_32K_PARAM_H
#define _M68HC11_ARCH_32K_PARAM_H

/* Generic 32K board.  */
#define M6811_ARCH_32K

#undef M6811_DEF_BAUD
#define M6811_DEF_BAUD 0x30            /* 9600 baud.  */

#undef RAM_SIZE
#define RAM_SIZE       32768           /* 32Kb of memory.  */

#undef ROM_SIZE
#define ROM_SIZE       32768           /* 32Kb of rom.  */

#undef DATA_SIZE
#define DATA_SIZE      (0x8000-0x1040) /* Data section size.  */

#undef TEXT_SIZE
#define TEXT_SIZE      ROM_SIZE        /* Text section size.  */

#endif
