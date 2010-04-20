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
#include <process.h>
#include <string>
#include <list>

using namespace std;

typedef std::list<Socket*> socket_list;

socket_list g_connections;

unsigned __stdcall Connection(void* a) {
  Socket* s = (Socket*) a;

  g_connections.push_back(s);

  //s->SendLine("Welcome to the Message Distributor");
  printf("Accepted new connection (0x%x).\n");
  while (1) {
    std::string r = s->ReceiveLine();
    if (r.empty()) break;
    //cout << r  << endl;    
    for (socket_list::iterator os =g_connections.begin();
                               os!=g_connections.end(); 
                               os++) {
      if (*os != s) (*os)->SendLine(r);
    }
  }

  g_connections.remove(s);

  delete s;

  printf("Connection closed (0x%x).\n",s);

  return 0;
}

int main() {
  printf("************************************************\n"
		 "* TCP broadcasting chat server for CanFestival *\n"
		 "*  for use with can_tcp_win32 CAN driver DLL   *\n"
		 "************************************************\n"
		 "\n"
		 "Accepts connections on port 11898\n"
		 "and repeat '\\n' terminated lines \n"
		 "to each connected client\n"
		 "\n"
		 "Use netcat to monitor trafic\n"
		 " nc 127.0.0.1 11898\n"
		 "\n");
  fflush(stdout);
  SocketServer in(11898,5);
  
  while (1) {
    Socket* s=in.Accept();
  
    unsigned ret;
    _beginthreadex(0,0,Connection,(void*) s,0,&ret);
  }
  
  return 0;
}
