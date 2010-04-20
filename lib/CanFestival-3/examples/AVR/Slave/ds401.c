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

// Includes for the Canfestival
#include "ds401.h"

unsigned char digital_input_handler(CO_Data* d, unsigned char *newInput, unsigned char size)
{
  unsigned char loops, i, input, transmission = 0;

  loops = (sizeof(Read_Inputs_8_Bit) <= size) ? sizeof(Read_Inputs_8_Bit) : size;

  for (i=0; i < loops; i++)
  {
    input = *newInput ^ Polarity_Input_8_Bit[i];
    if (Read_Inputs_8_Bit[i] != input)
    {
      if (Global_Interrupt_Enable_Digital)
      {
        if ((Interrupt_Mask_Any_Change_8_Bit[i] & (Read_Inputs_8_Bit[i] ^ input)) 
	 || (Interrupt_Mask_Low_to_High_8_Bit[i] & ~Read_Inputs_8_Bit[i] & input)
	 || (Interrupt_Mask_High_to_Low_8_Bit[i] & Read_Inputs_8_Bit[i] & ~input))
	   transmission = 1;
      }
      // update object dict
      Read_Inputs_8_Bit[i] = input;
    }
    newInput++;
  }
  if (transmission)
  {
  /* force emission of PDO by artificially changing last emitted*/
    d->PDO_status[0].last_message.cob_id = 0;
    sendPDOevent(d);
  }

  return 1;
}

unsigned char digital_output_handler(CO_Data* d, unsigned char *newOutput, unsigned char size)
{
  unsigned char loops, i, error, type;
  unsigned char varsize = 1;

  loops = (sizeof(Write_Outputs_8_Bit) <= size) ? sizeof(Write_Outputs_8_Bit) : size;

  for (i=0; i < loops; i++)
  {
    getODentry(d, 0x1001, 0x0, &error, &varsize, &type, RO);
    if ((getState(d) == Stopped) || (error != 0))	// node stopped or error
    {
      Write_Outputs_8_Bit[i] &= (~Error_Mode_Outputs_8_Bit[i] | Error_Value_Outputs_8_Bit[i]);
      Write_Outputs_8_Bit[i] |= (Error_Mode_Outputs_8_Bit[i] & Error_Value_Outputs_8_Bit[i]);
    }
    *newOutput = Write_Outputs_8_Bit[i] ^ Change_Polarity_Outputs_8_Bit[i];
    newOutput++;
  }
  return 1;
}

unsigned char analog_input_handler(CO_Data* d, unsigned int *newInput, unsigned char size)
{
  return 0;
}

unsigned char analog_output_handler(CO_Data* d, unsigned int *newOutput, unsigned char size)
{
  return 0;
}



