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
#ifndef __KERNEL__
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#else
#include <linux/module.h>
#endif

#ifndef NOT_USE_DYNAMIC_LOADING
#define DLL_CALL(funcname) (* funcname##_driver)
#define FCT_PTR_INIT =NULL

#define DLSYM(name)\
	*(void **) (&name##_driver) = dlsym(handle, #name"_driver");\
	if ((error = dlerror()) != NULL)  {\
		fprintf (stderr, "%s\n", error);\
		UnLoadCanDriver(handle);\
		return NULL;\
	}

#else /*NOT_USE_DYNAMIC_LOADING*/

/*Function call is direct*/
#define DLL_CALL(funcname) funcname##_driver

#endif /*NOT_USE_DYNAMIC_LOADING*/

#include "data.h"
#include "canfestival.h"
#include "timers_driver.h"

#define MAX_NB_CAN_PORTS 16

/** CAN port structure */
typedef struct {
  char used;  /**< flag indicating CAN port usage, will be used to abort Receiver task*/
  CAN_HANDLE fd; /**< CAN port file descriptor*/
  TASK_HANDLE receiveTask; /**< CAN Receiver task*/
  CO_Data* d; /**< CAN object data*/
} CANPort;

#include "can_driver.h"

/*Declares the funtion pointers for dll binding or simple protos*/
/*UNS8 DLL_CALL(canReceive)(CAN_HANDLE, Message *);
UNS8 DLL_CALL(canSend)(CAN_HANDLE, Message *);
CAN_HANDLE DLL_CALL(canOpen)(s_BOARD *);
int DLL_CALL(canClose)(CAN_HANDLE);
*/
CANPort canports[MAX_NB_CAN_PORTS] = {{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,}};

#ifndef NOT_USE_DYNAMIC_LOADING

/*UnLoads the dll*/
UNS8 UnLoadCanDriver(LIB_HANDLE handle)
{
	if(handle!=NULL)
	{
		dlclose(handle);

		handle=NULL;
		return 0;
	}
	return -1;
}

/**
 * Loads the dll and get funcs ptr
 *
 * @param driver_name String containing driver's dynamic library name
 * @return Library handle
 */
LIB_HANDLE LoadCanDriver(const char* driver_name)
{
	LIB_HANDLE handle = NULL;
	char *error;
	

	if(handle==NULL)
	{
		handle = dlopen(driver_name, RTLD_LAZY);
	}

	if (!handle) {
		fprintf (stderr, "%s\n", dlerror());
        	return NULL;
	}
 
	/*Get function ptr*/
	DLSYM(canReceive)
	DLSYM(canSend)
	DLSYM(canOpen)
	DLSYM(canChangeBaudRate)
	DLSYM(canClose)

	return handle;
}

#endif



/*Not needed -- canReceiveLoop calls _canReceive directly *//*
UNS8 canReceive(CAN_PORT port, Message *m)
{
	return DLL_CALL(canReceive)(port->fd, Message *m);
}
*/

/**
 * CAN send routine
 * @param port CAN port
 * @param m CAN message
 * @return success or error
 */
UNS8 canSend(CAN_PORT port, Message *m)
{
	if(port){
		UNS8 res;
	        //LeaveMutex();
		res = DLL_CALL(canSend)(((CANPort*)port)->fd, m);
		//EnterMutex();
		return res; // OK
	}               
	return 1; // NOT OK
}

/**
 * CAN Receiver Task
 * @param port CAN port
 */
void canReceiveLoop(CAN_PORT port)
{
       Message m;

       while (((CANPort*)port)->used) {
               if (DLL_CALL(canReceive)(((CANPort*)port)->fd, &m) != 0)
                       break;

               EnterMutex();
               canDispatch(((CANPort*)port)->d, &m);
               LeaveMutex();
       }
}

/**
 * CAN open routine
 * @param board device name and baudrate
 * @param d CAN object data
 * @return valid CAN_PORT pointer or NULL
 */
CAN_PORT canOpen(s_BOARD *board, CO_Data * d)
{
	int i;
	for(i=0; i < MAX_NB_CAN_PORTS; i++)
	{
		if(!canports[i].used)
		break;
	}
	
#ifndef NOT_USE_DYNAMIC_LOADING
	if (&DLL_CALL(canOpen)==NULL) {
        	fprintf(stderr,"CanOpen : Can Driver dll not loaded\n");
        	return NULL;
	}
#endif	
	CAN_HANDLE fd0 = DLL_CALL(canOpen)(board);
	if(fd0){
		canports[i].used = 1;
		canports[i].fd = fd0;
		canports[i].d = d;
		d->canHandle = (CAN_PORT)&canports[i];		
		CreateReceiveTask(&(canports[i]), &canports[i].receiveTask, &canReceiveLoop);
		return (CAN_PORT)&canports[i];
	}else{
        	MSG("CanOpen : Cannot open board {busname='%s',baudrate='%s'}\n",board->busname, board->baudrate);
		return NULL;
	}
}

/**
 * CAN close routine
 * @param d CAN object data
 * @return success or error
 */
int canClose(CO_Data * d)
{
	UNS8 res;
	
	((CANPort*)d->canHandle)->used = 0;
	CANPort* tmp = (CANPort*)d->canHandle;
	d->canHandle = NULL;
	
	// close CAN port
	res = DLL_CALL(canClose)(tmp->fd);

	// kill receiver task
	WaitReceiveTaskEnd(&tmp->receiveTask);
	
	return res;
}


/**
 * CAN change baudrate routine
 * @param port CAN port
 * @param baud baudrate
 * @return success or error
 */
UNS8 canChangeBaudRate(CAN_PORT port, char* baud)
{
   if(port){
		UNS8 res;
	        //LeaveMutex();
		res = DLL_CALL(canChangeBaudRate)(((CANPort*)port)->fd, baud);
		//EnterMutex();
		return res; // OK
	}               
	return 1; // NOT OK
}


#ifdef __KERNEL__
EXPORT_SYMBOL (canOpen);
EXPORT_SYMBOL (canClose);
EXPORT_SYMBOL (canSend);
EXPORT_SYMBOL (canChangeBaudRate);
#endif
