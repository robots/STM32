/*
This file is part of CanFestival, a library implementing CanOpen Stack.

CanFestival Copyright (C): Edouard TISSERANT and Francis DUPIN
CanFestival Win32 port Copyright (C) 2007 Leonid Tochinski, ChattenAssociates, Inc.

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

// Critical Section Autorelease
#ifndef __autoreleasecs_h__
#define __autoreleasecs_h__

class AutoReleaseCS
   {
   public:
      AutoReleaseCS(CRITICAL_SECTION& cs) : m_cs(cs)
         {
         ::EnterCriticalSection(&m_cs);
         }
      ~AutoReleaseCS()
         {
         ::LeaveCriticalSection(&m_cs);
         }
      CRITICAL_SECTION& m_cs;
   };
 #endif // __autoreleasecs_h__