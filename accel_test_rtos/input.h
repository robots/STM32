/*
 * Input driver
 *
 * 2009-2010 Michal Demin
 *
 */
#ifndef INPUT_H_
#define INPUT_H_

#include "stm32f10x.h"

#define UP_VALUE        960
#define DOWN_VALUE      190
#define LEFT_VALUE     1990
#define RIGHT_VALUE     470
#define DIVERSION        30

enum {
	KEY_NONE=0,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_CENTER,
	KEY_BACK
};

void Input_Init();
uint8_t Input_GetEvent();

#endif

