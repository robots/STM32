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

#ifndef __APPLICFG_WIN32__
#define __APPLICFG_WIN32__

#include <windows.h>
#include <string.h>
#include <stdio.h>

// Define the architecture : little_endian or big_endian
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

// Integers
#define INTEGER8 char
#define INTEGER16 short
#define INTEGER24 long
#define INTEGER32 long
#define INTEGER40 long long
#define INTEGER48 long long
#define INTEGER56 long long
#define INTEGER64 long long

// Unsigned integers
#define UNS8   unsigned char
#define UNS16  unsigned short
#define UNS32  unsigned long
#define UNS24  unsigned long 
#define UNS40  unsigned long long
#define UNS48  unsigned long long
#define UNS56  unsigned long long
#define UNS64  unsigned long long

// Reals
#define REAL32 float
#define REAL64 double

// Custom integer types sizes
#define sizeof_INTEGER24 3
#define sizeof_INTEGER40 5
#define sizeof_INTEGER48 6
#define sizeof_INTEGER56 7

#define sizeof_UNS24  3
#define sizeof_UNS40  5
#define sizeof_UNS48  6
#define sizeof_UNS56  7

// Non integral integers conversion macros
#define INT24_2_32(a) (a <= 0x7FFFFF ? a : a|0xFF000000)
#define INT40_2_64(a) (a <= 0x0000007FFFFFFFFF ? a : a|0xFFFFFF0000000000)
#define INT48_2_64(a) (a <= 0x00007FFFFFFFFFFF ? a : a|0xFFFF000000000000)
#define INT56_2_64(a) (a <= 0x007FFFFFFFFFFFFF ? a : a|0xFF00000000000000)

#define INT32_2_24(a) (a&0x00FFFFFF)
#define INT64_2_40(a) (a&0x000000FFFFFFFFFF)
#define INT64_2_48(a) (a&0x0000FFFFFFFFFFFF)
#define INT64_2_56(a) (a&0x00FFFFFFFFFFFFFF)

/// Definition of error and warning macros
// --------------------------------------

#ifdef UNICODE
#define MSG(...) \
  do{wchar_t msg[300];\
   swprintf(msg,L##__VA_ARGS__);\
   OutputDebugString(msg);}while(0)
#else
#define MSG(...) \
  do{printf(__VA_ARGS__);fflush(stdout);}while(0)

/*do{char msg[300];\
   sprintf(msg,##__VA_ARGS__);\
   OutputDebugString(msg);}while(0)
*/
#endif  
#define CANFESTIVAL_DEBUG_MSG(num, str, val)\
  {unsigned long value = val;\
   MSG("%s(%d) : 0x%X %s 0x%X\n",__FILE__, __LINE__,num, str, value);\
   }

/// Definition of MSG_WAR
// ---------------------
#ifdef DEBUG_WAR_CONSOLE_ON
    #define MSG_WAR(num, str, val) CANFESTIVAL_DEBUG_MSG(num, str, val)
#else
#    define MSG_WAR(num, str, val)
#endif

/// Definition of MSG_ERR
// ---------------------
#ifdef DEBUG_ERR_CONSOLE_ON
#    define MSG_ERR(num, str, val) CANFESTIVAL_DEBUG_MSG(num, str, val)
#else
#    define MSG_ERR(num, str, val)
#endif



typedef void* CAN_HANDLE;

typedef void* CAN_PORT;

#endif // __APPLICFG_WIN32__
