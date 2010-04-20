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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

/* driver pcan pci for Peak board */
//#include "libpcan.h"
//#include "pcan.h"

#include "libpcan.h" // for CAN_HANDLE

#include "can_driver.h"

// Define for rtr CAN message
#define CAN_INIT_TYPE_ST_RTR MSGTYPE_STANDARD | MSGTYPE_RTR 

/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
{
  UNS8 data; 
  TPCANMsg peakMsg;
  if ((errno = CAN_Read(fd0, & peakMsg))) {		// Blocks until no new message or error.
    if(errno != -EIDRM && errno != -EPERM) // error is not "Can Port closed while reading" 
    {
    	perror("canReceive_driver (Peak_Linux) : error of reading.\n");
    }
    return 1;
  }
  m->cob_id = peakMsg.ID;   
  if (peakMsg.MSGTYPE == CAN_INIT_TYPE_ST)         	/* bits of MSGTYPE_*/
    m->rtr = 0;
  else 
    m->rtr = 1;
  m->len = peakMsg.LEN;					/* count of data bytes (0..8) */
  for(data = 0  ; data < peakMsg.LEN ; data++)             			
    m->data[data] = peakMsg.DATA[data];         	/* data bytes, up to 8 */

#if defined DEBUG_MSG_CONSOLE_ON
  MSG("in : ");
  print_message(m);
#endif
  
  return 0;
}

/***************************************************************************/
UNS8 canSend_driver(CAN_HANDLE fd0, Message *m)
{
  UNS8 data;
  TPCANMsg peakMsg;
  peakMsg.ID=m -> cob_id;              			/* 11/29 bit code */
  if(m->rtr == 0)	
    peakMsg.MSGTYPE = CAN_INIT_TYPE_ST;       /* bits of MSGTYPE_*/
  else {
    peakMsg.MSGTYPE = CAN_INIT_TYPE_ST_RTR;       /* bits of MSGTYPE_*/
  }
  peakMsg.LEN = m->len;   
          			/* count of data bytes (0..8) */
  for(data = 0 ; data <  m->len; data ++)
  	peakMsg.DATA[data] = m->data[data];         	/* data bytes, up to 8 */
  
#if defined DEBUG_MSG_CONSOLE_ON
  MSG("out : ");
  print_message(m);
#endif
  if((errno = CAN_Write(fd0, & peakMsg))) {
    perror("canSend_driver (Peak_Linux) : error of writing.\n");
    return 1;
  }
  return 0;

}


/***************************************************************************/
int TranslateBaudeRate(char* optarg){
	if(!strcmp( optarg, "1M")) return CAN_BAUD_1M;
	if(!strcmp( optarg, "500K")) return CAN_BAUD_500K;
	if(!strcmp( optarg, "250K")) return CAN_BAUD_250K;
	if(!strcmp( optarg, "125K")) return CAN_BAUD_125K;
	if(!strcmp( optarg, "100K")) return CAN_BAUD_100K;
	if(!strcmp( optarg, "50K")) return CAN_BAUD_50K;
	if(!strcmp( optarg, "20K")) return CAN_BAUD_20K;
	if(!strcmp( optarg, "10K")) return CAN_BAUD_10K;
	if(!strcmp( optarg, "5K")) return CAN_BAUD_5K;
	if(!strcmp( optarg, "none")) return 0;
	return 0x0000;
}

UNS8 canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
{
	printf("canChangeBaudRate not yet supported by this driver\n");
	return 0;
}

/***************************************************************************/
CAN_HANDLE canOpen_driver(s_BOARD *board)
{
  HANDLE fd0 = NULL;
  char busname[64];
  char* pEnd;
  int baudrate;
  
  if(strtol(board->busname, &pEnd,0) >= 0)
  {
    sprintf(busname,"/dev/pcan%s",board->busname);
    fd0 = LINUX_CAN_Open(busname, O_RDWR);
  }

  if(fd0 && (baudrate = TranslateBaudeRate(board->baudrate)))
  {
   	CAN_Init(fd0, baudrate, CAN_INIT_TYPE_ST);
  }else{
  	fprintf(stderr, "canOpen_driver (Peak_Linux) : error opening %s\n", busname);
  }

   return (CAN_HANDLE)fd0;
}

/***************************************************************************/
int canClose_driver(CAN_HANDLE fd0)
{
  CAN_Close(fd0);
  return 0;
}
