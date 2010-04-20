/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT

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

#include "Socket.h"

#include <iostream>

using namespace std;

extern "C" {
#include "can_driver.h"
}

//------------------------------------------------------------------------
extern "C"
   UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
   {

      string l = reinterpret_cast<SocketClient*>(fd0)->ReceiveLine();

      int res = sscanf(l.c_str(),"{0x%3hx,%1hhd,%1hhd,{0x%2hhx,0x%2hhx,0x%2hhx,0x%2hhx,0x%2hhx,0x%2hhx,0x%2hhx,0x%2hhx}}",
                &m->cob_id,
                &m->rtr,
                &m->len,
                &m->data[0],
                &m->data[1],
                &m->data[2],
                &m->data[3],
                &m->data[4],
                &m->data[5],
                &m->data[6],
                &m->data[7]
                );


#if defined DEBUG_MSG_CONSOLE_ON
  printf("in : ");
  print_message(m);
#endif
   
      return res==11 ? 0 : 1 ;
   }

extern "C"
   UNS8 canSend_driver(CAN_HANDLE fd0, Message *m)
   {
        char s[1024];        
        sprintf(s,"{0x%3.3x,%1d,%1d,{0x%2.2x,0x%2.2x,0x%2.2x,0x%2.2x,0x%2.2x,0x%2.2x,0x%2.2x,0x%2.2x}}",
                m->cob_id,
                m->rtr,
                m->len,
                m->data[0],
                m->data[1],
                m->data[2],
                m->data[3],
                m->data[4],
                m->data[5],
                m->data[6],
                m->data[7]
                );
    
        reinterpret_cast<SocketClient*>(fd0)->SendLine(s);
#if defined DEBUG_MSG_CONSOLE_ON
  printf("out : ");
  print_message(m);
#endif
        return 0;
   }

extern "C"
   CAN_HANDLE canOpen_driver(s_BOARD *board)
   {
      Socket* s;
      try {
        s = new SocketClient(board->busname, 11898);
    
        //s.SendLine("GET / HTTP/1.0");
        //s.SendLine("Host: www.google.com");
      } 
      catch (const char* _s) {
        cerr << _s << endl;
        return NULL;
      } 
      catch (std::string _s) {
        cerr << _s << endl;
        return NULL;
      } 
      catch (...) {
        cerr << "unhandled exception\n";
        return NULL;
      }
      return (CAN_HANDLE) s;
   }

extern "C"
   int canClose_driver(CAN_HANDLE inst)
   {
   delete reinterpret_cast<SocketClient*>(inst);
   return 1;
   }
extern "C"
	UNS8 canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
	{
	cerr << "canChangeBaudRate not yet supported by this driver\n";
	return 0;
	}