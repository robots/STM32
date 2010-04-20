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

#include "canmsg.h"
#include "lincan.h"

#include "can_driver.h"

/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
{
  int res;
  struct canmsg_t canmsg;

  canmsg.flags = 0; /* Ensure standard receive, not required for LinCAN>=0.3.1 */
  do{
    res = read(fd0,&canmsg,sizeof(canmsg_t));
    if((res<0)&&(errno == -EAGAIN)) res = 0;
  }while(res==0);

  if(res != sizeof(canmsg_t)) // No new message
    return 1;

  if(canmsg.flags&MSG_EXT){
    /* There is no mark for extended messages in CanFestival */;
  }

  m->cob_id = canmsg.id;
  m->len = canmsg.length;
  if(canmsg.flags&MSG_RTR){
    m->rtr = 1;
  }else{
    m->rtr = 0;
    memcpy(m->data,canmsg.data,8);
  }

  return 0;
}

/***************************************************************************/
UNS8 canSend_driver(CAN_HANDLE fd0, Message *m)
{
  int res;
  struct canmsg_t canmsg;


  canmsg.flags = 0;
  canmsg.id = m->cob_id;
  canmsg.length = m->len;
  if(m->rtr){
    canmsg.flags |= MSG_RTR;
  }else{
    memcpy(canmsg.data,m->data,8);
  }

  if(canmsg.id >= 0x800){
    canmsg.flags |= MSG_EXT;
  }

  res = write(fd0,&canmsg,sizeof(canmsg_t));
  if(res!=sizeof(canmsg_t))
    return 1;

  return 0;
}

/***************************************************************************/
int TranslateBaudeRate(char* optarg){
	if(!strcmp( optarg, "1M")) return 1000000;
	if(!strcmp( optarg, "500K")) return 500000;
	if(!strcmp( optarg, "250K")) return 250000;
	if(!strcmp( optarg, "125K")) return 125000;
	if(!strcmp( optarg, "100K")) return 100000;
	if(!strcmp( optarg, "50K")) return 50000;
	if(!strcmp( optarg, "20K")) return 20000;
	if(!strcmp( optarg, "none")) return 0;
	return 0x0000;
}

/***************************************************************************/
UNS8 canChangeBaudRate_driver( CAN_HANDLE fd0, char* baud)
{
	struct can_baudparams_t params;

	params.baudrate = TranslateBaudeRate(baud);
	if(params.baudrate == 0)
		return 0;
	params.flags = -1;	// use driver defaults
	params.sjw = -1;	// use driver defaults
	params.sample_pt = -1;	// use driver defaults
	if(ioctl((int)fd0, CONF_BAUDPARAMS, &params) < 0)
	{
		fprintf(stderr, "canOpen_driver (lincan): IOCTL set speed failed\n");
		return 0;
	}
	return 1;
}

/***************************************************************************/
static const char lnx_can_dev_prefix[] = "/dev/can";

CAN_HANDLE canOpen_driver(s_BOARD *board)
{
	int name_len = strlen(board->busname);
	int prefix_len = strlen(lnx_can_dev_prefix);
	char dev_name[prefix_len+name_len+1];
	struct can_baudparams_t params;
	int o_flags = 0;
	int fd;

	/*o_flags = O_NONBLOCK;*/

	memcpy(dev_name,lnx_can_dev_prefix,prefix_len);
	memcpy(dev_name+prefix_len,board->busname,name_len);
	dev_name[prefix_len+name_len] = 0;
	printf("dev_name %s\n", dev_name);

	fd = open(dev_name, O_RDWR|o_flags);
	if(fd < 0)
	{
		fprintf(stderr,"!!! Board %s is unknown. See can_lincan.c\n", board->busname);
		goto error_ret;
	}
	printf("fd = %d\n", fd);

	// set baudrate
	params.baudrate = TranslateBaudeRate(board->baudrate);
	if(params.baudrate == 0)
		goto error_ret;
	params.flags = -1;	// use driver defaults
	params.sjw = -1;	// use driver defaults
	params.sample_pt = -1;	// use driver defaults
	if(ioctl(fd, CONF_BAUDPARAMS, &params) < 0)
	{
		fprintf(stderr, "canOpen_driver (lincan): IOCTL set speed failed\n");
		goto error_ret;
	}

	return (CAN_HANDLE)fd;

error_ret:
	return NULL;
}

/***************************************************************************/
int canClose_driver(CAN_HANDLE fd0)
{
  if(!fd0)
    return 0;
  close(fd0);
  return 0;
}
