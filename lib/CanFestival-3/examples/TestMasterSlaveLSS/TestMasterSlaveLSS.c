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
#include "SlaveA.h"
#include "SlaveB.h"
#include "TestMasterSlaveLSS.h"

s_BOARD SlaveBoardA = {"0", "125K"};
s_BOARD SlaveBoardB = {"1", "125K"};
s_BOARD MasterBoard = {"2", "125K"};

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
  printf("*  TestMasterSlaveLSS                                        *\n");
  printf("*                                                            *\n");
  printf("*  A LSS example for PC. It does implement 3 CanOpen         *\n");
  printf("*  nodes in the same process. A master and 2 slaves. All     *\n");
  printf("*  communicate together, exchanging periodically NMT, SYNC,  *\n");
  printf("*  SDO and PDO. Master configure heartbeat producer time     *\n");
  printf("*  at 1000 ms for the slaves by concise DCF.                 *\n");                                  
  printf("*                                                            *\n");
  printf("*   Usage:                                                   *\n");
  printf("*   ./TestMasterSlaveLSS  [OPTIONS]                          *\n");
  printf("*                                                            *\n");
  printf("*   OPTIONS:                                                 *\n");
  printf("*     -l : Can library [\"libcanfestival_can_virtual.so\"]     *\n");
  printf("*                                                            *\n");
  printf("*    SlaveA:                                                 *\n");
  printf("*     -a : bus name [\"0\"]                                    *\n");
  printf("*     -A : 1M,500K,250K,125K,100K,50K,20K,10K,none(disable)  *\n");
  printf("*                                                            *\n");
  printf("*    SlaveB:                                                 *\n");
  printf("*     -b : bus name [\"1\"]                                    *\n");
  printf("*     -B : 1M,500K,250K,125K,100K,50K,20K,10K,none(disable)  *\n");
  printf("*                                                            *\n");
  printf("*    Master:                                                 *\n");
  printf("*     -m : bus name [\"2\"]                                    *\n");
  printf("*     -M : 1M,500K,250K,125K,100K,50K,20K,10K,none(disable)  *\n");
  printf("*                                                            *\n");
  printf("**************************************************************\n");
}

/***************************  INIT  *****************************************/
void InitNodes(CO_Data* d, UNS32 id)
{
	/****************************** INITIALISATION SLAVE_A *******************************/
	if(strcmp(SlaveBoardA.baudrate, "none")) {
		/* Set an invalid nodeID */
		setNodeId(&TestSlaveA_Data, 0xFF);

		/* init */
		setState(&TestSlaveA_Data, Initialisation);
	}
	
	/****************************** INITIALISATION SLAVE_B *******************************/
	if(strcmp(SlaveBoardB.baudrate, "none")) {

		/* Set an invalid nodeID */
		setNodeId(&TestSlaveB_Data, 0xFF);

		/* init */
		setState(&TestSlaveB_Data, Initialisation);
	}

	/****************************** INITIALISATION MASTER *******************************/
	if(strcmp(MasterBoard.baudrate, "none")){
		
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
	eprintf("Finishing.\n");
	masterSendNMTstateChange (&TestMaster_Data, 0x00, NMT_Stop_Node);

	eprintf("reset\n");

	// Stop master
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

  while ((c = getopt(argc, argv, "-m:a:b:M:A:B:l:")) != EOF)
  {
    switch(c)
    {
      case 'a' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        SlaveBoardA.busname = optarg;
        break;
      case 'b' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        SlaveBoardB.busname = optarg;
        break;
      case 'm' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        MasterBoard.busname = optarg;
        break;
      case 'A' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        SlaveBoardA.baudrate = optarg;
        break;
      case 'B' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        SlaveBoardB.baudrate = optarg;
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

	if(strcmp(SlaveBoardA.baudrate, "none")){
		
		TestSlaveA_Data.heartbeatError = TestSlaveA_heartbeatError;
		TestSlaveA_Data.initialisation = TestSlaveA_initialisation;
		TestSlaveA_Data.preOperational = TestSlaveA_preOperational;
		TestSlaveA_Data.operational = TestSlaveA_operational;
		TestSlaveA_Data.stopped = TestSlaveA_stopped;
		TestSlaveA_Data.post_sync = TestSlaveA_post_sync;
		TestSlaveA_Data.post_TPDO = TestSlaveA_post_TPDO;
		TestSlaveA_Data.storeODSubIndex = TestSlaveA_storeODSubIndex;
		TestSlaveA_Data.post_emcy = TestSlaveA_post_emcy;
		/* in this example the slave doesn't implement NMT_Slave_Communications_Reset_Callback */
		//TestSlaveA_Data.NMT_Slave_Communications_Reset_Callback = TestSlaveA_NMT_Slave_Communications_Reset_Callback;
		TestSlaveA_Data.lss_StoreConfiguration = TestSlaveA_StoreConfiguration;

		if(!canOpen(&SlaveBoardA,&TestSlaveA_Data)){
			eprintf("Cannot open SlaveA Board (%s,%s)\n",SlaveBoardA.busname, SlaveBoardA.baudrate);
			goto fail_slaveA;
		}
	}
	
	if(strcmp(SlaveBoardB.baudrate, "none")){
		
		TestSlaveB_Data.heartbeatError = TestSlaveB_heartbeatError;
		TestSlaveB_Data.initialisation = TestSlaveB_initialisation;
		TestSlaveB_Data.preOperational = TestSlaveB_preOperational;
		TestSlaveB_Data.operational = TestSlaveB_operational;
		TestSlaveB_Data.stopped = TestSlaveB_stopped;
		TestSlaveB_Data.post_sync = TestSlaveB_post_sync;
		TestSlaveB_Data.post_TPDO = TestSlaveB_post_TPDO;
		TestSlaveB_Data.storeODSubIndex = TestSlaveB_storeODSubIndex;
		TestSlaveB_Data.post_emcy = TestSlaveB_post_emcy;
		TestSlaveB_Data.NMT_Slave_Communications_Reset_Callback = TestSlaveB_NMT_Slave_Communications_Reset_Callback;
		TestSlaveB_Data.lss_StoreConfiguration = TestSlaveB_StoreConfiguration;

		if(!canOpen(&SlaveBoardB,&TestSlaveB_Data)){
			eprintf("Cannot open SlaveB Board (%s,%s)\n",SlaveBoardB.busname, SlaveBoardB.baudrate);
			goto fail_slaveB;
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
	if(strcmp(SlaveBoardB.baudrate, "none")) canClose(&TestSlaveB_Data);
fail_slaveB:
	if(strcmp(SlaveBoardA.baudrate, "none")) canClose(&TestSlaveA_Data);
fail_slaveA:
	TimerCleanup();
	return 0;
}
