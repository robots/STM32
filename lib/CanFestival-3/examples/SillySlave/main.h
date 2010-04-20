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
 * @file main.h
 * @author GMB
 * @date 17/1/08
 *
 * This file is part of SillySlave demo for CANfestival
 * open source CANopen stack.
 */ 



#ifndef _MAIN_H
 #define _MAIN_H

#if defined(WIN32) && !defined(__CYGWIN__)
#include <windows.h>
#include "getopt.h"
void pause(void)
{
	system("PAUSE");
}
#else
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#endif

#include "canfestival.h"

/*
 * Please tune the following defines to suit your needs:
 */
#define NODE_MASTER 0x1        
#define NODE_SLAVE  0x12        
#define DRIVER_LIBRARY "libcanfestival_can_kvaser.so"
#define BAUDRATE 250000
#define BUS 0

#endif /* _MAIN_H */


