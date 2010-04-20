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
/*
Variables to access to the io_ports
*/


#ifndef __PORTS_ACCESS__
#define __PORTS_ACCESS__

/*
By default the address is 0X0000 on HCS12.
But the ports can be remaped. See the file ports.h
*/


extern volatile unsigned char _io_ports[];

/* To use for a 8 bits access */
#define IO_PORTS_8(adr) \
_io_ports[adr]

/* To use for a 16 bits access */
/* Example : IO_PORTS_16(CAN0IDAR1)= 0xABCD;
will put AB at @CAN0IDAR1 and CD at  @CAN0IDAR1 + 1
*/
#define IO_PORTS_16(adr) \
*((unsigned volatile short*) (_io_ports + (adr)))

#endif
