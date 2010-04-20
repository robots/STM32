/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): James Steward

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
	Pseudo CAN hub application.
*/

#define _GNU_SOURCE  //for asprintf()

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <dlfcn.h>

#define NEED_PRINT_MESSAGE

#ifdef DLL_CALL
#undef DLL_CALL
#endif

#define DLL_CALL(funcname) (* funcname##_driver)

#include "canfestival.h"

int DLL_CALL(canfd)(CAN_HANDLE)FCT_PTR_INIT;

#ifdef DLL_CALL
#undef DLL_CALL
#endif

#define DLL_CALL(funcname) funcname##_driver

#define DLSYM(name)\
        *(void **) (&name##_driver) = dlsym(handle, #name"_driver");\
        if ((error = dlerror()) != NULL)  {\
                fprintf (stderr, "%s\n", error);\
                UnLoadCanDriver(handle);\
                return NULL;\
        }

#define MAX_HUB_PORTS 16

typedef struct {
	int fd;
	struct termios old_termio, new_termio;
	char *name;
} port;

port hub_ports[MAX_HUB_PORTS + 1];  //An extra for a CAN driver port

s_BOARD bus_if = { "/dev/ttyS0", "125K" };

/* Receive a CAN message from a hub (pseudo) port */
int hub_receive(port *p, Message *m)
{
	int rv, N, n = 0;
	fd_set rfds;
	struct timeval tv;
	
	N = 4; //initially try to read 4 bytes, including the length byte

retry:
	rv = read(p->fd, &((char *)m)[n], N - n);

	if (rv == -1) {
		fprintf(stderr, "read: %d, %s\n", p->fd, strerror(errno));
		return -1;
	}

	n += rv;

	if (n == 4) {
		N = (4 + m->len);
		
		if (m->len > 8) {
			fprintf(stderr, "Warning: invalid message length %d\n", 
					m->len);
			
			//try to resync
			return 0;
		}

	}

	if (n < N) {

		FD_ZERO(&rfds);
		FD_SET(p->fd, &rfds);

		tv.tv_sec = 0;
		tv.tv_usec=100000;
		
		rv = select(p->fd + 1, &rfds, NULL, NULL, &tv);
		if (rv == 0 || rv == -1) {
			fprintf(stderr, "select: %s\n", strerror(errno));
			return 0;
		}

		goto retry;	
	}

	return 1;
}

/* send a CAN message to one of the hub ports */
UNS8 hub_send(port *p, Message *m)
{
	int rv;

	rv = write(p->fd, m, 4 + m->len);

	if (rv != 4 + m->len) {
		return 1;
	}

	return 0;
}

/* Open a hub port */
int hub_open(port *p)
{
	if (p->fd != -1) {
		fprintf(stderr, "Warning, port %s is already open, fd %d!\n", 
					p->name, p->fd);	
	}

	p->fd = open(p->name, O_RDWR);
	
	if (p->fd < 0) {
		fprintf(stderr, "open: %s, %s\n", p->name, strerror(errno));
		goto exit_here;
	}

	if (tcgetattr(p->fd, &p->old_termio) != 0) {
		fprintf(stderr, "tcgetattr: %s, %s\n", 
					p->name, strerror(errno));
		close(p->fd);
		p->fd = -1;
		goto exit_here;
	}

	memcpy(&p->new_termio, &p->old_termio, sizeof(p->old_termio));
	cfmakeraw(&p->new_termio);
	cfsetispeed(&p->new_termio, B115200);
	cfsetospeed(&p->new_termio, B115200);
	tcsetattr(p->fd, TCSANOW, &p->new_termio);

exit_here:
	return p->fd;
}

/* Close a hub port*/
int hub_close(port *p)
{
	if (p->fd >= 0) {
		tcsetattr(p->fd, TCSANOW, &p->old_termio);
		close(p->fd);
		p->fd = -1;
	}

	return 0;
}

/** Read from the port index rd_port, and write to all other ports. */
int read_write(int rd_port, port *p, CAN_HANDLE h, fd_set *wfds, int max_fd)
{
	Message m;
	int rv, i;
	fd_set wfds_copy;
	struct timeval tv = {.tv_sec = 0, .tv_usec = 0}; //wait 0 msec

	if (rd_port == MAX_HUB_PORTS) {
		rv = DLL_CALL(canReceive)(h, &m);

		if (rv == 1) {
			return 0;
		}
	} else {
		rv = hub_receive(&p[rd_port], &m);

		if (rv != 1) {
			return rv;
		}
	}
	
	memcpy(&wfds_copy, wfds, sizeof(fd_set));

	rv = select(max_fd + 1, NULL, &wfds_copy, NULL, &tv);
	
	if (rv <= 0) {
		return 0;
	}

	for (i = 0; i < MAX_HUB_PORTS + 1; i++) {

		if (i == rd_port) {
			fprintf(stderr, "[%d] ", i);
			continue;
		}

		if (p[i].fd < 0 || !FD_ISSET(p[i].fd, &wfds_copy)) {
			fprintf(stderr, "{%d} ", i);
			continue;
		}

		fprintf(stderr, "<%d> ", i);

		if (i == MAX_HUB_PORTS && h) {
			DLL_CALL(canSend)(h, &m);
		} else {
			hub_send(&p[i], &m);
		}
	}

	print_message(&m);

	return 0;
}

void help(void)
{
	printf("\n\n");
	printf("This is a software hub for the CANFestival library, \n");
	printf("based on the *nix pseudo tty.  It supports up to 16\n");
	printf("connections from clients, and a connection to a CANFestival\n");
	printf("driver, for connection to the outside world.\n");
	printf("\n");
	printf("Basic use is simply to run can_hub.  Without arguments, it\n");
	printf("will use /dev/ptya[0..f] ptys.  You should then run your\n");
	printf("linux CANFestival app using libcanfestival_can_serial.so\n");
	printf("with the bus name /dev/ttyaX, where X is 0..f and unused.\n");
	printf("\n");
	printf("You can alter the pty base with -p /dev/ptyx .\n");
	printf("\n");
	printf("If you want to interface with some other CAN driver, supply\n");
	printf("the option -l /path/to/libcanfestival_can_foo.so .\n");
	printf("The default bus name and baud are /dev/ttyS0 and 125k.\n");
	printf("These can be overridden with -b /dev/{bus name} and -s {baud}.\n");
}

/*UnLoads the dll*/
UNS8 UnLoadCanDriver(LIB_HANDLE handle)
{
        if(handle!=NULL)
        {
                dlclose(handle);

                handle=NULL;
                return 0;
        }
        return -1;
}

/*Loads the dll and get funcs ptr*/
LIB_HANDLE LoadCanDriver(char* driver_name)
{
        LIB_HANDLE handle = NULL;
        char *error;

        if(handle==NULL)
        {
                handle = dlopen(driver_name, RTLD_LAZY);
        }

        if (!handle) {
                fprintf (stderr, "%s\n", dlerror());
                return NULL;
        }

        /*Get function ptr*/
        DLSYM(canReceive)
        DLSYM(canSend)
        DLSYM(canOpen)
        DLSYM(canChangeBaudRate)
        DLSYM(canClose)
	DLSYM(canfd)

        return handle;
}

/**
*/
int main(int argc, char **argv)
{
	int i, rv, max_fd = 0, ret = 0;
	fd_set rfds, rfds_copy;
	CAN_HANDLE can_h = NULL;
        LIB_HANDLE lib_h = NULL;

	int c;
	extern char *optarg;

	char *can_drv = NULL;
	char *pty_base = "/dev/ptya";

	while ((c = getopt(argc, argv, "-b:s:l:p:h")) != EOF) {
		switch (c) {
		case 'b':
			if (optarg[0] == 0) {
				help();
				exit(1);
			}
			bus_if.busname = optarg;
			break;
		case 's':
			if (optarg[0] == 0) {
				help();
				exit(1);
			}
			bus_if.baudrate = optarg;
			break;
		case 'l':
			if (optarg[0] == 0) {
				help();
				exit(1);
			}
			can_drv = optarg;
			break;
		case 'p':
			if (optarg[0] == 0) {
				help();
				exit(1);
			}
			pty_base = optarg;
			break;
		case 'h':
			help();
			exit(1);
			break;
		default:
			help();
			exit(1);
		}
	}

	FD_ZERO(&rfds);

	hub_ports[MAX_HUB_PORTS].fd = -1;

	if (can_drv) {
		lib_h = LoadCanDriver(can_drv);
		if (lib_h == NULL) {
			printf("Unable to load library: %s\n", can_drv);
			exit(1);
		}

	        can_h = DLL_CALL(canOpen)(&bus_if);
        	if(!can_h) {
        	        fprintf(stderr,"canOpen : failed\n");
        	        exit(1);
        	}

		hub_ports[MAX_HUB_PORTS].fd = DLL_CALL(canfd)(can_h);

		FD_SET(hub_ports[MAX_HUB_PORTS].fd, &rfds);

		if (hub_ports[MAX_HUB_PORTS].fd > max_fd) {
			max_fd = hub_ports[MAX_HUB_PORTS].fd;
		}

	}

	for (i = 0; i < MAX_HUB_PORTS; i++) {

		hub_ports[i].fd = -1;
		hub_ports[i].name = NULL;

		rv = asprintf(&hub_ports[i].name, "%s%x", pty_base, i);

		if (rv < 0) {
			fprintf(stderr, "asprintf: %s\n", strerror(errno));
			ret = 1;
			break;
		}

		rv = hub_open(&hub_ports[i]);

		if (rv < 0) {
			ret = 1;
			break;
		}

		FD_SET(rv, &rfds);

		if (rv > max_fd) {
			max_fd = rv;
		}
	}

	if (ret) {
		return ret;
	}

	while (!ret) {
		memcpy(&rfds_copy, &rfds, sizeof(rfds));

		rv = select(max_fd + 1, &rfds_copy, NULL, NULL, NULL);

		if (rv < 0) {
			//select error
			fprintf(stderr, "select: %s\n", strerror(errno));
			ret = 1;
			continue;
		}

		//as timeout is NULL, must be a rfds set.
		for (i = 0; i < MAX_HUB_PORTS + 1; i++) {
			if (hub_ports[i].fd >= 0 && 
					FD_ISSET(hub_ports[i].fd, &rfds_copy)) {

				rv = read_write(i, hub_ports, can_h, &rfds, max_fd);

				if (rv < 0 && i < MAX_HUB_PORTS) {

					FD_CLR(hub_ports[i].fd, &rfds);

					hub_close(&hub_ports[i]);
				}						
			}

			if (hub_ports[i].fd < 0 && i < MAX_HUB_PORTS) {
				rv = hub_open(&hub_ports[i]);

				if (rv >= 0) {
					FD_SET(rv, &rfds);

					if (rv > max_fd) {
						max_fd = rv;
					}
				}
			}
		}
	}

	for (i = 0; i < MAX_HUB_PORTS; i++) {
		hub_close(&hub_ports[i]);
	}

	if (hub_ports[MAX_HUB_PORTS].fd >= 0) {
		DLL_CALL(canClose)(&bus_if);
	}

	return ret;
}

