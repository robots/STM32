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
Functions called by interrupts vectors.
*/
/*
This is a part of the driver, of course !
But you have to put your code in this functions,
if you plan to use interruptions.
*/

#include "../include/data.h"
#include "../include/hcs12/applicfg.h"
#include "../include/hcs12/error.h"
#include "../include/hcs12/candriver.h"
#include "../include/hcs12/asm-m68hc12/regs.h"
#include "../include/hcs12/asm-m68hc12/portsaccess.h"
#include "../include/hcs12/asm-m68hc12/ports_def.h"
#include "../include/hcs12/asm-m68hc12/ports.h"
#include "../include/hcs12/interrupt.h"

extern volatile char msgRecu;
extern volatile Message canMsgRcv;



/* Inhibe les interruptions */

void lock (void)
{
   unsigned short mask;
   __asm__ __volatile__ ("tpa\n\tsei" : "=d"(mask));

}

/* Autorise les interruptions */
void unlock (void)
{ 
   __asm__ __volatile__ ("cli");
}




