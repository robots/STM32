/*
 * nRF24 driver for STM32 family processors
 *
 * 2009-2010 Michal Demin
 *
 */
#ifndef NRF_H_
#define NRF_H_

/* commands */

/* reg operations */
#define R_REGISTER    0x00 // 1-5 bytes
#define W_REGISTER    0x20 // 1-5 bytes

#define R_RX_PAYLOAD  0x61 // 1-32 bytes
#define W_TX_PAYLOAD  0xa0 // 1-32 bytes

#define FLUSH_TX      0xe1 // 0 bytes
#define FLUSH_RX      0xe2 // 0 bytes

#define REUSE_TX_PL   0xe3 // 0 bytes

#define R_RX_PL_WID   0x60 // 1 byte

#define W_ACK_PAYLOAD 0xa8 // 1-32 bytes
#define W_TX_PAYLOAD_NOACK 0xb0 // 1-32 bytes

#define NRF_NOP       0xff

/* reg addresses */
#define CONFIG        0x00
#define EN_AA         0x01
#define EN_RXADDR     0x02
#define SETUP_AW      0x03
#define SETUP_RETR    0x04
#define RF_CH         0x05
#define RF_SETUP      0x06
#define STATUS        0x07
#define OBSERVE_TX    0x08
#define RPD           0x09
#define RX_ADDR_P0    0x0a
#define RX_ADDR_P1    0x0b
#define RX_ADDR_P2    0x0c
#define RX_ADDR_P3    0x0d
#define RX_ADDR_P4    0x0e
#define RX_ADDR_P5    0x0f
#define TX_ADDR       0x10
#define RX_PW_P0      0x11
#define RX_PW_P1      0x12
#define RX_PW_P2      0x13
#define RX_PW_P3      0x14
#define RX_PW_P4      0x15
#define RX_PW_P5      0x16
#define FIFO_STATUS   0x17

#define DYNPD         0x1c
#define FEATURE       0x1d


void nRF_Init(void);

#endif

