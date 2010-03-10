/*
 * Simple menu implementation
 *
 * 2009-2010 Michal Demin
 *
 */
#ifndef MENU_H_
#define MENU_H_

#include "stm32f10x.h"

#define MENU_MAX_DEPTH 3

#define MENU_ARRAY_SIZE(x) (sizeof(x)/sizeof(menuitem_t))

enum {
	MENU_LAST=0,
	MENU_LABEL,
	MENU_LABEL_INV,
	MENU_SUB,
	MENU_EXEC,
	MENU_VAL,
};

struct menuitem_t {
	const char *text;
	uint8_t type;
	void (*callback)(void);
	union {
		const struct menuitem_t* sub_menu;
		uint32_t *val;
		const uint32_t num; /* first label contains number of items :) */
	};
};



void Menu_Init();
void Menu_Redraw();
void Menu_Action();
void Menu_ValChanger(uint32_t *ptr);

#endif

