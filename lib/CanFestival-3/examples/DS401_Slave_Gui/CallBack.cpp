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
#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/textctrl.h>
#include <iostream>

#include "CallBack.h"
#include "main.h"
extern "C"
{
#include "ObjDict.h"
}


extern wxTextCtrl *textLog;

/*****************************************************************************/
void
Call_heartbeatError (CO_Data * d, UNS8 heartbeatID)
{
  //*textLog << wxT("HeartbeatError\n");
}

void
Call_initialisation (CO_Data * d)
{
  //*textLog << wxT("Initialisation\n");
}

void
Call_preOperational (CO_Data * d)
{
  //*textLog << wxT("PreOperational\n");
}

void
Call_operational (CO_Data * d)
{
  //*textLog << wxT("Operational\n");
}

void
Call_stopped (CO_Data * d)
{
  //*textLog << wxT("Stopped\n");
}

void
Call_post_sync (CO_Data * d)
{
  //*textLog << wxT("Post_sync\n");
 /* printf ("POST SYNC:\n");
  printf
    ("     Write_Outputs: %x \n     Analogue_Output1: %d \n     Analogue_Output2: %d \n",
     Write_Outputs_8_Bit[0], Write_Analogue_Output_16_Bit[0],
     Write_Analogue_Output_16_Bit[1]);
  printf
    ("     Read_Input: %x \n     Analogue_Input1: %d \n     Analogue_Input2: %d \n",
     Read_Inputs_8_Bit[0], Read_Analogue_Input_16_Bit[0],
     Read_Analogue_Input_16_Bit[1]);*/
}

void
Call_post_TPDO (CO_Data * d)
{
  //*textLog << wxT("Post_TPDO\n");
}

void
Call_storeODSubIndex (CO_Data * d, UNS16 wIndex, UNS8 bSubindex)
{
  /*printf ("StoreODSubIndex : %4.4x %2.2x\n", wIndex, bSubindex);*/
  //*textLog << wxT("StoreODSubIndex :\n");
}
