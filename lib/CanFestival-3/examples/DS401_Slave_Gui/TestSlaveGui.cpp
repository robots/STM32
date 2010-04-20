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

#if defined(WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#else
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#endif

//#include <can_driver.h>
//#include <timers_driver.h>
extern "C"
{
#include "canfestival.h"
}
#include "CallBack.h"
#include "TestSlaveGui.h"
#include "main.h"

extern wxTextCtrl *textLog;
extern int node_id_ext;

//***************************  INIT  *****************************************
void
InitNodes (CO_Data * d, UNS32 id)
{
  //****************************** INITIALISATION SLAVE *******************************
  /* Defining the node Id */
  setNodeId (&ObjDict_Data, node_id_ext);
  /* init */
  setState (&ObjDict_Data, Initialisation);
}

/***************************  EXIT  *****************************************/
void Exit(CO_Data* d, UNS32 id)
{
  	setState (&ObjDict_Data, Stopped);
	canClose (&ObjDict_Data);
}
//****************************************************************************
//***************************  MAIN  *****************************************
//****************************************************************************
int
main_can (s_BOARD SlaveBoard, char *LibraryPath)
{
#if !defined(WIN32) && !defined(__CYGWIN__)
	TimerInit();
#endif	
	
	printf ("Bus name: %s        Freq: %s       Driver: %s\n",
	  SlaveBoard.busname, SlaveBoard.baudrate, LibraryPath);

#ifndef NOT_USE_DYNAMIC_LOADING
  if (LoadCanDriver (LibraryPath) == NULL)
    *textLog << wxT ("Unable to load library\n");
#endif
  // Open CAN devices

  ObjDict_Data.heartbeatError = Call_heartbeatError;
  ObjDict_Data.initialisation = Call_initialisation;
  ObjDict_Data.preOperational = Call_preOperational;
  ObjDict_Data.operational = Call_operational;
  ObjDict_Data.stopped = Call_stopped;
  ObjDict_Data.post_sync = Call_post_sync;
  ObjDict_Data.post_TPDO = Call_post_TPDO;
  ObjDict_Data.storeODSubIndex = Call_storeODSubIndex;

  if (!canOpen (&SlaveBoard, &ObjDict_Data))
    {
      printf ("Cannot open Slave Board (%s,%s)\n", SlaveBoard.busname,
	      SlaveBoard.baudrate);
      return (1);
    }

  StartTimerLoop (&InitNodes);

  return 0;
}

void
stop_slave ()
{
  StopTimerLoop (&Exit);
  TimerCleanup();
  return;
}
