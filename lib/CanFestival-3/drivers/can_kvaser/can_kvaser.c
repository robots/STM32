
/*
Copyright (C): Giuseppe Massimo BERTANI
gmbertani@users.sourceforge.net


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

/**
 * @file can_kvaser.c
 * @author GMB
 * @date 17/1/08
 *
 * This file is needed to interface Kvaser's CAN Leaf (USB-CAN adapter) 
 * and probably others Kvaser's products compatible with Kvaser's CANLIB,
 * to CANfestival open source CANopen stack.
 *
 * It was tested under Linux 2.6 with "Leaf Professional" and CANLIB 4.72 Beta (Oct 1,2007)
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

/* includes Kvaser's CANLIB header */
#include <canlib.h>

#include "can_driver.h"
 
    
/**
 * CAN_HANDLES must have value >=1 while CANLIB wants handles >= 0
 * so fd0 needs to be decremented before use.
 *
 */ 
UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
{
canStatus retval = canOK;
unsigned flags = 0;
unsigned long timeStamp;

    fd0--;

    /* checking for input message (blocking) */
    retval = canReadWait((int)fd0, (long*)&m->cob_id, &m->data, (unsigned*)&m->len, &flags, &timeStamp, -1);
    if (retval != canOK)
    {
  	    fprintf(stderr, "canReceive_driver (Kvaser) : canReadWait() error, cob_id=%08X, len=%u, flags=%08X, returned value = %d\n", 
                m->cob_id, m->len, flags, retval);
        canClose((int)fd0);
        return retval;
    }

    m->rtr = 0;
    if (flags & canMSG_RTR)
    {
        m->rtr = 1;
    }

    if (flags & canMSG_EXT)
    {
        /* TODO: is it correct to set this info in cob_id? */
        m->cob_id |= 0x20000000;
    }
    
    //fprintf(stderr, "canReceive_driver (Kvaser) : canReadWait() received packet, cob_id=%08X, len=%u, flags=%08X, timestamp=%d  returned value = %d\n", 
    //       m->cob_id, m->len, flags, timeStamp, retval);

    return retval;
}

/**
 *
 * CAN_HANDLES must have value >=1 while CANLIB wants handles >= 0
 * so fd0 needs to be decremented before use.
 *
 */ 
UNS8 canSend_driver(CAN_HANDLE fd0, Message *m)
{
canStatus retval = canOK;
unsigned flags = 0;

    fd0--;

    flags |= canMSG_STD;
    
    if (m->cob_id & 0x20000000)
    {
        /* TODO: is it correct to desume this info from cob_id? */
        flags |= canMSG_EXT;
    }

    if (m->cob_id & 0x40000000)
    {
        flags |= canMSG_RTR;
    }

    /*
     * TODO: when should I set canMSG_ERROR_FRAME?
     */ 
    
    retval = canWriteWait((int)fd0, m->cob_id, m->data, m->len, 10000, flags);
    if (retval != canOK)
    {
  	    fprintf(stderr, "canSend_driver (Kvaser) :  canWriteWait() error, cob_id=%08X, len=%u, flags=%08X, returned value = %d\n", 
                m->cob_id, m->len, flags, retval);
        canClose((int)fd0);
        return retval;
    }
   
    //fprintf(stderr, "canSend_driver (Kvaser) :  canWriteWait() send packet, cob_id=%08X, len=%u, flags=%08X, returned value = %d\n", 
    //            m->cob_id, m->len, flags, retval);
    return retval; 

}


/**
 * 
 */ 
int TranslateBaudRate(char* optarg)
{
	if(!strcmp( optarg, "1M")) 
        return BAUD_1M;
	if(!strcmp( optarg, "500K")) 
        return BAUD_500K;
	if(!strcmp( optarg, "250K")) 
        return BAUD_250K;
	if(!strcmp( optarg, "125K")) 
        return BAUD_125K;
	if(!strcmp( optarg, "100K")) 
        return BAUD_100K;
	if(!strcmp( optarg, "62K")) 
        return BAUD_62K;
	if(!strcmp( optarg, "50K")) 
        return BAUD_50K;

	return 0;
}

/**
 * Channels and their descriptors are numbered starting from zero.
 * So I need to increment by 1 the handle returned by CANLIB because
 * CANfestival CAN_HANDLEs with value zero are considered NOT VALID. 
 * 
 * The baud rate could be given directly as bit/s
 * or using one of the BAUD_* constants defined
 * in canlib.h
 */ 
CAN_HANDLE canOpen_driver(s_BOARD *board)
{
int fd0 = -1;
int channel, baud;
canStatus retval = canOK;

    fd0--;

    sscanf(board->busname, "%d", &channel);  

    baud = TranslateBaudRate(board->baudrate);

    if (baud == 0)
    {
        sscanf(board->baudrate, "%d", &baud);
    }

    fd0 = canOpenChannel(channel, canWANT_EXCLUSIVE|canWANT_EXTENDED);
    if (fd0 < 0)
    {
  	    fprintf(stderr, "canOpen_driver (Kvaser) : error opening channel %d\n", channel);
        return (CAN_HANDLE)(fd0+1);
    }
    canBusOff(fd0);

    /* values for tseg1, tseg2, sjw, noSamp and  syncmode
     * come from canlib example "simplewrite.c". The doc
     * says that default values will be taken if baud is one of
     * the BAUD_* values
     */ 
    retval = canSetBusParams(fd0, baud, 4, 3, 1, 1, 0);
    if (retval != canOK)
    {
  	    fprintf(stderr, "canOpen_driver (Kvaser) :  canSetBusParams() error, returned value = %d, baud=%d, \n", retval, baud);
        canClose((int)fd0);
        return (CAN_HANDLE)retval;
    }
    
    canSetBusOutputControl(fd0, canDRIVER_NORMAL);
    if (retval != canOK)
    {
  	    fprintf(stderr, "canOpen_driver (Kvaser) :  canSetBusOutputControl() error, returned value = %d\n", retval);
        canClose((int)fd0);
        return (CAN_HANDLE)retval;
    }



    
    retval = canBusOn(fd0);
    if (retval != canOK)
    {
  	    fprintf(stderr, "canOpen_driver (Kvaser) :  canBusOn() error, returned value = %d\n", retval);
        canClose((int)fd0);
        return (CAN_HANDLE)retval;
    }
    
    return (CAN_HANDLE)(fd0+1);

}

UNS8 canChangeBaudRate_driver( CAN_HANDLE fd0, char* baud)
{
int baudrate;
canStatus retval = canOK;


    baudrate = TranslateBaudRate(baud);
    if (baudrate == 0)
    {
        sscanf(baud, "%d", &baudrate);
    }


    fprintf(stderr, "%x-> changing to baud rate %s[%d]\n", (int)fd0, baud, baudrate); 
    
    canBusOff((int)fd0);

    /* values for tseg1, tseg2, sjw, noSamp and  syncmode
     * come from canlib example "simplewrite.c". The doc
     * says that default values will be taken if baud is one of
     * the BAUD_* values
     */ 
    retval = canSetBusParams((int)fd0, baudrate, 4, 3, 1, 1, 0);
    if (retval != canOK)
    {
  	    fprintf(stderr, "canChangeBaudRate_driver (Kvaser) :  canSetBusParams() error, returned value = %d, baud=%d, \n", retval, baud);
        canClose((int)fd0);
        return (UNS8)retval;
    }
    
    canSetBusOutputControl((int)fd0, canDRIVER_NORMAL);
    if (retval != canOK)
    {
  	    fprintf(stderr, "canChangeBaudRate_driver (Kvaser) :  canSetBusOutputControl() error, returned value = %d\n", retval);
        canClose((int)fd0);
        return (UNS8)retval;
    }
    
    retval = canBusOn((int)fd0);
    if (retval != canOK)
    {
  	    fprintf(stderr, "canChangeBaudRate_driver (Kvaser) :  canBusOn() error, returned value = %d\n", retval);
        canClose((int)fd0);
        return (UNS8)retval;
    }

    return 0;
}


/**
 *
 * CAN_HANDLES must have value >=1 while CANLIB wants handles >= 0
 * so fd0 needs to be decremented before use.
 */ 
int canClose_driver(CAN_HANDLE fd0)
{
canStatus retval = canOK;

    fd0--;
    
    retval = canBusOff((int)fd0);
    if (retval != canOK)
    {
  	    fprintf(stderr, "canClose_driver (Kvaser) :  canBusOff() error, returned value = %d\n", retval);
        canClose((int)fd0);
        return retval;
    }
    
    retval = canClose((int)fd0);
    if (retval != canOK)
    {
  	    fprintf(stderr, "canClose_driver (Kvaser) :  canClose() error, returned value = %d\n", retval);
        return retval;
    }
    
    return retval;
}

