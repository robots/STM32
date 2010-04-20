/*
  This file is part of CanFestival, a library implementing CanOpen
  Stack.

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
  USA
*/


/**
** @file   dcf.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Mon Jun  4 17:06:12 2007
**
** @brief EXEMPLE OF SOMMARY
**
**
*/


#include "data.h"
#include "sysdep.h"

extern UNS8 _writeNetworkDict (CO_Data* d, UNS8 nodeId, UNS16 index,
                               UNS8 subIndex, UNS8 count, UNS8 dataType, void *data, SDOCallback_t Callback, UNS8 endianize);


static void send_consise_dcf_loop(CO_Data* d,UNS8 nodeId);

/* Seek to next NodeID's DCF */
#define SEEK_NEXT_DCF() \
   	nodeId=(nodeId+1) % d->dcf_odentry->bSubCount; \
   	if(nodeId==0) nodeId=1; \
   	d->dcf_cursor = NULL;

/**
**
**
** @param d
** @param nodeId
*/
static void CheckSDOAndContinue(CO_Data* d, UNS8 nodeId)
{
  UNS32 abortCode = 0;

  if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED)
    {
      MSG_ERR(0x1A01, "SDO error in consise DCF", abortCode);
      MSG_WAR(0x2A02, "server node : ", nodeId);
    }

  closeSDOtransfer(d, nodeId, SDO_CLIENT);
  /* Timedout ? */
  if(abortCode == SDOABT_TIMED_OUT){
    /* Node may not be ready, try another one */
    /* Warning, this might leed to endless attempts */
    /* if node does never answer */
	SEEK_NEXT_DCF()
  }
  send_consise_dcf_loop(d,nodeId);
}


/**
**
**
** @param d
** @param nodeId
**
** @return
*/
UNS8 send_consise_dcf(CO_Data* d,UNS8 nodeId)
{
  UNS32 szData;
  /* Fetch DCF OD entry, if not already done */
  if(!d->dcf_odentry)
  {
    UNS32 errorCode;
    ODCallback_t *Callback;
    d->dcf_odentry = (*d->scanIndexOD)(0x1F22, &errorCode, &Callback);
    /* If DCF entry do not exist... Nothing to do.*/
    if (errorCode != OD_SUCCESSFUL) goto DCF_finish;
  }

  szData = d->dcf_odentry->pSubindex[nodeId].size;
  
  /* if the entry for the nodeId is not empty. */
  if(szData!=0){
  	/* if the entry for the nodeId is already been processing, quit.*/
  	if(d->dcf_odentry->pSubindex[nodeId].bAccessType & DCF_TO_SEND) return 1;
  	
  	d->dcf_odentry->pSubindex[nodeId].bAccessType|=DCF_TO_SEND;
  	d->dcf_request++;
  	if(d->dcf_request==1)
  		send_consise_dcf_loop(d,nodeId);
  	return 1;
  }
  
  DCF_finish:
  return 0;
}

static void send_consise_dcf_loop(CO_Data* d,UNS8 nodeId)
{
  if(nodeId > d->dcf_odentry->bSubCount) return;
/* Loop on all DCF subindexes, corresponding to node ID until there is no request*/
  //while (nodeId < d->dcf_odentry->bSubCount){
  while (d->dcf_request>0){
  	if(d->dcf_odentry->pSubindex[nodeId].bAccessType & DCF_TO_SEND){   	 
        UNS8* dcfend;
  		UNS32 nb_entries;
  		UNS32 szData = d->dcf_odentry->pSubindex[nodeId].size;
      	 
   		{
	   		UNS8* dcf = *((UNS8**)d->dcf_odentry->pSubindex[nodeId].pObject);
   			dcfend = dcf + szData;
	   		if (!d->dcf_cursor){
    	  		d->dcf_cursor = (UNS8*)dcf + 4;
       			d->dcf_entries_count = 0;
   			}
   			nb_entries = UNS32_LE(*((UNS32*)dcf));
   		}

    	/* condition on consise DCF string for NodeID, if big enough */
    	if((UNS8*)d->dcf_cursor + 7 < (UNS8*)dcfend && d->dcf_entries_count < nb_entries){
    	
        	UNS16 target_Index;
        	UNS8 target_Subindex;
        	UNS32 target_Size;

			/* DCF data may not be 32/16b aligned, 
			 * we cannot directly dereference d->dcf_cursor 
			 * as UNS16 or UNS32 
			 * Do it byte per byte taking care on endianess*/
#ifdef CANOPEN_BIG_ENDIAN
        	target_Index = *(d->dcf_cursor++) << 8 | 
        	               *(d->dcf_cursor++);
#else
        	memcpy(&target_Index, d->dcf_cursor,2);
        	d->dcf_cursor+=2;
#endif

        	target_Subindex = *(d->dcf_cursor++);

#ifdef CANOPEN_BIG_ENDIAN
        	target_Size = *(d->dcf_cursor++) << 24 | 
        	              *(d->dcf_cursor++) << 16 | 
        	              *(d->dcf_cursor++) << 8 | 
        	              *(d->dcf_cursor++);
#else
        	memcpy(&target_Size, d->dcf_cursor,4);
        	d->dcf_cursor+=4;
#endif
	
    	    _writeNetworkDict(d, /* CO_Data* d*/
                                nodeId, /* UNS8 nodeId*/
                                target_Index, /* UNS16 index*/
                                target_Subindex, /* UNS8 subindex*/
                                (UNS8)target_Size, /* UNS8 count*/
                                0, /* UNS8 dataType*/
                                d->dcf_cursor,/* void *data*/
                                CheckSDOAndContinue,/* SDOCallback_t
                                                      Callback*/
                                0); /* no endianize*/
        	/* Push d->dcf_cursor to the end of data*/

        	d->dcf_cursor += target_Size;
        	d->dcf_entries_count++;

        	/* send_consise_dcf_loop will be called by CheckSDOAndContinue for next DCF entry*/
        	return;
      	}
      	else
      	{
      		/* We have finished with the dcf entry. Change the flag, decrement the request
      		 *  and execute the bootup callback. */
      		d->dcf_odentry->pSubindex[nodeId].bAccessType&=~DCF_TO_SEND;
      		d->dcf_request--;
      		(*d->post_SlaveBootup)(d, nodeId);
      	}
 	}
 	
	SEEK_NEXT_DCF()
  }

}
