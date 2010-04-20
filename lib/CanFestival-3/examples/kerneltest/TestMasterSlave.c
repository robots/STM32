#include "canfestival.h"

#include "Master.h"
#include "Slave.h"
#include "TestMasterSlave.h"


static UNS32 OnMasterMap1Update(CO_Data* d, const indextable * unsused_indextable, UNS8 unsused_bSubindex)
{
	eprintf("OnSlaveMap1Update:%d\n", SlaveMap1);
	return 0;
}

s_BOARD SlaveBoard = {"0", "125K"};
s_BOARD MasterBoard = {"1", "125K"};

/***************************  INIT  *****************************************/
static void InitNodes(CO_Data* d, UNS32 id)
{
	/****************************** INITIALISATION SLAVE *******************************/
	if(strcmp(SlaveBoard.baudrate, "none")) {
		setNodeId(&TestSlave_Data, 0x02);

		setState(&TestSlave_Data, Initialisation);
	}

	/****************************** INITIALISATION MASTER *******************************/
	if(strcmp(MasterBoard.baudrate, "none")){
 		RegisterSetODentryCallBack(&TestMaster_Data, 0x2000, 0, &OnMasterMap1Update);
		
		// Defining the node Id
		setNodeId(&TestMaster_Data, 0x01);

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


int TestMasterSlave_start (void)
{
	TimerInit();

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
			return 1;
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
			if(strcmp(MasterBoard.baudrate, "none")) canClose(&TestMaster_Data);
			return 1;
		}
	}

	// Start timer thread
	StartTimerLoop(&InitNodes);

	return 0;
}

void TestMasterSlave_stop (void)
{
	eprintf("Finishing.\n");
	
	// Stop timer thread
	StopTimerLoop(&Exit);
	
	// Close CAN devices (and can threads)
	if(strcmp(SlaveBoard.baudrate, "none")) canClose(&TestSlave_Data);
	if(strcmp(MasterBoard.baudrate, "none")) canClose(&TestMaster_Data);

	TimerCleanup();
	eprintf("End.\n");
}
