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

// thread safe que
#ifndef __async_access_que_h__
#define __async_access_que_h__

#include <deque>
#include "AutoReleaseCS.h"

template<typename type>
class async_access_que
   {
   public:
      async_access_que()
         {
         ::InitializeCriticalSection(&m_cs);
         }
      ~async_access_que()
         {
         ::DeleteCriticalSection(&m_cs);
         }

      void append(const type& data)
         {
         AutoReleaseCS acs(m_cs);
         m_data.push_back(data);
         }

      bool extract_top(type& data)
         {
         AutoReleaseCS acs(m_cs);
         if (m_data.empty())
            return false;
          data = m_data.front();
          m_data.pop_front();
         return true;
         }
         
      void clear()
         {
         AutoReleaseCS acs(m_cs);
         m_data.clear();
         }
         
   protected:
      std::deque<type> m_data;
      CRITICAL_SECTION m_cs;
   };
#endif //__async_access_que_h__