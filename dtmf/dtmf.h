#ifndef DTMF_h_
#define DTMF_h_

#include "stm32f10x.h"

#define DTMF_697     0x01
#define DTMF_770     0x02
#define DTMF_852     0x03
#define DTMF_941     0x04

#define DTMF_1209    0x10
#define DTMF_1336    0x20
#define DTMF_1477    0x30
#define DTMF_1633    0x40

#define DTMF_1       (DTMF_697 + DTMF_1209)
#define DTMF_2       (DTMF_697 + DTMF_1336)
#define DTMF_3       (DTMF_697 + DTMF_1477)
#define DTMF_A       (DTMF_697 + DTMF_1633)
#define DTMF_4       (DTMF_770 + DTMF_1209)
#define DTMF_5       (DTMF_770 + DTMF_1336)
#define DTMF_6       (DTMF_770 + DTMF_1477)
#define DTMF_B       (DTMF_770 + DTMF_1633)
#define DTMF_7       (DTMF_852 + DTMF_1209)
#define DTMF_8       (DTMF_852 + DTMF_1336)
#define DTMF_9       (DTMF_852 + DTMF_1477)
#define DTMF_C       (DTMF_852 + DTMF_1633)
#define DTMF_S       (DTMF_941 + DTMF_1209)
#define DTMF_0       (DTMF_941 + DTMF_1336)
#define DTMF_H       (DTMF_941 + DTMF_1477)
#define DTMF_D       (DTMF_941 + DTMF_1633)

void dtmf_init(void);
void dtmf_set(uint8_t code);
void dtmf_deinit(void);
uint8_t dtmf_sending(void);

#endif
