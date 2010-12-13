#ifndef RFM12_h_
#define RFM12_h_

enum {
	RFM_STATE_SLEEP,
	RFM_STATE_IDLE,
	RFM_STATE_SCAN,
	RFM_STATE_SNIFF,
	RFM_STATE_RX,
	RFM_STATE_TX,
}; 

#define RFM_BAND_433MHZ  1
#define RFM_BAND_868MHZ  2
#define RFM_BAND_915MHZ  3


#define RFM_RECV_MODE    0x82D9
#define RFM_XMIT_MODE    0x8239
#define RFM_IDLE_MODE    0x8209
#define RFM_SLEEP_MODE   0x8201
#define RFM_WAKEUP_MODE  0x8203

#define RFM_TX_REG       0xB800
#define RFM_RX_REG       0xB000
#define RFM_WAKEUP_REG   0xE000


void RFM_Init(void);
void RFM_SetDataRate(uint8_t r);
void RFM_SetTXDeviation(uint8_t d);
void RFM_SetRXWidth(uint8_t w);
void RFM_SetGroup(uint8_t g);
void RFM_SetBand(uint8_t b);
void RFM_SetFreq(uint16_t f);
void RFM_SetFIFO(uint8_t g);

void RFM_IdleMode();
void RFM_SniffMode(uint8_t a);
void RFM_ScanMode();
void RFM_RecvMode();

uint8_t RFM_Send(uint16_t id, uint8_t *data, uint8_t len);
uint8_t RFM_ReadFifo();
uint8_t RFM_IsIdle();

#endif

