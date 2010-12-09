#ifndef BUF_h_
#define BUF_h_

//extern uint8_t buffer_in[1024];
extern uint8_t sending_in;
//extern uint32_t count_in;
//extern uint32_t index_in;

void Buf_PushByte(uint8_t b);
void Buf_PushWord(uint16_t w);
void Buf_PushString(uint8_t *p, uint8_t len);

uint8_t Buf_Need(uint8_t len);

void Buf_Send();

#endif

