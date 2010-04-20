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

// can_uvccm_win32 adapter (http://www.gridconnect.com)
// driver for CanFestival-3 Win32 port

#include <sstream>
#include <iomanip>
#if 0  // change to 1 if you use boost
#include <boost/algorithm/string/case_conv.hpp>
#else
#include <algorithm>
#endif

extern "C" {
#include "can_driver.h"
}
class can_uvccm_win32
   {
   public:
      class error
        {
        };
      can_uvccm_win32(s_BOARD *board);
      ~can_uvccm_win32();
      bool send(const Message *m);
      bool receive(Message *m);
   private:
      bool open_rs232(int port = 1, int baud_rate = 57600);
      bool close_rs232();
      bool get_can_data(const char* can_cmd_buf, long& bufsize, Message* m);
      bool set_can_data(const Message& m, std::string& can_cmd);
   private:
      HANDLE m_port;
      HANDLE m_read_event;
      HANDLE m_write_event;
      std::string m_residual_buffer;
   };

can_uvccm_win32::can_uvccm_win32(s_BOARD *board) : m_port(INVALID_HANDLE_VALUE),
      m_read_event(0),
      m_write_event(0)
   {
   if (strcmp( board->baudrate, "125K") || !open_rs232(1))
      throw error();
   }

can_uvccm_win32::~can_uvccm_win32()
   {
   close_rs232();
   }

bool can_uvccm_win32::send(const Message *m)
   {
   if (m_port == INVALID_HANDLE_VALUE)
      return true;

   // build can_uvccm_win32 command string
   std::string can_cmd;
   set_can_data(*m, can_cmd);

   OVERLAPPED overlapped;
   ::memset(&overlapped, 0, sizeof overlapped);
   overlapped.hEvent = m_write_event;
   ::ResetEvent(overlapped.hEvent);

   unsigned long bytes_written = 0;
   ::WriteFile(m_port, can_cmd.c_str(), (unsigned long)can_cmd.size(), &bytes_written, &overlapped);
   // wait for write operation completion
   enum { WRITE_TIMEOUT = 1000 };
   ::WaitForSingleObject(overlapped.hEvent, WRITE_TIMEOUT);
   // get number of bytes written
   ::GetOverlappedResult(m_port, &overlapped, &bytes_written, FALSE);

   bool result = (bytes_written == can_cmd.size());
   
   return false;
   }


bool can_uvccm_win32::receive(Message *m)
   {
   if (m_port == INVALID_HANDLE_VALUE)
      return false;

   long res_buffer_size = (long)m_residual_buffer.size();
   bool result = get_can_data(m_residual_buffer.c_str(), res_buffer_size, m);
   if (result)
      {
      m_residual_buffer.erase(0, res_buffer_size);
      return true;
      }

   enum { READ_TIMEOUT = 500 };

   OVERLAPPED overlapped;
   ::memset(&overlapped, 0, sizeof overlapped);
   overlapped.hEvent = m_read_event;
   ::ResetEvent(overlapped.hEvent);
   unsigned long event_mask = 0;

   if (FALSE == ::WaitCommEvent(m_port, &event_mask, &overlapped) && ERROR_IO_PENDING == ::GetLastError())
      {
      if (WAIT_TIMEOUT == ::WaitForSingleObject(overlapped.hEvent, READ_TIMEOUT))
         return false;
      }

   // get number of bytes in the input que
   COMSTAT stat;
   ::memset(&stat, 0, sizeof stat);
   unsigned long errors = 0;
   ::ClearCommError(m_port, &errors, &stat);
   if (stat.cbInQue == 0)
      return false;
   char buffer[3000];

   unsigned long bytes_to_read = min(stat.cbInQue, sizeof (buffer));

   unsigned long bytes_read = 0;
   ::ReadFile(m_port, buffer, bytes_to_read, &bytes_read, &overlapped);
   // wait for read operation completion
   ::WaitForSingleObject(overlapped.hEvent, READ_TIMEOUT);
   // get number of bytes read
   ::GetOverlappedResult(m_port, &overlapped, &bytes_read, FALSE);
   result = false;
   if (bytes_read > 0)
      {
      m_residual_buffer.append(buffer, bytes_read);
      res_buffer_size = (long)m_residual_buffer.size();
      result = get_can_data(m_residual_buffer.c_str(), res_buffer_size, m);
      if (result)
         m_residual_buffer.erase(0, res_buffer_size);
      }
   return result;
   }

bool can_uvccm_win32::open_rs232(int port, int baud_rate)
   {
   if (m_port != INVALID_HANDLE_VALUE)
      return true;

   std::ostringstream device_name;
   device_name << "COM" << port;

   m_port = ::CreateFile(device_name.str().c_str(),
                         GENERIC_READ | GENERIC_WRITE,
                         0,   // exclusive access
                         NULL,   // no security
                         OPEN_EXISTING,
                         FILE_FLAG_OVERLAPPED,   // overlapped I/O
                         NULL); // null template

   // Check the returned handle for INVALID_HANDLE_VALUE and then set the buffer sizes.
   if (m_port == INVALID_HANDLE_VALUE)
      return false;

   //  SetCommMask(m_hCom,EV_RXCHAR|EV_TXEMPTY|EV_CTS|EV_DSR|EV_RLSD|EV_BREAK|EV_ERR|EV_RING); //
   ::SetCommMask(m_port, EV_RXFLAG);

   COMMTIMEOUTS timeouts;
   ::memset(&timeouts, 0, sizeof (timeouts));
   timeouts.ReadIntervalTimeout = -1;
   timeouts.ReadTotalTimeoutConstant = 0;
   timeouts.ReadTotalTimeoutMultiplier = 0;
   timeouts.WriteTotalTimeoutConstant = 5000;
   timeouts.WriteTotalTimeoutMultiplier = 0;
   SetCommTimeouts(m_port, &timeouts); //

   ::SetupComm(m_port, 1024, 512); // set buffer sizes

   // Port settings are specified in a Data Communication Block (DCB). The easiest way to initialize a DCB is to call GetCommState to fill in its default values, override the values that you want to change and then call SetCommState to set the values.
   DCB dcb;
   ::memset(&dcb, 0, sizeof (dcb));
   ::GetCommState(m_port, &dcb);
   dcb.BaudRate = baud_rate;
   dcb.ByteSize = 8;
   dcb.Parity = NOPARITY;
   dcb.StopBits = ONESTOPBIT;
   dcb.fAbortOnError = TRUE;
   dcb.EvtChar = 0x0A; // '\n' character
   ::SetCommState(m_port, &dcb);

   ::PurgeComm(m_port, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);

   m_read_event = ::CreateEvent(NULL, TRUE, FALSE, NULL);
   m_write_event = ::CreateEvent(NULL, TRUE, FALSE, NULL);

   return true;
   }

bool can_uvccm_win32::close_rs232()
   {
   if (m_port != INVALID_HANDLE_VALUE)
      {
      ::PurgeComm(m_port, PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);
      ::CloseHandle(m_port);
      m_port = INVALID_HANDLE_VALUE;
      ::CloseHandle(m_read_event);
      m_read_event = 0;
      ::CloseHandle(m_write_event);
      m_write_event = 0;
      m_residual_buffer.clear();
      }
   return true;
   }

bool can_uvccm_win32::get_can_data(const char* can_cmd_buf, long& bufsize, Message* m)
   {
   if (bufsize < 5)
      {
      bufsize = 0;
      return false;
      }

   Message msg;
   ::memset(&msg, 0 , sizeof (msg));
   char colon = 0, type = 0, request = 0;
   std::istringstream buf(std::string(can_cmd_buf, bufsize));
   buf >> colon >> type >> std::hex >> msg.cob_id >> request;
   if (colon != ':' || (type != 'S' && type != 'X'))
      {
      bufsize = 0;
      return false;
      }
   if (request == 'N')
      {
      msg.rtr = 0;
      for (msg.len = 0; msg.len < 8; ++msg.len)
         {
         std::string data_byte_str;
         buf >> std::setw(2) >> data_byte_str;
         if (data_byte_str[0] == ';')
            break;
         long byte_val = -1;
         std::istringstream(data_byte_str) >> std::hex >> byte_val;
         if (byte_val == -1)
            {
            bufsize = 0;
            return false;
            }
         msg.data[msg.len] = (UNS8)byte_val;
         }
      if (msg.len == 8)
         {
         char semicolon = 0;
         buf >> semicolon;
         if (semicolon != ';')
            {
            bufsize = 0;
            return false;
            }
         }

      }
   else if (request == 'R')
      {
      msg.rtr = 1;
      buf >> msg.len;
      }
   else
      {
      bufsize = 0;
      return false;
      }

   bufsize = buf.tellg();

   *m = msg;
   return true;
   }

bool can_uvccm_win32::set_can_data(const Message& m, std::string& can_cmd)
   {
   // build can_uvccm_win32 command string
   std::ostringstream can_cmd_str;
   can_cmd_str << ":S" << std::hex << m.cob_id;
   if (m.rtr == 1)
      {
      can_cmd_str << 'R' << (long)m.len;
      }
   else
      {
      can_cmd_str << 'N';
      for (int i = 0; i < m.len; ++i)
         can_cmd_str << std::hex << std::setfill('0') << std::setw(2) << (long)m.data[i];
      }
   can_cmd_str << ';';
   can_cmd = can_cmd_str.str();
#ifdef BOOST_VERSION
   boost::to_upper(can_cmd);
#else
   std::transform(can_cmd.begin(),can_cmd.end(),can_cmd.begin(),::toupper);
#endif
   return true;
   }


//------------------------------------------------------------------------
extern "C"
   UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
   {
   return (UNS8)(!(reinterpret_cast<can_uvccm_win32*>(fd0)->receive(m)));
   }

extern "C"
   UNS8 canSend_driver(CAN_HANDLE fd0, Message *m)
   {
   return (UNS8)reinterpret_cast<can_uvccm_win32*>(fd0)->send(m);
   }

extern "C"
   CAN_HANDLE canOpen_driver(s_BOARD *board)
   {
   try
      {
      return (CAN_HANDLE) new can_uvccm_win32(board);
      }
   catch (can_uvccm_win32::error&)
      {
      return NULL;
      }
   }

extern "C"
   int canClose_driver(CAN_HANDLE inst)
   {
   delete reinterpret_cast<can_uvccm_win32*>(inst);
   return 1;
   }

extern "C"
	UNS8 canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
	{
	return 0;
	} 