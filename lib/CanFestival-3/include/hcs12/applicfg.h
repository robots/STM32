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

#ifndef __APPLICFG_HC12__
#define __APPLICFG_HC12__

#include <string.h>
#include <stdio.h>

#include <../include/hcs12/asm-m68hc12/portsaccess.h>
#include <../include/hcs12/asm-m68hc12/ports_def.h>
#include <../include/hcs12/asm-m68hc12/ports.h>
#include <../include/hcs12/interrupt.h>


/// Define the architecture : little_endian or big_endian
// -----------------------------------------------------
// Test :
// UNS32 v = 0x1234ABCD;
// char *data = &v;
//
// Result for a little_endian architecture :
// data[0] = 0xCD;
// data[1] = 0xAB;
// data[2] = 0x34;
// data[3] = 0x12;
//
// Result for a big_endian architecture :
// data[0] = 0x12;
// data[1] = 0x34;
// data[2] = 0xAB;
// data[3] = 0xCD;

/* CANOPEN_BIG_ENDIAN now defined in config.h*/
#ifndef CANOPEN_BIG_ENDIAN
#  define CANOPEN_BIG_ENDIAN 1
#endif


// Use or not the PLL
//#define USE_PLL

#ifdef USE_PLL
#  define BUS_CLOCK 24 // If the quartz on the board is 16 MHz. If different, change this value
#else 
#  define BUS_CLOCK 8  // If the quartz on the board is 16 MHz. If different, change this value
#endif

/// Configuration of the serials port SCI0 and SCI1
// Tested : 
//   SERIAL_SCI0_BAUD_RATE 9600      BUS_CLOCK 8   Send OK      Receive not tested
//   SERIAL_SCI0_BAUD_RATE 19200     BUS_CLOCK 8   Send OK      Receive not tested
//   SERIAL_SCI0_BAUD_RATE 38400     BUS_CLOCK 8   Send OK      Receive not tested
//   SERIAL_SCI0_BAUD_RATE 57600     BUS_CLOCK 8   Send Failed  Receive not tested
//   SERIAL_SCI0_BAUD_RATE 115200    BUS_CLOCK 8   Send Failed  Receive not tested

//   SERIAL_SCI0_BAUD_RATE 9600      BUS_CLOCK 24  Send OK      Receive not tested
//   SERIAL_SCI0_BAUD_RATE 19200     BUS_CLOCK 24  Send OK      Receive not tested
//   SERIAL_SCI0_BAUD_RATE 38400     BUS_CLOCK 24  Send OK but init problems     Receive not tested
//   SERIAL_SCI0_BAUD_RATE 57600     BUS_CLOCK 24  Send Failed  Receive not tested
//   SERIAL_SCI0_BAUD_RATE 115200    BUS_CLOCK 24  Send Failed  Receive not tested

#define SERIAL_SCI0_BAUD_RATE 38400
#define SERIAL_SCI1_BAUD_RATE 9600






// Several hardware definitions functions
// --------------------------------------


/// Initialisation of the serial port 0
extern void initSCI_0 (void);

/// Initialisation of the serial port 1
extern void initSCI_1 (void);

/// Convert an integer to a string in hexadecimal format
/// If you do not wants to use a lastCar, put lastCar = '\0' (end of string)
/// ex : value = 0XABCDEF and lastCar = '\n'
/// buf[0] = '0'
/// buf[1] = 'X'
/// buf[2] = 'A'
/// ....
/// buf[7] = 'F'
/// buf[8] = '\n'
/// buf[9] = '\0'
extern char *
hex_convert (char *buf, unsigned long value, char lastCar);

/// Print the string to the serial port sci 
/// (sci takes the values SCI0 or SCI1)
extern void printSCI_str (char sci, const char * str); 

/// Print the number in hexadecimal  to the serial port sci 
/// (sci takes the values SCI0 or SCI1)
extern void printSCI_nbr (char sci, unsigned long nbr, char lastCar);

/// to start using the PLL. Bus clock becomes 24 MHz if the quartz 
/// on the board is at 16 MHz
extern void initPLL (void);

/// Put here all the code to init the HCS12
extern void initHCS12 (void);

// Integers
#define INTEGER8 signed char
#define INTEGER16 short
#define INTEGER24
#define INTEGER32 long
#define INTEGER40
#define INTEGER48
#define INTEGER56
#define INTEGER64
 
// Unsigned integers
#define UNS8   unsigned char
#define UNS16  unsigned short
#define UNS32  unsigned long
/*
#define UNS24
#define UNS40
#define UNS48
#define UNS56
#define UNS64
*/ 

// Whatever your microcontroller, the timer wont work if 
// TIMEVAL is not at least on 32 bits
#define TIMEVAL UNS32 

// The timer of the hcs12 counts from 0000 to 0xFFFF
#define TIMEVAL_MAX 0xFFFF

// The timer is incrementing every 4 us.
#define MS_TO_TIMEVAL(ms) (ms * 250)
#define US_TO_TIMEVAL(us) (us>>2)


// Reals
#define REAL32	float
#define REAL64 double

#include "can.h"


/// Definition of MSG_ERR
// ---------------------
#ifdef DEBUG_ERR_CONSOLE_ON
#    define MSG_ERR(num, str, val)            \
          initSCI_0();                        \
          printSCI_nbr(SCI0, num, ' ');       \
          /* large printing on console  */    \
          printSCI_str(SCI0, str);            \
          printSCI_nbr(SCI0, val, '\n');      
#else
#    define MSG_ERR(num, str, val)
#endif

/// Definition of MSG_WAR
// ---------------------
#ifdef DEBUG_WAR_CONSOLE_ON
#    define MSG_WAR(num, str, val)          \
        initSCI_0();                        \
        printSCI_nbr(SCI0, num, ' ');       \
        /* large printing on console  */    \
        printSCI_str(SCI0, str);            \
        printSCI_nbr(SCI0, val, '\n');      
#else
#    define MSG_WAR(num, str, val)
#endif


#endif


