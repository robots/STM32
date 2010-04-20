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
#ifdef USE_XENO
#define eprintf(...)
#else
#define eprintf(...) printf (__VA_ARGS__)
#endif

#include "canfestival.h"

void help(void);
void StartNode(UNS8);
void StopNode(UNS8);
void ResetNode(UNS8);
void DiscoverNodes(void);
void CheckReadInfoSDO(CO_Data*, UNS8);
void GetSlaveNodeInfo(UNS8);
void CheckReadSDO(CO_Data*, UNS8);
void CheckWriteSDO(CO_Data*, UNS8);
void ReadDeviceEntry(char*);
void WriteDeviceEntry(char*);
void SleepFunction(int);
