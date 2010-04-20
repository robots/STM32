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

// Uncomment if you don't need console informations.
#define DEBUG_WAR_CONSOLE_ON
#define DEBUG_ERR_CONSOLE_ON

#include <stddef.h> /* for NULL */

#include <asm-m68hc12/portsaccess.h>
#include <asm-m68hc12/ports_def.h>
#include <asm-m68hc12/ports.h>
#include  <interrupt.h>

#include "../include/data.h"
#include <applicfg.h>



#include "../include/hcs12/candriver.h"
#include "../include/hcs12/canOpenDriver.h"
#include "../include/def.h"
#include "../include/can.h"
#include "../include/objdictdef.h"
#include "../include/objacces.h"
#include "../include/sdo.h"
#include "../include/pdo.h"
#include "../include/timer.h"
#include "../include/lifegrd.h"
#include "../include/sync.h"

#include "../include/nmtSlave.h"

// File created by the GUI 
#include "objdict.h"





// HCS12 configuration
// -----------------------------------------------------

enum E_CanBaudrate 
{
   CAN_BAUDRATE_250K,
   CAN_BAUDRATE_500K,
   CAN_BAUDRATE_1M,
};


const canBusTime CAN_Baudrates[] =
{
   {
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      3,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      0,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      0,  /* samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              */
      1,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
     12,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 13 tq for the TIME 1 segment (tseg1 = 12)
          - 2 tq for the TIME 2 segment (tseg2 = 1)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 4 MHZ (brp = 3+1), and  there are 16 tq in the bit time,
        so the freq of the bit time is 250 kHz.
      */
   },

   {
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      1,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      0,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      0,  /* samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              */
      1,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
     12,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 13 tq for the TIME 1 segment (tseg1 = 12)
          - 2 tq for the TIME 2 segment (tseg2 = 1)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 8 MHZ (brp = 1+1), and  there are 16 tq in the bit time,
        so the freq of the bit time is 500 kHz.
      */
    },

	{
      1,  /* clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        */
      1,  /* brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) */
      0,  /* sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          */
      0,  /* samp : chose btw 0 and 3 (2 bits) (samp +MSG_WAR(0x3F33, "Je suis le noeud ", getNodeId());    1 ) samples per bit              */
      1,  /* tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       */
      4,  /* tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      */

      /*
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 5 tq for the TIME 1 segment (tseg1 = 4)
          - 2 tq for the TIME 2 segment (tseg2 = 1)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 8 MHZ (brp = 1+1), and  there are 8 tq in the bit time,
        so the freq of the bit time is 1 MHz.
      */
    }
};




/**************************prototypes*****************************************/

//Init can bus and Canopen
void initCanopencapteur (void);
// Init the sensor
void initSensor(void);
void initPortB(void);
void initPortH(void);


//------------------------------------------------------------------------------
//Initialisation of the port B for the leds.
void initPortB(void)
{
  // Port B is output
  IO_PORTS_8(DDRB)= 0XFF;
  // RAZ
  IO_PORTS_8(PORTB) = 0xFF;
}

//------------------------------------------------------------------------------
// Init of port H to choose the CAN rate by switch, and the nodeId
void initPortH(void)
{
  // Port H is input
  IO_PORTS_8(DDRH)= 0X00;
  // Enable pull device
  IO_PORTS_8(PERH) = 0XFF;
  // Choose the pull-up device
  IO_PORTS_8(PPSH) = 0X00;
}

//------------------------------------------------------------------------------
void initSensor(void)
{ 
  UNS8 baudrate = 0; 
  UNS8 nodeId = 0; 
  // Init led control
  initPortB(); 
  IO_PORTS_8(PORTB) &= ~ 0x01; //One led ON
  initPortH();
  
    /* Defining the node Id */
  // Uncomment to have a fixed nodeId
  //setNodeId(&gene_SYNC_Data, 0x03);
  
  // Comment below to have a fixed nodeId
  nodeId = ~(IO_PORTS_8(PTH)) & 0x3F;
  if (nodeId == 0) {
  	MSG_WAR(0x3F33, "Using default nodeId :  ", getNodeId(&gene_SYNC_Data));
  }
  else	
    setNodeId(&gene_SYNC_Data, nodeId);
  
  MSG_WAR(0x3F33, "My nodeId is :  ", getNodeId(&gene_SYNC_Data));
  
  canBusInit bi0 = {
    0,    /* no low power                 */ 
    0,    /* no time stamp                */
    1,    /* enable MSCAN                 */
    0,    /* clock source : oscillator (In fact, it is not used)   */
    0,    /* no loop back                 */
    0,    /* no listen only               */
    0,    /* no low pass filter for wk up */
	CAN_Baudrates[CAN_BAUDRATE_250K],
    {
      0x00,    /* Filter on 16 bits. See Motorola Block Guide V02.14 fig 4-3 */
      0x00, 0xFF, /* filter 0 hight accept all msg      */
      0x00, 0xFF, /* filter 0 low accept all msg        */
      0x00, 0xFF, /* filter 1 hight filter all of  msg  */
      0x00, 0xFF, /* filter 1 low filter all of  msg    */
      0x00, 0xFF, /* filter 2 hight filter most of  msg */
      0x00, 0xFF, /* filter 2 low filter most of  msg   */
      0x00, 0xFF, /* filter 3 hight filter most of  msg */
      0x00, 0xFF, /* filter 3 low filter most of  msg   */
    }
  };
  
  //Init the HCS12 microcontroler for CanOpen 
  initHCS12();
   
  // Chose the CAN rate (On our board, whe have switch for all purpose)
  //  7    8
  //  ON   ON   => 1000 kpbs
  //  OFF  ON   =>  500 kpbs
  //  ON   OFF  =>  250 kpbs
  
  baudrate = ~(IO_PORTS_8(PTH)) & 0xC0;
  
  // Uncomment to have a fixed baudrate of 250 kbps
  //baudrate = 1;
  
  switch (baudrate) {
  case 0x40:
    bi0.clk = CAN_Baudrates[CAN_BAUDRATE_250K];
    MSG_WAR(0x3F30, "CAN 250 kbps ", 0);
    break;
  case 0x80:
    bi0.clk = CAN_Baudrates[CAN_BAUDRATE_500K];
    MSG_WAR(0x3F31, "CAN 500 kbps ", 0);
    break;
  case 0xC0:
    bi0.clk = CAN_Baudrates[CAN_BAUDRATE_1M];
    MSG_WAR(0x3F31, "CAN 1000 kbps ", 0);
    break;   
  default:
    bi0.clk = CAN_Baudrates[CAN_BAUDRATE_250K];
    MSG_WAR(0x2F32, "CAN BAUD RATE NOT DEFINED => 250 kbps ", 0);
  }

   

  MSG_WAR(0x3F33, "SYNC signal generator ", 0);

  canInit(CANOPEN_LINE_NUMBER_USED, bi0);  //initialize filters...
  initTimer(); // Init hcs12 timer used by CanFestival. (see timerhw.c)
  unlock(); // Allow interruptions  
}






//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// FUNCTIONS WHICH ARE PART OF CANFESTIVAL and *must* be implemented here.

//------------------------------------------------------------------------------
void gene_SYNC_heartbeatError(CO_Data* d,  UNS8 heartbeatID )
{
  
  MSG_ERR(0x1F00, "HeartBeat not received from node : ", heartbeatID);
}

//------------------------------------------------------------------------------
void gene_SYNC_initialisation(CO_Data* d)
{  
  MSG_WAR (0x3F00, "Entering in INIT ", 0); 
  initSensor();  

  IO_PORTS_8(PORTB) &= ~ 0x01; // led  0         : ON
  IO_PORTS_8(PORTB) |=   0x0E; // leds 1, 2, 3   : OFF
  
}


//------------------------------------------------------------------------------
void gene_SYNC_preOperational(CO_Data* d)
{
  MSG_WAR (0x3F01, "Entering in PRE-OPERATIONAL ", 0); 
  IO_PORTS_8(PORTB) &= ~ 0x03; // leds 0, 1      : ON
  IO_PORTS_8(PORTB) |=   0x0C; // leds 2, 3      : OFF
    /* default values for the msg CAN filters */
  /* Accept all */
    {
   	  canBusFilterInit filterConfiguration = 
   	  {
   	  0x01,  /* Filter on 16 bits. See Motorola Block Guide V02.14 */
      /*canidarx, canidmrx */                        
      0x00, 0xFF,          /* filter 0 */
      0x00, 0xFF, 		   /* filter 0 */
      0x00, 0xFF,          /* filter 1 */
      0x00, 0xFF, 		   /* filter 1 */
      0x00, 0xFF,          /* filter 2 */
      0x00, 0xFF, 		   /* filter 2 */
      0x00, 0xFF,          /* filter 3 */
      0x00, 0xFF, 	       /* filter 3 */
   	 };
   	 canChangeFilter(CANOPEN_LINE_NUMBER_USED, filterConfiguration);
   } 
   // Reset the automatic change by SDO
   applyDownloadedFilters = 0;
   
}


//------------------------------------------------------------------------------
void gene_SYNC_operational(CO_Data* d)
{ 
   MSG_WAR (0x3F02, "Entering in OPERATIONAL ", 0); 
   IO_PORTS_8(PORTB) &= ~ 0x07; // leds 0, 1, 2   : ON
   IO_PORTS_8(PORTB) |=   0x08; // leds 3         : OFF
   
   // Filtering the CAN received msgs.
   // 2 ways
   // First :applying an automatic filter
   // Second : The values of the filtering registers are mapped in the object dictionary,
   // So that a filtering configuration can be downloaded by SDO in pre-operational mode
   	
   	if (applyDownloadedFilters == 0) {// No downloaded configuration to apply
  		UNS16 accept1 = 0x0000; // Accept NMT
   		UNS16 mask1 = 0x0FFF;   // Mask NMT
   		UNS16 accept2 = 0xE000; // Accept NMT error control (heartbeat, nodeguard)
   		UNS16 mask2 = 0x0FFF;   // Mask NMT error control (heartbeat, nodeguard)
   		
   		canBusFilterInit filterConfiguration = 
				{// filters 3 and 4 not used, so configured like filter 1.
   		  	0x01,  /* Filter on 16 bits. See Motorola Block Guide V02.14 */
     	    /*canidarx, canidmrx */                        
      		(UNS8)(accept1 >> 8), (UNS8)(mask1 >> 8),    /* filter 1 id10...3*/
      		(UNS8)accept1       , (UNS8)mask1, 		       /* filter 1 id2 ... */
      		(UNS8)(accept2 >> 8), (UNS8)(mask2 >> 8),    /* filter 2 id10...3*/
      		(UNS8)accept2       , (UNS8)mask2, 		       /* filter 2 id2 ... */
       		(UNS8)(accept1 >> 8), (UNS8)(mask1 >> 8),    /* filter 3 id10...3*/
      		(UNS8)accept1       , (UNS8)mask1, 		       /* filter 3 id2 ... */
      		(UNS8)(accept1 >> 8), (UNS8)(mask1 >> 8),    /* filter 4 id10...3*/
      		(UNS8)accept1       , (UNS8)mask1 		       /* filter 4 id2 ... */     	
   	 	 	};
   	 		canChangeFilter(CANOPEN_LINE_NUMBER_USED, filterConfiguration);
   	 		MSG_WAR (0x3F03, "Internal CAN Rcv filter applied ", 0); 
    	} 
   	  else { // Apply filters downnloaded
   			canBusFilterInit filterConfiguration = 
				{// filters 3 and 4 not used, so configured like filter 1.
   		  	0x01,  /* Filter on 16 bits. See Motorola Block Guide V02.14 */
     	    /*canidarx, canidmrx */                        
      		(UNS8)( acceptanceFilter1>> 8), (UNS8)(mask1 >> 8),    /* filter 1 id10...3*/
      		(UNS8)acceptanceFilter1       , (UNS8)mask1, 		       /* filter 1 id2 ... */
      		(UNS8)(acceptanceFilter2 >> 8), (UNS8)(mask2 >> 8),     /* filter 2 id10...3*/
      		(UNS8)acceptanceFilter2       , (UNS8)mask2, 		       /* filter 2 id2 ... */
       		(UNS8)(acceptanceFilter3 >> 8), (UNS8)(mask3 >> 8),    /* filter 3 id10...3*/
      		(UNS8)acceptanceFilter3       , (UNS8)mask3, 		       /* filter 3 id2 ... */
      		(UNS8)(acceptanceFilter4 >> 8), (UNS8)(mask4 >> 8),    /* filter 4 id10...3*/
      		(UNS8)acceptanceFilter4       , (UNS8)mask4 		       /* filter 4 id2 ... */     	
   	 	 	};
   	 		canChangeFilter(CANOPEN_LINE_NUMBER_USED, filterConfiguration);  		
   	 		MSG_WAR (0x3F04, "Downloaded CAN Rcv filter applied ", 0); 
   	}
}

//------------------------------------------------------------------------------
void gene_SYNC_stopped(CO_Data* d)
{
  MSG_WAR (0x3F02, "Entering in STOPPED ", 0); 
  IO_PORTS_8(PORTB) |=   0x0E; // leds 1, 2, 3, 4   : OFF
}

// End functions which are part of Canfestival
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


/******************************************************************************/
/********************************* MAIN ***************************************/
/******************************************************************************/

 
UNS8 main (void)
{

  MSG_WAR(0x3F34, "Entering in the main ", 0);
  //----------------------------- INITIALISATION --------------------------------
  gene_SYNC_Data.heartbeatError = gene_SYNC_heartbeatError;
  gene_SYNC_Data.initialisation = gene_SYNC_initialisation;
  gene_SYNC_Data.preOperational = gene_SYNC_preOperational;
  gene_SYNC_Data.preOperational = gene_SYNC_operational;
  gene_SYNC_Data.stopped = gene_SYNC_stopped;

  
  /* Put the node in Initialisation mode */
  MSG_WAR(0x3F35, "Will entering in INIT ", 0);
  setState(&gene_SYNC_Data, Initialisation);

  //----------------------------- START -----------------------------------------
  /* Put the node in pre-operational mode */
  //MSG_WAR(0x3F36, "va passer en pre-op", 0);
  //setState(&gene_SYNC_Data, Pre_operational);

	// Loop of receiving messages
  while (1) {
	Message m;
	if (f_can_receive(0, &m)) {
	  //MSG_WAR(0x3F36, "Msg received", m.cob_id);
	  lock(); // Not to have interruptions by timer, which can corrupt the data
	  canDispatch(&gene_SYNC_Data, &m);
	  unlock();
	}  
    	}

  return (0); 
}



