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
#define usleep(micro) Sleep(micro%1000 ? (micro/1000) + 1 : (micro/1000))
#define _CRT_SECURE_NO_DEPRECATE
#else
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#endif

#include "can_driver.h"
#include "AnaGateDllCan.h"


typedef struct SAnaGatePort
{
int		hHandle;
HANDLE	hAnaRecEvent;
HANDLE	hFesticalRecAcknowledge;
Message sMsgBuffer;
struct SAnaGatePort *pNext;
struct SAnaGatePort *pPrev;
} SAnaGatePort;


SAnaGatePort *pFirstAnaGatePort=NULL;


/********* AnaGate API CAN receive callback Funciton  ****************/
void WINAPI AnaGateReceiveCallBack (int nIdentifier, const char* pcBuffer, int nBufferLen, int nFlags, int hHandle)
{
	SAnaGatePort *pAnaGatePort = pFirstAnaGatePort;
   int           i;
	
	while (pAnaGatePort->hHandle != hHandle )
	{
		pAnaGatePort = pAnaGatePort->pNext;
		if (pAnaGatePort == pFirstAnaGatePort )
		{
			pAnaGatePort = NULL;
			printf("AnaGateReceiveCallBack (AnaGate_Win32): ERROR: Can't find AnaGatePort-Objekt to the Received Handle %d\n",hHandle);
			return;
		}
	}
	
	pAnaGatePort->sMsgBuffer.cob_id =   nIdentifier;
	pAnaGatePort->sMsgBuffer.len= nBufferLen;
	if (nFlags == 2)
	 pAnaGatePort->sMsgBuffer.rtr = 1;
	else
	 pAnaGatePort->sMsgBuffer.rtr = 0;

	for (i = 0 ; i < nBufferLen; i++)
	{
		pAnaGatePort->sMsgBuffer.data[i] = pcBuffer[i];
	}

	SetEvent(pAnaGatePort->hAnaRecEvent);

	WaitForSingleObject(pAnaGatePort->hFesticalRecAcknowledge,INFINITE);
}


/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
{
	SAnaGatePort*  pAnaGatePort = (SAnaGatePort*)fd0;
   int            i;

	WaitForSingleObject(			// Wait for receive event
		pAnaGatePort->hAnaRecEvent, // event handle
		INFINITE);				    // indefinite wait


	m->cob_id = pAnaGatePort->sMsgBuffer.cob_id;
	m->len	  = pAnaGatePort->sMsgBuffer.len;
	m->rtr	  = pAnaGatePort->sMsgBuffer.rtr;
	for (i = 0 ; i < pAnaGatePort->sMsgBuffer.len; i++)
	{
	  m->data[i] = pAnaGatePort->sMsgBuffer.data[i];
	}

	SetEvent(pAnaGatePort->hFesticalRecAcknowledge);	//Set Acknollede event

	return 0;
}

/***************************************************************************/
UNS8 canSend_driver(CAN_HANDLE fd0, Message *m)
{
	SAnaGatePort*  pAnaCanPort = (SAnaGatePort*)fd0;
	char cErrorMsg[100];
	int nRetCode;
	int nMsgTyp;

	if (m->rtr == 0)
	{
	  nMsgTyp = 0; //Normal;
	}
	else
	{
	  nMsgTyp = 2; //Remote frame;
	}

	if ( (nRetCode = CANWrite(pAnaCanPort->hHandle , m->cob_id,(const char*) m->data, m->len, nMsgTyp) ) )
	{
	  CANErrorMessage( nRetCode, cErrorMsg ,100 ); // Convert returncode to error messge
	  fprintf(stderr,"canSend_driver (AnaGate_Win32) %s \n",nRetCode);
	  //printf("canSend_driver (AnaGate_Win32) %s \n",nRetCode);
	  return 1;
	}

	return 0;
}


/***************************************************************************/
int TranslateBaudeRate(char* optarg){
	if(!strcmp( optarg, "1M"))   return 1000000;
    if(!strcmp( optarg, "800K")) return  800000;
    if(!strcmp( optarg, "500K")) return  500000;
    if(!strcmp( optarg, "250K")) return  250000;
    if(!strcmp( optarg, "125K")) return  125000;
	if(!strcmp( optarg, "100K")) return  100000;
	if(!strcmp( optarg, "50K"))  return   50000;
	if(!strcmp( optarg, "20K"))  return   20000;
	if(!strcmp( optarg, "10K"))  return   10000;
 
	return 0x0000;
}
/****************************************************************************/

UNS8 canChangeBaudRate_driver( CAN_HANDLE fd0, char* baud)
{
   int nRetCode;
   char cErrorMsg[100];
   struct SAnaGatePort*  pAnaGatePort = (struct SAnaGatePort*)fd0;
   
   if (nRetCode = CANSetGlobals (pAnaGatePort->hHandle, TranslateBaudeRate(baud), 0, 0, 1) ) 
   {
      CANErrorMessage( nRetCode, cErrorMsg ,100 ); // Convert returncode to error messge
      fprintf(stderr, "canChangeBaudRate_drive (AnaGate_Win32): %s\n", cErrorMsg);
      printf("canChangeBaudRate_drive (AnaGate_Win32): %s\n", cErrorMsg);
      return 1;
   }
   return 0;
}

/***************************************************************************/
/* To open a connection to AnaGate CAN the s_BOARD board->busname must be 
 the AnaGate IP-Adresse followed from the CAN-Port (A or B) you want to use 
 For example "192.168.1.254:A"
*/

CAN_HANDLE canOpen_driver(s_BOARD *board)
{
	int PortNr;
	char cErrorMsg[100];
	int nRetCode;
	char sIPAddress[16];
	struct SAnaGatePort *pNewAnaGatePort;
    char bBusnameValid = TRUE;
	unsigned int nBusnameLen; 

   ///////////////////////////////////////////
   // Do some checkings concerning the busname
   // format should be IP-Adress:Port
   // e.g. 192.168.1.254:A
   ///////////////////////////////////////////
   nBusnameLen = strlen(board->busname);
   
   if ( nBusnameLen < strlen( "1.2.3.4:A" ) )         bBusnameValid = FALSE;  // check minimum length of busname
   if ( nBusnameLen > strlen( "123.234.345.456:A" ) ) bBusnameValid = FALSE;  // check maximum length of busname
   if ( bBusnameValid )
   {
      switch (board->busname[nBusnameLen-1])                                 // check Portname of busname
	   {
		   case ('A'): PortNr = 0; break;
		   case ('B'): PortNr = 1; break;
		   case ('C'): PortNr = 2; break;
		   case ('D'): PortNr = 3; break;
		   default :   bBusnameValid = FALSE; break;
	   }
      if (board->busname[nBusnameLen-2] != ':' )    bBusnameValid = FALSE;   // check Colon before Portname
   }

   if ( ! bBusnameValid )
   {
	   fprintf(stderr, "canOpen_driver (AnaGate_Win32): busname (\"%s\") has a wrong format. Use IPAddr:CANPort for example \"192.168.1.254:A\"\n",board->busname);
	   return (CAN_HANDLE) NULL;
   }

   board->busname[nBusnameLen-2] = 0; // NULL Terminator for IP Address string
	strcpy (sIPAddress, board->busname);

	pNewAnaGatePort = (SAnaGatePort*) malloc(sizeof (struct SAnaGatePort));
	if (pFirstAnaGatePort == NULL)
	{
		pFirstAnaGatePort = pNewAnaGatePort;
		pNewAnaGatePort->pNext =  pNewAnaGatePort;
		pNewAnaGatePort->pPrev =  pNewAnaGatePort;
	}
	else
	{   pNewAnaGatePort->pNext =  pFirstAnaGatePort;
		pNewAnaGatePort->pPrev =  pFirstAnaGatePort->pPrev;
		pFirstAnaGatePort->pPrev->pNext = pNewAnaGatePort;
		pFirstAnaGatePort->pPrev = pNewAnaGatePort;

	}
	// Connect to AnaGate
	if ( nRetCode = CANOpenDevice (&pNewAnaGatePort->hHandle, 
		 						0,		/*confirmation*/ 
								1,		/*Monitor*/ 
								PortNr,
								sIPAddress, 
								1000	/*TimeOut*/ )  )
	{
		CANErrorMessage( nRetCode, cErrorMsg ,100 ); // Convert returncode to error messge
		fprintf(stderr, "canOpen_driver (AnaGate_Win32): %s @ %s\n", cErrorMsg,sIPAddress);
		//printf( "canOpen_driver (AnaGate_Win32): %s @ %s\n", cErrorMsg,sIPAddress);
		return (CAN_HANDLE) NULL;
	}

	// Inizial Baudrate
	if (nRetCode = CANSetGlobals (pNewAnaGatePort->hHandle, 
							   TranslateBaudeRate(board->baudrate), 
							   0,/*OperatingMode = normal*/ 
							   0,/*CAN-Termination = off*/ 
							   1 /*HighSpeedMode = on*/) ) 
	{
		CANErrorMessage( nRetCode, cErrorMsg ,100 ); // Convert returncode to error messge
		fprintf(stderr, "canOpen_driver (AnaGate_Win32): %s @ %s\n", cErrorMsg,sIPAddress);
		//printf("canOpen_driver (AnaGate_Win32): %s @ %s\n", cErrorMsg,sIPAddress);
		return (CAN_HANDLE) NULL;
	}
	
	// Creat receive and receive-acknoledge event 
	pNewAnaGatePort->hAnaRecEvent = CreateEvent(	
												NULL,  // default security attributes
												FALSE, // manual-reset event
												FALSE, // initial state is nonsignaled
												NULL  // object name
												); 

	pNewAnaGatePort->hFesticalRecAcknowledge = CreateEvent( 
												NULL,  // default security attributes
												FALSE, // manual-reset event
												FALSE, // initial state is nonsignaled
												NULL   // object name
												); 

	// Install receive callback funktion
	if (nRetCode = CANSetCallback(pNewAnaGatePort->hHandle,  AnaGateReceiveCallBack) ) 
	{
		canClose_driver (pNewAnaGatePort);
		CANErrorMessage( nRetCode, cErrorMsg ,100 ); // Convert returncode to error messge
		fprintf(stderr, "canOpen_driver (AnaGate_Win32): %s @ %s\n", cErrorMsg,sIPAddress);
		//printf("canOpen_driver (AnaGate_Win32): %s @ %s\n", cErrorMsg,sIPAddress);
		return (CAN_HANDLE) NULL;
	}

  return (CAN_HANDLE)pNewAnaGatePort;
}

/***************************************************************************/
int canClose_driver(CAN_HANDLE fd0)
{
	SAnaGatePort*  pAnaCanPort = (SAnaGatePort*)fd0;
	char cErrorMsg[100];
	int nRetCode;


	SetEvent	(pAnaCanPort->hAnaRecEvent);
	CloseHandle (pAnaCanPort->hAnaRecEvent);
	CloseHandle (pAnaCanPort->hFesticalRecAcknowledge);

	if ( nRetCode = CANCloseDevice(pAnaCanPort->hHandle) )
	{
	  CANErrorMessage( nRetCode, cErrorMsg ,100 ); // Convert returncode to error messge
	  fprintf(stderr, "canClose_driver (AnaGate_Linux): %s\n", cErrorMsg);
	  //printf("canClose_driver (AnaGate_Linux): %s\n", cErrorMsg);
	}

	if (pAnaCanPort->pNext == pAnaCanPort)
	{
		 free (pAnaCanPort);
		 pFirstAnaGatePort=NULL;
	}
	else
	{   pAnaCanPort->pNext->pPrev = pAnaCanPort->pPrev;
		pAnaCanPort->pPrev->pNext = pAnaCanPort->pNext;
		free (pAnaCanPort);
	}

	return 0;
}





