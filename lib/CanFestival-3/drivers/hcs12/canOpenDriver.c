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

#define DEBUG_WAR_CONSOLE_ON
#define DEBUG_ERR_CONSOLE_ON

#include <stddef.h> /* for NULL */

#include "../include/hcs12/asm-m68hc12/portsaccess.h"
#include "../include/hcs12/asm-m68hc12/ports_def.h"
#include "../include/hcs12/asm-m68hc12/ports.h"
#include "../include/data.h"
#include "../include/hcs12/applicfg.h"
#include "../include/hcs12/candriver.h"
#include "../include/hcs12/interrupt.h"
#include "../include/hcs12/canOpenDriver.h"
#include "../include/can.h"
#include "../include/objdictdef.h"
#include "../include/timer.h"






volatile static Message stackMsgRcv[NB_LINE_CAN][MAX_STACK_MSG_RCV]; 
volatile static t_pointerStack ptrMsgRcv[NB_LINE_CAN];
 
volatile static TIMEVAL last_time_set = TIMEVAL_MAX;
static UNS8 timer_is_set = 0;

/* Prototypes */
UNS8 f_can_receive(UNS8 notused, Message *m);
UNS8 canSend(UNS8 notused, Message *m);
void __attribute__((interrupt)) timer4Hdl (void);

#define max(a,b) a>b?a:b

/******************************************************************************/
void setTimer(TIMEVAL value)
{
  IO_PORTS_16(TC4H) += value;
  timer_is_set = 1; 
}

/******************************************************************************/
TIMEVAL getElapsedTime()
{
  return (IO_PORTS_16(TC4H) > last_time_set ? IO_PORTS_16(TC4H) - last_time_set : last_time_set - IO_PORTS_16(TC4H));
}


/******************************************************************************/
void resetTimer(void)
{

}

/******************************************************************************/
void initTimer(void)
{
  lock();   // Inhibition of interruptions
 
  // Configure the timer channel 4
  IO_PORTS_8(TIOS) |= 0x10; // Canal 4 in output
  IO_PORTS_8(TCTL1) &= ~(0x01 + 0x02); // Canal 4 unconnected to pin output
  IO_PORTS_8(TIE) |= 0x10; // allow interruption channel 4
  IO_PORTS_8(TSCR2) |= 0X05; // Pre-scaler = 32 
                             // If this value is changed, change must be done also
                             // in void __attribute__((interrupt)) timer4Hdl (void)

  IO_PORTS_8(TSCR1) |= 0x80; // Start timer
  unlock(); // Allow interruptions
}

/******************************************************************************/
void __attribute__((interrupt)) timer4Hdl (void)
{
  lock();
  last_time_set = IO_PORTS_16(TC4H);
  IO_PORTS_8(TFLG1) = 0x10; // RAZ flag interruption timer channel 4
  // Compute the next event : When the timer reach the value of TC4, an interrupt is
  // started 
  //IO_PORTS_16(TC4H) += 250; // To have an interruption every 1 ms
  //timerInterrupt(0);
  //MSG_WAR(0xFFFF, "timer4 IT", 0);
  {
    //MSG_WAR(0xFFFF, "t4 ", IO_PORTS_16(TCNTH) - IO_PORTS_16(TC4H));
  }
  TimeDispatch();
  unlock();
}


/******************************************************************************/
void initSCI_0(void)
{
  IO_PORTS_16(SCI0 + SCIBDH) = 
    ((1000000 / SERIAL_SCI0_BAUD_RATE) * BUS_CLOCK) >> 4 ;  
  IO_PORTS_8(SCI0  + SCICR1) = 0;    // format 8N1
  IO_PORTS_8(SCI0  + SCICR2) = 0x08; // Transmit enable only
}

/******************************************************************************/
void initSCI_1(void)
{
  IO_PORTS_16(SCI1 + SCIBDH) = 
    ((1000000 / SERIAL_SCI1_BAUD_RATE) * BUS_CLOCK) >> 4 ;  
  IO_PORTS_8(SCI1  + SCICR1) = 0;    // format 8N1
  IO_PORTS_8(SCI1  + SCICR2) = 0x08; // Transmit enable only
}


/******************************************************************************/
char *
hex_convert (char *buf, unsigned long value, char lastCar)
{
  //Thanks to Stphane Carrez for this function
  char num[32];
  int pos;

  *buf++ = '0';
  *buf++ = 'x';

  pos = 0;
  while (value != 0) {
    char c = value & 0x0F;
    num[pos++] = "0123456789ABCDEF"[(unsigned) c];
    value = (value >> 4) & (0x0fffffffL);
    }
  if (pos == 0)
    num[pos++] = '0';

  while (--pos >= 0)
    *buf++ = num[pos];

  *buf++ = lastCar;
  *buf = 0;
  return buf;
}

/******************************************************************************/
void printSCI_str(char sci, const char * str) 
{
  char i = 0;
  
  while ((*(str + i) != 0) && (i < 0xFF)) {
    if (*(str + i) == '\n')
      {
	while ((IO_PORTS_8(sci + SCISR1) & 0X80) == 0); // wait if buffer not empty	
	IO_PORTS_8(sci + SCIDRL) = 13; // return to start of line
      }
    while ((IO_PORTS_8(sci + SCISR1) & 0X80) == 0); // wait if buffer not empty
    IO_PORTS_8(sci + SCIDRL) = *(str + i++);
  }

}

/******************************************************************************/
void printSCI_nbr(char sci, unsigned long nbr, char lastCar) 
{
  char strNbr[12];
  hex_convert(strNbr, nbr, lastCar);
  printSCI_str(sci, strNbr);
}

/******************************************************************************/
// PLL 24 MHZ if quartz on board is 16 MHZ
void initPLL(void)
{
  IO_PORTS_8(CLKSEL) &= ~0x80; // unselect the PLL
  IO_PORTS_8(PLLCTL) |= 0X60;  // PLL ON and bandwidth auto
  IO_PORTS_8(SYNR) = 0x02;
  IO_PORTS_8(REFDV) = 0x01;
  while ((IO_PORTS_8(CRGFLG) & 0x08) == 0);
  IO_PORTS_8(CLKSEL) |= 0x80;
}

/******************************************************************************/
void initHCS12(void)
{

# ifdef USE_PLL
  MSG_WAR(0x3620, "Use the PLL ", 0);
  initPLL();
# endif

}

/***************************************************************************/
char canAddIdToFilter(UNS16 adrCAN, UNS8 nFilter, UNS16 id)
{
  UNS8 fiMsb;
  UNS8 fiLsb;
  UNS8 idMsb = (UNS8) (id >> 3);
  UNS8 idLsb = (UNS8) (id << 5);

  if (! canTestInitMode(adrCAN)) {
    /* Error because not init mode */
    MSG_WAR(0X2600, "Not in init mode ", 0);
    return 1;   
  }
  switch (nFilter) {
    case 0:
      nFilter = CANIDAR0; /* First  bank */
      break;
    case 1:
      nFilter = CANIDAR2; /* First  bank */
      break;
    case 2:
      nFilter = CANIDAR4; /* Second bank */
      break;
    case 3:
      nFilter = CANIDAR6; /* Second bank */   
  }
  if (! IO_PORTS_16(adrCAN + nFilter)) {
    /* if CANIDARx = 0 */
    IO_PORTS_8(adrCAN + nFilter) = idMsb;
    IO_PORTS_8(adrCAN + nFilter + 1) = idLsb;
  }
  fiMsb = IO_PORTS_8(adrCAN + nFilter) ^ idMsb;
  fiLsb = IO_PORTS_8(adrCAN + nFilter + 1) ^ idLsb;
  /* address of CANIDMRx */
  IO_PORTS_8(adrCAN + nFilter + 4) = IO_PORTS_8(adrCAN + nFilter + 4) | fiMsb;
  IO_PORTS_8(adrCAN + nFilter + 5) = IO_PORTS_8(adrCAN + nFilter + 5) | fiLsb;
  IO_PORTS_8(adrCAN + nFilter + 5) |= 0x10; /* Not filtering on rtr value */
  return 0;
}

/***************************************************************************/
char canChangeFilter(UNS16 adrCAN, canBusFilterInit fi)
{
  /* If not in init mode, go to sleep before going in init mode*/
  if (! canTestInitMode(adrCAN)) {
    canSleepMode(adrCAN);
    canInitMode(adrCAN); 
  }
  //update the filters configuration
  canInitFilter(adrCAN, fi);
  canInitModeQ(adrCAN);
  canSleepModeQ(adrCAN);
  canSetInterrupt(adrCAN);
  return 0;
}

/***************************************************************************/
char canEnable(UNS16 adrCAN)
{
  /* Register CANCTL1
  bit 7 : 1 MSCAN enabled
  Other bits : default reset values
  */
  IO_PORTS_8(adrCAN + CANCTL1) = 0X80;
  return 0;
}

/***************************************************************************/
char canInit(UNS16 adrCAN, canBusInit bi)
{
  /* If not in init mode, go to sleep before going in init mode*/
  if (! canTestInitMode(adrCAN)) {
    canSleepMode(adrCAN);
    canInitMode(adrCAN); 
  }
  
  canEnable(adrCAN);                  /* Does nothing if already enable */
     /* The most secure way to go in init mode : put before MSCAN in sleep mode */
      //canSleepMode(adrCAN);
     /* Put MSCAN in Init mode */ 
     //canInitMode(adrCAN);
  canInitClock(adrCAN, bi.clk); 
  /* Init CANCTL1 register. Must be in init mode */
  IO_PORTS_8(adrCAN + CANCTL1) &=0xC4;// 0xCB; /* Clr the bits that may be modified */
  IO_PORTS_8(adrCAN + CANCTL1) = (bi.cane << 7) | (bi.loopb << 5 ) | 
    (bi.listen << 4) | (bi.wupm << 2);
  /* Initialize the filters for received msgs */
  /* We should decide to accept all the msgs  */
  canInitFilter(adrCAN, bi.fi);
  /* Before to modify CANCTL0, we must leave the init mode */
  canInitModeQ(adrCAN);
  /* Init CANCTL0 register. MSCAN must not be in init mode */
  /* Do not change the value of wupe (should be 0) and slprq (should be 1) */
  /* Do not change the value of initrq (should be 0) */	
  /* rxfrm is cleared, mupe also (should be before)*/
    IO_PORTS_8(adrCAN + CANCTL0) &= 0x53; /* Clr the bits that may be modified */
    IO_PORTS_8(adrCAN + CANCTL0) = (bi.cswai << 5) | (bi.time << 3);
    canSetInterrupt(adrCAN);
    canInitModeQ(adrCAN); /* Leave the init mode */
    canSleepModeQ(adrCAN); /* Leave the sleep mode */    
  return 0;
}

/***************************************************************************/
char canInitClock(UNS16 adrCAN, canBusTime clk)
{
  if (! canTestInitMode(adrCAN)) {
    /* Not in Init mode */
    MSG_WAR(0X2601, "not in init mode ", 0);
    return 1;   
  }
  /* Set or reset CLKSRC (register CANCTL1). Does not change the other bits*/
  clk.clksrc = clk.clksrc << 6;
  IO_PORTS_8(adrCAN + CANCTL1) &= 0xBF;
  IO_PORTS_8(adrCAN + CANCTL1) |= clk.clksrc;
  /* Build the CANBTR0 register */
  IO_PORTS_8(adrCAN + CANBTR0) = 0x00;        /* Clear before changes */
  IO_PORTS_8(adrCAN + CANBTR0) = (clk.sjw << 6) | (clk.brp);
  /* Build the CANBTR1 register */
  IO_PORTS_8(adrCAN + CANBTR1) = 0x00;        /* Clear before changes */
  IO_PORTS_8(adrCAN + CANBTR1) = (clk.samp << 7) | (clk.tseg2 << 4) | 
    (clk.tseg1);  
  return 0;
}

/***************************************************************************/
char canInit1Filter(UNS16 adrCAN, UNS8 nFilter, UNS16 ar, UNS16 mr)
{
  if (! canTestInitMode(adrCAN)) {
    /* Error because not init mode */
    MSG_WAR(0X2602, "not in init mode ", 0);
    return 1;   
  }
  switch (nFilter) {
    case 0:
      nFilter = CANIDAR0; /* First  bank */
      break; 
    case 1:
      nFilter = CANIDAR2; /* First  bank */
      break;
    case 2:
      nFilter = CANIDAR4; /* Second bank */
      break;
    case 3:
      nFilter = CANIDAR6; /* Second bank */   
  }
  /* address of CANIDARx */
  IO_PORTS_8(adrCAN + nFilter)     = (UNS8) (ar >> 8);
  IO_PORTS_8(adrCAN + nFilter + 1) = (UNS8) (ar);
  IO_PORTS_8(adrCAN + nFilter + 4) = (UNS8) (mr >> 8);
  IO_PORTS_8(adrCAN + nFilter + 5) = (UNS8) (mr);
  return 0;
}

/***************************************************************************/
char canInitFilter(UNS16 adrCAN, canBusFilterInit fi)
{
if (! canTestInitMode(adrCAN)) {
    /* Error because not init mode */
    MSG_WAR(0X2603, "not in init mode ", 0);
    return 1;   
  }
  IO_PORTS_8(adrCAN + CANIDAC)  = fi.idam << 4;
  IO_PORTS_8(adrCAN + CANIDAR0) = fi.canidar0;
  IO_PORTS_8(adrCAN + CANIDMR0) = fi.canidmr0;
  IO_PORTS_8(adrCAN + CANIDAR1) = fi.canidar1;
  IO_PORTS_8(adrCAN + CANIDMR1) = fi.canidmr1;
  IO_PORTS_8(adrCAN + CANIDAR2) = fi.canidar2;
  IO_PORTS_8(adrCAN + CANIDMR2) = fi.canidmr2;
  IO_PORTS_8(adrCAN + CANIDAR3) = fi.canidar3;
  IO_PORTS_8(adrCAN + CANIDMR3) = fi.canidmr3;
  IO_PORTS_8(adrCAN + CANIDAR4) = fi.canidar4;
  IO_PORTS_8(adrCAN + CANIDMR4) = fi.canidmr4;
  IO_PORTS_8(adrCAN + CANIDAR5) = fi.canidar5;
  IO_PORTS_8(adrCAN + CANIDMR5) = fi.canidmr5;
  IO_PORTS_8(adrCAN + CANIDAR6) = fi.canidar6;
  IO_PORTS_8(adrCAN + CANIDMR6) = fi.canidmr6;
  IO_PORTS_8(adrCAN + CANIDAR7) = fi.canidar7;
  IO_PORTS_8(adrCAN + CANIDMR7) = fi.canidmr7;
  return 0;
}

/***************************************************************************/
char canInitMode(UNS16 adrCAN)
{
  IO_PORTS_8(adrCAN + CANCTL0) |= 0x01; /* Set the bit INITRQ */ 
  while (! canTestInitMode(adrCAN)) {
  }
  return 0;
}

/***************************************************************************/
char canInitModeQ(UNS16 adrCAN)
{
  IO_PORTS_8(adrCAN + CANCTL0) &= 0xFE; /* Clear the bit INITRQ */ 
  while (canTestInitMode(adrCAN)) {
  }
  return 0;
}

/***************************************************************************/
char canMsgTransmit(UNS16 adrCAN, Message msg)
{
  /* Remind : only CAN A msg implemented. ie id on 11 bits, not 29 */
  UNS8 cantflg;
  UNS8 i;
  /* Looking for a free buffer */
  cantflg = IO_PORTS_8(adrCAN + CANTFLG);
  if ( cantflg == 0) { /* all the TXEx are set */
    MSG_WAR(0X2604, "No buffer free. Msg to transmit is losted ", 0);
    return 1; /* No buffer free */
  }
  else{
    /* Selecting a buffer */
    IO_PORTS_8(adrCAN + CANTBSEL) = cantflg;
    /* We put ide = 0 because id is on 11 bits only */
     IO_PORTS_8(adrCAN + CANTRSID) = (UNS8)(msg.cob_id >> 3);
    IO_PORTS_8(adrCAN + CANTRSID + 1) = (UNS8)((msg.cob_id << 5)|
    (msg.rtr << 4));
   
    IO_PORTS_8(adrCAN + CANTRSLEN) = msg.len & 0X0F;
    /* For the priority, we put the highter bits of the cob_id */
    IO_PORTS_8(adrCAN + CANTRSPRI) = IO_PORTS_8(adrCAN + CANTRSID);
    for (i = 0 ; i < msg.len ; i++) {
      IO_PORTS_8(adrCAN + CANTRSDTA + i) = msg.data[i];
    } 
    /* Transmitting the message */ 
    cantflg = IO_PORTS_8(adrCAN + CANTBSEL);/* to know which buf is selected */
    IO_PORTS_8(adrCAN + CANTBSEL) = 0x00;  
    IO_PORTS_8(adrCAN + CANTFLG) = cantflg;  /* Ready to transmit ! */  
  
 }
  return 0;
}

/***************************************************************************/
char canSetInterrupt(UNS16 adrCAN) 
{
  IO_PORTS_8(adrCAN + CANRIER) = 0X01; /* Allow interruptions on receive */
  IO_PORTS_8(adrCAN + CANTIER) = 0X00; /* disallow  interruptions on transmit */
  return 0;  
}
/***************************************************************************/
char canSleepMode(UNS16 adrCAN)
{
  IO_PORTS_8(adrCAN + CANCTL0) &= 0xFB;   /* clr the bit WUPE  to avoid a wake-up*/ 
  IO_PORTS_8(adrCAN + CANCTL0) |= 0x02;   /* Set the bit SLPRQ. go to Sleep !*/ 

   // IO_PORTS_8(adrCAN + CANCTL1) |= 0x04;
   // IO_PORTS_8(adrCAN + CANCTL0) |= 0x02;   /* Set the bit SLPRQ */ 
   while ( ! canTestSleepMode(adrCAN)) {
     }

  return 0;
}

/***************************************************************************/
char canSleepModeQ(UNS16 adrCAN)
{
  if (canTestInitMode(adrCAN)) {
    /* Error because in init mode */
    MSG_WAR(0X2606, "not in init mode ", 0);
    return 1;   
  }
  IO_PORTS_8(adrCAN + CANCTL0) &= 0xFD;   /* clr the bit SLPRQ */ 
    while ( canTestSleepMode(adrCAN)) {
    }
  return 0;
}
 
/***************************************************************************/
char canSleepWupMode(UNS16 adrCAN)
{
  if (canTestInitMode(adrCAN)) {
    MSG_WAR(0X2607, "not in init mode ", 0);
    return 1;   
  }
  IO_PORTS_8(adrCAN + CANCTL0) |= 0x06;   /* Set the bits WUPE & SLPRQ */ 
  while ( ! canTestSleepMode(adrCAN)) {
  }
  return 0;
}

/***************************************************************************/
char canTestInitMode(UNS16 adrCAN)
{
  return IO_PORTS_8(adrCAN + CANCTL1) & 0x01; /* Test the bit INITAK */
}

/***************************************************************************/
char canTestSleepMode(UNS16 adrCAN)
{
  return IO_PORTS_8(adrCAN + CANCTL1) & 0x02; /* Test the bit SLPAK */
}

/***************************************************************************/
UNS8 canSend(UNS8 notused, Message *m)
{
  canMsgTransmit(CANOPEN_LINE_NUMBER_USED, *m);
  return 0;
}

UNS8 canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
{
    return 0;
}
/**************************************************************************/
UNS8 f_can_receive(UNS8 notused, Message *msgRcv)
{ 
  UNS8 i, j;

  switch (CANOPEN_LINE_NUMBER_USED) {
  case CAN0 : j = 0; break;
  case CAN1 : j = 1; break;
  case CAN2 : j = 2; break;
  case CAN3 : j = 3; break;
  case CAN4 : j = 4; break;
  }

  /* See if a message is pending in the stack */
  if (ptrMsgRcv[j].r == ptrMsgRcv[j].w)
    return 0x0; // No new message

  /* Increment the reading pointer of the stack */
  if (ptrMsgRcv[j].r == (MAX_STACK_MSG_RCV - 1)) 
     ptrMsgRcv[j].r = 0;
  else
    ptrMsgRcv[j].r ++;

  /* Store the message from the stack*/
  msgRcv->cob_id = stackMsgRcv[j][ptrMsgRcv[j].r].cob_id;
  msgRcv->len = stackMsgRcv[j][ptrMsgRcv[j].r].len;
  msgRcv->rtr = stackMsgRcv[j][ptrMsgRcv[j].r].rtr;
  for (i = 0 ; i < stackMsgRcv[j][ptrMsgRcv[j].r].len ; i++)
    msgRcv->data[i] = stackMsgRcv[j][ptrMsgRcv[j].r].data[i];
  return 0xFF;
}


/******************************************************************************
 ******************************* CAN INTERRUPT  *******************************/

void __attribute__((interrupt)) can0HdlTra (void)
{
 
}

void __attribute__((interrupt)) can0HdlRcv (void)
{ 
  UNS8 i;
  lock();
  IO_PORTS_8(PORTB) &= ~ 0x40; // led 6 port B : ON
  UNS8 NewPtrW; 
  /* We are obliged to save the message while the interruption is pending */
  /* Increment the writing stack pointer before writing the msg */
  if (ptrMsgRcv[0].w == (MAX_STACK_MSG_RCV - 1)) 
    NewPtrW = 0;
  else
    NewPtrW = ptrMsgRcv[0].w + 1;
  
  if (NewPtrW == ptrMsgRcv[0].r) {
    /* The stack is full. The last msg received before this one is lost */
    MSG_WAR(0X1620, "Stack for received msg is full", 0);
    //IO_PORTS_8(PORTB) &= ~0x40; // led  6         : ON (for debogue)
  }
  else
    ptrMsgRcv[0].w = NewPtrW;
  
  /* Store the message */
  stackMsgRcv[0][ptrMsgRcv[0].w].cob_id = IO_PORTS_16(CAN0 + CANRCVID) >> 5;
  stackMsgRcv[0][ptrMsgRcv[0].w].len = IO_PORTS_8(CAN0 + CANRCVLEN) & 0x0F;
  stackMsgRcv[0][ptrMsgRcv[0].w].rtr = (IO_PORTS_8(CAN0 + CANRCVID + 1) >> 4) & 0x01;
  for (i = 0 ; i < stackMsgRcv[0][ptrMsgRcv[0].w].len ; i++)
    stackMsgRcv[0][ptrMsgRcv[0].w].data[i] = IO_PORTS_8(CAN0 + CANRCVDTA + i);
  
  // The message is stored , so
  // we can now release the receive foreground buffer
  // and acknowledge the interruption
  IO_PORTS_8(CAN0 + CANRFLG) |= 0x01;
  // Not very usefull
  IO_PORTS_8(CAN0 + CANCTL0) |= 0x80;
  IO_PORTS_8(PORTB) |= 0x40; // led 6 port B : OFF
  unlock();
}

void __attribute__((interrupt)) can0HdlWup (void)
{

}

void __attribute__((interrupt)) can0HdlErr (void)
{

}

void __attribute__((interrupt)) can1HdlTra (void)
{
 
}

void __attribute__((interrupt)) can1HdlRcv (void)
{
  UNS8 i;
  lock();
  UNS8 NewPtrW; 
  /* We are obliged to save the message while the interruption is pending */
  /* Increment the writing stack pointer before writing the msg */
  if (ptrMsgRcv[1].w == (MAX_STACK_MSG_RCV - 1)) 
    NewPtrW = 0;
  else
    NewPtrW = ptrMsgRcv[1].w + 1;
  
  if (NewPtrW == ptrMsgRcv[1].r) {
    /* The stack is full. The last msg received before this one is lost */
    MSG_WAR(0X2620, "Stack for received msg is full", 0);
  }
  else
    ptrMsgRcv[1].w = NewPtrW;
  
  /* Store the message */
  stackMsgRcv[1][ptrMsgRcv[1].w].cob_id = IO_PORTS_16(CAN1 + CANRCVID) >> 5;
  stackMsgRcv[1][ptrMsgRcv[1].w].len = IO_PORTS_8(CAN1 + CANRCVLEN) & 0x0F;
  stackMsgRcv[0][ptrMsgRcv[0].w].rtr = (IO_PORTS_8(CAN1 + CANRCVID + 1) >> 4) & 0x01;
  for (i = 0 ; i < stackMsgRcv[1][ptrMsgRcv[1].w].len ; i++)
    stackMsgRcv[1][ptrMsgRcv[1].w].data[i] = IO_PORTS_8(CAN1 + CANRCVDTA + i);
  
  // The message is stored , so
  // we can now release the receive foreground buffer
  // and acknowledge the interruption
  IO_PORTS_8(CAN1 + CANRFLG) |= 0x01;
  // Not very usefull
  IO_PORTS_8(CAN1 + CANCTL0) |= 0x80;
  unlock();
}

void __attribute__((interrupt)) can1HdlWup (void)
{

}

void __attribute__((interrupt)) can1HdlErr (void)
{

}

void __attribute__((interrupt)) can2HdlTra (void)
{

}

void __attribute__((interrupt)) can2HdlRcv (void)
{

}

void __attribute__((interrupt)) can2HdlWup (void)
{

}

void __attribute__((interrupt)) can2HdlErr (void)
{

}

void __attribute__((interrupt)) can3HdlTra (void)
{

}

void __attribute__((interrupt)) can3HdlRcv (void)
{

}

void __attribute__((interrupt)) can3HdlWup (void)
{

}

void __attribute__((interrupt)) can3HdlErr (void)
{

}

void __attribute__((interrupt)) can4HdlTra (void)
{

}

void __attribute__((interrupt)) can4HdlRcv (void)
{

}

void __attribute__((interrupt)) can4HdlWup (void)
{

}

void __attribute__((interrupt)) can4HdlErr (void)
{

}



