/*
 * bxCAN driver for STM32 family processors
 *
 * 2010 Michal Demin
 *
 */

#ifndef CANCONTROLLER_H_
#define CANCONTROLLER_H_

#include "canfestival.h"

extern uint32_t CAN_Error;

struct can_timing_t {
	const char *baud;
	uint16_t brp; // brp[0:9]
	uint16_t ts; // res[15] lbkm[14] res[13:10] swj[9:8] res[7] ts2[6:4] ts1[3:0]
} __attribute__ ((packed));

#define CAN_MSG_SIZE  0x0F // DLC[0:3]
#define CAN_MSG_RTR   0x10 // RTR[4]
#define CAN_MSG_EID   0x20 // EID[5]
#define CAN_MSG_INV   0x40 // is message in-valid

struct can_message_t {
	uint8_t flags;
	uint32_t id;
	uint8_t data[8];
} __attribute__ ((packed));

uint8_t can_baudrate(char* baud);
void can_init(char *baud);
uint8_t can_receive(struct can_message_t *m);
uint8_t can_send(struct can_message_t *m);

void can_filter_clear();
void can_filter_addmask(uint16_t cobid, uint16_t cobid_mask, uint8_t prio);
void can_filter_apply();

#endif

