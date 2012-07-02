#ifndef AD_H_
#define AD_H_

#include "platform.h"

typedef void (*ad_callback_t)(int16_t);

void AD_Init(ad_callback_t);
void AD_Start(void);
void AD_Stop(void);
void AD_SetTimer(uint16_t, uint16_t);

#endif

