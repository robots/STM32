/*
Copyright (C): Giuseppe Massimo BERTANI
gmbertani@users.sourceforge.net


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

/**
 * @file slave.c
 * @author GMB
 * @date 17/1/08
 *
 * This file is part of SillySlave demo for CANfestival
 * open source CANopen stack.
 */ 

#include "SillySlave.h"
#include "slave.h"
#include "main.h"


static UNS8 slaveNode = 0;  

void InitNode(CO_Data* d, UNS32 id)
{
    /* Defining the node Id */
    setNodeId(&SillySlave_Data, slaveNode);
    /* CAN init */
    setState(&SillySlave_Data, Initialisation);
}

void Exit(CO_Data* d, UNS32 id)
{
	/* Stop slave */
    setState(&SillySlave_Data, Stopped);
}

INTEGER8 InitCANdevice( UNS8 bus, UNS32 baudrate, UNS8 node )
{ 
char busName[2];
char baudRate[7];
s_BOARD board;

    sprintf(busName, "%u", bus);
    sprintf(baudRate, "%u", baudrate);
    board.busname = busName;
    board.baudrate = baudRate;

    slaveNode = node;

    SillySlave_Data.heartbeatError = SillySlave_heartbeatError;
    SillySlave_Data.initialisation = SillySlave_initialisation;
    SillySlave_Data.preOperational = SillySlave_preOperational;
    SillySlave_Data.operational = SillySlave_operational;
    SillySlave_Data.stopped = SillySlave_stopped;
    SillySlave_Data.post_sync = SillySlave_post_sync;
    SillySlave_Data.post_TPDO = SillySlave_post_TPDO;
    SillySlave_Data.storeODSubIndex = SillySlave_storeODSubIndex;
    SillySlave_Data.post_emcy = SillySlave_post_emcy;
    
    if(!canOpen(&board, &SillySlave_Data))
    {
        printf("\n\aInitCANdevice() CAN bus %s opening error, baudrate=%s\n",board.busname, board.baudrate);
        return -1;
    }


    printf("\nInitCANdevice(), canOpen() OK, starting timer loop...\n");

    /* Start timer thread */
    StartTimerLoop(&InitNode); 
    
	/* wait Ctrl-C */
	pause();
	printf("\nFinishing.\n");
	
	/* Stop timer thread */
	StopTimerLoop(&Exit);
    return 0;
}

void SillySlave_heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
    printf("SillySlave_heartbeatError %d\n", heartbeatID);
}

void SillySlave_initialisation(CO_Data* d )
{
    UNS32 PDO1_COBID = 0x0180 + NODE_MASTER; 
    UNS8 size = sizeof(PDO1_COBID); 
    
    printf("SillySlave_initialisation\n");

    /* sets TXPDO1 COB-ID to match master node ID */
    writeLocalDict( 
            &SillySlave_Data,       /*CO_Data* d*/
            0x1800,                 /*UNS16 index*/
            0x01,                   /*UNS8 subind*/ 
            &PDO1_COBID,            /*void * pSourceData,*/ 
            &size,                  /* UNS8 * pExpectedSize*/
            RW);                    /* UNS8 checkAccess */
            
    /* value sent to master at each SYNC received */
    LifeSignal = 0;
}

void SillySlave_preOperational(CO_Data* d)
{
    printf("SillySlave_preOperational\n");
}

void SillySlave_operational(CO_Data* d)
{
    printf("SillySlave_operational\n");
}

void SillySlave_stopped(CO_Data* d)
{
    printf("SillySlave_stopped\n");
}

void SillySlave_post_sync(CO_Data* d)
{
    printf("SillySlave_post_sync: \n");
    LifeSignal++;
}

void SillySlave_post_TPDO(CO_Data* d)
{
    printf("SillySlave_post_TPDO: \n");
    printf("LifeSignal = %u\n", LifeSignal);    
}

void SillySlave_storeODSubIndex(CO_Data* d, UNS16 wIndex, UNS8 bSubindex)
{
    /*TODO : 
     * - call getODEntry for index and subindex, 
     * - save content to file, database, flash, nvram, ...
     * 
     * To ease flash organisation, index of variable to store
     * can be established by scanning d->objdict[d->ObjdictSize]
     * for variables to store.
     * 
     * */
    printf("SillySlave_storeODSubIndex : %4.4x %2.2xh\n", wIndex,  bSubindex);
}

void SillySlave_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg)
{
    printf("Slave received EMCY message. Node: %2.2xh  ErrorCode: %4.4x  ErrorRegister: %2.2xh\n", nodeID, errCode, errReg);
}

