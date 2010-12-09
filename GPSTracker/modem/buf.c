#include "platform.h"
#include "stm32f10x.h"

#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"

#include "buf.h"

#define IDX(x) (count_in + index_in + (x))

uint8_t buffer_in[1024];
uint8_t sending_in = 0;
uint32_t count_in = 0;
uint32_t index_in = 0;

void Buf_PushByte(uint8_t b)
{
	buffer_in[IDX(0)] = b;

	count_in ++;
}

void Buf_PushWord(uint16_t w)
{
	buffer_in[IDX(0)] = w & 0xff;
	buffer_in[IDX(1)] = w >> 8;
	
	count_in += 2;	
}

// TODO: check buffer overflow ...
void Buf_PushString(uint8_t *p, uint8_t len)
{
	uint8_t i;

	for(i = 0; i < len; i++) {
		buffer_in[IDX(i)] = p[i];
	}

	count_in += len;
}

void Buf_Send()
{
	uint8_t cnt = 64;

	if ((count_in == 0) || (sending_in))
		return;

	sending_in = 1;

	if (count_in < 64)
		cnt = count_in;

	USB_SIL_Write(EP1_IN, buffer_in + index_in, count_in);
	SetEPTxValid(ENDP1);

	index_in += cnt;
	count_in -= cnt;

	if (count_in == 0)
		index_in = 0;
}

uint8_t Buf_Need(uint8_t len)
{
	if (VIRTUAL_COM_PORT_DATA_SIZE - count_in < len) {
		SetEPTxValid(ENDP1);
		return 0;
	}
	
	return 1;
}

