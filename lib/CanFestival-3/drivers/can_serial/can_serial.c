/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

Ruthlessly butchered by James Steward to produce a serial (tty) port
driver.

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

/*
	Single serial port CAN driver.
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <stdlib.h>

#define NEED_PRINT_MESSAGE
#include "can_driver.h"
#include "def.h"

typedef struct {
	int fd;
	struct termios old_termio, new_termio;
} CANPort;

/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver(CAN_HANDLE fd0, Message * m)
{
	int rv, N, n = 0;
	fd_set rfds;
	struct timeval tv;

	N = 4; //initially try to read 4 bytes, including the length byte
	
retry:
	rv = read(((CANPort *) fd0)->fd, &((char *)m)[n], N - n);

	if (rv == -1) {
		fprintf(stderr, "read: %s\n", strerror(errno));
		return 1;
	}

	n += rv;

	if (n == 4) {
		N = (4 + m->len);
		
		if (m->len > 8) {
			fprintf(stderr, "Warning: invalid message length %d\n", 
					m->len);
			
			//try to resync
			n = 0;
			N = 4;
		}

	}

	if (n < N) {

		FD_ZERO(&rfds);
		FD_SET(((CANPort *) fd0)->fd, &rfds);

		tv.tv_sec = 0;
		tv.tv_usec=100000;
		
		rv = select(((CANPort *) fd0)->fd + 1, &rfds, NULL, NULL, &tv);
		if (rv == -1) {
			fprintf(stderr, "select: %s\n", strerror(errno));
			return 1;
		} else if (rv == 0) {
			n = 0;
		}

		goto retry;	
	}

	print_message(m);

	return 0;
}

/***************************************************************************/
UNS8 canSend_driver(CAN_HANDLE fd0, Message * m)
{
	int rv;

	print_message(m);

	// Send to serial port 
	rv = write(((CANPort *) fd0)->fd, m, 4 + m->len);

	if (rv != 4 + m->len) {
		return 1;
	}

	return 0;
}

/***************************************************************************/
int TranslateBaudRate(char *optarg)
{
	if (!strcmp(optarg, "1M"))
		return (int) 1000;
	if (!strcmp(optarg, "500K"))
		return (int) 500;
	if (!strcmp(optarg, "250K"))
		return (int) 250;
	if (!strcmp(optarg, "125K"))
		return (int) 125;
	if (!strcmp(optarg, "100K"))
		return (int) 100;
	if (!strcmp(optarg, "50K"))
		return (int) 50;
	if (!strcmp(optarg, "20K"))
		return (int) 20;
	if (!strcmp(optarg, "10K"))
		return (int) 10;
	if (!strcmp(optarg, "5K"))
		return (int) 5;
	if (!strcmp(optarg, "none"))
		return 0;
	return 0x0000;
}

UNS8 canChangeBaudRate_driver(CAN_HANDLE fd0, char *baud)
{
	printf("Faked changing to baud rate %s[%d]\n", 
			baud, TranslateBaudRate(baud));
	return 0;
}

/***************************************************************************/
CAN_HANDLE canOpen_driver(s_BOARD * board)
{
	int rv;

	CANPort *p;

	p = (CANPort *)calloc(1, sizeof(CANPort));
	
	if (p == NULL) {
		fprintf(stderr, "calloc: %s\n", strerror(errno));
		return (CAN_HANDLE) NULL;
	}

	p->fd = open(board->busname, O_RDWR);
	
	if (p->fd < 0) {
		fprintf(stderr, "open: %s, %s\n", 
				board->busname, strerror(errno));
		free(p);
		return (CAN_HANDLE) NULL;
	}

	if (tcgetattr(p->fd, &p->old_termio) != 0) {
		fprintf(stderr, "tcgetattr: %s, %s\n", 
				board->busname, strerror(errno));
		close(p->fd);
		free(p);
		return (CAN_HANDLE) NULL;
	}

	memcpy(&p->new_termio, &p->old_termio, 
					sizeof(p->old_termio));
	cfmakeraw(&p->new_termio);
	cfsetispeed(&p->new_termio, B115200);
	cfsetospeed(&p->new_termio, B115200);
	tcsetattr(p->fd, TCSANOW, &p->new_termio);

	return (CAN_HANDLE) p;
}

/***************************************************************************/
int canClose_driver(CAN_HANDLE fd0)
{
	if ((CANPort *) fd0 && ((CANPort *) fd0)->fd >= 0) {
		tcsetattr(((CANPort *) fd0)->fd, TCSANOW, 
					&((CANPort *) fd0)->old_termio);
		close(((CANPort *) fd0)->fd);
		free((CANPort *) fd0);
	}

	return 0;
}

int canfd_driver(CAN_HANDLE fd0)
{
	if ((CANPort *) fd0) {
		return ((CANPort *) fd0)->fd;
	}

	return -1;
}

