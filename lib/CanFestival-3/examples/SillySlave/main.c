/*
Copyright (C): Giuseppe Massimo BERTANI
gmbertani@users.sourceforge.net


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
 * @file main.c
 * @author GMB
 * @date 17/1/08
 *
 * This file is part of SillySlave demo for CANfestival
 * open source CANopen stack.
 */ 


#include "main.h"                   
#include "slave.h"

#if !defined(WIN32) || defined(__CYGWIN__)
void catch_signal(int sig)
{
  signal(SIGTERM, catch_signal);
  signal(SIGINT, catch_signal);
  
  printf("Got Signal %d\n",sig);
}
#endif


/**
 * Please edit main.h defines before compiling
 */ 
int main(int argc,char **argv)
{
    char* LibraryPath= DRIVER_LIBRARY; 
    
#if !defined(WIN32) || defined(__CYGWIN__)
    /* install signal handler for manual break */
	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);
#endif

#ifndef NOT_USE_DYNAMIC_LOADING
	LoadCanDriver(LibraryPath);
#endif		

    
    if(InitCANdevice( BUS, BAUDRATE,  NODE_SLAVE ) < 0)
    {
        printf("\nInitCANdevice() failed, exiting.\n");
        return -1;
    }

    return 0;
}


