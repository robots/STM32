#ifndef MEMS_H_
#define MEMS_H_

#include "mems_pos.h"

extern struct mems_sensors_t mems_sensors;
extern struct position_t position;

extern uint16_t mems_go;

void Mems_Init();

#endif

