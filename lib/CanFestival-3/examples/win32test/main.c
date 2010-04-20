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

/**************************************************************************
CanFestival3 win32 port example

This sample demonstrates CanFestival usage with Win32

Program implements master node. It starts CANOpen slave node, modifies OD, 
performs SDO reads and prints some slave node information.

Usage:

   win32test <node_id> [can driver dll filename [baud rate]]

 where node_id is node_id in decimal format

If driver is not specified, CAN-uVCCM.dll will be used by default.
If baudrate is not specified, 125K will be used by default.
You should have CanFestival-3.dll can driver dll in the search path to run this sample.

Sample can work on other platdorms as well.
***************************************************************************/

#include <stdio.h>
#include <stdlib.h> 
#include "win32test.h"
#include "canfestival.h"

#ifdef WIN32
#define sleep_proc(ms) Sleep(ms)
#define uptime_ms_proc() GetTickCount()
#else
#include <time.h> 
#define sleep_proc(ms)
#define uptime_ms_proc (1000*(time()%86400))  // TOD
#endif

UNS8 GetChangeStateResults(UNS8 node_id, UNS8 expected_state, unsigned long timeout_ms)
   {
   unsigned long start_time = 0;
   
   // reset nodes state
   win32test_Data.NMTable[node_id] = Unknown_state;

   // request node state
   masterRequestNodeState(&win32test_Data, node_id);
   
   start_time = uptime_ms_proc();
   while(uptime_ms_proc() - start_time < timeout_ms)
      {
      if (getNodeState(&win32test_Data, node_id) == expected_state)
         return 0;
      sleep_proc(1);   
      }
   return 0xFF;
   }

UNS8 ReadSDO(UNS8 nodeId, UNS16 index, UNS8 subIndex, UNS8 dataType, void* data, UNS8* size)
   {
   UNS32 abortCode = 0;
   UNS8 res = SDO_UPLOAD_IN_PROGRESS;
   // Read SDO
   UNS8 err = readNetworkDict (&win32test_Data, nodeId, index, subIndex, dataType);
   if (err)
      return 0xFF;
   for(;;)
      {
      res = getReadResultNetworkDict (&win32test_Data, nodeId, data, size, &abortCode);
      if (res != SDO_UPLOAD_IN_PROGRESS)
         break;   
      sleep_proc(1);
      continue;
      }
   closeSDOtransfer(&win32test_Data, nodeId, SDO_CLIENT);
   if (res == SDO_FINISHED)
      return 0;
   return 0xFF;   
   }

int main(int argc, char *argv[])
  {
   UNS8 node_id = 0;
   s_BOARD MasterBoard = {"1", "125K"};
   char* dll_file_name;

   /* process command line arguments */
   if (argc < 2)
      {
      printf("USAGE: win32test <node_id> [can driver dll filename [baud rate]]\n");
      return 1;
      }

   node_id = atoi(argv[1]);
   if (node_id < 2 || node_id > 127)
      {
      printf("ERROR: node_id shoule be >=2 and <= 127\n");
      return 1;
      }

   if (argc > 2)
      dll_file_name = argv[2];
   else
      dll_file_name = "can_uvccm_win32.dll";

   if (argc > 3)
      MasterBoard.baudrate = argv[3];

   // load can driver
   if (!LoadCanDriver(dll_file_name))
      {
      printf("ERROR: could not load diver %s\n", dll_file_name);
      return 1;
      }
   
   if (canOpen(&MasterBoard,&win32test_Data))
      {
      /* Defining the node Id */
      setNodeId(&win32test_Data, 0x01);

      /* init */
      setState(&win32test_Data, Initialisation);

      /****************************** START *******************************/
      /* Put the master in operational mode */
      setState(&win32test_Data, Operational);

      /* Ask slave node to go in operational mode */
      masterSendNMTstateChange (&win32test_Data, 0, NMT_Start_Node);

      printf("\nStarting node %d (%xh) ...\n",(int)node_id,(int)node_id);
      
      /* wait untill mode will switch to operational state*/
      if (GetChangeStateResults(node_id, Operational, 3000) != 0xFF)
         {
         /* modify Client SDO 1 Parameter */
         UNS32 COB_ID_Client_to_Server_Transmit_SDO = 0x600 + node_id;
         UNS32 COB_ID_Server_to_Client_Receive_SDO  = 0x580 + node_id;
         UNS32 Node_ID_of_the_SDO_Server = node_id;
         UNS8 ExpectedSize = sizeof (UNS32);

         if (OD_SUCCESSFUL ==  writeLocalDict(&win32test_Data, 0x1280, 1, &COB_ID_Client_to_Server_Transmit_SDO, &ExpectedSize, RW) 
              && OD_SUCCESSFUL ==  writeLocalDict(&win32test_Data, 0x1280, 2, &COB_ID_Server_to_Client_Receive_SDO, &ExpectedSize, RW) 
              && OD_SUCCESSFUL ==  writeLocalDict(&win32test_Data, 0x1280, 3, &Node_ID_of_the_SDO_Server, &ExpectedSize, RW))
            {
            UNS32 dev_type = 0;
            char device_name[64]="";
            char hw_ver[64]="";
            char sw_ver[64]="";   
            UNS32 vendor_id = 0;            
            UNS32 prod_code = 0;
            UNS32 ser_num = 0;
            UNS8 size;
            UNS8 res;

            printf("\nnode_id: %d (%xh) info\n",(int)node_id,(int)node_id);
            printf("********************************************\n");

            size = sizeof (dev_type);
            res = ReadSDO(node_id, 0x1000, 0, uint32, &dev_type, &size);
            printf("device type: %d\n",dev_type & 0xFFFF);
           
            size = sizeof (device_name);
            res = ReadSDO(node_id, 0x1008, 0, visible_string, device_name, &size);
            printf("device name: %s\n",device_name);

            size = sizeof (hw_ver);
            res = ReadSDO(node_id, 0x1009, 0, visible_string, hw_ver, &size);
            printf("HW version: %s\n",hw_ver);

            size = sizeof (sw_ver);
            res = ReadSDO(node_id, 0x100A, 0, visible_string, sw_ver, &size);
            printf("SW version: %s\n",sw_ver);            
            
            size = sizeof (vendor_id);
            res = ReadSDO(node_id, 0x1018, 1, uint32, &vendor_id, &size);
            printf("vendor id: %d\n",vendor_id);

            size = sizeof (prod_code);
            res = ReadSDO(node_id, 0x1018, 2, uint32, &prod_code, &size);
            printf("product code: %d\n",prod_code);

            size = sizeof (ser_num);
            res = ReadSDO(node_id, 0x1018, 4, uint32, &ser_num, &size);
            printf("serial number: %d\n",ser_num);
            
            printf("********************************************\n");
            } 
         else
            {
            printf("ERROR: Object dictionary access failed\n");
            }
         }
      else
         {
         printf("ERROR: node_id %d (%xh) is not responding\n",(int)node_id,(int)node_id);
         }
         
      masterSendNMTstateChange (&win32test_Data, 0x02, NMT_Stop_Node);

      setState(&win32test_Data, Stopped);
      
      canClose(&win32test_Data);
      }
   return 0;
  }
  
  