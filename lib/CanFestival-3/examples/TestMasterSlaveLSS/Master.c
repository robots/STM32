/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT , Francis DUPIN and Jorge BERZOSA

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

#include "Master.h"
#include "SlaveA.h"
#include "SlaveB.h"
#include "TestMasterSlaveLSS.h" 

extern s_BOARD MasterBoard;
/*****************************************************************************/
void TestMaster_heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
	eprintf("TestMaster_heartbeatError %d\n", heartbeatID);
}

/********************************************************
 * TestMaster_initialisation is responsible to
 *  - setup master RPDO 1 to receive TPDO 1 from id 2
 *  - setup master RPDO 2 to receive TPDO 1 from id 3
 ********************************************************/
void TestMaster_initialisation(CO_Data* d)
{
	UNS32 PDO1_COBID = 0x0182; 
	UNS32 PDO2_COBID = 0x0183;
	UNS8 size = sizeof(UNS32); 
	
	eprintf("TestMaster_initialisation\n");

	/*****************************************
	 * Define RPDO to match slave ID=2 TPDO1*
	 *****************************************/
	writeLocalDict( &TestMaster_Data, /*CO_Data* d*/
			0x1400, /*UNS16 index*/
			0x01, /*UNS8 subind*/ 
			&PDO1_COBID, /*void * pSourceData,*/ 
			&size, /* UNS8 * pExpectedSize*/
			RW);  /* UNS8 checkAccess */
			
	/*****************************************
	 * Define RPDO to match slave ID=3 TPDO1*
	 *****************************************/		
	writeLocalDict( &TestMaster_Data, /*CO_Data* d*/
			0x1401, /*UNS16 index*/
			0x01, /*UNS8 subind*/ 
			&PDO2_COBID, /*void * pSourceData,*/ 
			&size, /* UNS8 * pExpectedSize*/
			RW);  /* UNS8 checkAccess */

}

// Step counts number of times ConfigureSlaveNode is called
// There is one per each slave
static init_step[] ={0,0};

/*Forward declaration*/
static void ConfigureSlaveNode(CO_Data* d, UNS8 nodeId);

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
 *  - setup slave 'n' TPDO 1 transmit type
 *  - setup slave 'n' Producer Hertbeat Time
 *  - setup the Consumer Heartbeat Time for slave 'n'
 *  - switch to operational mode
 *  - send NMT to slave
 ********************************************************
 * This an example of :
 * Network Dictionary Access (SDO) with Callback 
 * Slave node state change request (NMT) 
 ********************************************************
 * This is called first by TestMaster_post_SlaveBootup
 * after the LSS configuration has been done
 * then it called again each time a SDO exchange is
 * finished.
 ********************************************************/
 
static void ConfigureSlaveNode(CO_Data* d, UNS8 nodeId)
{
	/* Master configure heartbeat producer time at 0 ms 
	 * for slaves node-id 0x02 and 0x03 by DCF concise */
	 
	UNS8 Transmission_Type = 0x01;
	UNS16 Slave_Prod_Heartbeat_T=1000;//ms
	UNS32 Master_Cons_Heartbeat_Base=0x05DC; //1500ms
	UNS32 abortCode;
	UNS8 res;
	eprintf("Master : ConfigureSlaveNode %2.2x\n", nodeId);

	switch(++init_step[nodeId-2]){
		case 1: /*First step : setup Slave's TPDO 1 to be transmitted on SYNC*/
			eprintf("Master : set slave %2.2x TPDO 1 transmit type\n", nodeId);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					nodeId, /*UNS8 nodeId*/
					0x1800, /*UNS16 index*/
					0x02, /*UNS8 subindex*/
					1, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&Transmission_Type,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
					break;
		case 2: /* Second step : Set the new heartbeat producer time in the slave */
		{
			UNS32 Master_Cons_Heartbeat_T=Master_Cons_Heartbeat_Base + (nodeId * 0x10000);
			UNS8 size = sizeof(UNS32); 
			
			eprintf("Master : set slave %2.2x Producer Heartbeat Time = %d\n", nodeId,Slave_Prod_Heartbeat_T);
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
					nodeId, /*UNS8 nodeId*/
					0x1017, /*UNS16 index*/
					0x00, /*UNS8 subindex*/
					2, /*UNS8 count*/
					0, /*UNS8 dataType*/
					&Slave_Prod_Heartbeat_T,/*void *data*/
					CheckSDOAndContinue); /*SDOCallback_t Callback*/
					break;
					
			/* Set the new heartbeat consumer time in the master*/
			eprintf("Master : set Consumer Heartbeat Time for slave %2.2x = %d\n", nodeId,Master_Cons_Heartbeat_T);
			writeLocalDict( &TestMaster_Data, /*CO_Data* d*/
				0x1016, /*UNS16 index*/
				nodeId-1, /*UNS8 subind*/ 
				&Master_Cons_Heartbeat_T, /*void * pSourceData,*/ 
				&size, /* UNS8 * pExpectedSize*/
				RW);  /* UNS8 checkAccess */
		}		
		break;
		case 3: 
		
		/****************************** START *******************************/
		
			/* Put the master in operational mode */
			setState(d, Operational);
		 
			/* Ask slave node to go in operational mode */
			masterSendNMTstateChange (d, nodeId, NMT_Start_Node);
	}
}

static void ConfigureLSSNode(CO_Data* d);
// Step counts number of times ConfigureLSSNode is called
UNS8 init_step_LSS=1;

static void CheckLSSAndContinue(CO_Data* d, UNS8 command)
{
	UNS32 dat1;
	UNS8 dat2;
	
	printf("CheckLSS->");
	if(getConfigResultNetworkNode (d, command, &dat1, &dat2) != LSS_FINISHED){
			eprintf("Master : Failed in LSS comand %d.  Trying again\n", command);
	}
	else
	{
		init_step_LSS++;
	
		switch(command){
		case LSS_CONF_NODE_ID:
   			switch(dat1){
   				case 0: printf("Node ID change succesful\n");break;
   				case 1: printf("Node ID change error:out of range\n");break;
   				case 0xFF:printf("Node ID change error:specific error\n");break;
   				default:break;
   			}
   			break;
   		case LSS_CONF_BIT_TIMING:
   			switch(dat1){
   				case 0: printf("Baud rate change succesful\n");break;
   				case 1: printf("Baud rate change error: change baud rate not supported\n");break;
   				case 0xFF:printf("Baud rate change error:specific error\n");break;
   				default:break;
   			}
   			break;
   		case LSS_CONF_STORE:
   			switch(dat1){
   				case 0: printf("Store configuration succesful\n");break;
   				case 1: printf("Store configuration error:not supported\n");break;
   				case 0xFF:printf("Store configuration error:specific error\n");break;
   				default:break;
   			}
   			break;
   		case LSS_CONF_ACT_BIT_TIMING:
   			if(dat1==0){
   				UNS8 LSS_mode=LSS_WAITING_MODE;
				UNS32 SINC_cicle=50000;// us
				UNS8 size = sizeof(UNS32); 
	
				/* The slaves are now configured (nodeId and Baudrate) via the LSS services.
   			 	* Switch the LSS state to WAITING and restart the slaves. */
				
				/*TODO: change the baud rate of the master!!*/
   			 	MasterBoard.baudrate="250K";
   			 	
   			 	
	   			printf("Master : Switch Delay period finished. Switching to LSS WAITING state\n");
   				configNetworkNode(d,LSS_SM_GLOBAL,&LSS_mode,0,NULL);
	   			
   				printf("Master : Restarting all the slaves\n");
   				masterSendNMTstateChange (d, 0x00, NMT_Reset_Comunication);
	   			
   				printf("Master : Starting the SYNC producer\n");
   				writeLocalDict( d, /*CO_Data* d*/
					0x1006, /*UNS16 index*/
					0x00, /*UNS8 subind*/ 
					&SINC_cicle, /*void * pSourceData,*/ 
					&size, /* UNS8 * pExpectedSize*/
					RW);  /* UNS8 checkAccess */
					
				return;
			}
   			else{
   				UNS16 Switch_delay=1;
				UNS8 LSS_mode=LSS_CONFIGURATION_MODE;
				
	   			eprintf("Master : unable to activate bit timing. trying again\n");
				configNetworkNode(d,LSS_CONF_ACT_BIT_TIMING,&Switch_delay,0,CheckLSSAndContinue);
				return;
   			}
   			break;	
		case LSS_SM_SELECTIVE_SERIAL:
   			printf("Slave in LSS CONFIGURATION state\n");
   			break;
   		case LSS_IDENT_REMOTE_SERIAL_HIGH:
   			printf("node identified\n");
   			break;
   		case LSS_IDENT_REMOTE_NON_CONF:
   			if(dat1==0)
   				eprintf("There are no-configured remote slave(s) in the net\n");
   			else
   			{
   				UNS16 Switch_delay=1;
				UNS8 LSS_mode=LSS_CONFIGURATION_MODE;
			
				/*The configuration of the slaves' nodeId ended.
				 * Start the configuration of the baud rate. */
				eprintf("Master : There are not no-configured slaves in the net\n", command);
				eprintf("Switching all the nodes to LSS CONFIGURATION state\n");
				configNetworkNode(d,LSS_SM_GLOBAL,&LSS_mode,0,NULL);
				eprintf("LSS=>Activate Bit Timing\n");
				configNetworkNode(d,LSS_CONF_ACT_BIT_TIMING,&Switch_delay,0,CheckLSSAndContinue);
				return;
   			}
   			break;
   		case LSS_INQ_VENDOR_ID:
   			printf("Slave VendorID %x\n", dat1);
   			break;
   		case LSS_INQ_PRODUCT_CODE:
   			printf("Slave Product Code %x\n", dat1);
   			break;
   		case LSS_INQ_REV_NUMBER:
   			printf("Slave Revision Number %x\n", dat1);
   			break;
   		case LSS_INQ_SERIAL_NUMBER:
   			printf("Slave Serial Number %x\n", dat1);
   			break;
   		case LSS_INQ_NODE_ID:
   			printf("Slave nodeid %x\n", dat1);
   			break;
#ifdef CO_ENABLE_LSS_FS
   		case LSS_IDENT_FASTSCAN:
   			if(dat1==0)
   				printf("Slave node identified with FastScan\n");
   			else
   			{
   				printf("There is not unconfigured node in the net\n");
   				return;
   			}	
   			init_step_LSS++;
   			break;
#endif	
	
		}
	}

	printf("\n");
	ConfigureLSSNode(d);
}

/* Initial nodeID and VendorID. They are incremented by one for each slave*/
UNS8 NodeID=0x02;
UNS32 Vendor_ID=0x12345678;

/* Configuration of the nodeID and baudrate with LSS services:
 * --First ask if there is a node with an invalid nodeID.
 * --If FastScan is activated it is used to put the slave in the state “configuration”.
 * --If FastScan is not activated, identification services are used to identify the slave. Then 
 * 	 switch mode service is used to put it in configuration state.
 * --Next, all the inquire services are used (only for example) and a valid nodeId and a
 * 	 new baudrate are assigned to the slave.
 * --Finally, the slave's LSS state is restored to “waiting” and all the process is repeated 
 * 	 again until there isn't any node with an invalid nodeID.
 * --After the configuration of all the slaves finished the LSS state of all of them is switched 
 * 	 again to "configuration" and the Activate Bit Timing service is requested. On sucessfull, the 
 * 	 LSS state is restored to "waiting" and NMT state is changed to reset (by means of the NMT services).
 * */
static void ConfigureLSSNode(CO_Data* d)
{
	UNS32 Product_Code=0x90123456;
	UNS32 Revision_Number=0x78901234;
	UNS32 Serial_Number=0x56789012;
	UNS32 Revision_Number_high=0x78901240;
	UNS32 Revision_Number_low=0x78901230;
	UNS32 Serial_Number_high=0x56789020;
	UNS32 Serial_Number_low=0x56789010;
	UNS8 LSS_mode=LSS_WAITING_MODE;
	UNS8 Baud_Table=0;
	//UNS8 Baud_BitTiming=3;
	char* Baud_BitTiming="250K";
	UNS8 res;
	eprintf("ConfigureLSSNode step %d -> ",init_step_LSS);

	switch(init_step_LSS){
		case 1:	/* LSS=>identify non-configured remote slave */
			eprintf("LSS=>identify no-configured remote slave(s)\n");
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_NON_CONF,0,0,CheckLSSAndContinue);
			break;
#ifdef CO_ENABLE_LSS_FS
		case 2:	/* LSS=>FastScan */
		{
			lss_fs_transfer_t lss_fs;
			eprintf("LSS=>FastScan\n");
			/* The VendorID and ProductCode are partialy known, except the last two digits (8 bits). */
			lss_fs.FS_LSS_ID[0]=Vendor_ID;
			lss_fs.FS_BitChecked[0]=8;
			lss_fs.FS_LSS_ID[1]=Product_Code;
			lss_fs.FS_BitChecked[1]=8;
			/* serialNumber and RevisionNumber are unknown, i.e. the 8 digits (32bits) are unknown. */
			lss_fs.FS_BitChecked[2]=32;
			lss_fs.FS_BitChecked[3]=32;
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_FASTSCAN,&lss_fs,0,CheckLSSAndContinue);
		}
		break;
#else
		case 2:	/* LSS=>identify node */
			eprintf("LSS=>identify node\n");
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_VENDOR,&Vendor_ID,0,NULL);
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_PRODUCT,&Product_Code,0,NULL);
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_REV_LOW,&Revision_Number_low,0,NULL);
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_REV_HIGH,&Revision_Number_high,0,NULL);
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_SERIAL_LOW,&Serial_Number_low,0,NULL);
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_SERIAL_HIGH,&Serial_Number_high,0,CheckLSSAndContinue);
			break;
		case 3: /*LSS=>put in configuration mode*/
			eprintf("LSS=>put in configuration mode\n");
			res=configNetworkNode(&TestMaster_Data,LSS_SM_SELECTIVE_VENDOR,&Vendor_ID,0,NULL);
			res=configNetworkNode(&TestMaster_Data,LSS_SM_SELECTIVE_PRODUCT,&Product_Code,0,NULL);
			res=configNetworkNode(&TestMaster_Data,LSS_SM_SELECTIVE_REVISION,&Revision_Number,0,NULL);
			res=configNetworkNode(&TestMaster_Data,LSS_SM_SELECTIVE_SERIAL,&Serial_Number,0,CheckLSSAndContinue);
			Vendor_ID++;
			break;
#endif
		case 4:	/* LSS=>inquire nodeID */
			eprintf("LSS=>inquire nodeID\n");
			res=configNetworkNode(&TestMaster_Data,LSS_INQ_NODE_ID,0,0,CheckLSSAndContinue);
			break;
		case 5:	/* LSS=>inquire VendorID */
			eprintf("LSS=>inquire VendorID\n");
			res=configNetworkNode(&TestMaster_Data,LSS_INQ_VENDOR_ID,0,0,CheckLSSAndContinue);
			break;
		case 6:	/* LSS=>inquire Product code */
			eprintf("LSS=>inquire Product code\n");
			res=configNetworkNode(&TestMaster_Data,LSS_INQ_PRODUCT_CODE,0,0,CheckLSSAndContinue);
			break;
		case 7:	/* LSS=>inquire Revision Number */
			eprintf("LSS=>inquire Revision Number\n");
			res=configNetworkNode(&TestMaster_Data,LSS_INQ_REV_NUMBER,0,0,CheckLSSAndContinue);
			break;
		case 8:	/* LSS=>inquire Serial Number */
			eprintf("LSS=>inquire Serial Number\n");
			res=configNetworkNode(&TestMaster_Data,LSS_INQ_SERIAL_NUMBER,0,0,CheckLSSAndContinue);
			break;
		case 9:	/* LSS=>change the nodeID */
			eprintf("LSS=>change the nodeId\n");
			res=configNetworkNode(&TestMaster_Data,LSS_CONF_NODE_ID,&NodeID,0,CheckLSSAndContinue);
			NodeID++;
			break;
		case 10:	/* LSS=>change the Baud rate */
			eprintf("LSS=>change the Baud rate\n");
			res=configNetworkNode(&TestMaster_Data,LSS_CONF_BIT_TIMING,&Baud_Table,&Baud_BitTiming,CheckLSSAndContinue);
			break;
		case 11:
			/*LSS=>store configuration*/
			eprintf("LSS=>store configuration\n");
			res=configNetworkNode(&TestMaster_Data,LSS_CONF_STORE,0,0,CheckLSSAndContinue);
			break;
		case 12: /* LSS=>put in waiting mode */
			eprintf("LSS=>put in waiting mode\n");
			res=configNetworkNode(&TestMaster_Data,LSS_SM_GLOBAL,&LSS_mode,0,NULL);
			/* Search again for no-configured slaves*/
			eprintf("LSS=>identify no-configured remote slave(s)\n");
			res=configNetworkNode(&TestMaster_Data,LSS_IDENT_REMOTE_NON_CONF,0,0,CheckLSSAndContinue);
			init_step_LSS=1;
			break;
	}
}

void TestMaster_preOperational(CO_Data* d)
{
	eprintf("TestMaster_preOperational\n");

	/* Ask slaves to go in stop mode */
	masterSendNMTstateChange (d, 0, NMT_Stop_Node);
	ConfigureLSSNode(&TestMaster_Data);
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
	eprintf("TestMaster_post_sync\n");
	eprintf("Master: %d %d %d\n",
		MasterMap1,
		MasterMap2,
		MasterMap3);
}

void TestMaster_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg)
{
	eprintf("Master received EMCY message. Node: %2.2x  ErrorCode: %4.4x  ErrorRegister: %2.2x\n", nodeID, errCode, errReg);
}

void TestMaster_post_TPDO(CO_Data* d)
{
	eprintf("TestMaster_post_TPDO\n");
}

void TestMaster_post_SlaveBootup(CO_Data* d, UNS8 nodeid)
{
	eprintf("TestMaster_post_SlaveBootup %x\n", nodeid);
	/* Wait until the new baud rate is stored before configure the slaves*/
	if(MasterBoard.baudrate=="250K")
		ConfigureSlaveNode(d, nodeid);
}

