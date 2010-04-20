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

#ifndef __CANDRIVER__
#define __CANDRIVER__

//#include DEBUG_CAN

#include <can.h>
#include <objdictdef.h>


/*
The CAN message received are stored in a fifo stack
We consider one stack for all the 5 can devices. It is a choice !
*/

/* Be free to change this value */
#define MAX_STACK_MSG_RCV 5

/* Number of incomings and outcomings CAN Line. The layer CanOpen must be
used only for ONE line CAN. But you may used one or more CAN drivers, without
a CanOpen layer.
Only 2 lines are implemented. If more lines are needed, copy void __attribute__((interrupt)) can0HdlRcv (void) to void __attribute__((interrupt)) canXHdlRcv (void) and make 
changes : [0] to [x], CAN0 to CANX, etc
*/
#define NB_LINE_CAN 1

/* Whose hardware HCS12 CAN block is used for CanOpen ? Chose between CAN0, ..., CAN4
If you use CANOPEN_LINE_NUMBER_USED = CANI, the value of NB_LINE_CAN must be
more or equal to I + 1
Value other than CAN0 not tested, but should work fine.
 */
#define CANOPEN_LINE_NUMBER_USED CAN0

/* Stack of received messages 
MSG received on CAN0 module is stored in stackMsgRcv[0], etc
*/
extern volatile Message stackMsgRcv[NB_LINE_CAN][MAX_STACK_MSG_RCV];


/* Copy from the stack of the message to treat */
extern Message msgRcv;


/* To move on the stack of messages 
 */
typedef struct {
  UNS8 w ; /* received */
  UNS8 r ; /* To transmit */
} t_pointerStack;


/* Pointer for write or read a message in/from the reception stack */
extern volatile t_pointerStack ptrMsgRcv[NB_LINE_CAN];

/* 
Parameters to define the  clock system for the CAN bus
example : 
CAN_BUS_TIME clk = {
      1,  // clksrc: Use the bus clock : 16 MHz, the freq. of the quartz's board        
      0,  // brp :  chose btw 0 and 63 (6 bits).  freq time quantum = 16MHz / (brp + 1) 
      1,  // sjw : chose btw 0 and 3 (2 bits). Sync on (sjw + 1 ) time quantum          
      1,  // samp : chose btw 0 and 3 (2 bits) (samp + 1 ) samples per bit              
      4,  // tseg2 : chose btw 0 and 7 (3 bits) Segment 2 width = (tseg2 + 1)  tq       
      9,  // tseg1 : chose btw 0 and 15 (4 bits) Segment 1 width = (tseg1 + 1)  tq      

      
      With these values, 
      - The width of the bit time is 16 time quantum :
          - 1 tq for the SYNC segment (could not be modified)
          - 10 tq for the TIME 1 segment (tseg1 = 9)
          - 5 tq for the TIME 2 segment (tseg2 = 4)
      - Because the bus clock of the MSCAN is 16 MHZ, and the 
        freq of the time quantum is 16 MHZ (brp = 0), and  there are 16 tq in the bit time,
        so the freq of the bit time is 1 MHz.
      
  };
*/
typedef struct {
  UNS8  clksrc;     /* use of internal clock  or clock bus        */
  UNS8  brp;        /* define the bus speed                       */
  UNS8  sjw;        /* Number of time quantum for synchro - 1     */
  UNS8  samp;       /* Number of sample per bit (1 or 3)          */
  UNS8  tseg2;      /* Width of the time segment 2 (in tq) - 1    */
  UNS8  tseg1;      /* Width of the time segment 1 (in tq) - 1    */
} canBusTime;

/* 
Parameters to init the filters for received messages
*/
typedef struct {
  UNS8  idam;        /* Put 0x01 for 16 bits acceptance filter    */
  UNS8  canidar0;
  UNS8  canidmr0;
  UNS8  canidar1;
  UNS8  canidmr1; 
  UNS8  canidar2;
  UNS8  canidmr2;
  UNS8  canidar3;
  UNS8  canidmr3;
  UNS8  canidar4;
  UNS8  canidmr4;
  UNS8  canidar5;
  UNS8  canidmr5; 
  UNS8  canidar6;
  UNS8  canidmr6;
  UNS8  canidar7;
  UNS8  canidmr7;  
} canBusFilterInit;

/*
Parameters to init MSCAN
Example
CAN_BUS_INIT bi = {
    0,     no low power                  
    0,     no time stamp                
    1,     enable MSCAN                 
    0,     clock source : oscillator    
    0,     no loop back                 
    0,     no listen only               
    0,     no low pass filter for wk up 
    {
      1,       Use the oscillator clock                         
      0,       Quartz oscillator : freq time quantum =  freq oscillator clock / (0 + 1)
      1,       Sync on (1 + 1) time quantum                            
      1,       1 sample per bit                                  
      4,       time segment 2 width : (4 + 1) tq                     
      9,       time segment 1 width : (9 + 1) tq                     
    }
  };   
*/

typedef struct {
  UNS8  cswai;      /* Low power/normal in wait mode   (1/0)      */
  UNS8  time;       /* Timer for time-stamp enable/disable (1/0)  */
  UNS8  cane;       /* Enable MSCAN (yes=1) Do it !               */
  UNS8  clksrc;     /* clock source bus/oscillator (1/0)          */
  UNS8  loopb;      /* loop back mode for test (yes=1/no=0)       */
  UNS8  listen;     /* MSCAN is listen only (yes=1/no=0 ie normal)*/
  UNS8  wupm;       /* low pas filter for wake up (yes=1/no=0)    */
  canBusTime 
        clk;        /* Values for clock system init               */
  canBusFilterInit
  fi;               /* Values for filter acceptance msg init      */
  
} canBusInit;

extern canBusInit bi;



/*
For the received messsage, add a Identificator to
the list of ID to accept.
You can use several times this function to accept several messages.
It configures registers on 16 bits.
Automatically, it configure the filter to
- not accepting the msg on 29 bits (ide=1 refused)
- not filtering on rtr state (rtr = 1 and rtr = 0 are accepted)
Algo :
if CANIDARx = 0 then  CANIDARx = id . else do nothing
CANIDMRx = CANIDMRx OR (CANIDARx XOR id )
nFilter : 0 to 3
Must be in init mode before.
*/
char canAddIdToFilter (
		       UNS16 adrCAN,
		       UNS8 nFilter,
		       UNS16 id /* 11 bits, the 5 msb not used */
		       );

/*
 Use this function to change the CAN message acceptance filters and masks.
 */
char canChangeFilter (UNS16 adrCAN, canBusFilterInit fi);


/*
Enable one of the 5 MSCAN.
Must be done only one time after a reset of the CPU.
To do before any CAN initialisation
*/
char canEnable (
		UNS16 adrCAN /* First address of MSCANx registers */
		);


/* 
Initialize one of the 5 mscan
can be done multiple times in your code
Return 0 : OK
When it return from the function,
mscan is on sleep mode with wake up disabled.
      is not on init mode
*/
char canInit (
	      UNS16 adrCAN,   /* First address of MSCANx registers  */
	      canBusInit 
	      bi       /* All the parameters to init the bus */
	      );
/*
Initialize the parameters of the system clock for the MSCAN
You must put the MSCAN in sleep mode before with canSleepMode()
Return 0 : OK
       1 : Not in sleep mode. Unable to init MSCAN 
*/
char canInitClock (
		   UNS16 adrCAN, /* First address of MSCANx registers */
		   canBusTime clk);

/* 
Initialize one filter for acceptance of received msg.
Filters MUST be configured on 16 bits 
(See doc Motorola mscan bloc guide fig 4.3)
Must be in init mode before.
adrCAN  : adress of the first register of the mscan module
nFilter : the filter : 0 to 3.
ar : Value to write in acceptance register
     Beware ! hight byte and low byte inverted.
     for example if nFilter = 0, hight byte of ar -> CANIDAR0
                                 low   byte of ar -> CANIDAR1
mr : Value to write in mask register
     Beware ! hight byte and low byte inverted.
     for example if nFilter = 2, hight byte of ar -> CANIDMR4
                                 low   byte of ar -> CANIDMR5
*/
char canInit1Filter (
		     UNS16 adrCAN, 
		     UNS8 nFilter,
		     UNS16 ar,
		     UNS16 mr
		     );

/*
Initialise the parameters for filtering the messages received.
You must put the MSCAN in init mode before with canInitMode()
Return 0 : OK
       1 : Not in init mode. Unable to init MSCAN 
*/

char canInitFilter (
		    UNS16 adrCAN, /* First address of MSCANx registers */
		    canBusFilterInit fi);
/*
Put one of the 5 mscan in Init mode
Loop until init mode is reached.
*/

char canInitMode (
		  UNS16 adrCAN /* First address of MSCANx registers */
		  );	

/*
Leave the Init mode
loop until init mode leaved.
*/

char canInitModeQ (
		   UNS16 adrCAN /* First address of MSCANx registers */
		   );



/*
Transmit a msg on CAN "adrCan"
Return : 0      No error
         other  error : no buffer available to make the transmission
*/	

char canMsgTransmit (
		     UNS16 adrCAN,  /* First address of MSCANx registers */
		     Message msg  /* Message to transmit                */
		     );

/*
 Set the interruptions. Must be call just after having left the init mode.
 */	     
char canSetInterrupt (UNS16 adrCAN);		     

/*
Put one of the 5 mscan in sleep mode
Beware! If some messages are to be sent,
or if it is receiving, going into sleep mode
may take time.
Wake up is disabled : stay in sleep mode even if
bus traffic detected.
return 0 if 0K, other if error : mscan is on init mode.
Stay in this function until the sleep mode
is reached.
*/
char canSleepMode (
		   UNS16 adrCAN /* First address of MSCANx registers */
		   );	

/*
Leave the sleep mode
loop until sleep mode leaved.
return 0 : OK
return 1 : error : in init mode
*/
char canSleepModeQ (
		    UNS16 adrCAN /* First address of MSCANx registers */
		    );	

/*
Put one of the 5 mscan in sleep mode
MSCAN must not be in init mode.
wake up is enabled : wake up if traffic on CAN is detected
Beware! If some messages are to be sent,
or if it is receiving, going into sleep mode
may take time.
Loop until sleep mode reached.
return 0 if 0K, other if error
*/
char canSleepWupMode (
		      UNS16 adrCAN /* First address of MSCANx registers */
		      );	

/*
Test if one of the 5 mscan is in init mode.
Return 
       0     -> Not in init mode
       other -> In init mode
*/
char canTestInitMode (
		      UNS16 adrCAN /* First address of MSCANx registers */
		      );   

/*
Test if one of the 5 mscan is in sleep mode.
Return 
       0     -> Not in sleep mode
       other -> In sleep mode
*/
char canTestSleepMode (
		       UNS16 adrCAN /* First address of MSCANx registers */
		       );   



#endif /*__CANDRIVER__*/

