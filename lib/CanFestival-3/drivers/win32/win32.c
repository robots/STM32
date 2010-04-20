/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Edouard TISSERANT and Francis DUPIN
Copyright (C) Win32 Port Leonid Tochinski

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
 CAN driver interface.
*/

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "canfestival.h"
#include "timer.h"
#include "timers_driver.h"

#ifdef __cplusplus
};
#endif

// GetProcAddress doesn't have an UNICODE version for NT
#ifdef UNDER_CE
  #define myTEXT(str) TEXT(str)
#else
  #define myTEXT(str) str
#endif

#define MAX_NB_CAN_PORTS 16

typedef UNS8 (CALLBACK* CANRECEIVE_DRIVER_PROC)(void* inst, Message *m);
typedef UNS8 (CALLBACK* CANSEND_DRIVER_PROC)(void* inst, const Message *m);
typedef void* (CALLBACK* CANOPEN_DRIVER_PROC)(s_BOARD *board);
typedef int (CALLBACK* CANCLOSE_DRIVER_PROC)(void* inst);
typedef UNS8 (CALLBACK* CANCHANGEBAUDRATE_DRIVER_PROC)(void* fd, char* baud);

CANRECEIVE_DRIVER_PROC m_canReceive;
CANSEND_DRIVER_PROC m_canSend;
CANOPEN_DRIVER_PROC m_canOpen;
CANCLOSE_DRIVER_PROC m_canClose;
CANCHANGEBAUDRATE_DRIVER_PROC m_canChangeBaudRate;

/* CAN port structure */
typedef struct
{
  char used;  /**< flag indicating CAN port usage, will be used to abort Receiver task*/
  CAN_HANDLE fd; /**< CAN port file descriptor*/
  TASK_HANDLE receiveTask; /**< CAN Receiver task*/
  CO_Data* d; /**< CAN object data*/
}CANPort;

CANPort canports[MAX_NB_CAN_PORTS] = {{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,}};


/***************************************************************************/
UNS8 UnLoadCanDriver(LIB_HANDLE handle)
{
	if(handle != NULL)
	{
		FreeLibrary(handle);
		handle=NULL;
		return 0;
	}
	return -1;
}

/***************************************************************************/
/**
 * Loads the dll and get funcs ptr
 *
 * @param driver_name String containing driver's dynamic library name
 * @return Library handle
 */
LIB_HANDLE LoadCanDriver(LPCTSTR driver_name)
{
	// driver module handle
	LIB_HANDLE handle = NULL;

	if(handle == NULL)
	{
		handle = LoadLibrary(driver_name);
	}

	if (!handle)
	{
		fprintf (stderr, "%d\n", GetLastError());
    	return NULL;
	}

	m_canReceive = (CANRECEIVE_DRIVER_PROC)GetProcAddress(handle, myTEXT("canReceive_driver"));
	m_canSend = (CANSEND_DRIVER_PROC)GetProcAddress(handle, myTEXT("canSend_driver"));
	m_canOpen = (CANOPEN_DRIVER_PROC)GetProcAddress(handle, myTEXT("canOpen_driver"));
	m_canClose = (CANCLOSE_DRIVER_PROC)GetProcAddress(handle, myTEXT("canClose_driver"));
	m_canChangeBaudRate = (CANCHANGEBAUDRATE_DRIVER_PROC)GetProcAddress(handle, myTEXT("canChangeBaudRate_driver"));

	return handle;
}

/***************************************************************************/
UNS8 canSend(CAN_PORT port, Message *m)
{
	UNS8 res;
	if (port && (m_canSend != NULL))
	{
		res = m_canSend(((CANPort*)port)->fd, m);
		if (res) return 1; // OK
	}
	return 0; // NOT OK
}

/***************************************************************************/
void canReceiveLoop(CAN_PORT port)
{
	Message m;
	while(((CANPort*)port)->used)
	{
		if(m_canReceive(((CANPort*)port)->fd, &m) != 0) break;
		EnterMutex();
		canDispatch(((CANPort*)port)->d, &m);
		LeaveMutex();
	}
}

/***************************************************************************/
CAN_HANDLE canOpen(s_BOARD *board, CO_Data * d)
{
	int i;
    CAN_HANDLE fd0;

	for(i=0; i < MAX_NB_CAN_PORTS; i++)
	{
		if(!canports[i].used)
		break;
	}

	#ifndef NOT_USE_DYNAMIC_LOADING
	if (m_canOpen == NULL)
	{
	   	fprintf(stderr,"CanOpen : Can Driver dll not loaded\n");
	   	return NULL;
	}
	#endif

	fd0 = m_canOpen(board);
	if(fd0)
	{
		canports[i].used = 1;
		canports[i].fd = fd0;
		canports[i].d = d;
		d->canHandle = (CAN_PORT)&canports[i];
		CreateReceiveTask(&(canports[i]), &canports[i].receiveTask, (void *)canReceiveLoop);
		return (CAN_PORT)&canports[i];
	}
	else
	{
		MSG("CanOpen : Cannot open board {busname='%s',baudrate='%s'}\n",board->busname, board->baudrate);
		return NULL;
	}
}

/***************************************************************************/
int canClose(CO_Data * d)
{
	UNS8 res = 1;
	CANPort* tmp;

	if((CANPort*)d->canHandle)
	{
	  ((CANPort*)d->canHandle)->used = 0;
	}

	tmp = (CANPort*)d->canHandle;
	d->canHandle = NULL;

	if(tmp)
	{
	  // close CAN port
	  res = m_canClose(tmp->fd);

	  // kill receiver task
	  WaitReceiveTaskEnd(&tmp->receiveTask);
	}
	return res;
}

UNS8 canChangeBaudRate(CAN_PORT port, char* baud)
{
   if(port){
		UNS8 res;
	    //LeaveMutex();
		res = m_canChangeBaudRate(((CANPort*)port)->fd, baud);
		//EnterMutex();
		return res; // OK
	}
	return 1; // NOT OK
}

