/*
This file is part of CanFestival, a library implementing CanOpen Stack. 


Copyright (C): Jorge Berzosa


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


/*!
** @file   lss.c
** @author Jorge Berzosa
** @date   Mon Oct  22 05:44:32 2007
**
** @brief
**
**
*/


#include "data.h"
#include "lss.h"
#include "canfestival.h"
#include "sysdep.h"

#ifdef CO_ENABLE_LSS

//#define LSS_TIMEOUT_MS	(TIMEVAL)1000  /* ms */
//#define LSS_FS_TIMEOUT_MS	(TIMEVAL)100  /* ms */

/* Returns the LSS ident field from a Message struct */
#define getLSSIdent(msg) (((UNS32)msg->data[4] << 24) | ((UNS32)msg->data[3] << 16) | (msg->data[2] << 8) | (msg->data[1]))


/* Returns the LSS switch delay field from a Message struct */
#define getLSSDelay(msg) ((msg->data[2] << 8) | (msg->data[1]))

/* Returns the LSS FastScan BitCheck field from a Message struct */
#define getLSSBitCheck(msg) msg->data[5]

/* Returns the LSS FastScan LSSSub field from a Message struct */
#define getLSSSub(msg) msg->data[6]

/* Returns the LSS FastScan LSSNext field from a Message struct */
#define getLSSNext(msg) msg->data[7]

/* Prototypes for internals functions */
UNS8 sendMasterLSSMessage(CO_Data* d, UNS8 command,void *dat1,void *dat2);
void LssAlarmMSG(CO_Data* d, UNS32 id);
void LssAlarmSDELAY(CO_Data* d, UNS32 id);


#define StopLSS_MSG_TIMER(){\
 MSG_WAR(0x3D01, "StopLSS_MSG_TIMER", 0);\
 d->lss_transfer.timerMSG = DelAlarm(d->lss_transfer.timerMSG);}

#define StartLSS_MSG_TIMER(){\
 MSG_WAR(0x3D02, "StartLSS_MSG_TIMER",0);\
 d->lss_transfer.timerMSG = SetAlarm(d,0,&LssAlarmMSG,MS_TO_TIMEVAL(LSS_TIMEOUT_MS),0);}
 
#define StopLSS_SDELAY_TIMER(){\
 MSG_WAR(0x3D03, "StopLSS_SDELAY_TIMER", 0);\
 d->lss_transfer.timerSDELAY = DelAlarm(d->lss_transfer.timerSDELAY);}

#define StartLSS_SDELAY_TIMER(){\
 MSG_WAR(0x3D04, "StartLSS_SDELAY_TIMER",0);\
 d->lss_transfer.timerSDELAY= SetAlarm(d,0,&LssAlarmSDELAY,MS_TO_TIMEVAL(d->lss_transfer.switchDelay),MS_TO_TIMEVAL(d->lss_transfer.switchDelay));}


#ifdef CO_ENABLE_LSS_FS
/* Prototypes for internals functions */
void LssAlarmFS(CO_Data* d, UNS32 id);

#define StopLSS_FS_TIMER(){\
 MSG_WAR(0x3D05, "StopLSS_FS_TIMER", id);\
 d->lss_transfer.timerFS = DelAlarm(d->lss_transfer.timerFS);}

#define StartLSS_FS_TIMER(){\
 MSG_WAR(0x3D06, "StartLSS_FS_TIMER",0);\
 d->lss_transfer.timerFS = SetAlarm(d,0,&LssAlarmFS,MS_TO_TIMEVAL(LSS_FS_TIMEOUT_MS),0);}
#endif


void LssAlarmMSG(CO_Data* d, UNS32 id)
{	
 	StopLSS_MSG_TIMER();
#ifdef CO_ENABLE_LSS_FS
	if(d->lss_transfer.command==LSS_IDENT_FASTSCAN){
		if(d->lss_transfer.FastScan_SM==LSS_FS_RESET){
   			/* if at least one node had answered before the timer expired, start the FastScan protocol*/
   			if(d->lss_transfer.LSSanswer!=0){
   				UNS32 Mask=0xFFFFFFFF;
   				d->lss_transfer.LSSanswer=0;
   				d->lss_transfer.BitChecked=d->lss_transfer.lss_fs_transfer.FS_BitChecked[0];
   				Mask=(UNS32)((UNS64)Mask<<(d->lss_transfer.BitChecked+1));
   				d->lss_transfer.IDNumber=d->lss_transfer.lss_fs_transfer.FS_LSS_ID[0] & Mask;
   				d->lss_transfer.FastScan_SM=LSS_FS_PROCESSING;
   				//printf("BitChecked=%d, IDNumber=%x MASK=%x\n",d->lss_transfer.BitChecked,d->lss_transfer.IDNumber,Mask);
   				StartLSS_FS_TIMER();
   				sendMasterLSSMessage(d,LSS_IDENT_FASTSCAN,0,0);
   				return;
   			}
   			else{ 
   			
    			d->lss_transfer.state = LSS_FINISHED;
    			/* Inform the application that there aren't not configured nodes in the net  */
    			d->lss_transfer.dat1=1;
   			}
   		}
   		else{
			/* This should not happen, an error ocurred*/
			MSG_ERR(0x1D07, "LSS FastScan timeout. FastScan_SM inconsisten state.", d->lss_transfer.FastScan_SM);
			d->lss_transfer.state = LSS_ABORTED_INTERNAL;
			d->lss_transfer.FastScan_SM=LSS_FS_RESET;
   		}
	}
	else
#endif
	if(d->lss_transfer.command==LSS_IDENT_REMOTE_NON_CONF){
			MSG_WAR(0x2D08, "LSS timeout. There are not no-configured slaves in the net", 0);
			d->lss_transfer.state = LSS_FINISHED;
			d->lss_transfer.dat1=1;
	}
	else{
		MSG_ERR(0x1D09, "LSS timeout. LSS response not received.", 0);
    	MSG_WAR(0x2D0A, "LSS timeout command specifier : ", d->lss_transfer.command);
    	/* Set aborted state */
    	d->lss_transfer.state = LSS_ABORTED_INTERNAL;
#ifdef CO_ENABLE_LSS_FS
    	d->lss_transfer.FastScan_SM = LSS_FS_RESET;
#endif
    }
    	
    /* Call the user function to inform of the problem.*/
    if(d->lss_transfer.Callback){
	   	/*If there is a callback, it is responsible of the error*/
    	(*d->lss_transfer.Callback)(d,d->lss_transfer.command);
    }
}
    
    
/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param id                                                                                       
**/   
void LssAlarmSDELAY(CO_Data* d, UNS32 id)
{	
	
	/* The first switch_delay period expired. Store the node state, change it 
 	 * so no CAN messages will be sent or received, call the ChangeBaudRate function*/
   	if(d->lss_transfer.switchDelayState==SDELAY_FIRST){
   		MSG_WAR(0x3D0B, "LSS switch delay first period expired",0);
    	d->lss_transfer.switchDelayState=SDELAY_SECOND;
    	//(*d->lss_ChangeBaudRate)(d,d->lss_transfer.baudRate);
    	canChangeBaudRate(d->lss_transfer.canHandle_t, d->lss_transfer.baudRate);
    }
    else{ /* d->lss_transfer.switchDelayState==SDELAY_SECOND */
    	MSG_WAR(0x3D0C, "LSS switch delay second period expired",0);
    	d->lss_transfer.switchDelayState=SDELAY_OFF;
    	StopLSS_SDELAY_TIMER();
    		
    	if (*(d->iam_a_slave))
    		d->canHandle=d->lss_transfer.canHandle_t;
    	else{
    		d->lss_transfer.dat1=0;
    		d->lss_transfer.state=LSS_FINISHED;
    		/* Call the user function */
    		if(d->lss_transfer.Callback){
	    		(*d->lss_transfer.Callback)(d,d->lss_transfer.command);
    		}
    	}
    }
} 

#ifdef CO_ENABLE_LSS_FS
/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param id                                                                                       
**/   
void LssAlarmFS(CO_Data* d, UNS32 id)
{	
	StopLSS_FS_TIMER();
		
	switch(d->lss_transfer.FastScan_SM){
   	case LSS_FS_RESET:
   	{
   	   	/* This should not happen, an error ocurred*/
		MSG_ERR(0x1D0D, "LSS FastScan timeout. FastScan_SM inconsisten state.", d->lss_transfer.FastScan_SM);
   	}
   	break;
	case LSS_FS_PROCESSING:
	{
		/* If there isn't any answer, set the bit */
		if(d->lss_transfer.LSSanswer==0){
			UNS32 Mask=0x1;
			Mask<<=d->lss_transfer.BitChecked;
			d->lss_transfer.IDNumber|=Mask;
		}
		
		if(d->lss_transfer.BitChecked==0){
			/* We finished with the current LSS-ID[sub], confirm it */
			d->lss_transfer.FastScan_SM=LSS_FS_CONFIRMATION;
			if(d->lss_transfer.LSSNext<3)d->lss_transfer.LSSNext++;
		}
		else{
			d->lss_transfer.BitChecked--;
		}
		//printf("BitChecked=%d, IDNumber=%x\n",d->lss_transfer.BitChecked,d->lss_transfer.IDNumber);
   		d->lss_transfer.LSSanswer=0;
  		StartLSS_FS_TIMER();
   		sendMasterLSSMessage(d,LSS_IDENT_FASTSCAN,0,0);
   		return;
   	}
	break;
	case LSS_FS_CONFIRMATION:
	{
		if(d->lss_transfer.LSSanswer!=0){
			d->lss_transfer.LSSanswer=0;
			
			if(d->lss_transfer.LSSSub==3){
				/* The LSS FastScan protocol finished correctly. Restore the parameters */
				d->lss_transfer.BitChecked=128;
				d->lss_transfer.FastScan_SM=LSS_FS_RESET;
				d->lss_transfer.LSSSub=0;
				d->lss_transfer.LSSNext=0;
   				d->lss_transfer.IDNumber=0;
				
				/* Inform the application that the FastScan finished correctly */
				d->lss_transfer.state = LSS_FINISHED;
				d->lss_transfer.dat1=0;
			}
			else{
				UNS32 Mask=0xFFFFFFFF;
				/* Start with the next LSS-ID[sub] */
				d->lss_transfer.LSSSub++;
				d->lss_transfer.BitChecked=d->lss_transfer.lss_fs_transfer.FS_BitChecked[d->lss_transfer.LSSSub];
				Mask=(UNS32)((UNS64)Mask<<(d->lss_transfer.BitChecked+1));
   				d->lss_transfer.IDNumber=d->lss_transfer.lss_fs_transfer.FS_LSS_ID[d->lss_transfer.LSSSub] & Mask;
   				d->lss_transfer.FastScan_SM=LSS_FS_PROCESSING;
   				//printf("BitChecked=%d, IDNumber=%x MASK=%x\n",d->lss_transfer.BitChecked,d->lss_transfer.IDNumber,Mask);
   				StartLSS_FS_TIMER();
   				sendMasterLSSMessage(d,LSS_IDENT_FASTSCAN,0,0);
   				return;
			}
		}
		else{
			/* This should not happen, an error ocurred*/
			MSG_ERR(0x1D0E, "LSS FastScan timeout. FastScan response not received.", 0);
			MSG_ERR(0x1D0E, "There is not any node with LSS_ID# =>", d->lss_transfer.LSSSub);
			MSG_ERR(0x1D0E, "with the value =>", d->lss_transfer.IDNumber);
			/* Set aborted state */
    		d->lss_transfer.state = LSS_ABORTED_INTERNAL;
    		d->lss_transfer.FastScan_SM = LSS_FS_RESET;
		}
	}
	break;
	}

   	/* Call the user function to inform of the problem.*/
   	if(d->lss_transfer.Callback){
    	/*If there is a callback, it is responsible of the error*/
   		(*d->lss_transfer.Callback)(d,d->lss_transfer.command);
   	}
}
#endif

	
/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
**/ 
void startLSS(CO_Data* d)
{
	/*MSG_WAR(0x3D09, "LSS services started",0);*/
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
**/   
void stopLSS(CO_Data* d)
{
	/*MSG_WAR(0x3D09, "LSS services stopped",0);*/
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param cob_id                                                                                   
**                                                                                                 
** @return                                                                                         
**/  
UNS8 sendSlaveLSSMessage(CO_Data* d, UNS8 command,void *dat1,void *dat2)
{
  Message m;
  UNS8 i;
  
  if (!d->CurrentCommunicationState.csLSS){
  	MSG_WAR(0x2D17, "unable to send the LSS message, not in the proper state =>", d->nodeState);
  	return 0xFF;
  }
   
  for(i=1;i<8;i++)m.data[i]=0;
  m.len = 8;
  m.rtr = NOT_A_REQUEST;
  m.data[0]=command;
  m.cob_id=UNS16_LE(SLSS_ADRESS);
  
  /* Tha data sent with the msg depends on the command */
  switch(command){
  case LSS_INQ_NODE_ID: /* Inquire Node-ID */
  	m.data[1]=*(UNS8 *)dat1;
  	break;
  case LSS_CONF_NODE_ID: /* Configure Node-ID */
  case LSS_CONF_BIT_TIMING: /* Configure Bit Timing Parameters */
  case LSS_CONF_STORE: /* Store Configured Parameters */
  	m.data[1]=*(UNS8 *)dat1;
  	m.data[2]=*(UNS8 *)dat2;
  	break; 
  case LSS_INQ_VENDOR_ID: /* Inquire Identity Vendor-ID */
  case LSS_INQ_PRODUCT_CODE: /* Inquire Identity Product-Code */
  case LSS_INQ_REV_NUMBER: /* Inquire Identity Revision-Number */
  case LSS_INQ_SERIAL_NUMBER: /* Inquire Identity Serial-Number */
	m.data[1]=(UNS8)(*(UNS32*)dat1 & 0xFF);
	m.data[2]=(UNS8)(*(UNS32*)dat1>>8 & 0xFF);
	m.data[3]=(UNS8)(*(UNS32*)dat1>>16 & 0xFF);
	m.data[4]=(UNS8)(*(UNS32*)dat1>>24 & 0xFF);
	break;
  case LSS_SM_SELECTIVE_RESP: /* Switch Mode Selective response*/
  case LSS_IDENT_SLAVE: /* LSS Identify Slave */
  case LSS_IDENT_NON_CONF_SLAVE: /* LSS identify non-configured remote slave */
  	break;
  default:
  	MSG_ERR(0x1D18, "send Slave LSS command not implemented", command);
  	return 0xFF;
  }
  
  return canSend(d->canHandle,&m);
}
			
/* If a baud rate is not supported just comment the line. */
static UNS8 CO_TranslateBaudRate(char* optarg){
	if(!strcmp( optarg, "1M")) return 0x00;
	if(!strcmp( optarg, "800K")) return 0x01;
	if(!strcmp( optarg, "500K")) return 0x02;
	if(!strcmp( optarg, "250K")) return 0x03;
	if(!strcmp( optarg, "125K")) return 0x04;
	if(!strcmp( optarg, "100K")) return 0x05;
	if(!strcmp( optarg, "50K")) return 0x06;
	if(!strcmp( optarg, "20K")) return 0x07;
	if(!strcmp( optarg, "10K")) return 0x08;
	return 0xFF;
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param cob_id                                                                                   
**                                                                                                 
** @return                                                                                         
**/  
UNS8 sendMasterLSSMessage(CO_Data* d, UNS8 command,void *dat1,void *dat2)
{
  Message m;
  UNS8 i;
  UNS8 res;
  UNS8 hasResponse=0;
   
  for(i=1;i<8;i++)m.data[i]=0;
  m.len = 8;
  m.rtr = NOT_A_REQUEST;
  m.data[0]=command;
  m.cob_id=UNS16_LE(MLSS_ADRESS);
  
  /* Tha data sent with the msg depends on the command */	
  switch(command){
  case LSS_CONF_NODE_ID: /* Configure Node-ID */
  	hasResponse=1;
  case LSS_SM_GLOBAL: /* Switch Mode Global */
  	m.data[1]=*(UNS8 *)dat1;
  	break;
  case LSS_CONF_BIT_TIMING: /* Configure Bit Timing Parameters */
  	
  	m.data[1]=*(UNS8 *)dat1;
  	d->lss_transfer.baudRate=*(char **)dat2;
  	
  	if((m.data[2]=CO_TranslateBaudRate(d->lss_transfer.baudRate))!=0xFF){
  		hasResponse=1;
		break;	 
  	}
  		
	MSG_ERR(0x1D19, "Master-> Baud rate not supported",0);
	d->lss_transfer.dat1=0xFF;
	
	/* if bit timing is not supported comment the previous code and uncomment the following one*/
	/*{
		MSG_ERR(0x1D1A, "Master-> Bit timing not supported",0);
		d->lss_transfer.dat1=0x01;
	}*/
	
	d->lss_transfer.dat2=0;
 	/* If there is a callback, it is responsible of the error */
	if(d->lss_transfer.Callback)
	   	(*d->lss_transfer.Callback)(d,d->lss_transfer.command);
	return 0xFF;
  	//break;
  case LSS_CONF_ACT_BIT_TIMING: /* Activate Bit Timing Parameters */
	m.data[1]=(UNS8)(*(UNS32*)dat1 & 0xFF);
	m.data[2]=(UNS8)(*(UNS32*)dat1>>8 & 0xFF);
	if(d->lss_transfer.baudRate!="none"){
		d->lss_transfer.switchDelay=(UNS16)(*(UNS32*)dat1 & 0xFFFF);
		d->lss_transfer.switchDelayState=SDELAY_FIRST;
		d->lss_transfer.canHandle_t=d->canHandle;
		res=canSend(d->canHandle,&m);
  		if(res==0){
  			StartLSS_SDELAY_TIMER();
  			d->lss_transfer.state=LSS_TRANS_IN_PROGRESS;
  		}
  		return res;	
	}
	else{
		MSG_ERR(0x1D1B, "Master-> Baud rate not specified",0);
		d->lss_transfer.dat1=1;
    	/* If there is a callback, it is responsible of the error */
    	if(d->lss_transfer.Callback){
	    	(*d->lss_transfer.Callback)(d,d->lss_transfer.command);
    	}
		return 0xFF;
	}
  	//break;
  case LSS_SM_SELECTIVE_SERIAL:
  case LSS_IDENT_REMOTE_SERIAL_HIGH:
  	hasResponse=1;
  case LSS_SM_SELECTIVE_VENDOR: /* Switch Mode Selective */
  case LSS_SM_SELECTIVE_PRODUCT:
  case LSS_SM_SELECTIVE_REVISION:
  case LSS_IDENT_REMOTE_VENDOR: /* LSS Identify Remote Slaves */
  case LSS_IDENT_REMOTE_PRODUCT:
  case LSS_IDENT_REMOTE_REV_LOW:
  case LSS_IDENT_REMOTE_REV_HIGH:
  case LSS_IDENT_REMOTE_SERIAL_LOW:
	m.data[1]=(UNS8)(*(UNS32*)dat1 & 0xFF);
	m.data[2]=(UNS8)(*(UNS32*)dat1>>8 & 0xFF);
	m.data[3]=(UNS8)(*(UNS32*)dat1>>16 & 0xFF);
	m.data[4]=(UNS8)(*(UNS32*)dat1>>24 & 0xFF);
	break;
	
  case LSS_CONF_STORE: /* Store Configured Parameters */
  case LSS_IDENT_REMOTE_NON_CONF: /* LSS identify non-configured remote slave */
  case LSS_INQ_VENDOR_ID: /* Inquire Identity Vendor-ID */
  case LSS_INQ_PRODUCT_CODE: /* Inquire Identity Product-Code */
  case LSS_INQ_REV_NUMBER: /* Inquire Identity Revision-Number */
  case LSS_INQ_SERIAL_NUMBER: /* Inquire Identity Serial-Number */
  case LSS_INQ_NODE_ID: /* Inquire Node-ID */
	 hasResponse=1;
  	break;
#ifdef CO_ENABLE_LSS_FS
  case LSS_IDENT_FASTSCAN:
	  	if(d->lss_transfer.FastScan_SM==LSS_FS_RESET){
	  		UNS8 i;
	  		 /* Initialize the lss_fs_transfer FastScan parameters */
	  		for(i=0;i<4;i++){
	  			d->lss_transfer.lss_fs_transfer.FS_LSS_ID[i]=(*(lss_fs_transfer_t*)dat1).FS_LSS_ID[i];
	  			d->lss_transfer.lss_fs_transfer.FS_BitChecked[i]=(*(lss_fs_transfer_t*)dat1).FS_BitChecked[i];
	  			/* Adjust BitChecked from 32-1 to 31-0 */
	  			if(d->lss_transfer.lss_fs_transfer.FS_BitChecked[i]>0)d->lss_transfer.lss_fs_transfer.FS_BitChecked[i]--;
	  		}
	  		
	  		d->lss_transfer.IDNumber=0;
	  		d->lss_transfer.BitChecked=128;
	  		d->lss_transfer.LSSSub=0;
	  		d->lss_transfer.LSSNext=0;
	  				
	  		/* it will generate a response only if it is the start of the FastScan protocol*/
	  		hasResponse=1;
	  	}
		m.data[1]=(UNS8)(d->lss_transfer.IDNumber & 0xFF);
		m.data[2]=(UNS8)(d->lss_transfer.IDNumber>>8 & 0xFF);
		m.data[3]=(UNS8)(d->lss_transfer.IDNumber>>16 & 0xFF);
		m.data[4]=(UNS8)(d->lss_transfer.IDNumber>>24 & 0xFF);
		m.data[5]=d->lss_transfer.BitChecked;
		m.data[6]=d->lss_transfer.LSSSub;
		m.data[7]=d->lss_transfer.LSSNext;
	break;
#endif
  default:
   	MSG_ERR(0x1D1C, "send Master LSS command not implemented", command);
  	return 0xFF;
  }
	
  res=canSend(d->canHandle,&m);
  if(res==0 && hasResponse==1){
  	StartLSS_MSG_TIMER();
  	d->lss_transfer.state=LSS_TRANS_IN_PROGRESS;
  }
  return res;
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param cob_id                                                                                   
**                                                                                                 
** @return                                                                                         
**/  
UNS8 sendLSS(CO_Data* d, UNS8 command,void *dat1,void *dat2)
{
  UNS8 res=1;
  
  /* Tha data sent with the msg depends on the command and if the sender is a master or a slave */
  if (*(d->iam_a_slave)){ 
  	res = sendSlaveLSSMessage(d, command,dat1,dat2);
  }
  else {/* It is a Master */
  	res = sendMasterLSSMessage(d, command,dat1,dat2);
  }
  return res ;
}


/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param m                                                                                        
**                                                                                                 
** @return                                                                                         
**/ 
UNS8 proceedLSS_Master(CO_Data* d, Message* m )
{ 
	UNS8 msg_cs;
	UNS32 Dat1=0;
	UNS8 Dat2=0;
	
	if(d->lss_transfer.state!=LSS_TRANS_IN_PROGRESS)
	{
		//MSG_WAR(0x3D0D, "MasterLSS proceedLSS; unexpected message arrived;command ", m->data[0]);
		//return 0;
		goto ErrorProcessMaster;
	}
	
 	MSG_WAR(0x3D1E, "MasterLSS proceedLSS; command ", m->data[0]);
	
   	switch(msg_cs=m->data[0]){
   		case LSS_INQ_NODE_ID: /* Inquire Node-ID */
   			if(d->lss_transfer.command!=LSS_INQ_NODE_ID)goto ErrorProcessMaster;
   			Dat1=m->data[1];
   			break;
   		case LSS_CONF_NODE_ID: /* Configure Node-ID */
   		case LSS_CONF_BIT_TIMING: /* Configure Bit Timing Parameters */
   		case LSS_CONF_STORE: /* Store Configured Parameters */
   			if(d->lss_transfer.command!=msg_cs)goto ErrorProcessMaster;
   			Dat1=m->data[1];
   			Dat2=m->data[2];
   			break;
   		case LSS_INQ_VENDOR_ID: /* Inquire Identity Vendor-ID */
   		case LSS_INQ_PRODUCT_CODE: /* Inquire Identity Product-Code */
   		case LSS_INQ_REV_NUMBER: /* Inquire Identity Revision-Number */
   		case LSS_INQ_SERIAL_NUMBER: /* Inquire Identity Serial-Number */
   			if(d->lss_transfer.command!=msg_cs)goto ErrorProcessMaster;
   			Dat1=getLSSIdent(m);
 			break;
 		case LSS_IDENT_SLAVE: /* LSS Identify Slave */
#ifdef CO_ENABLE_LSS_FS
   			if(d->lss_transfer.command==LSS_IDENT_FASTSCAN){
   				/* A message arrived during the timer period */
   				d->lss_transfer.LSSanswer=1;
   				return 0;
  			}
  			else
#endif
			if(d->lss_transfer.command!=LSS_IDENT_REMOTE_VENDOR && \
 				d->lss_transfer.command!=LSS_IDENT_REMOTE_PRODUCT && \
 				d->lss_transfer.command!=LSS_IDENT_REMOTE_REV_LOW && \
 				d->lss_transfer.command!=LSS_IDENT_REMOTE_REV_HIGH && \
 				d->lss_transfer.command!=LSS_IDENT_REMOTE_SERIAL_LOW && \
 				d->lss_transfer.command!=LSS_IDENT_REMOTE_SERIAL_HIGH )
 					goto ErrorProcessMaster;
  		break;
 		case LSS_SM_SELECTIVE_RESP: /* Switch Mode Selective response */
 			if(d->lss_transfer.command!=LSS_SM_SELECTIVE_VENDOR && \
 				d->lss_transfer.command!=LSS_SM_SELECTIVE_PRODUCT && \
 				d->lss_transfer.command!=LSS_SM_SELECTIVE_REVISION && \
 				d->lss_transfer.command!=LSS_SM_SELECTIVE_SERIAL )
 					goto ErrorProcessMaster;
 			break;
   		case LSS_IDENT_NON_CONF_SLAVE: /* LSS identify non-configured remote slave */
   			if(d->lss_transfer.command!=LSS_IDENT_REMOTE_NON_CONF)goto ErrorProcessMaster;
   			break;
   		default:
   			MSG_ERR(0x1D1F, "Master LSS command not implemented", msg_cs);
  			return 0xFF;
   	}
	
	StopLSS_MSG_TIMER();
    d->lss_transfer.state = LSS_FINISHED;
    	
	d->lss_transfer.dat1=Dat1;
	d->lss_transfer.dat2=Dat2;
 	/* If there is a callback, it is responsible of the received response */
	if(d->lss_transfer.Callback)
    	(*d->lss_transfer.Callback)(d,d->lss_transfer.command);
    			
   return 0;
   
ErrorProcessMaster:
    MSG_WAR(0x3D20, "MasterLSS proceedLSS; unexpected message arrived;command ", m->data[0]);
	return 0xFF;
		
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param d                                                                                        
** @param m                                                                                        
**                                                                                                 
** @return                                                                                         
**/ 
UNS8 proceedLSS_Slave(CO_Data* d, Message* m )
{  
	UNS8 msg_cs;
	
  	MSG_WAR(0x3D21, "SlaveLSS proceedLSS; command ", m->data[0]);
  	
   	switch(msg_cs=m->data[0]){
   	case LSS_SM_GLOBAL:		/* Switch Mode Global */
   		/* if there is not a mode change break*/
   		if(m->data[1] == d->lss_transfer.mode){
   			MSG_WAR(0x3D22, "SlaveLSS already in the mode ", m->data[1]);
   			break;
   		}
   		
		if(m->data[1]==LSS_CONFIGURATION_MODE)	{
			MSG_WAR(0x3D23, "SlaveLSS switching to configuration mode ", 0);
			/* Store the NodeId in case it will be changed */
			//d->lss_transfer.nodeID=getNodeId(d);
			d->lss_transfer.mode=LSS_CONFIGURATION_MODE;
		}
		else if(m->data[1]==LSS_WAITING_MODE){
			MSG_WAR(0x3D24, "SlaveLSS switching to operational mode ", 0);
			
			/* If the nodeID has changed update it and put the node state to Initialisation. */
			if(d->lss_transfer.nodeID!=getNodeId(d)){
				if(getNodeId(d)==0xFF){/* The nodeID was 0xFF; initialize the application*/
					MSG_WAR(0x3D25, "The node Id has changed. Reseting to Initialisation state",0);
					setNodeId(d, d->lss_transfer.nodeID);
					setState(d, Initialisation);
				}
				else{/* The nodeID will be changed on NMT_Reset_Comunication Request*/
				}
			}
			d->lss_transfer.mode=LSS_WAITING_MODE;
		}
	break;
	case LSS_CONF_NODE_ID: /* Configure Node-ID */
	{ 
		UNS8 error_code=0;
		UNS8 spec_error=0;
			
		if(d->lss_transfer.mode==LSS_CONFIGURATION_MODE){
			if(m->data[1]>127 && m->data[1]!=0xFF){
				MSG_ERR(0x1D26, "NodeID out of range",0);
				error_code=1; /* NodeID out of range */
			}
			else{
				d->lss_transfer.nodeID=m->data[1];
			}
		}
		else{
			MSG_WAR(0x3D27, "SlaveLSS not in configuration mode",0);
			//error_code=0xFF;
			break;
		}
		sendSlaveLSSMessage(d,msg_cs,&error_code,&spec_error);
	}	
	break;
	case LSS_CONF_BIT_TIMING: /* Configure Bit Timing Parameters */
	{
		UNS8 error_code=0;
		UNS8 spec_error=0;
			
		if(d->lss_transfer.mode==LSS_CONFIGURATION_MODE){
			/* If a baud rate is not supported just comment the line. */
			switch(m->data[2]){
			case 0x00:d->lss_transfer.baudRate="1M";break;
			case 0x01:d->lss_transfer.baudRate="800K";break;
			case 0x02:d->lss_transfer.baudRate="500K";break;
			case 0x03:d->lss_transfer.baudRate="250K";break;
			case 0x04:d->lss_transfer.baudRate="125K";break;
			case 0x05:d->lss_transfer.baudRate="100K";break;
			case 0x06:d->lss_transfer.baudRate="50K";break;
			case 0x07:d->lss_transfer.baudRate="20K";break;
			case 0x08:d->lss_transfer.baudRate="10K";break;
			default:
				MSG_ERR(0x1D28, "Baud rate not supported",0);
				error_code=0xFF; /* Baud rate not supported*/
				break; 		
			}
		}
		else{
			MSG_WAR(0x3D2A, "SlaveLSS not in configuration mode",0);
			//error_code=0xFF;
			break;
		}
		
		/* if bit timing is not supported comment the previous code and uncomment the following */
		/*{
			MSG_ERR(0x1D29, "Bit timing not supported",0);
			error_code=0x01; // bit timing not supported 
		}*/
			
		sendSlaveLSSMessage(d,msg_cs,&error_code,&spec_error);
	}
	break;
	case LSS_CONF_ACT_BIT_TIMING: /* Activate Bit Timing Parameters */
		
		if(d->lss_transfer.mode!=LSS_CONFIGURATION_MODE){
			MSG_ERR(0x3D2B, "SlaveLSS not in configuration mode",0);
			break;
		}
		
		if(d->lss_transfer.baudRate!="none"){
			d->lss_transfer.switchDelay=getLSSDelay(m);
			MSG_WAR(0x3D2C, "Slave Switch Delay set to: ",d->lss_transfer.switchDelay);
			d->lss_transfer.switchDelayState=SDELAY_FIRST;
			//d->lss_transfer.currentState=getState(d);
			//setState(d, LssTimingDelay);
			d->lss_transfer.canHandle_t=d->canHandle;
			d->canHandle=NULL;
			StartLSS_SDELAY_TIMER();
		}
	break;
	case LSS_CONF_STORE: /* Store Configured Parameters */
	{
		UNS8 error_code=0;
		UNS8 spec_error=0;
		
		if(d->lss_transfer.mode==LSS_CONFIGURATION_MODE){ 
			if(d->lss_StoreConfiguration){
				 /* call lss_StoreConfiguration with NodeId */
	  			(*d->lss_StoreConfiguration)(d,&error_code,&spec_error);
			}
			else{
				MSG_ERR(0x1D2E, "Store configuration not supported",0);
				error_code=1; /* store configuration is not supported */
			}	
		}
		else{
			MSG_WAR(0x3D2F, "SlaveLSS not in configuration mode",0);
			//error_code=0xFF;
			break;
		}
		sendSlaveLSSMessage(d,msg_cs,&error_code,&spec_error);
	}
	break;
	case LSS_SM_SELECTIVE_VENDOR:	/* Switch Mode Selective */
	case LSS_SM_SELECTIVE_PRODUCT:
	case LSS_SM_SELECTIVE_REVISION:
	case LSS_SM_SELECTIVE_SERIAL:
	{
		UNS32 errorCode;
  		const indextable *ptrTable;
  		ODCallback_t *Callback;
  		UNS32 _SpecificNodeInfo;
  
  		if(d->lss_transfer.mode==LSS_CONFIGURATION_MODE)
  		{
  			MSG_ERR(0x1D30, "Switch Mode Selective only supported in operational mode",0);
  			break;
  		}
  			
		_SpecificNodeInfo=getLSSIdent(m);
				
		ptrTable = (*d->scanIndexOD)(0x1018, &errorCode, &Callback);
		if(_SpecificNodeInfo==*(UNS32*)ptrTable->pSubindex[msg_cs-(LSS_SM_SELECTIVE_VENDOR-1)].pObject){
			
			d->lss_transfer.addr_sel_match|=(0x01<<(msg_cs-LSS_SM_SELECTIVE_VENDOR));
			/* If all the fields has been set */
			if(d->lss_transfer.addr_sel_match==0x0F){
				
				MSG_WAR(0x3D31, "SlaveLSS switching to configuration mode ", 0);
				d->lss_transfer.addr_sel_match=0;
				d->lss_transfer.nodeID=getNodeId(d);
				d->lss_transfer.mode=LSS_CONFIGURATION_MODE;

				sendSlaveLSSMessage(d,LSS_SM_SELECTIVE_RESP,0,0);
			}
		}	
		else {
			MSG_WAR(0x3D32, "LSS identity field doesn't match ", _SpecificNodeInfo);
			d->lss_transfer.addr_sel_match=0;
		}	
	}	
	break;
	case LSS_IDENT_REMOTE_VENDOR: /* LSS Identify Remote Slaves */
	case LSS_IDENT_REMOTE_PRODUCT:
	case LSS_IDENT_REMOTE_REV_LOW:
	case LSS_IDENT_REMOTE_REV_HIGH:
	case LSS_IDENT_REMOTE_SERIAL_LOW:
	case LSS_IDENT_REMOTE_SERIAL_HIGH:
	{
		UNS32 errorCode;
  		const indextable *ptrTable;
  		ODCallback_t *Callback;
  		UNS32 _SpecificNodeInfo;
  		
		_SpecificNodeInfo=getLSSIdent(m);
		
		ptrTable = (*d->scanIndexOD)(0x1018, &errorCode, &Callback);
			
		/* Check if the data match the identity object. */
		switch(msg_cs){
		case LSS_IDENT_REMOTE_VENDOR:d->lss_transfer.addr_ident_match=(_SpecificNodeInfo == *(UNS32*)ptrTable->pSubindex[1].pObject)? d->lss_transfer.addr_ident_match|0x01:0;	break;
		case LSS_IDENT_REMOTE_PRODUCT:d->lss_transfer.addr_ident_match=(_SpecificNodeInfo == *(UNS32*)ptrTable->pSubindex[2].pObject)? d->lss_transfer.addr_ident_match|0x02:0;	break;
		case LSS_IDENT_REMOTE_REV_LOW:d->lss_transfer.addr_ident_match=(_SpecificNodeInfo <= *(UNS32*)ptrTable->pSubindex[3].pObject)? d->lss_transfer.addr_ident_match|0x04:0; break;
		case LSS_IDENT_REMOTE_REV_HIGH:d->lss_transfer.addr_ident_match=(_SpecificNodeInfo >= *(UNS32*)ptrTable->pSubindex[3].pObject)? d->lss_transfer.addr_ident_match|0x08:0;	break;
		case LSS_IDENT_REMOTE_SERIAL_LOW:d->lss_transfer.addr_ident_match=(_SpecificNodeInfo <= *(UNS32*)ptrTable->pSubindex[4].pObject)? d->lss_transfer.addr_ident_match|0x10:0;	break;
		case LSS_IDENT_REMOTE_SERIAL_HIGH:d->lss_transfer.addr_ident_match=(_SpecificNodeInfo >= *(UNS32*)ptrTable->pSubindex[4].pObject)? d->lss_transfer.addr_ident_match|0x20:0;	break;
		}
		/* If all the fields has been set.. */
		if(d->lss_transfer.addr_ident_match==0x3F){
			MSG_WAR(0x3D33, "SlaveLSS identified ", 0);
			d->lss_transfer.addr_ident_match=0;
			sendSlaveLSSMessage(d,LSS_IDENT_SLAVE,0,0);
		}
		else if(d->lss_transfer.addr_ident_match==0){
			MSG_WAR(0x3D34, "LSS identify field doesn't match ", _SpecificNodeInfo);
		}
	}
	break;
	case LSS_IDENT_REMOTE_NON_CONF: /* LSS identify non-configured remote slave */
	{
		if(getNodeId(d)==0xFF){		
			MSG_WAR(0x3D35, "SlaveLSS non-configured ", 0);
			sendSlaveLSSMessage(d,LSS_IDENT_NON_CONF_SLAVE,0,0);
		}
		else{
			MSG_WAR(0x3D36, "SlaveLSS already configured ", 0);
		}
	}
	break;
	case LSS_INQ_VENDOR_ID: /* Inquire Identity Vendor-ID */
	case LSS_INQ_PRODUCT_CODE: /* Inquire Identity Product-Code */
	case LSS_INQ_REV_NUMBER: /* Inquire Identity Revision-Number */
	case LSS_INQ_SERIAL_NUMBER: /* Inquire Identity Serial-Number */
	if(d->lss_transfer.mode==LSS_CONFIGURATION_MODE)
	{
	
		UNS32 errorCode;
  		const indextable *ptrTable;
  		ODCallback_t *Callback;
  		UNS32 _SpecificNodeInfo;
  
  		ptrTable = (*d->scanIndexOD)(0x1018, &errorCode, &Callback);
  		_SpecificNodeInfo=*(UNS32*)ptrTable->pSubindex[msg_cs-(LSS_INQ_VENDOR_ID-1)].pObject;
  		MSG_WAR(0x3D37, "SlaveLSS identity field inquired ", _SpecificNodeInfo);
			
		sendSlaveLSSMessage(d,msg_cs,&_SpecificNodeInfo,0);
	}
	break;
	case LSS_INQ_NODE_ID: /* Inquire Node-ID */
		if(d->lss_transfer.mode==LSS_CONFIGURATION_MODE)
		{
			UNS8 NodeID;
	
			NodeID=getNodeId(d);
			MSG_WAR(0x3D38, "SlaveLSS Node ID inquired ", NodeID);
			sendSlaveLSSMessage(d,msg_cs,&NodeID,0);
		}
		else{
			MSG_WAR(0x3D39, "SlaveLSS not in configuration mode",0);
		}
	break;
#ifdef CO_ENABLE_LSS_FS
	case LSS_IDENT_FASTSCAN:
	{
		/* If the nodeID isn't 0xFF the slave shall not participate  */
		if(getNodeId(d)!=0xFF)break;
		if(getLSSBitCheck(m)==128)d->lss_transfer.FastScan_SM=LSS_FS_RESET;
		
   		switch(d->lss_transfer.FastScan_SM){
   		case LSS_FS_RESET:
   		{
   			UNS32 errorCode;
  			const indextable *ptrTable;
  			ODCallback_t *Callback;
  				
			MSG_WAR(0x3D3A, "SlaveLSS Reseting LSSPos", 0);
			d->lss_transfer.LSSPos=0;
			d->lss_transfer.FastScan_SM=LSS_FS_PROCESSING;
			
  			ptrTable = (*d->scanIndexOD)(0x1018, &errorCode, &Callback);
  			d->lss_transfer.IDNumber=*(UNS32*)ptrTable->pSubindex[d->lss_transfer.LSSPos+1].pObject;
			
			sendSlaveLSSMessage(d,LSS_IDENT_SLAVE,0,0);
   		}
		break;
		case LSS_FS_PROCESSING:/*if(getLSSBitCheck(m)<32)*/
			if(d->lss_transfer.LSSPos==getLSSSub(m))
			{
				UNS32 Mask=0xFFFFFFFF<<getLSSBitCheck(m);
				
				MSG_WAR(0x3D3B, "SlaveLSS FastScan IDNumber", getLSSIdent(m));
				MSG_WAR(0x3D3C, "SlaveLSS FastScan BitMask ", Mask);
				MSG_WAR(0x3D3D, "SlaveLSS FastScan LSS-ID  ", d->lss_transfer.IDNumber);
				
				if((getLSSIdent(m) & Mask)==(d->lss_transfer.IDNumber & Mask))
				{
					sendSlaveLSSMessage(d,LSS_IDENT_SLAVE,0,0);
				}
				
				if(getLSSBitCheck(m)==0)
				{
					d->lss_transfer.FastScan_SM=LSS_FS_CONFIRMATION;
				}
			}
			break;
		case LSS_FS_CONFIRMATION:
			if(d->lss_transfer.LSSPos==getLSSSub(m))
			{
				if(getLSSIdent(m)==d->lss_transfer.IDNumber)
				{
					/* Current LSS-ID[sub] confirmed correctly */
					MSG_WAR(0x3D3E, "SlaveLSS FastScan IDNumber and LSS-ID match=>", d->lss_transfer.IDNumber);
					if(d->lss_transfer.LSSPos==3)
					{
						/* All LSS-ID[sub] identified correctly, switching to configuration mode */
						MSG_WAR(0x3D3F, "SlaveLSS switching to configuration mode ", 0);
			   			d->lss_transfer.nodeID=getNodeId(d);
			   			d->lss_transfer.mode=LSS_CONFIGURATION_MODE;
			    		d->lss_transfer.FastScan_SM=LSS_FS_RESET;
			    		d->lss_transfer.LSSPos=0xFF;
					}		
					else
					{
						/* Switch to the next LSS-ID[sub] */
						UNS32 errorCode;
  						const indextable *ptrTable;
  						ODCallback_t *Callback;
		
						d->lss_transfer.LSSPos=getLSSNext(m);
						ptrTable = (*d->scanIndexOD)(0x1018, &errorCode, &Callback);
  						d->lss_transfer.IDNumber=*(UNS32*)ptrTable->pSubindex[d->lss_transfer.LSSPos+1].pObject;
						d->lss_transfer.FastScan_SM=LSS_FS_PROCESSING;						
					}
					sendSlaveLSSMessage(d,LSS_IDENT_SLAVE,0,0);
				}
			}
			break;
		}
	}	
	break;
#endif
   	default:
   		MSG_ERR(0x1D40, "SlaveLSS command not implemented", msg_cs);
  		return 0xFF;
   	}
   
    return 0;
}

/*UNS8 configNetworkNode(CO_Data* d, UNS8 command, void *dat1, void* dat2)
{
	return sendMasterLSSMessage(d,command,dat1,dat2);
}*/

UNS8 configNetworkNode (CO_Data* d, UNS8 command, void *dat1, void* dat2, LSSCallback_t Callback)
{
	//d->lss_transfer.state=LSS_TRANS_IN_PROGRESS;
	d->lss_transfer.Callback=Callback;
	d->lss_transfer.command=command;
	
	StopLSS_MSG_TIMER();
  	//StartLSS_MSG_TIMER();
  	
	return sendMasterLSSMessage(d,command,dat1,dat2);
}

UNS8 getConfigResultNetworkNode (CO_Data* d, UNS8 command, UNS32* dat1, UNS8* dat2)
{ 
  *dat1=d->lss_transfer.dat1;
  *dat2=d->lss_transfer.dat2;
  return d->lss_transfer.state;
}

//void _lss_StoreConfiguration(UNS8 *error, UNS8 *spec_error){printf("_lss_StoreConfiguration\n");}

#endif
