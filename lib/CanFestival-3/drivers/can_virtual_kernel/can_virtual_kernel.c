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

/*
	Virtual CAN driver.

	Modification of drivers/can_virtual/can_virtual.c:
	Rewritten from user-space library to kernel-space module with exported symbols
*/

#include <linux/module.h>
#include <linux/err.h>
#include <linux/kfifo.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/string.h>

#define NEED_PRINT_MESSAGE
#include "can_driver.h"
#include "def.h"

MODULE_LICENSE("GPL");

#define MAX_NB_CAN_PIPES 16

typedef struct {
  char used;
  struct kfifo *pipe;
  spinlock_t lock;
  wait_queue_head_t w_queue;
} CANPipe;

CANPipe canpipes[MAX_NB_CAN_PIPES] = {{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},{0,},};


/*********functions which permit to communicate with the board****************/
UNS8 canReceive_driver(CAN_HANDLE fd0, Message *m)
{
	long ret = wait_event_interruptible (((CANPipe*)fd0)->w_queue,
		kfifo_len (((CANPipe*)fd0)->pipe) >= sizeof(Message));

	// interrupted by signal
	if (ret == -ERESTARTSYS)
		return 2;

	if (kfifo_get (((CANPipe*)fd0)->pipe, (unsigned char *)m, sizeof(Message)) != sizeof(Message))
	{
		printk(KERN_NOTICE "can_virtual: error receiving data from kernel fifo.\n");
		return 1;
	}
	
	return 0;
}

UNS8 canSend_driver(CAN_HANDLE fd0, Message *m)
{
	int i;

	printk(KERN_INFO "%x->[ ", (CANPipe*)fd0 - &canpipes[0]); 
	for(i=0; i < MAX_NB_CAN_PIPES; i++)
	{
		if(canpipes[i].used && &canpipes[i] != (CANPipe*)fd0)
		{
			printk("%x ",i);	
		}
	}
	printk(" ]");
	print_message(m);
  
	// send to all readers, except myself
	for(i=0; i < MAX_NB_CAN_PIPES; i++)
	{
		if(canpipes[i].used && &canpipes[i] != (CANPipe*)fd0)
		{
			unsigned int len;
			len = kfifo_put (canpipes[i].pipe, (unsigned char *)m, sizeof(Message));
			if (len != sizeof(Message)) {
				printk(KERN_NOTICE "can_virtual: error sending data to kernel fifo.\n");
				return 1;
			}

			// wake up canReceive_driver()
			wake_up_interruptible (&canpipes[i].w_queue);
		}
	}

	return 0;
}

int TranslateBaudRate(char* optarg){
	if(!strcmp( optarg, "1M")) return (int)1000;
	if(!strcmp( optarg, "500K")) return (int)500;
	if(!strcmp( optarg, "250K")) return (int)250;
	if(!strcmp( optarg, "125K")) return (int)125;
	if(!strcmp( optarg, "100K")) return (int)100;
	if(!strcmp( optarg, "50K")) return (int)50;
	if(!strcmp( optarg, "20K")) return (int)20;
	if(!strcmp( optarg, "10K")) return (int)10;
	if(!strcmp( optarg, "5K")) return (int)5;
	if(!strcmp( optarg, "none")) return 0;
	return 0x0000;
}

UNS8 canChangeBaudRate_driver( CAN_HANDLE fd0, char* baud)
{
    printk("%x-> changing to baud rate %s[%d]\n", (CANPipe*)fd0 - &canpipes[0],baud,TranslateBaudRate(baud)); 
    return 0;
}


CAN_HANDLE canOpen_driver(s_BOARD *board)
{
	int i;
	for(i=0; i < MAX_NB_CAN_PIPES; i++)
	{
		if(!canpipes[i].used)
			break;
	}
	
	// initialize CANPipe structure
	init_waitqueue_head(&canpipes[i].w_queue);
	canpipes[i].lock = SPIN_LOCK_UNLOCKED;
	canpipes[i].pipe = kfifo_alloc (10*sizeof(Message), GFP_KERNEL, &canpipes[i].lock);

	if (i==MAX_NB_CAN_PIPES || (PTR_ERR(canpipes[i].pipe) == -ENOMEM))
	{
		printk (KERN_NOTICE "can_virtual: error initializing driver.\n");
		return (CAN_HANDLE)NULL;
	}
	
	canpipes[i].used = 1;	
	return (CAN_HANDLE) &canpipes[i];
}

int canClose_driver(CAN_HANDLE fd0)
{
	((CANPipe*)fd0)->used = 0;
	kfifo_free (((CANPipe*)fd0)->pipe);
	
	return 0;
}

EXPORT_SYMBOL(canOpen_driver);
EXPORT_SYMBOL(canClose_driver);
EXPORT_SYMBOL(canSend_driver);
EXPORT_SYMBOL(canReceive_driver);
EXPORT_SYMBOL(canChangeBaudRate_driver);
