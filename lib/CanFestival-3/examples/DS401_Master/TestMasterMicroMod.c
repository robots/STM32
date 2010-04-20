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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#endif

#include "canfestival.h"
#include "TestMasterMicroMod.h"
#include "TestMaster.h"
unsigned int slavenodeid;


/*****************************************************************************/
void TestMaster_heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
	eprintf("TestMaster_heartbeatError %d\n", heartbeatID);
}

/********************************************************
 * ConfigureSlaveNode is responsible to
 *  - setup master RPDO 1 to receive TPDO 1 from id 0x40
 *  - setup master TPDO 1 to send RPDO 1 to id 0x40
 ********************************************************/
void TestMaster_initialisation(CO_Data* d)
{
	UNS32 PDO1_COBID = 0x0180 + slavenodeid; 
	UNS32 PDO2_COBID = 0x0200 + slavenodeid;
	UNS32 size = sizeof(UNS32); 

	eprintf("TestMaster_initialisation\n");

	/*****************************************
	 * Define RPDOs to match slave ID=0x40 TPDOs*
	 *****************************************/
	writeLocalDict( &TestMaster_Data, /*CO_Data* d*/
			0x1400, /*UNS16 index*/
			0x01, /*UNS8 subind*/ 
			&PDO1_COBID, /*void * pSourceData,*/ 
			&size, /* UNS8 * pExpectedSize*/
			RW);  /* UNS8 checkAccess */
			

	/*****************************************
	 * Define TPDOs to match slave ID=0x40 RPDOs*
	 *****************************************/
	writeLocalDict( &TestMaster_Data, /*CO_Data* d*/
			0x1800, /*UNS16 index*/
			0x01, /*UNS8 subind*/ 
			&PDO2_COBID, /*void * pSourceData,*/ 
			&size, /* UNS8 * pExpectedSize*/
			RW);  /* UNS8 checkAccess */
}

static int init_step = 0;

/*Froward declaration*/
static void ConfigureSlaveNode(CO_Data* d, UNS8 nodeId);

/**/
static void CheckSDOAndContinue(CO_Data* d, UNS8 nodeId)
{
	UNS32 abortCode;

	if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED)
		eprintf("Master : Failed in initializing slave %2.2x, step %d, AbortCode :%4.4x \n", nodeId, init_step, abortCode);

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(&TestMaster_Data, nodeId, SDO_CLIENT);

	ConfigureSlaveNode(d, nodeId);
}

/********************************************************
 * ConfigureSlaveNode is responsible to
 *  - setup slave TPDO 1 transmit time
 *  - setup slave TPDO 2 transmit time
 *  - setup slave Heartbeat Producer time
 *  - switch to operational mode
 *  - send NMT to slave
 ********************************************************
 * This an example of :
 * Network Dictionary Access (SDO) with Callback 
 * Slave node state change request (NMT) 
 ********************************************************
 * This is called first by TestMaster_preOperational
 * then it called again each time a SDO exchange is
 * finished.
 ********************************************************/
static void ConfigureSlaveNode(CO_Data* d, UNS8 nodeId)
{
	UNS8 res;
	eprintf("Master : ConfigureSlaveNode %2.2x\n", nodeId);
	printf("nodeid slave=%x\n",nodeId);
	switch(++init_step){
		case 1: 
		{	/*disable Slave's TPDO 1 */
			UNS32 TPDO_COBId = 0x80000180 + nodeId;
			
			eprintf("Master : disable slave %2.2x TPDO 1 \n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1800, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
		}			
		break;

		case 2: 
		{	/*setup Slave's TPDO 1 to be transmitted on SYNC*/
			UNS8 Transmission_Type = 0x01;
			
			eprintf("Master : set slave %2.2x TPDO 1 transmit type\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1800, /*UNS16 index*/
					0x02, /*UNS8 subindex*/
					1, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&Transmission_Type,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
		}			
		break;

		case 3: 
		{	/*re-enable Slave's TPDO 1 */
			UNS32 TPDO_COBId = 0x00000180 + nodeId;
			
			eprintf("Master : re-enable slave %2.2x TPDO 1\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1800, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
		}			
		break;
					
		case 4: 
		{	/*disable Slave's TPDO 2 */
			UNS32 TPDO_COBId = 0x80000200 + nodeId;
			
			eprintf("Master : disable slave %2.2x RPDO 1\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1400, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
		}			
		break;

					
		case 5:
		{	
			UNS8 Transmission_Type = 0x01;
			
			eprintf("Master : set slave %2.2x RPDO 1 receive type\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1400, /*UNS16 index*/
					0x02, /*UNS8 subindex*/
					1, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&Transmission_Type,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
		}	
		break;

		case 6: 
		{	/*re-enable Slave's TPDO 1 */
			UNS32 TPDO_COBId = 0x00000200 + nodeId;
			
			eprintf("Master : re-enable %2.2x RPDO 1\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1400, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
		}			
		break;
		
		case 7:	
		{
			UNS16 Heartbeat_Producer_Time = 0x03E8; 
			eprintf("Master : set slave %2.2x heartbeat producer time \n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1017, /*UNS16 index*/
					0x00, /*UNS8 subindex*/
					2, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&Heartbeat_Producer_Time,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
		}			
		break;

		case 8: 
		{	/*disable Slave's TPDO 2 */
			UNS32 TPDO_COBId = 0x80000280 + nodeId;
			
			eprintf("Master : disable slave %2.2x TPDO 2 \n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1801, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
		}			
		break;

		case 9: 
		{	/*disable Slave's TPDO 3 */
			UNS32 TPDO_COBId = 0x80000380 + nodeId;
			
			eprintf("Master : disable slave %2.2x TPDO 3 \n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1802, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
		}
		break;			

		case 10: 
		{	/*disable Slave's TPDO 2 */
			UNS32 TPDO_COBId = 0x80000480 + nodeId;
			
			eprintf("Master : disable slave %2.2x TPDO 4 \n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					/**TestSlave_Data.bDeviceNodeId, UNS8 nodeId*/
					nodeId, /*UNS8 nodeId*/
					0x1803, /*UNS16 index*/
					0x01, /*UNS8 subindex*/
					4, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&TPDO_COBId,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
		}			
		break;			
		
		case 11:
			/* Put the master in operational mode */
			setState(d, Operational);
			  
			/* Ask slave node to go in operational mode */
			masterSendNMTstateChange (d, nodeId, NMT_Start_Node);
	}
			
}

void TestMaster_preOperational(CO_Data* d)
{

	eprintf("TestMaster_preOperational\n");
	ConfigureSlaveNode(&TestMaster_Data, slavenodeid);
	
}

void TestMaster_operational(CO_Data* d)
{
	eprintf("TestMaster_operational\n");
}

void TestMaster_stopped(CO_Data* d)
{
	eprintf("TestMaster_stopped\n");
}

void TestMaster_post_sync(CO_Data* d)
{
	DO++;
	
	AO1 = AI1 / 2;
	AO2 = AI2 / 2;
	AO3 = AI3 / 2;
	AO4 = AI4 / 2;
	eprintf("MicroMod Digital Out: %2.2x\n",DO);
	eprintf("MicroMod Analogue Out1: %d\n",AO1);
	eprintf("MicroMod Analogue Out2: %d\n",AO2);
	eprintf("MicroMod Analogue Out3: %d\n",AO3);
	eprintf("MicroMod Analogue Out4: %d\n",AO4);
	eprintf("MicroMod Digital In (by bit): DI1: %2.2x DI2: %2.2x DI3: %2.2x DI4: %2.2x DI5: %2.2x DI6: %2.2x DI7: %2.2x DI8: %2.2x\n",DI1,DI2,DI3,DI4,DI5,DI6,DI7,DI8);
	eprintf("MicroMod Analogue In1: %d\n", AI1);
	eprintf("MicroMod Analogue In2: %d\n", AI2);
	eprintf("MicroMod Analogue In3: %d\n", AI3);
	eprintf("MicroMod Analogue In4: %d\n", AI4);
	eprintf("MicroMod Analogue In5: %d\n", AI5);
	eprintf("MicroMod Analogue In6: %d\n", AI6);
	eprintf("MicroMod Analogue In7: %d\n", AI7);
	eprintf("MicroMod Analogue In8: %d\n", AI8);
}

void TestMaster_post_TPDO(CO_Data* d)
{
//	eprintf("TestMaster_post_TPDO\n");	
}

//s_BOARD SlaveBoard = {"0", "500K"};
s_BOARD MasterBoard = {"32", "125K"};

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
  printf("*  TestMasterMicroMod                                        *\n");
  printf("*                                                            *\n");
  printf("*  A simple example for PC.                                  *\n");
  printf("*  A CanOpen master that control a MicroMod module:          *\n");
  printf("*  - setup module TPDO 1 transmit type                       *\n");
  printf("*  - setup module RPDO 1 transmit type                       *\n");
  printf("*  - setup module hearbeatbeat period                        *\n");
  printf("*  - disable others TPDOs                                    *\n");
  printf("*  - set state to operational                                *\n");
  printf("*  - send periodic SYNC                                      *\n");
  printf("*  - send periodic RPDO 1 to Micromod (digital output)       *\n");
  printf("*  - listen Micromod's TPDO 1 (digital input)                *\n");
  printf("*  - Mapping RPDO 1 bit per bit (digital input)              *\n");
  printf("*                                                            *\n");
  printf("*   Usage:                                                   *\n");
  printf("*   ./TestMasterMicroMod  [OPTIONS]                          *\n");
  printf("*                                                            *\n");
  printf("*   OPTIONS:                                                 *\n");
  printf("*     -l : Can library [\"libcanfestival_can_virtual.so\"]     *\n");
  printf("*                                                            *\n");
  printf("*    Slave:                                                  *\n");
  printf("*     -i : Slave Node id format [0x01 , 0x7F]                *\n");
  printf("*                                                            *\n");
  printf("*    Master:                                                 *\n");
  printf("*     -m : bus name [\"1\"]                                    *\n");
  printf("*     -M : 1M,500K,250K,125K,100K,50K,20K,10K                *\n");
  printf("*                                                            *\n");
  printf("**************************************************************\n");
}

/***************************  INIT  *****************************************/
void InitNodes(CO_Data* d, UNS32 id)
{
	/****************************** INITIALISATION MASTER *******************************/
	if(MasterBoard.baudrate){
		/* Defining the node Id */
		//setNodeId(&TestMaster_Data, 0x01);

		/* init */
		setState(&TestMaster_Data, Initialisation);
	}
}

/***************************  EXIT  *****************************************/
void Exit(CO_Data* d, UNS32 id)
{
	masterSendNMTstateChange(&TestMaster_Data, 0x02, NMT_Reset_Node);

    //Stop master
	setState(&TestMaster_Data, Stopped);
}

/****************************************************************************/
/***************************  MAIN  *****************************************/
/****************************************************************************/
int main(int argc,char **argv)
{

  int c;
  extern char *optarg;
  char* LibraryPath="libcanfestival_can_virtual.so";
  char *snodeid;
  while ((c = getopt(argc, argv, "-m:s:M:S:l:i:")) != EOF)
  {
    switch(c)
    {
      case 'm' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        MasterBoard.busname = optarg;
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
      case 'i' :
        if (optarg[0] == 0)
        {
          help();
          exit(1);
        }
        snodeid = optarg;
		sscanf(snodeid,"%x",&slavenodeid);
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
	LoadCanDriver(LibraryPath);
#endif		

	TestMaster_Data.heartbeatError = TestMaster_heartbeatError;
	TestMaster_Data.initialisation = TestMaster_initialisation;
	TestMaster_Data.preOperational = TestMaster_preOperational;
	TestMaster_Data.operational = TestMaster_operational;
	TestMaster_Data.stopped = TestMaster_stopped;
	TestMaster_Data.post_sync = TestMaster_post_sync;
	TestMaster_Data.post_TPDO = TestMaster_post_TPDO;
	
	if(!canOpen(&MasterBoard,&TestMaster_Data)){
		eprintf("Cannot open Master Board\n");
		goto fail_master;
	}
	
	// Start timer thread
	StartTimerLoop(&InitNodes);

	// wait Ctrl-C
	pause();
	eprintf("Finishing.\n");
	
	// Reset the slave node for next use (will stop emitting heartbeat)
	masterSendNMTstateChange (&TestMaster_Data, slavenodeid, NMT_Reset_Node);
	
	// Stop master
	setState(&TestMaster_Data, Stopped);
	
	// Stop timer thread
	StopTimerLoop(&Exit);
	
fail_master:
	if(MasterBoard.baudrate) canClose(&TestMaster_Data);	

	TimerCleanup();
  	return 0;
}


