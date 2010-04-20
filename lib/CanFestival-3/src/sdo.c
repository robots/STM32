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
/*!
** @file   sdo.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 09:32:32 2007
**
** @brief
**
**
*/

/* #define DEBUG_WAR_CONSOLE_ON */
/* #define DEBUG_ERR_CONSOLE_ON */

#include "canfestival.h"
#include "sysdep.h"

/* Uncomment if your compiler does not support inline functions */
#define NO_INLINE

#ifdef NO_INLINE
  #define INLINE
#else
  #define INLINE inline
#endif

/*Internals prototypes*/

/*!
** Called by writeNetworkDict
**
** @param d
** @param nodeId
** @param index
** @param subIndex
** @param count
** @param dataType
** @param data
** @param Callback
** @param endianize
**
** @return
**/
INLINE UNS8 _writeNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data, SDOCallback_t Callback, UNS8 endianize);

/*!
** Called by readNetworkDict
**
** @param d
** @param nodeId
** @param index
** @param subIndex
** @param dataType
** @param Callback
**
** @return
**/
INLINE UNS8 _readNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex,
	UNS8 dataType, SDOCallback_t Callback);


/***************************************************************************/
/* SDO (un)packing macros */

/** Returns the command specifier (cs, ccs, scs) from the first byte of the SDO
 */
#define getSDOcs(byte) (byte >> 5)

/** Returns the number of bytes without data from the first byte of the SDO. Coded in 2 bits
 */
#define getSDOn2(byte) ((byte >> 2) & 3)

/** Returns the number of bytes without data from the first byte of the SDO. Coded in 3 bits
 */
#define getSDOn3(byte) ((byte >> 1) & 7)

/** Returns the transfer type from the first byte of the SDO
 */
#define getSDOe(byte) ((byte >> 1) & 1)

/** Returns the size indicator from the first byte of the SDO
 */
#define getSDOs(byte) (byte & 1)

/** Returns the indicator of end transmission from the first byte of the SDO
 */
#define getSDOc(byte) (byte & 1)

/** Returns the toggle from the first byte of the SDO
 */
#define getSDOt(byte) ((byte >> 4) & 1)

/** Returns the index from the bytes 1 and 2 of the SDO
 */
#define getSDOindex(byte1, byte2) (((UNS16)byte2 << 8) | ((UNS16)byte1))

/** Returns the subIndex from the byte 3 of the SDO
 */
#define getSDOsubIndex(byte3) (byte3)

/*!
**
**
** @param d
** @param id
**/
void SDOTimeoutAlarm(CO_Data* d, UNS32 id)
{
    MSG_ERR(0x1A01, "SDO timeout. SDO response not received.", 0);
    MSG_WAR(0x2A02, "server node : ", d->transfers[id].nodeId);
    MSG_WAR(0x2A02, "      index : ", d->transfers[id].index);
    MSG_WAR(0x2A02, "   subIndex : ", d->transfers[id].subIndex);
    /* Reset timer handler */
    d->transfers[id].timer = TIMER_NONE;
    /*Set aborted state*/
    d->transfers[id].state = SDO_ABORTED_INTERNAL;
    /* Sending a SDO abort */
    sendSDOabort(d, d->transfers[id].whoami, d->transfers[id].nodeId,
		 d->transfers[id].index, d->transfers[id].subIndex, SDOABT_TIMED_OUT);
    d->transfers[id].abortCode = SDOABT_TIMED_OUT;
    /* Call the user function to inform of the problem.*/
    if(d->transfers[id].Callback)
    	/*If ther is a callback, it is responsible to close SDO transfer (client)*/
    	(*d->transfers[id].Callback)(d,d->transfers[id].nodeId);
    else if(d->transfers[id].whoami == SDO_SERVER)
    	/*Else, if server, reset the line*/
    	resetSDOline(d, (UNS8)id);
}

#define StopSDO_TIMER(id) \
MSG_WAR(0x3A05, "StopSDO_TIMER for line : ", line);\
d->transfers[id].timer = DelAlarm(d->transfers[id].timer);

#define StartSDO_TIMER(id) \
MSG_WAR(0x3A06, "StartSDO_TIMER for line : ", line);\
d->transfers[id].timer = SetAlarm(d,id,&SDOTimeoutAlarm,MS_TO_TIMEVAL(SDO_TIMEOUT_MS),0);

#define RestartSDO_TIMER(id) \
MSG_WAR(0x3A07, "restartSDO_TIMER for line : ", line);\
if(d->transfers[id].timer != TIMER_NONE) { StopSDO_TIMER(id) StartSDO_TIMER(id) }

/*!
** Reset all sdo buffers
**
** @param d
**/
void resetSDO (CO_Data* d)
{
  UNS8 j;

  /* transfer structure initialization */
    for (j = 0 ; j < SDO_MAX_SIMULTANEOUS_TRANSFERTS ; j++)
      resetSDOline(d, j);
}

/*!
**
**
** @param d
** @param line
**
** @return
**/
UNS32 SDOlineToObjdict (CO_Data* d, UNS8 line)
{
  UNS32 size;
  UNS32 errorCode;
  MSG_WAR(0x3A08, "Enter in SDOlineToObjdict ", line);
  /* if SDO initiated with e=0 and s=0 count is null, offset carry effective size*/
  if( d->transfers[line].count == 0)
  	d->transfers[line].count = d->transfers[line].offset;
  size = d->transfers[line].count;
  errorCode = setODentry(d, d->transfers[line].index, d->transfers[line].subIndex,
			 (void *) d->transfers[line].data, &size, 1);
  if (errorCode != OD_SUCCESSFUL)
    return errorCode;
  MSG_WAR(0x3A08, "exit of SDOlineToObjdict ", line);
  return 0;

}

/*!
**
**
** @param d
** @param line
**
** @return
**/
UNS32 objdictToSDOline (CO_Data* d, UNS8 line)
{
  UNS32  size = 0;
  UNS8  dataType;
  UNS32 errorCode;

  MSG_WAR(0x3A05, "objdict->line index : ", d->transfers[line].index);
  MSG_WAR(0x3A06, "  subIndex : ", d->transfers[line].subIndex);

  errorCode = getODentry(d, 	d->transfers[line].index,
  				d->transfers[line].subIndex,
  				(void *)d->transfers[line].data,
  				&size, &dataType, 1);

  if (errorCode != OD_SUCCESSFUL)
    return errorCode;

  d->transfers[line].count = size;
  d->transfers[line].offset = 0;

  return 0;
}

/*!
**
**
** @param d
** @param line
** @param nbBytes
** @param data
**
** @return
**/
UNS8 lineToSDO (CO_Data* d, UNS8 line, UNS32 nbBytes, UNS8* data) {
  UNS8 i;
  UNS32 offset;

  if ((d->transfers[line].offset + nbBytes) > SDO_MAX_LENGTH_TRANSFERT) {
    MSG_ERR(0x1A10,"SDO Size of data too large. Exceed SDO_MAX_LENGTH_TRANSFERT", nbBytes);
    return 0xFF;
  }
    if ((d->transfers[line].offset + nbBytes) > d->transfers[line].count) {
    MSG_ERR(0x1A11,"SDO Size of data too large. Exceed count", nbBytes);
    return 0xFF;
  }
  offset = d->transfers[line].offset;
  for (i = 0 ; i < nbBytes ; i++)
    * (data + i) = d->transfers[line].data[offset + i];
  d->transfers[line].offset = d->transfers[line].offset + nbBytes;
  return 0;
}

/*!
**
**
** @param d
** @param line
** @param nbBytes
** @param data
**
** @return
**/
UNS8 SDOtoLine (CO_Data* d, UNS8 line, UNS32 nbBytes, UNS8* data)
{
  UNS8 i;
  UNS32 offset;

  if ((d->transfers[line].offset + nbBytes) > SDO_MAX_LENGTH_TRANSFERT) {
    MSG_ERR(0x1A15,"SDO Size of data too large. Exceed SDO_MAX_LENGTH_TRANSFERT", nbBytes);
    return 0xFF;
  }
  offset = d->transfers[line].offset;
  for (i = 0 ; i < nbBytes ; i++)
    d->transfers[line].data[offset + i] = * (data + i);
  d->transfers[line].offset = d->transfers[line].offset + nbBytes;
  return 0;
}

/*!
**
**
** @param d
** @param nodeId
** @param whoami
** @param index
** @param subIndex
** @param abortCode
**
** @return
**/
UNS8 failedSDO (CO_Data* d, UNS8 nodeId, UNS8 whoami, UNS16 index,
		UNS8 subIndex, UNS32 abortCode)
{
  UNS8 err;
  UNS8 line;
  err = getSDOlineOnUse( d, nodeId, whoami, &line );
  if (!err) /* If a line on use have been found.*/
    MSG_WAR(0x3A20, "FailedSDO : line found : ", line);
  if ((! err) && (whoami == SDO_SERVER)) {
    resetSDOline( d, line );
    MSG_WAR(0x3A21, "FailedSDO : line released : ", line);
  }
  if ((! err) && (whoami == SDO_CLIENT)) {
    StopSDO_TIMER(line);
    d->transfers[line].state = SDO_ABORTED_INTERNAL;
  }
  MSG_WAR(0x3A22, "Sending SDO abort ", 0);
  err = sendSDOabort(d, whoami, nodeId, index, subIndex, abortCode);
  if (err) {
    MSG_WAR(0x3A23, "Unable to send the SDO abort", 0);
    return 0xFF;
  }
  return 0;
}

/*!
**
**
** @param d
** @param line
**/
void resetSDOline ( CO_Data* d, UNS8 line )
{
  UNS32 i;
  MSG_WAR(0x3A25, "reset SDO line nb : ", line);
  initSDOline(d, line, 0, 0, 0, SDO_RESET);
  for (i = 0 ; i < SDO_MAX_LENGTH_TRANSFERT ; i++)
    d->transfers[line].data[i] = 0;
  d->transfers[line].whoami = 0;
  d->transfers[line].abortCode = 0;
}

/*!
**
**
** @param d
** @param line
** @param nodeId
** @param index
** @param subIndex
** @param state
**
** @return
**/
UNS8 initSDOline (CO_Data* d, UNS8 line, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 state)
{
  MSG_WAR(0x3A25, "init SDO line nb : ", line);
  if (state == SDO_DOWNLOAD_IN_PROGRESS || state == SDO_UPLOAD_IN_PROGRESS){
  	StartSDO_TIMER(line)
  }else{
  	StopSDO_TIMER(line)
  }
  d->transfers[line].nodeId = nodeId;
  d->transfers[line].index = index;
  d->transfers[line].subIndex = subIndex;
  d->transfers[line].state = state;
  d->transfers[line].toggle = 0;
  d->transfers[line].count = 0;
  d->transfers[line].offset = 0;
  d->transfers[line].dataType = 0;
  d->transfers[line].Callback = NULL;
  return 0;
}

/*!
**
**
** @param d
** @param whoami
** @param line
**
** @return
**/
UNS8 getSDOfreeLine ( CO_Data* d, UNS8 whoami, UNS8 *line )
{

  UNS8 i;

  for (i = 0 ; i < SDO_MAX_SIMULTANEOUS_TRANSFERTS ; i++){
    if ( d->transfers[i].state == SDO_RESET ) {
      *line = i;
      d->transfers[i].whoami = whoami;
      return 0;
    } /* end if */
  } /* end for */
  MSG_ERR(0x1A25, "Too many SDO in progress. Aborted.", i);
  return 0xFF;
}

/*!
**
**
** @param d
** @param nodeId
** @param whoami
** @param line
**
** @return
**/
UNS8 getSDOlineOnUse (CO_Data* d, UNS8 nodeId, UNS8 whoami, UNS8 *line)
{

  UNS8 i;

  for (i = 0 ; i < SDO_MAX_SIMULTANEOUS_TRANSFERTS ; i++){
    if ( (d->transfers[i].state != SDO_RESET) &&
	 (d->transfers[i].nodeId == nodeId) &&
	 (d->transfers[i].whoami == whoami) ) {
      *line = i;
      return 0;
    }
  }
  return 0xFF;
}

/*!
**
**
** @param d
** @param nodeId
** @param whoami
**
** @return
**/
UNS8 closeSDOtransfer (CO_Data* d, UNS8 nodeId, UNS8 whoami)
{
  UNS8 err;
  UNS8 line;
  err = getSDOlineOnUse(d, nodeId, whoami, &line);
  if (err) {
    MSG_WAR(0x2A30, "No SDO communication to close for node : ", nodeId);
    return 0xFF;
  }
  resetSDOline(d, line);
  return 0;
}

/*!
**
**
** @param d
** @param line
** @param nbBytes
**
** @return
**/
UNS8 getSDOlineRestBytes (CO_Data* d, UNS8 line, UNS32 * nbBytes)
{
  /* SDO initiated with e=0 and s=0 have count set to null */
  if (d->transfers[line].count == 0)
    * nbBytes = 0;
  else
    * nbBytes = d->transfers[line].count - d->transfers[line].offset;
  return 0;
}

/*!
**
**
** @param d
** @param line
** @param nbBytes
**
** @return
**/
UNS8 setSDOlineRestBytes (CO_Data* d, UNS8 line, UNS32 nbBytes)
{
  if (nbBytes > SDO_MAX_LENGTH_TRANSFERT) {
    MSG_ERR(0x1A35,"SDO Size of data too large. Exceed SDO_MAX_LENGTH_TRANSFERT", nbBytes);
    return 0xFF;
  }
  d->transfers[line].count = nbBytes;
  return 0;
}

/*!
**
**
** @param d
** @param whoami
** @param sdo
**
** @return
**/
UNS8 sendSDO (CO_Data* d, UNS8 whoami, s_SDO sdo)
{
  UNS16 offset;
  UNS16 lastIndex;
  UNS8 found = 0;
  Message m;
  UNS8 i;
  UNS32 * pwCobId = NULL;
  UNS8 * pwNodeId = NULL;

  MSG_WAR(0x3A38, "sendSDO",0);
  if( !((d->nodeState == Operational) ||  (d->nodeState == Pre_operational ))) {
    MSG_WAR(0x2A39, "unable to send the SDO (not in op or pre-op mode", d->nodeState);
    return 0xFF;
  }

  /*get the server->client cobid*/
  if ( whoami == SDO_SERVER )	{/*case server. only one SDO server*/
    offset = d->firstIndex->SDO_SVR;
    if (offset == 0) {
      MSG_ERR(0x1A42, "SendSDO : No SDO server found", 0);
      return 0xFF;
    }
    pwCobId = (UNS32*) d->objdict[offset].pSubindex[2].pObject;
    MSG_WAR(0x3A41, "I am server. cobId : ", *pwCobId);
  }
  else {			/*case client*/
    /* Get the client->server cobid.*/
    UNS16 sdoNum = 0;
    offset = d->firstIndex->SDO_CLT;
    lastIndex = d->lastIndex->SDO_CLT;
    if (offset == 0) {
      MSG_ERR(0x1A42, "SendSDO : No SDO client index found", 0);
      return 0xFF;
    }
    /* find index for communication server node */
    while (offset <= lastIndex){
      MSG_WAR(0x3A43,"Reading index : ", 0x1280 + sdoNum);
      if (d->objdict[offset].bSubCount <= 3) {
	MSG_ERR(0x1A28, "Subindex 3  not found at index ", 0x1280 + sdoNum);
	return 0xFF;
      }
      pwNodeId = (UNS8*) d->objdict[offset].pSubindex[3].pObject;
      MSG_WAR(0x3A44, "Found nodeId server = ", *pwNodeId);
      if(*pwNodeId == sdo.nodeId) {
	found = 1;
	break;
      }
      offset ++;
      sdoNum ++;
    }
    if (! found){
      MSG_WAR (0x2A45, "No SDO client corresponds to the mesage to send to node ", sdo.nodeId);
      return 0xFF;
    }
    /* read the client->server cobid */
    pwCobId = (UNS32*) d->objdict[offset].pSubindex[1].pObject;
  }
  /* message copy for sending */
  m.cob_id = (UNS16)UNS16_LE(*pwCobId);
  m.rtr = NOT_A_REQUEST;
  /* the length of SDO must be 8 */
  m.len = 8;
  for (i = 0 ; i < 8 ; i++) {
    m.data[i] =  sdo.body.data[i];
  }
  return canSend(d->canHandle,&m);
}

/*!
**
**
** @param d
** @param whoami
** @param index
** @param subIndex
** @param abortCode
**
** @return
**/
UNS8 sendSDOabort (CO_Data* d, UNS8 whoami, UNS8 nodeID, UNS16 index, UNS8 subIndex, UNS32 abortCode)
{
  s_SDO sdo;
  UNS8 ret;
  
  MSG_WAR(0x2A50,"Sending SDO abort ", abortCode);
  if(whoami == SDO_SERVER)
  {
	sdo.nodeId = *d->bDeviceNodeId;
  }
  else
  {
    sdo.nodeId = nodeID;
  }
  sdo.body.data[0] = 0x80;
  /* Index */
  sdo.body.data[1] = index & 0xFF; /* LSB */
  sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
  /* Subindex */
  sdo.body.data[3] = subIndex;
  /* Data */
  sdo.body.data[4] = (UNS8)(abortCode & 0xFF);
  sdo.body.data[5] = (UNS8)((abortCode >> 8) & 0xFF);
  sdo.body.data[6] = (UNS8)((abortCode >> 16) & 0xFF);
  sdo.body.data[7] = (UNS8)((abortCode >> 24) & 0xFF);
  ret = sendSDO(d, whoami, sdo);

  return ret;
}

/*!
**
**
** @param d
** @param m
**
** @return
**/
UNS8 proceedSDO (CO_Data* d, Message *m)
{
  UNS8 err;
  UNS8 line;
  UNS32 nbBytes; /* received or to be transmited. */
  UNS8 nodeId = 0;  /* The node from which the SDO is received */
  UNS8 *pNodeId = NULL;
  UNS8 whoami = SDO_UNKNOWN;  /* SDO_SERVER or SDO_CLIENT.*/
  UNS32 errorCode; /* while reading or writing in the local object dictionary.*/
  s_SDO sdo;    /* SDO to transmit */
  UNS16 index;
  UNS8 subIndex;
  UNS32 abortCode;
  UNS32 i;
  UNS8	j;
  UNS32 *pCobId = NULL;
  UNS16 offset;
  UNS16 lastIndex;

  MSG_WAR(0x3A60, "proceedSDO ", 0);
  whoami = SDO_UNKNOWN;
  /* Looking for the cobId in the object dictionary. */
  /* Am-I a server ? */
  offset = d->firstIndex->SDO_SVR;
  lastIndex = d->lastIndex->SDO_SVR;
  j = 0;
  if(offset) while (offset <= lastIndex) {
     if (d->objdict[offset].bSubCount <= 1) {
	  MSG_ERR(0x1A61, "Subindex 1  not found at index ", 0x1200 + j);
	  return 0xFF;
	}
      pCobId = (UNS32*) d->objdict[offset].pSubindex[1].pObject;
      if ( *pCobId == UNS16_LE(m->cob_id) ) {
	whoami = SDO_SERVER;
	MSG_WAR(0x3A62, "proceedSDO. I am server. index : ", 0x1200 + j);
	/* In case of server, the node id of the client may be unknown. So we put the index minus offset */
	/* 0x1200 where the cobid received is defined. */
	nodeId = j;
	break;
      }
      j++;
      offset++;
  } /* end while */
  if (whoami == SDO_UNKNOWN) {
    /* Am-I client ? */
    offset = d->firstIndex->SDO_CLT;
    lastIndex = d->lastIndex->SDO_CLT;
    j = 0;
    if(offset) while (offset <= lastIndex) {
       if (d->objdict[offset].bSubCount <= 3) {
	 MSG_ERR(0x1A63, "Subindex 3  not found at index ", 0x1280 + j);
	 return 0xFF;
       }
       /* a) Looking for the cobid received. */
       pCobId = (UNS32*) d->objdict[offset].pSubindex[2].pObject;
       if (*pCobId == UNS16_LE(m->cob_id) ) {
	 /* b) cobid found, so reading the node id of the server. */
	 pNodeId = (UNS8*) d->objdict[offset].pSubindex[3].pObject;
	 whoami = SDO_CLIENT;
	 nodeId = *pNodeId;
	 MSG_WAR(0x3A64, "proceedSDO. I am server. index : ", 0x1280 + j);
	 MSG_WAR(0x3A65, "                 Server nodeId : ", nodeId);
	 break;
	}
       j++;
       offset++;
    } /* end while */
  }
  if (whoami == SDO_UNKNOWN) {
    return 0xFF;/* This SDO was not for us ! */
  }

  /* Test if the size of the SDO is ok */
  if ( (*m).len != 8) {
    MSG_ERR(0x1A67, "Error size SDO. CobId  : ", UNS16_LE(m->cob_id));
    failedSDO(d, nodeId, whoami, 0, 0, SDOABT_GENERAL_ERROR);
    return 0xFF;
  }

  if (whoami == SDO_CLIENT) {
    MSG_WAR(0x3A68, "I am CLIENT. Received SDO from nodeId : ", nodeId);
  }
  else {
    MSG_WAR(0x3A69, "I am SERVER. Received SDO cobId : ", UNS16_LE(m->cob_id));
  }

  /* Testing the command specifier */
  /* Allowed : cs = 0, 1, 2, 3, 4. (=  all except those for block tranfert). */
  /* cs = other : Not allowed -> abort. */
  switch (getSDOcs(m->data[0])) {

  case 0:
    /* I am SERVER */
    if (whoami == SDO_SERVER) {
      /* Receiving a download segment data. */
      /* A SDO transfert should have been yet initiated. */
      err = getSDOlineOnUse( d, nodeId, whoami, &line );
      if (!err)
	err = d->transfers[line].state != SDO_DOWNLOAD_IN_PROGRESS;
      if (err) {
	MSG_ERR(0x1A70, "SDO error : Received download segment for unstarted trans. index 0x1200 + ",
		nodeId);
	failedSDO(d, nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
	return 0xFF;
      }
      /* Reset the wathdog */
      RestartSDO_TIMER(line)
      MSG_WAR(0x3A71, "Received SDO download segment defined at index 0x1200 + ", nodeId);
      index = d->transfers[line].index;
      subIndex = d->transfers[line].subIndex;
      /* Toggle test. */
      if (d->transfers[line].toggle != getSDOt(m->data[0])) {
	MSG_ERR(0x1A72, "SDO error : Toggle error : ", getSDOt(m->data[0]));
	failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_TOGGLE_NOT_ALTERNED);
	return 0xFF;
      }
      /* Nb of data to be downloaded */
      nbBytes = 7 - getSDOn3(m->data[0]);
      /* Store the data in the transfert structure. */
      err = SDOtoLine(d, line, nbBytes, (*m).data + 1);
      if (err) {
	failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	return 0xFF;
      }
      /* Sending the SDO response, CS = 1 */
      sdo.nodeId = *d->bDeviceNodeId; /* The node id of the server, (here it is the sender). */
      sdo.body.data[0] = (1 << 5) | (d->transfers[line].toggle << 4);
      for (i = 1 ; i < 8 ; i++)
	sdo.body.data[i] = 0;
      MSG_WAR(0x3A73, "SDO. Send response to download request defined at index 0x1200 + ", nodeId);
      sendSDO(d, whoami, sdo);
      /* Inverting the toggle for the next segment. */
      d->transfers[line].toggle = ! d->transfers[line].toggle & 1;
      /* If it was the last segment, */
      if (getSDOc(m->data[0])) {
	/* Transfering line data to object dictionary. */
	/* The code does not use the "d" of initiate frame. So it is safe if e=s=0 */
	errorCode = SDOlineToObjdict(d, line);
	if (errorCode) {
	  MSG_ERR(0x1A54, "SDO error : Unable to copy the data in the object dictionary", 0);
	  failedSDO(d, nodeId, whoami, index, subIndex, errorCode);
	  return 0xFF;
	}
	/* Release of the line */
	resetSDOline(d, line);
	MSG_WAR(0x3A74, "SDO. End of download defined at index 0x1200 + ", nodeId);
      }
    } /* end if SERVER */
    else { /* if CLIENT */
      /* I am CLIENT */
      /* It is a request for a previous upload segment. We should find a line opened for this.*/
      err = getSDOlineOnUse( d, nodeId, whoami, &line);
      if (!err)
	err = d->transfers[line].state != SDO_UPLOAD_IN_PROGRESS;
      if (err) {
	MSG_ERR(0x1A75, "SDO error : Received segment response for unknown trans. from nodeId", nodeId);
	failedSDO(d, nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
	return 0xFF;
      }
      /* Reset the wathdog */
      RestartSDO_TIMER(line)
      index = d->transfers[line].index;
      subIndex = d->transfers[line].subIndex;
      /* test of the toggle; */
      if (d->transfers[line].toggle != getSDOt(m->data[0])) {
	MSG_ERR(0x1A76, "SDO error : Received segment response Toggle error. from nodeId", nodeId);
	failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_TOGGLE_NOT_ALTERNED);
	return 0xFF;
      }
      /* nb of data to be uploaded */
      nbBytes = 7 - getSDOn3(m->data[0]);
      /* Storing the data in the line structure. */
      err = SDOtoLine(d, line, nbBytes, (*m).data + 1);
      if (err) {
	failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	return 0xFF;
      }
      /* Inverting the toggle for the next segment. */
      d->transfers[line].toggle = ! d->transfers[line].toggle & 1;
      /* If it was the last segment,*/
      if ( getSDOc(m->data[0])) {
	/* Put in state finished */
	/* The code is safe for the case e=s=0 in initiate frame. */
	StopSDO_TIMER(line)
	d->transfers[line].state = SDO_FINISHED;
	if(d->transfers[line].Callback) (*d->transfers[line].Callback)(d,nodeId);

	MSG_WAR(0x3A77, "SDO. End of upload from node : ", nodeId);
      }
      else { /* more segments to receive */
	     /* Sending the request for the next segment. */
	sdo.nodeId = nodeId;
	sdo.body.data[0] = (3 << 5) | (d->transfers[line].toggle << 4);
	for (i = 1 ; i < 8 ; i++)
	  sdo.body.data[i] = 0;
	sendSDO(d, whoami, sdo);
	MSG_WAR(0x3A78, "SDO send upload segment request to nodeId", nodeId);
      }
    } /* End if CLIENT */
    break;

  case 1:
    /* I am SERVER */
    /* Receive of an initiate download */
    if (whoami == SDO_SERVER) {
      index = getSDOindex(m->data[1],m->data[2]);
      subIndex = getSDOsubIndex(m->data[3]);
      MSG_WAR(0x3A79, "Received SDO Initiate Download (to store data) defined at index 0x1200 + ",
	      nodeId);
      MSG_WAR(0x3A80, "Writing at index : ", index);
      MSG_WAR(0x3A80, "Writing at subIndex : ", subIndex);

      /* Search if a SDO transfert have been yet initiated */
      err = getSDOlineOnUse( d, nodeId, whoami, &line );
      if (! err) {
	MSG_ERR(0x1A81, "SDO error : Transmission yet started.", 0);
	failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_LOCAL_CTRL_ERROR);
	return 0xFF;
      }
      /* No line on use. Great ! */
      /* Try to open a new line. */
      err = getSDOfreeLine( d, whoami, &line );
      if (err) {
	MSG_ERR(0x1A82, "SDO error : No line free, too many SDO in progress. Aborted.", 0);
	failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_LOCAL_CTRL_ERROR);
	return 0xFF;
      }
      initSDOline(d, line, nodeId, index, subIndex, SDO_DOWNLOAD_IN_PROGRESS);

      if (getSDOe(m->data[0])) { /* If SDO expedited */
	/* nb of data to be downloaded */
	nbBytes = 4 - getSDOn2(m->data[0]);
	/* Storing the data in the line structure. */
	d->transfers[line].count = nbBytes;
	err = SDOtoLine(d, line, nbBytes, (*m).data + 4);

	if (err) {
	  failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	  return 0xFF;
	}

	/* SDO expedited -> transfert finished. Data can be stored in the dictionary. */
	/*The line will be reseted when it is downloading in the dictionary. */
	MSG_WAR(0x3A83, "SDO Initiate Download is an expedited transfert. Finished.: ", nodeId);
	/* Transfering line data to object dictionary. */
	errorCode = SDOlineToObjdict(d, line);
	if (errorCode) {
	  MSG_ERR(0x1A84, "SDO error : Unable to copy the data in the object dictionary", 0);
	  failedSDO(d, nodeId, whoami, index, subIndex, errorCode);
	  return 0xFF;
	}
	/* Release of the line. */
	resetSDOline(d, line);
      }
      else {/* So, if it is not an expedited transfert */
	if (getSDOs(m->data[0])) {
	  nbBytes = (m->data[4]) + ((UNS32)(m->data[5])<<8) + ((UNS32)(m->data[6])<<16) + ((UNS32)(m->data[7])<<24);
	  err = setSDOlineRestBytes(d, nodeId, nbBytes);
	  if (err) {
	    failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	    return 0xFF;
	  }
	}
      }
      /*Sending a SDO, cs=3*/
      sdo.nodeId = *d->bDeviceNodeId; /* The node id of the server, (here it is the sender).*/
      sdo.body.data[0] = 3 << 5;
      sdo.body.data[1] = index & 0xFF;        /* LSB */
      sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
      sdo.body.data[3] = subIndex;
      for (i = 4 ; i < 8 ; i++)
		sdo.body.data[i] = 0;
      sendSDO(d, whoami, sdo);
    } /* end if I am SERVER */
    else {
      /* I am CLIENT */
      /* It is a response for a previous download segment. We should find a line opened for this. */
      err = getSDOlineOnUse( d, nodeId, whoami, &line);
      if (!err)
	err = d->transfers[line].state != SDO_DOWNLOAD_IN_PROGRESS;
      if (err) {
	MSG_ERR(0x1A85, "SDO error : Received segment response for unknown trans. from nodeId", nodeId);
	failedSDO(d, nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
	return 0xFF;
      }
      /* Reset the wathdog */
      RestartSDO_TIMER(line)
      index = d->transfers[line].index;
      subIndex = d->transfers[line].subIndex;
      /* test of the toggle; */
      if (d->transfers[line].toggle != getSDOt(m->data[0])) {
	MSG_ERR(0x1A86, "SDO error : Received segment response Toggle error. from nodeId", nodeId);
	failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_TOGGLE_NOT_ALTERNED);
	return 0xFF;
      }

      /* End transmission or downloading next segment. We need to know if it will be the last one. */
      getSDOlineRestBytes(d, line, &nbBytes);
      if (nbBytes == 0) {
	MSG_WAR(0x3A87, "SDO End download. segment response received. OK. from nodeId", nodeId);
	StopSDO_TIMER(line)
	d->transfers[line].state = SDO_FINISHED;
	if(d->transfers[line].Callback) (*d->transfers[line].Callback)(d,nodeId);
	return 0x00;
      }
      /* At least one transfer to send.	*/
      if (nbBytes > 7) {
	/* several segments to download.*/
	/* code to send the next segment. (cs = 0; c = 0) */
	d->transfers[line].toggle = ! d->transfers[line].toggle & 1;
	sdo.nodeId = nodeId; /* The server node Id; */
	sdo.body.data[0] = (d->transfers[line].toggle << 4);
	err = lineToSDO(d, line, 7, sdo.body.data + 1);
	if (err) {
	  failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	  return 0xFF;
	}
      }
      else {
	/* Last segment. */
	/* code to send the last segment. (cs = 0; c = 1)*/
	d->transfers[line].toggle = ! d->transfers[line].toggle & 1;
	sdo.nodeId = nodeId; /* The server node Id; */
	sdo.body.data[0] = (UNS8)((d->transfers[line].toggle << 4) | ((7 - nbBytes) << 1) | 1);
	err = lineToSDO(d, line, nbBytes, sdo.body.data + 1);
	if (err) {
	  failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	  return 0xFF;
	}
	for (i = nbBytes + 1 ; i < 8 ; i++)
	  sdo.body.data[i] = 0;
      }
      MSG_WAR(0x3A88, "SDO sending download segment to nodeId", nodeId);
      sendSDO(d, whoami, sdo);
    } /* end if I am a CLIENT */
    break;

  case 2:
    /* I am SERVER */
    /* Receive of an initiate upload.*/
    if (whoami == SDO_SERVER) {
      index = getSDOindex(m->data[1],m->data[2]);
      subIndex = getSDOsubIndex(m->data[3]);
      MSG_WAR(0x3A89, "Received SDO Initiate upload (to send data) defined at index 0x1200 + ",
	      nodeId);
      MSG_WAR(0x3A90, "Reading at index : ", index);
      MSG_WAR(0x3A91, "Reading at subIndex : ", subIndex);
      /* Search if a SDO transfert have been yet initiated*/
      err = getSDOlineOnUse( d, nodeId, whoami, &line );
      if (! err) {
	    MSG_ERR(0x1A92, "SDO error : Transmission yet started at line : ", line);
        MSG_WAR(0x3A93, "nodeId = ", nodeId);
	    failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_LOCAL_CTRL_ERROR);
	    return 0xFF;
      }
      /* No line on use. Great !*/
      /* Try to open a new line.*/
      err = getSDOfreeLine( d, whoami, &line );
      if (err) {
	MSG_ERR(0x1A71, "SDO error : No line free, too many SDO in progress. Aborted.", 0);
	failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_LOCAL_CTRL_ERROR);
	return 0xFF;
      }
      initSDOline(d, line, nodeId, index, subIndex, SDO_UPLOAD_IN_PROGRESS);
      /* Transfer data from dictionary to the line structure. */
      errorCode = objdictToSDOline(d, line);

      if (errorCode) {
	MSG_ERR(0x1A94, "SDO error : Unable to copy the data from object dictionary. Err code : ",
		errorCode);
	failedSDO(d, nodeId, whoami, index, subIndex, errorCode);
	return 0xFF;
	}
      /* Preparing the response.*/
      getSDOlineRestBytes(d, line, &nbBytes);	/* Nb bytes to transfer ? */
      sdo.nodeId = nodeId; /* The server node Id; */
      if (nbBytes > 4) {
	/* normal transfert. (segmented). */
	/* code to send the initiate upload response. (cs = 2) */
	sdo.body.data[0] = (2 << 5) | 1;
	sdo.body.data[1] = index & 0xFF;        /* LSB */
	sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
	sdo.body.data[3] = subIndex;
    sdo.body.data[4] = (UNS8)nbBytes; /* Limitation of canfestival2 : Max tranfert is 256 bytes.*/
	/* It takes too much memory to upgrate to 2^32 because the size of data is also coded */
	/* in the object dictionary, at every index and subindex. */
	for (i = 5 ; i < 8 ; i++)
	  sdo.body.data[i] = 0;
	MSG_WAR(0x3A95, "SDO. Sending normal upload initiate response defined at index 0x1200 + ", nodeId);
	sendSDO(d, whoami, sdo);
      }
      else {
	/* Expedited upload. (cs = 2 ; e = 1) */
	sdo.body.data[0] = (UNS8)((2 << 5) | ((4 - nbBytes) << 2) | 3);
	sdo.body.data[1] = index & 0xFF;        /* LSB */
	sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
	sdo.body.data[3] = subIndex;
	err = lineToSDO(d, line, nbBytes, sdo.body.data + 4);
	if (err) {
	  failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	  return 0xFF;
	}
	for (i = 4 + nbBytes ; i < 8 ; i++)
	  sdo.body.data[i] = 0;
	MSG_WAR(0x3A96, "SDO. Sending expedited upload initiate response defined at index 0x1200 + ",
		nodeId);
	sendSDO(d, whoami, sdo);
	/* Release the line.*/
	resetSDOline(d, line);
      }
    } /* end if I am SERVER*/
    else {
      /* I am CLIENT */
      /* It is the response for the previous initiate upload request.*/
      /* We should find a line opened for this. */
      err = getSDOlineOnUse( d, nodeId, whoami, &line);
      if (!err)
	err = d->transfers[line].state != SDO_UPLOAD_IN_PROGRESS;
      if (err) {
	MSG_ERR(0x1A97, "SDO error : Received response for unknown upload request from nodeId", nodeId);
	failedSDO(d, nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
	return 0xFF;
      }
      /* Reset the wathdog */
      RestartSDO_TIMER(line)
      index = d->transfers[line].index;
      subIndex = d->transfers[line].subIndex;

      if (getSDOe(m->data[0])) { /* If SDO expedited */
	/* nb of data to be uploaded */
	  nbBytes = 4 - getSDOn2(m->data[0]);
	/* Storing the data in the line structure. */
	err = SDOtoLine(d, line, nbBytes, (*m).data + 4);
	if (err) {
	  failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	  return 0xFF;
	}
	/* SDO expedited -> transfert finished. data are available via  getReadResultNetworkDict(). */
	MSG_WAR(0x3A98, "SDO expedited upload finished. Response received from node : ", nodeId);
	StopSDO_TIMER(line)
	d->transfers[line].count = nbBytes;
	d->transfers[line].state = SDO_FINISHED;
	if(d->transfers[line].Callback) (*d->transfers[line].Callback)(d,nodeId);
	return 0;
      }
      else { /* So, if it is not an expedited transfert */
	/* Storing the nb of data to receive. */
	if (getSDOs(m->data[0])) {
	  nbBytes = m->data[4] + ((UNS32)(m->data[5])<<8) + ((UNS32)(m->data[6])<<16) + ((UNS32)(m->data[7])<<24);
	  err = setSDOlineRestBytes(d, line, nbBytes);
	  if (err) {
	    failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	    return 0xFF;
	  }
	}
	/* Requesting next segment. (cs = 3) */
	sdo.nodeId = nodeId;
	sdo.body.data[0] = 3 << 5;
	for (i = 1 ; i < 8 ; i++)
	  sdo.body.data[i] = 0;
	MSG_WAR(0x3A99, "SDO. Sending upload segment request to node : ", nodeId);
	sendSDO(d, whoami, sdo);
      }
    } /* End if CLIENT */
    break;

  case 3:
    /* I am SERVER */
    if (whoami == SDO_SERVER) {
      /* Receiving a upload segment. */
      /* A SDO transfert should have been yet initiated. */
      err = getSDOlineOnUse( d, nodeId, whoami, &line );
      if (!err)
	err = d->transfers[line].state != SDO_UPLOAD_IN_PROGRESS;
      if (err) {
	MSG_ERR(0x1AA0, "SDO error : Received upload segment for unstarted trans. index 0x1200 + ",
		nodeId);
	failedSDO(d, nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
	return 0xFF;
      }
      /* Reset the wathdog */
      RestartSDO_TIMER(line)
      MSG_WAR(0x3AA1, "Received SDO upload segment defined at index 0x1200 + ", nodeId);
      index = d->transfers[line].index;
      subIndex = d->transfers[line].subIndex;
      /* Toggle test.*/
      if (d->transfers[line].toggle != getSDOt(m->data[0])) {
	MSG_ERR(0x1AA2, "SDO error : Toggle error : ", getSDOt(m->data[0]));
	failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_TOGGLE_NOT_ALTERNED);
	return 0xFF;
      }
      /* Uploading next segment. We need to know if it will be the last one. */
      getSDOlineRestBytes(d, line, &nbBytes);
      if (nbBytes > 7) {
	/* The segment to transfer is not the last one.*/
	/* code to send the next segment. (cs = 0; c = 0) */
	sdo.nodeId = nodeId; /* The server node Id; */
	sdo.body.data[0] = (d->transfers[line].toggle << 4);
	err = lineToSDO(d, line, 7, sdo.body.data + 1);
	if (err) {
	  failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	  return 0xFF;
	}
	/* Inverting the toggle for the next tranfert. */
	d->transfers[line].toggle = ! d->transfers[line].toggle & 1;
	MSG_WAR(0x3AA3, "SDO. Sending upload segment defined at index 0x1200 + ", nodeId);
	sendSDO(d, whoami, sdo);
      }
      else {
	/* Last segment. */
	/* code to send the last segment. (cs = 0; c = 1) */
	sdo.nodeId = nodeId; /** The server node Id; */
	sdo.body.data[0] = (UNS8)((d->transfers[line].toggle << 4) | ((7 - nbBytes) << 1) | 1);
	err = lineToSDO(d, line, nbBytes, sdo.body.data + 1);
	if (err) {
	  failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	  return 0xFF;
	}
	for (i = nbBytes + 1 ; i < 8 ; i++)
	  sdo.body.data[i] = 0;
	MSG_WAR(0x3AA4, "SDO. Sending last upload segment defined at index 0x1200 + ", nodeId);
	sendSDO(d, whoami, sdo);
	/* Release the line */
	resetSDOline(d, line);
      }
    } /* end if SERVER*/
    else {
      /* I am CLIENT */
      /* It is the response for the previous initiate download request. */
      /* We should find a line opened for this. */
      err = getSDOlineOnUse( d, nodeId, whoami, &line);
      if (!err)
	err = d->transfers[line].state != SDO_DOWNLOAD_IN_PROGRESS;
      if (err) {
	MSG_ERR(0x1AA5, "SDO error : Received response for unknown download request from nodeId", nodeId);
	failedSDO(d, nodeId, whoami, 0, 0, SDOABT_LOCAL_CTRL_ERROR);
	return 0xFF;
      }
      /* Reset the watchdog */
      RestartSDO_TIMER(line)
      index = d->transfers[line].index;
      subIndex = d->transfers[line].subIndex;
      /* End transmission or requesting  next segment. */
      getSDOlineRestBytes(d, line, &nbBytes);
      if (nbBytes == 0) {
	MSG_WAR(0x3AA6, "SDO End download expedited. Response received. from nodeId", nodeId);
	StopSDO_TIMER(line)
	d->transfers[line].state = SDO_FINISHED;
	if(d->transfers[line].Callback) (*d->transfers[line].Callback)(d,nodeId);
	return 0x00;
      }
      if (nbBytes > 7) {
	/* more than one request to send */
	/* code to send the next segment. (cs = 0; c = 0)	*/
	sdo.nodeId = nodeId; /** The server node Id; */
	sdo.body.data[0] = (d->transfers[line].toggle << 4);
	err = lineToSDO(d, line, 7, sdo.body.data + 1);
	if (err) {
	  failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	  return 0xFF;
	}
      }
      else {
	/* Last segment.*/
	/* code to send the last segment. (cs = 0; c = 1)	*/
	sdo.nodeId = nodeId; /* The server node Id; */
	sdo.body.data[0] = (UNS8)((d->transfers[line].toggle << 4) | ((7 - nbBytes) << 1) | 1);
	err = lineToSDO(d, line, nbBytes, sdo.body.data + 1);
	if (err) {
	  failedSDO(d, nodeId, whoami, index, subIndex, SDOABT_GENERAL_ERROR);
	  return 0xFF;
	}
	for (i = nbBytes + 1 ; i < 8 ; i++)
	  sdo.body.data[i] = 0;
      }
      MSG_WAR(0x3AA7, "SDO sending download segment to nodeId", nodeId);
      sendSDO(d, whoami, sdo);

    } /* end if I am a CLIENT		*/
    break;

   case 4:
     abortCode =
      (UNS32)m->data[4] |
      ((UNS32)m->data[5] << 8) |
      ((UNS32)m->data[6] << 16) |
      ((UNS32)m->data[7] << 24);
    /* Received SDO abort. */
    /* Looking for the line concerned. */
    if (whoami == SDO_SERVER) {
      err = getSDOlineOnUse( d, nodeId, whoami, &line );
      if (!err) {
	resetSDOline( d, line );
	MSG_WAR(0x3AA8, "SD0. Received SDO abort. Line released. Code : ", abortCode);
      }
      else
	MSG_WAR(0x3AA9, "SD0. Received SDO abort. No line found. Code : ", abortCode);
      /* Tips : The end user has no way to know that the server node has received an abort SDO. */
      /* Its is ok, I think.*/
    }
    else { /* If I am CLIENT */
      err = getSDOlineOnUse( d, nodeId, whoami, &line );
      if (!err) {
	/* The line *must* be released by the core program. */
	StopSDO_TIMER(line)
	d->transfers[line].state = SDO_ABORTED_RCV;
	d->transfers[line].abortCode = abortCode;
	MSG_WAR(0x3AB0, "SD0. Received SDO abort. Line state ABORTED. Code : ", abortCode);
	if(d->transfers[line].Callback) (*d->transfers[line].Callback)(d,nodeId);
      }
      else
	MSG_WAR(0x3AB1, "SD0. Received SDO abort. No line found. Code : ", abortCode);
    }
    break;
  default:
    /* Error : Unknown cs */
    MSG_ERR(0x1AB2, "SDO. Received unknown command specifier : ", getSDOcs(m->data[0]));
    return 0xFF;

  } /* End switch */
  return 0;
}

/*!
**
**
** @param d
** @param nodeId
** @param index
** @param subIndex
** @param count
** @param dataType
** @param data
** @param Callback
** @param endianize
**
** @return
**/
INLINE UNS8 _writeNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data, SDOCallback_t Callback, UNS8 endianize)
{
  UNS8 err;
  UNS8 SDOfound = 0;
  UNS8 line;
  s_SDO sdo;    /* SDO to transmit */
  UNS8 i;
  UNS32 j;
  UNS16     lastIndex;
  UNS16     offset;
  UNS8      *pNodeIdServer;
  UNS8      nodeIdServer;

  MSG_WAR(0x3AC0, "Send SDO to write in the dictionary of node : ", nodeId);
  MSG_WAR(0x3AC1, "                                   At index : ", index);
  MSG_WAR(0x3AC2, "                                   subIndex : ", subIndex);
  MSG_WAR(0x3AC3, "                                   nb bytes : ", count);

  /* Verify that there is no SDO communication yet. */
  err = getSDOlineOnUse(d, nodeId, SDO_CLIENT, &line);
  if (!err) {
    MSG_ERR(0x1AC4, "SDO error : Communication yet established. with node : ", nodeId);
    return 0xFF;
  }
  /* Taking the line ... */
  err = getSDOfreeLine( d, SDO_CLIENT, &line );
  if (err) {
    MSG_ERR(0x1AC5, "SDO error : No line free, too many SDO in progress. Aborted for node : ", nodeId);
    return (0xFF);
  }
  /* Check which SDO to use to communicate with the node */
  offset = d->firstIndex->SDO_CLT;
  lastIndex = d->lastIndex->SDO_CLT;
  if (offset == 0) {
    MSG_ERR(0x1AC6, "writeNetworkDict : No SDO client index found", 0);
    return 0xFF;
  }
  i = 0;
   while (offset <= lastIndex) {
     if (d->objdict[offset].bSubCount <= 3) {
	 MSG_ERR(0x1AC8, "Subindex 3  not found at index ", 0x1280 + i);
	 return 0xFF;
     }
     /* looking for the nodeId server */
     pNodeIdServer = (UNS8*) d->objdict[offset].pSubindex[3].pObject;
     nodeIdServer = *pNodeIdServer;
     MSG_WAR(0x1AD2, "index : ", 0x1280 + i);
     MSG_WAR(0x1AD3, "nodeIdServer : ", nodeIdServer);

    if(nodeIdServer == nodeId) {
      SDOfound = 1;
      break;
    }
    offset++;
    i++;
  } /* end while */
  if (!SDOfound) {
    MSG_ERR(0x1AC9, "SDO. Error. No client found to communicate with node : ", nodeId);
    return 0xFE;
  }
  MSG_WAR(0x3AD0,"        SDO client defined at index  : ", 0x1280 + i);
  initSDOline(d, line, nodeId, index, subIndex, SDO_DOWNLOAD_IN_PROGRESS);
  d->transfers[line].count = count;
  d->transfers[line].dataType = dataType;

  /* Copy data to transfers structure. */
  for (j = 0 ; j < count ; j++) {
# ifdef CANOPEN_BIG_ENDIAN
    if (dataType == 0 && endianize)
      d->transfers[line].data[count - 1 - j] = ((char *)data)[j];
    else /* String of bytes. */
      d->transfers[line].data[j] = ((char *)data)[j];
#  else
    d->transfers[line].data[j] = ((char *)data)[j];
#  endif
  }
  /* Send the SDO to the server. Initiate download, cs=1. */
  sdo.nodeId = nodeId;
  if (count <= 4) { /* Expedited transfert */
    sdo.body.data[0] = (UNS8)((1 << 5) | ((4 - count) << 2) | 3);
    for (i = 4 ; i < 8 ; i++)
      sdo.body.data[i] = d->transfers[line].data[i - 4];
    d->transfers[line].offset = count;
  }
  else { /** Normal transfert */
    sdo.body.data[0] = (1 << 5) | 1;
    for (i = 0 ; i < 4 ; i++)
      sdo.body.data[i+4] = (UNS8)((count << (i<<3))); /* i*8 */
  }
  sdo.body.data[1] = index & 0xFF;        /* LSB */
  sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
  sdo.body.data[3] = subIndex;

  d->transfers[line].Callback = Callback;

  err = sendSDO(d, SDO_CLIENT, sdo);
  if (err) {
    MSG_ERR(0x1AD1, "SDO. Error while sending SDO to node : ", nodeId);
    /* release the line */
    resetSDOline(d, line);
    return 0xFF;
  }


  return 0;
}

/*!
**
**
** @param d
** @param nodeId
** @param index
** @param subIndex
** @param count
** @param dataType
** @param data
**
** @return
**/
UNS8 writeNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data)
{
	return _writeNetworkDict (d, nodeId, index, subIndex, count, dataType, data, NULL, 1);
}

/*!
**
**
** @param d
** @param nodeId
** @param index
** @param subIndex
** @param count
** @param dataType
** @param data
** @param Callback
**
** @return
**/
UNS8 writeNetworkDictCallBack (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data, SDOCallback_t Callback)
{
	return _writeNetworkDict (d, nodeId, index, subIndex, count, dataType, data, Callback, 1);
}

UNS8 writeNetworkDictCallBackAI (CO_Data* d, UNS8 nodeId, UNS16 index,
		       UNS8 subIndex, UNS32 count, UNS8 dataType, void *data, SDOCallback_t Callback, UNS8 endianize)
{
	UNS8 ret;
	UNS16 lastIndex;
	UNS16 offset;
	UNS8 nodeIdServer;
	UNS8 i;

	ret = _writeNetworkDict (d, nodeId, index, subIndex, count, dataType, data, Callback, endianize);
	if(ret == 0xFE)
	{
		offset = d->firstIndex->SDO_CLT;
		lastIndex = d->lastIndex->SDO_CLT;
		if (offset == 0)
		{
			MSG_ERR(0x1AC6, "writeNetworkDict : No SDO client index found", 0);
			return 0xFF;
		}
		i = 0;
		while (offset <= lastIndex)
		{
			if (d->objdict[offset].bSubCount <= 3)
		    {
		    	MSG_ERR(0x1AC8, "Subindex 3  not found at index ", 0x1280 + i);
		    	return 0xFF;
		    }
			nodeIdServer = *(UNS8*) d->objdict[offset].pSubindex[3].pObject;
			if(nodeIdServer == 0)
			{
				*(UNS32*)d->objdict[offset].pSubindex[1].pObject = (UNS32)(0x600 + nodeId);
				*(UNS32*)d->objdict[offset].pSubindex[2].pObject = (UNS32)(0x580 + nodeId);
				*(UNS8*) d->objdict[offset].pSubindex[3].pObject = nodeId;
				return _writeNetworkDict (d, nodeId, index, subIndex, count, dataType, data, Callback, 1);
			}
			offset++;
		}
		return 0xFF;
	}
	else if(ret == 0)
	{
		return 0;
	}
	else
	{
		return 0xFF;
	}
}

/*!
**
**
** @param d
** @param nodeId
** @param index
** @param subIndex
** @param dataType
** @param Callback
**
** @return
**/
INLINE UNS8 _readNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback)
{
  UNS8 err;
  UNS8 SDOfound = 0;
  UNS8 i;
  UNS8 line;
  s_SDO sdo;    /* SDO to transmit */
  UNS8      *pNodeIdServer;
  UNS8      nodeIdServer;
  UNS16     offset;
  UNS16     lastIndex;
  MSG_WAR(0x3AD5, "Send SDO to read in the dictionary of node : ", nodeId);
  MSG_WAR(0x3AD6, "                                  At index : ", index);
  MSG_WAR(0x3AD7, "                                  subIndex : ", subIndex);


  /* Verify that there is no SDO communication yet. */
  err = getSDOlineOnUse(d, nodeId, SDO_CLIENT, &line);
  if (!err) {
    MSG_ERR(0x1AD8, "SDO error : Communication yet established. with node : ", nodeId);
    return 0xFF;
  }
  /* Taking the line ... */
  err = getSDOfreeLine( d, SDO_CLIENT, &line );
  if (err) {
    MSG_ERR(0x1AD9, "SDO error : No line free, too many SDO in progress. Aborted for node : ", nodeId);
    return (0xFF);
  }
  else
    MSG_WAR(0x3AE0, "Transmission on line : ", line);

  /* Check which SDO to use to communicate with the node */
  offset = d->firstIndex->SDO_CLT;
  lastIndex = d->lastIndex->SDO_CLT;
  if (offset == 0) {
    MSG_ERR(0x1AE1, "writeNetworkDict : No SDO client index found", 0);
    return 0xFF;
  }
  i = 0;
  while (offset <= lastIndex) {
     if (d->objdict[offset].bSubCount <= 3) {
	 MSG_ERR(0x1AE2, "Subindex 3  not found at index ", 0x1280 + i);
	 return 0xFF;
     }
     /* looking for the nodeId server */
     pNodeIdServer = (UNS8*) d->objdict[offset].pSubindex[3].pObject;
     nodeIdServer = *pNodeIdServer;

    if(nodeIdServer == nodeId) {
      SDOfound = 1;
      break;
    }
    offset++;
    i++;
  } /* end while */
  if (!SDOfound) {
    MSG_ERR(0x1AE3, "SDO. Error. No client found to communicate with node : ", nodeId);
    return 0xFE;
  }
  MSG_WAR(0x3AE4,"        SDO client defined at index  : ", 0x1280 + i);
  initSDOline(d, line, nodeId, index, subIndex, SDO_UPLOAD_IN_PROGRESS);
  getSDOlineOnUse(d, nodeId, SDO_CLIENT, &line);
  sdo.nodeId = nodeId;
  /* Send the SDO to the server. Initiate upload, cs=2. */
  d->transfers[line].dataType = dataType;
  sdo.body.data[0] = (2 << 5);
  sdo.body.data[1] = index & 0xFF;        /* LSB */
  sdo.body.data[2] = (index >> 8) & 0xFF; /* MSB */
  sdo.body.data[3] = subIndex;
  for (i = 4 ; i < 8 ; i++)
    sdo.body.data[i] = 0;
  d->transfers[line].Callback = Callback;
  err = sendSDO(d, SDO_CLIENT, sdo);
  if (err) {
    MSG_ERR(0x1AE5, "SDO. Error while sending SDO to node : ", nodeId);
    /* release the line */
    resetSDOline(d, line);
    return 0xFF;
  }
  return 0;
}

/*!
**
**
** @param d
** @param nodeId
** @param index
** @param subIndex
** @param dataType
**
** @return
**/
UNS8 readNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType)
{
	return _readNetworkDict (d, nodeId, index, subIndex, dataType, NULL);
}

/*!
**
**
** @param d
** @param nodeId
** @param index
** @param subIndex
** @param dataType
** @param Callback
**
** @return
**/
UNS8 readNetworkDictCallback (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback)
{
	return _readNetworkDict (d, nodeId, index, subIndex, dataType, Callback);
}

UNS8 readNetworkDictCallbackAI (CO_Data* d, UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, SDOCallback_t Callback)
{
	UNS8 ret;
	UNS16 lastIndex;
	UNS16 offset;
	UNS8 nodeIdServer;
	UNS8 i;

	ret = _readNetworkDict (d, nodeId, index, subIndex, dataType, Callback);
	if(ret == 0xFE)
	{
		offset = d->firstIndex->SDO_CLT;
		lastIndex = d->lastIndex->SDO_CLT;
		if (offset == 0)
		{
			MSG_ERR(0x1AC6, "writeNetworkDict : No SDO client index found", 0);
			return 0xFF;
		}
		i = 0;
		while (offset <= lastIndex)
		{
			if (d->objdict[offset].bSubCount <= 3)
		    {
		    	MSG_ERR(0x1AC8, "Subindex 3  not found at index ", 0x1280 + i);
		    	return 0xFF;
		    }
			nodeIdServer = *(UNS8*) d->objdict[offset].pSubindex[3].pObject;
			if(nodeIdServer == 0)
			{
				*(UNS32*)d->objdict[offset].pSubindex[1].pObject = (UNS32)(0x600 + nodeId);
				*(UNS32*)d->objdict[offset].pSubindex[2].pObject = (UNS32)(0x580 + nodeId);
				*(UNS8*) d->objdict[offset].pSubindex[3].pObject = nodeId;
				return _readNetworkDict (d, nodeId, index, subIndex, dataType, Callback);
			}
			offset++;
		}
		return 0xFF;
	}
	else if(ret == 0)
	{
		return 0;
	}
	else
	{
		return 0xFF;
	}
}

/*!
**
**
** @param d
** @param nodeId
** @param data
** @param size pointer to expected size, changed into returned size. Expected size will be truncated to transfered data size
** @param abortCode
**
** @return
**/
UNS8 getReadResultNetworkDict (CO_Data* d, UNS8 nodeId, void* data, UNS32 *size,
			       UNS32 * abortCode)
{
  UNS32 i;
  UNS8 err;
  UNS8 line;
  * abortCode = 0;

  /* Looking for the line tranfert. */
  err = getSDOlineOnUse(d, nodeId, SDO_CLIENT, &line);
  if (err) {
    MSG_ERR(0x1AF0, "SDO error : No line found for communication with node : ", nodeId);
    return SDO_ABORTED_INTERNAL;
  }
  * abortCode = d->transfers[line].abortCode;
  if (d->transfers[line].state != SDO_FINISHED)
    return d->transfers[line].state;

  /* if SDO initiated with e=0 and s=0 count is null, offset carry effective size*/
  if( d->transfers[line].count == 0)
  	d->transfers[line].count = d->transfers[line].offset;
  /* use transfers[line].count as max size */
  if( d->transfers[line].count < *size )
  	*size = d->transfers[line].count;
  /* Copy payload to data pointer */
  for  ( i = 0 ; i < *size ; i++) {
# ifdef CANOPEN_BIG_ENDIAN
    if (d->transfers[line].dataType != visible_string)
      ( (char *) data)[*size - 1 - i] = d->transfers[line].data[i];
    else /* String of bytes. */
      ( (char *) data)[i] = d->transfers[line].data[i];
# else
    ( (char *) data)[i] = d->transfers[line].data[i];
# endif
  }
  return SDO_FINISHED;
}

/*!
**
**
** @param d
** @param nodeId
** @param abortCode
**
** @return
**/
UNS8 getWriteResultNetworkDict (CO_Data* d, UNS8 nodeId, UNS32 * abortCode)
{
  UNS8 line = 0;
  UNS8 err;

  * abortCode = 0;
  /* Looking for the line tranfert. */
  err = getSDOlineOnUse(d, nodeId, SDO_CLIENT, &line);
  if (err) {
    MSG_ERR(0x1AF1, "SDO error : No line found for communication with node : ", nodeId);
    return SDO_ABORTED_INTERNAL;
  }
  * abortCode = d->transfers[line].abortCode;
  return d->transfers[line].state;
}
