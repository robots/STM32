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
#include <stdlib.h>
#include <stddef.h>		/* for NULL */
#include <errno.h>

#include "config.h"

#ifdef RTCAN_SOCKET
#include "rtdm/rtcan.h"
#define CAN_IFNAME     "rtcan%s"
#define CAN_SOCKET     rt_dev_socket
#define CAN_CLOSE      rt_dev_close
#define CAN_RECV       rt_dev_recv
#define CAN_SEND       rt_dev_send
#define CAN_BIND       rt_dev_bind
#define CAN_IOCTL      rt_dev_ioctl
#define CAN_SETSOCKOPT rt_dev_setsockopt
#define CAN_ERRNO(err) (-err)
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "linux/can.h"
#include "linux/can/raw.h"
#include "net/if.h"
#define PF_CAN 29
#define AF_CAN PF_CAN
//#include "af_can.h"
#define CAN_IFNAME     "can%s"
#define CAN_SOCKET     socket
#define CAN_CLOSE      close
#define CAN_RECV       recv
#define CAN_SEND       send
#define CAN_BIND       bind
#define CAN_IOCTL      ioctl
#define CAN_ERRNO(err) errno
#define CAN_SETSOCKOPT setsockopt
#endif

#include "can_driver.h"

/*********functions which permit to communicate with the board****************/
UNS8
canReceive_driver (CAN_HANDLE fd0, Message * m)
{
  int res;
  struct can_frame frame;

  res = CAN_RECV (*(int *) fd0, &frame, sizeof (frame), 0);
  if (res < 0)
    {
      fprintf (stderr, "Recv failed: %s\n", strerror (CAN_ERRNO (res)));
      return 1;
    }

  m->cob_id = frame.can_id & CAN_EFF_MASK;
  m->len = frame.can_dlc;
  if (frame.can_id & CAN_RTR_FLAG)
    m->rtr = 1;
  else
    m->rtr = 0;
  memcpy (m->data, frame.data, 8);

#if defined DEBUG_MSG_CONSOLE_ON
  MSG("in : ");
  print_message(m);
#endif
  return 0;
}


/***************************************************************************/
UNS8
canSend_driver (CAN_HANDLE fd0, Message * m)
{
  int res;
  struct can_frame frame;

  frame.can_id = m->cob_id;
  if (frame.can_id >= 0x800)
    frame.can_id |= CAN_EFF_FLAG;
  frame.can_dlc = m->len;
  if (m->rtr)
    frame.can_id |= CAN_RTR_FLAG;
  else
    memcpy (frame.data, m->data, 8);

#if defined DEBUG_MSG_CONSOLE_ON
  MSG("out : ");
  print_message(m);
#endif
  res = CAN_SEND (*(int *) fd0, &frame, sizeof (frame), 0);
  if (res < 0)
    {
      fprintf (stderr, "Send failed: %s\n", strerror (CAN_ERRNO (res)));
      return 1;
    }

  return 0;
}

/***************************************************************************/
#ifdef RTCAN_SOCKET
int
TranslateBaudRate (const char *optarg)
{
  int baudrate;
  int val, len;
  char *pos = NULL;

  len = strlen (optarg);
  if (!len)
    return 0;

  switch ((int) optarg[len - 1])
    {
    case 'M':
      baudrate = 1000000;
      break;
    case 'K':
      baudrate = 1000;
      break;
    default:
      baudrate = 1;
      break;
    }
  if ((sscanf (optarg, "%i", &val)) == 1)
    baudrate *= val;
  else
    baudrate = 0;;

  return baudrate;
}
#endif

UNS8 canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
{
	printf("canChangeBaudRate not yet supported by this driver\n");
	return 0;
}

/***************************************************************************/
CAN_HANDLE
canOpen_driver (s_BOARD * board)
{
  struct ifreq ifr;
  struct sockaddr_can addr;
  int err;
  CAN_HANDLE fd0 = malloc (sizeof (int));
#ifdef RTCAN_SOCKET
  can_baudrate_t *baudrate;
  can_mode_t *mode;
#endif

  *(int *) fd0 = CAN_SOCKET (PF_CAN, SOCK_RAW, CAN_RAW);
  if (*(int *) fd0 < 0)
    {
      fprintf (stderr, "Socket creation failed: %s\n",
	       strerror (CAN_ERRNO (*(int *) fd0)));
      goto error_ret;
    }

  if (*board->busname >= '0' && *board->busname <= '9')
    snprintf (ifr.ifr_name, IFNAMSIZ, CAN_IFNAME, board->busname);
  else
    strncpy (ifr.ifr_name, board->busname, IFNAMSIZ);
  err = CAN_IOCTL (*(int *) fd0, SIOCGIFINDEX, &ifr);
  if (err)
    {
      fprintf (stderr, "Getting IF index for %s failed: %s\n",
	       ifr.ifr_name, strerror (CAN_ERRNO (err)));
      goto error_close;
    }
  
  {
    int loopback = 1;
    err = CAN_SETSOCKOPT(*(int *)fd0, SOL_CAN_RAW, CAN_RAW_LOOPBACK,
               &loopback, sizeof(loopback));
    if (err) {
        fprintf(stderr, "rt_dev_setsockopt: %s\n", strerror (CAN_ERRNO (err)));
        goto error_close;
    }
  }
  
#ifndef RTCAN_SOCKET /*CAN_RAW_RECV_OWN_MSGS not supported in rtsocketcan*/
  {
    int recv_own_msgs = 0; /* 0 = disabled (default), 1 = enabled */
    err = CAN_SETSOCKOPT(*(int *)fd0, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS,
               &recv_own_msgs, sizeof(recv_own_msgs));
    if (err) {
        fprintf(stderr, "rt_dev_setsockopt: %s\n", strerror (CAN_ERRNO (err)));
        goto error_close;
    }
  }
#endif
  
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  err = CAN_BIND (*(int *) fd0, (struct sockaddr *) &addr, sizeof (addr));
  if (err)
    {
      fprintf (stderr, "Binding failed: %s\n", strerror (CAN_ERRNO (err)));
      goto error_close;
    }

#ifdef RTCAN_SOCKET
  baudrate = (can_baudrate_t *) & ifr.ifr_ifru;
  *baudrate = TranslateBaudRate (board->baudrate);
  if (!*baudrate)
    goto error_close;

  err = CAN_IOCTL (*(int *) fd0, SIOCSCANBAUDRATE, &ifr);
  if (err)
    {
      fprintf (stderr,
	       "Setting baudrate %d failed: %s\n",
	       *baudrate, strerror (CAN_ERRNO (err)));
      goto error_close;
    }

  mode = (can_mode_t *) & ifr.ifr_ifru;
  *mode = CAN_MODE_START;
  err = CAN_IOCTL (*(int *) fd0, SIOCSCANMODE, &ifr);
  if (err)
    {
      fprintf (stderr, "Starting CAN device failed: %s\n",
	       strerror (CAN_ERRNO (err)));
      goto error_close;
    }
#endif

  return fd0;

error_close:
  CAN_CLOSE (*(int *) fd0);

error_ret:
  free (fd0);
  return NULL;
}

/***************************************************************************/
int
canClose_driver (CAN_HANDLE fd0)
{
  if (fd0)
    {
      CAN_CLOSE (*(int *) fd0);
      free (fd0);
    }
  return 0;
}
