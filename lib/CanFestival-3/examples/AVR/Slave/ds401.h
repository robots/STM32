/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Andreas GLAUSER

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

// DS 401 Digital IO handling according DS 401 V2.1 "Device Profile for Generic I/O Modules"

#ifndef __DS_401_h__
#define __DS_401_h__

// Includes for the Canfestival
#include "canfestival.h"
#include "timer.h"
#include "objdict.h"


unsigned char digital_input_handler(CO_Data* d, unsigned char *newInput, unsigned char size);

unsigned char digital_output_handler(CO_Data* d, unsigned char *newOuput, unsigned char size);

unsigned char analog_input_handler(CO_Data* d, unsigned int *newInput, unsigned char size);

unsigned char analog_output_handler(CO_Data* d, unsigned int *newOutput, unsigned char size);

#endif //__DS_401_h__
