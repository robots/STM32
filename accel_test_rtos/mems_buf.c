/*
 * Ring buffer for CAN messages
 *
 * 2010 Michal Demin
 *
 */

#include "platform.h"
#include "stm32f10x.h"

#include "canbuf.h"

void CANBuf_Init(struct can_buffer_t *b) {
	uint16_t i;
	b->write = 0;
	b->read = 0;

	for (i = 0; i < CAN_BUFFER_SIZE; i++ ) {
		b->msgs[i].flags = CAN_MSG_INV;
	}
}


