/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): VScom

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
 * @file can_vscom.c
 * @author VScom (http://www.vscom.de)
 * @date 17/04/08
 *
 * This file implements interface between CanFestival and the VSCAN API 
 * (supported VScom products SER-CAN, USB-CAN, NET-CAN). 
 *
 * To build this interface following files will needed:
 * vs_can_api.h
 * va_can_api.lib (for Windows builds)
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

// VSCAN API header
#include "vs_can_api.h" // for CAN_HANDLE

#include "can_driver.h"

/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
{
  VSCAN_MSG Msg[1];
  UNS8 i; 
  DWORD dwRead; /* number of read frames */


  if (VSCAN_Read((VSCAN_HANDLE)fd0, Msg, 1, &dwRead) != VSCAN_ERR_OK)
  {
    printf("canReceive_driver (VScom): error receiving frame)\n");
    return 1;
  }

  /* identifier of the CAN frame */
  m->cob_id = Msg[0].Id;   

  /* CAN frame type */
  if (Msg[0].Flags == VSCAN_FLAGS_STANDARD)
    m->rtr = 0;
  else 
    m->rtr = 1;

  /* width of the data bytes */
  m->len = Msg[0].Size;	
  
  /* copy data bytes from the CAN frame, up to 8 */
  for(i = 0  ; i < Msg[0].Size ; i++)             			
  {
    m->data[i] = Msg[0].Data[i];           
  }

  return 0;
}

/***************************************************************************/
UNS8 canSend_driver(CAN_HANDLE fd0, Message *m)
{
  VSCAN_MSG Msg[1];
  UNS8 i;
  DWORD dwWritten; /* number of written frames */

  /* identifier of the CAN frame */
  Msg[0].Id = m->cob_id;

  /* CAN frame type */
  if(m->rtr == 0)	
    Msg[0].Flags = VSCAN_FLAGS_STANDARD;
  else
    Msg[0].Flags = VSCAN_FLAGS_REMOTE;

  /* width of the data bytes */
  Msg[0].Size = m->len;

  /* copy data bytes to the CAN frame, up to 8 */
  for(i = 0 ; i <  m->len; i++)
  	Msg[0].Data[i] = m->data[i];         	

  /* copy CAN frame to the output buffer */
  if (!(VSCAN_Write((VSCAN_HANDLE)fd0, Msg, (DWORD)1, &dwWritten) == VSCAN_ERR_OK && dwWritten))
  {
    perror("canSend_driver (VScom): error writing to output buffer.\n");
    return 1;
  }
  
  /* really send CAN frame */
  if(VSCAN_Flush((VSCAN_HANDLE)fd0) != VSCAN_ERR_OK)
  {
    perror("canSend_driver (VScom): error flushing.\n");
    return 1;
  }

  return 0;
}


/***************************************************************************/
int TranslateBaudeRate(char* optarg){
	if(!strcmp( optarg, "1M")) return (int)VSCAN_SPEED_1M;
	if(!strcmp( optarg, "500K")) return (int)VSCAN_SPEED_500K;
	if(!strcmp( optarg, "250K")) return (int)VSCAN_SPEED_250K;
	if(!strcmp( optarg, "125K")) return (int)VSCAN_SPEED_125K;
	if(!strcmp( optarg, "100K")) return (int)VSCAN_SPEED_100K;
	if(!strcmp( optarg, "50K")) return (int)VSCAN_SPEED_50K;
	if(!strcmp( optarg, "20K")) return (int)VSCAN_SPEED_20K;
	if(!strcmp( optarg, "none")) return 0;
	return 0x0000;
}

UNS8 canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
{
  int baudrate;

  baudrate = TranslateBaudeRate(baud);
  if(baudrate == 0)
    return 0;

  if (VSCAN_Ioctl((VSCAN_HANDLE)fd, VSCAN_IOCTL_SET_SPEED, (void *)baudrate) != VSCAN_ERR_OK)
  {
    fprintf(stderr, "canOpen_driver (VScom): IOCTL set speed failed\n");
    return 0;
  }

  return 1;
}

/***************************************************************************/
CAN_HANDLE canOpen_driver(s_BOARD *board)
{
  VSCAN_HANDLE fd0 = 0;
  char busname[64];
  char* pEnd;
  int i;  
  int baudrate;
  
  printf("bus %s ", board->busname);
  fd0 = VSCAN_Open(board->busname, VSCAN_MODE_NORMAL);
  if(fd0 <= 0)
  {
    fprintf(stderr, "canOpen_driver (VScom): error opening %s\n", board->busname);
    return (CAN_HANDLE)fd0;
  }
  printf("(fd = %d)\n", fd0);
  baudrate = TranslateBaudeRate(board->baudrate);
  if(baudrate == 0)
    return 0;

  if (VSCAN_Ioctl((VSCAN_HANDLE)fd0, VSCAN_IOCTL_SET_SPEED, (void *)baudrate) != VSCAN_ERR_OK)
  {
    fprintf(stderr, "canOpen_driver (VScom): IOCTL set speed failed\n");
    return 0;
  }

   return (CAN_HANDLE)fd0;
}

/***************************************************************************/
int canClose_driver(CAN_HANDLE fd0)
{
  VSCAN_Close((VSCAN_HANDLE)fd0);
  return 0;
}
