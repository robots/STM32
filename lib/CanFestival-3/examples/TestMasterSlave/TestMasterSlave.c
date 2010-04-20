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

#if defined(WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#include "getopt.h"
void pause(void)
{
	system("PAUSE");
}
#else
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#endif

#include "canfestival.h"
//#include <can_driver.h>
//#include <timers_driver.h>

#include "Master.h"
#include "Slave.h"
#include "TestMasterSlave.h"

UNS32 OnMasterMap1Update(CO_Data* d, const indextable * unsused_indextable, UNS8 unsused_bSubindex)
{
	eprintf("OnMasterMap1Update:%d\n", MasterMap1);
	return 0;
}

s_BOARD SlaveBoard = {"0", "125K"};
s_BOARD MasterBoard = {"1", "125K"};

#if !defined(WIN32) || defined(__CYGWIN__)
void catch_signal(int sig)
{
  signal(SIGTERM, catch_signal);
  signal(SIGINT, catch_signal);
  eprintf("Got Signal %d\n",sig);
}
#endif

void help(void)
{
  printf("**************************************************************\n");
  printf("*  TestMasterSlave                                           *\n");
  printf("*                                                            *\n");
  printf("*  A simple example for PC. It does implement 2 CanOpen      *\n");
  printf("*  nodes in the same process. A master and a slave. Both     *\n");
  printf("*  communicate together, exchanging periodically NMT, SYNC,  *\n");
  printf("*  SDO and PDO. Master configure heartbeat producer time     *\n");
  printf("*  at 1000 ms for slave node-id 0x02 by concise DCF.         *\n");                                  
  printf("*                                                            *\n");
  printf("*   Usage:                                                   *\n");
  printf("*   ./TestMasterSlave  [OPTIONS]                             *\n");
  printf("*                                                            *\n");
  printf("*   OPTIONS:                                                 *\n");
  printf("*     -l : Can library [\"libcanfestival_can_virtual.so\"]     *\n");
  printf("*                                                            *\n");
  printf("*    Slave:                                                  *\n");
  printf("*     -s : bus name [\"0\"]                                    *\n");
  printf("*     -S : 1M,500K,250K,125K,100K,50K,20K,10K,none(disable)  *\n");
  printf("*                                                            *\n");
  printf("*    Master:                                                 *\n");
  printf("*     -m : bus name [\"1\"]                                    *\n");
  printf("*     -M : 1M,500K,250K,125K,100K,50K,20K,10K,none(disable)  *\n");
  printf("*                                                            *\n");
  printf("**************************************************************\n");
}

/***************************  INIT  *****************************************/
void InitNodes(CO_Data* d, UNS32 id)
{
	/****************************** INITIALISATION SLAVE *******************************/
	if(strcmp(SlaveBoard.baudrate, "none")) {
		setNodeId(&TestSlave_Data, 0x02);

		/* init */
		setState(&TestSlave_Data, Initialisation);
	}

	/****************************** INITIALISATION MASTER *******************************/
	if(strcmp(MasterBoard.baudrate, "none")){
 		RegisterSetODentryCallBack(&TestMaster_Data, 0x2000, 0, &OnMasterMap1Update);
		
		/* Defining the node Id */
		setNodeId(&TestMaster_Data, 0x01);

		/* init */
		setState(&TestMaster_Data, Initialisation);
			
	}
}

/***************************  EXIT  *****************************************/
void Exit(CO_Data* d, UNS32 id)
{
	if(strcmp(MasterBoard.baudrate, "none")){
		
		masterSendNMTstateChange(&TestMaster_Data, 0x02, NMT_Reset_Node);    
    
    	//Stop master
		setState(&TestMaster_Data, Stopped);
	}
}

/****************************************************************************/
/***************************  MAIN  *****************************************/
/****************************************************************************/
int main(int argc,char **argv)
{

  int c;
  extern char *optarg;
  char* LibraryPath="../../drivers/can_virtual/libcanfestival_can_virtual.so";

  while ((c = getopt(argc, argv, "-m:s:M:S:l:")) != EOF)
  {
    switch(c)
    {
      case 's' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        SlaveBoard.busname = optarg;
        break;
      case 'm' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        MasterBoard.busname = optarg;
        break;
      case 'S' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        SlaveBoard.baudrate = optarg;
        break;
      case 'M' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        MasterBoard.baudrate = optarg;
        break;
      case 'l' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        LibraryPath = optarg;
        break;
      default:
        help();
        exit(1);
    }
  }

#if !defined(WIN32) || defined(__CYGWIN__)
  /* install signal handler for manual break */
	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);
	TimerInit();
#endif

#ifndef NOT_USE_DYNAMIC_LOADING
	if (LoadCanDriver(LibraryPath) == NULL)
	    printf("Unable to load library: %s\n",LibraryPath);
#endif		
	// Open CAN devices

	if(strcmp(SlaveBoard.baudrate, "none")){
		
		TestSlave_Data.heartbeatError = TestSlave_heartbeatError;
		TestSlave_Data.initialisation = TestSlave_initialisation;
		TestSlave_Data.preOperational = TestSlave_preOperational;
		TestSlave_Data.operational = TestSlave_operational;
		TestSlave_Data.stopped = TestSlave_stopped;
		TestSlave_Data.post_sync = TestSlave_post_sync;
		TestSlave_Data.post_TPDO = TestSlave_post_TPDO;
		TestSlave_Data.storeODSubIndex = TestSlave_storeODSubIndex;
		TestSlave_Data.post_emcy = TestSlave_post_emcy;

		if(!canOpen(&SlaveBoard,&TestSlave_Data)){
			eprintf("Cannot open Slave Board (%s,%s)\n",SlaveBoard.busname, SlaveBoard.baudrate);
			goto fail_slave;
		}
	}
	if(strcmp(MasterBoard.baudrate, "none")){
		
		TestMaster_Data.heartbeatError = TestMaster_heartbeatError;
		TestMaster_Data.initialisation = TestMaster_initialisation;
		TestMaster_Data.preOperational = TestMaster_preOperational;
		TestMaster_Data.operational = TestMaster_operational;
		TestMaster_Data.stopped = TestMaster_stopped;
		TestMaster_Data.post_sync = TestMaster_post_sync;
		TestMaster_Data.post_TPDO = TestMaster_post_TPDO;
		TestMaster_Data.post_emcy = TestMaster_post_emcy;
		TestMaster_Data.post_SlaveBootup=TestMaster_post_SlaveBootup;
		
		if(!canOpen(&MasterBoard,&TestMaster_Data)){
			eprintf("Cannot open Master Board (%s,%s)\n",MasterBoard.busname, MasterBoard.baudrate);
			goto fail_master;
		}
	}

	// Start timer thread
	StartTimerLoop(&InitNodes);

	// wait Ctrl-C
	pause();

	// Stop timer thread
	StopTimerLoop(&Exit);
	
	// Close CAN devices (and can threads)
	if(strcmp(MasterBoard.baudrate, "none")) canClose(&TestMaster_Data);	
fail_master:
	if(strcmp(SlaveBoard.baudrate, "none")) canClose(&TestSlave_Data);
fail_slave:
	TimerCleanup();
	return 0;
}
