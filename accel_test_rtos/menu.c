/*
 * Simple menu implementation
 *
 * 2009-2010 Michal Demin
 *
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include "platform.h"
#include "rtc_bkp.h"
#include "input.h"
#include "lcd.h"
#include "lisXXX.h"

#include "ADIS1625x.h"

#include "mems.h"
#include "mems_quaternion.h"
#include "mems_vector.h"
#include "mems_matrix.h"


#include "menu.h"

#define INPUT_PRIORITY     ( tskIDLE_PRIORITY + 1 )
static void taskMenu(void *pvParameters);

static void Menu_ContrastCallback();
static void Menu_AccelRunRaw();
static void Menu_AccelRunAligned();
static void Menu_AccelCalib();
static void Menu_AccelAlign();
static void Menu_GyroRun();

uint32_t blabla = 435;
uint32_t contrast = 0x45;

static const struct menuitem_t gyroMenu[] = {
	{
		.text = "     Gyro     ",
		.type = MENU_LABEL_INV,
		.callback = 0,
		{
			.num = 2,
		},
	},
	{
		.text = " Run Test     ",
		.type = MENU_EXEC,
		.callback = Menu_GyroRun,
	},
};

static const struct menuitem_t accelMenu[] = {
	{
		.text = "     Accel    ",
		.type = MENU_LABEL_INV,
		.callback = 0,
		{
			.num = 5,
		},
	},
	{
		.text = " Run Test RAW ",
		.type = MENU_EXEC,
		.callback = Menu_AccelRunRaw,
	},
	{
		.text = " Run Aligned  ",
		.type = MENU_EXEC,
		.callback = Menu_AccelRunAligned,
	},
	{
		.text = " Calibrate    ",
		.type = MENU_EXEC,
		.callback = Menu_AccelCalib,
	},
	{
		.text = " Alignment    ",
		.type = MENU_EXEC,
		.callback = Menu_AccelAlign,
	},
};

static const struct menuitem_t setupMenu[] = {
	{
		.text = "   Settings   ",
		.type = MENU_LABEL_INV,
		.callback = 0,
		{
			.num = 3,
		},
	},
	{
		.text = " change val",
		.type = MENU_VAL,
		.callback = 0,
		{
			.val = &blabla,
		},
	},
	{
		.text = " Contrast ",
		.type = MENU_VAL,
		.callback = Menu_ContrastCallback,
		{
			.val = &contrast,
		},
	},
};

static const struct menuitem_t mainMenu[] = {
	{
		.text = "  Main Menu   ",
		.type = MENU_LABEL_INV,
		.callback = 0,
		{
			.num = 6,
		},
	},
	{
		.text = " Gyro test    ",
		.type = MENU_SUB,
		.callback = 0,
		{
			.sub_menu = gyroMenu,
		},
	},
	{
		.text = " Accel test   ",
		.type = MENU_SUB,
		.callback = 0,
		{
			.sub_menu = accelMenu,
		},
	},
	{
		.text = " Settings     ",
		.type = MENU_SUB,
		.callback = 0,
		{
			.sub_menu = setupMenu,
		},
	},
	{
		.text = " change val",
		.type = MENU_VAL,
		.callback = 0,
		{
			.val = &blabla,
		},
	},
	{
		.text = " RTC_Init ",
		.type = MENU_EXEC,
		.callback = RTC_BKP_Init,
	}
};


const struct menuitem_t *menuItemStack[MENU_MAX_DEPTH];
uint8_t menuPosStack[MENU_MAX_DEPTH];
uint8_t menuDepth;

void Menu_Init() {
	xTaskCreate( taskMenu, ( signed char * ) "Menu", configMINIMAL_STACK_SIZE*4, NULL, INPUT_PRIORITY, NULL );
}

static void taskMenu(void *pvParameters) {
	const portTickType xDelay = 100 / portTICK_RATE_MS;
	uint16_t newPos;
	const struct menuitem_t *menu;
		
	vTaskDelay(500 / portTICK_RATE_MS);

	menuDepth = 0;
	menuPosStack[menuDepth] = 1;
	menuItemStack[menuDepth] = mainMenu;

	Menu_Redraw();
	while (1) {
		vTaskDelay(xDelay);

		uint8_t event = Input_GetEvent();
		switch (event) {
			case KEY_UP:
				if (menuPosStack[menuDepth]>1)
					menuPosStack[menuDepth]--;
				break;
			case KEY_DOWN:
				newPos = menuPosStack[menuDepth] + 1;
				menu = menuItemStack[menuDepth];
				if (newPos < menu[0].num)
					menuPosStack[menuDepth] = newPos;
				break;
			case KEY_CENTER:
				Menu_Action();
				break;
			case KEY_LEFT:
				if (menuDepth > 0)
					menuDepth--;
				break;
			default:
				break;
		}
		Menu_Redraw();
	}
}

void Menu_Redraw() {
	uint8_t menuPos;
	uint8_t drawPos;
	const struct menuitem_t *menu;

	uint8_t i;

	menuPos = menuPosStack[menuDepth];
	menu = menuItemStack[menuDepth];

	LCD_Clear();

	if (menuPos < 2) {
		drawPos = 0;
	} else {
		drawPos = menuPos - 2;
	}

	if (menuPos > menu[0].num - 3) {
		drawPos = (menu[0].num>5)?(menu[0].num - 5):0;
	}

	for (i = 0; i < 5; i++) {
		uint8_t inv;
		if(drawPos+i >= menu[0].num) break;
		inv = (menu[drawPos+i].type == MENU_LABEL_INV)?1:0;

		LCD_Str(i, menu[drawPos+i].text, inv);

		if (drawPos+i == menuPos) /* selected */
			LCD_Str(i, ">", inv);
	}

	if (menu == mainMenu) {
		LCD_Str ( 5, "      OK      ", 0 );
	} else {
		LCD_Str ( 5, "<-    OK      ", 0 );
	}

	LCD_Update();
}

void Menu_Action() {
	const struct menuitem_t *menu;
	uint8_t pos;

	pos = menuPosStack[menuDepth];
	menu = menuItemStack[menuDepth];

	switch (menu[pos].type) {
		case MENU_LABEL:
		case MENU_LABEL_INV:
			break; /* no action taken */
		case MENU_SUB:
			if (!(menuDepth+1 < MENU_MAX_DEPTH))
				break;
			menuDepth++;
			menuPosStack[menuDepth] = 1;
			menuItemStack[menuDepth] = menu[pos].sub_menu;
			break;
		case MENU_EXEC:
			menu[pos].callback();
			break;
		case MENU_VAL:
			Menu_ValChanger(menu[pos].val);
			if(menu[pos].callback != 0)
				menu[pos].callback();
			break;
	}
}

void Menu_ValChanger(uint32_t *ptr) {
	const portTickType xDelay = 100 / portTICK_RATE_MS;
	uint32_t old = *ptr;
	uint32_t new = old;
	char oldbuf[14];
	char newbuf[14];
	uint8_t done = 0;
	uint8_t update = 1;

	siprintf(oldbuf, "%u",(int) old);

	while (done==0) {
		if (update == 1) {
			update = 0;
			siprintf(newbuf, "%u", (int)new);
			LCD_Clear();
			LCD_Str(0, "  Value Set   ", 1);
			LCD_Str(1, " Old:         ", 0);
			LCD_Str(2, oldbuf , 0);
			LCD_Str(3, " New:         ", 0);
			LCD_Str(4, newbuf , 0);
			LCD_Str(5, "<-   Save     ", 0);
			LCD_Update();

		}
		vTaskDelay(xDelay);
		uint8_t event = Input_GetEvent();
		switch (event) {
			case KEY_UP:
				new++;
				update = 1; 
				break;
			case KEY_DOWN:
				new--;
				update = 1;
				break;
			case KEY_LEFT:
				done = 1;
				break;
			case KEY_CENTER:
				*ptr = new;
				done = 1;
				break;
			default:
				break;
		}

	}
	
}

static void Menu_ContrastCallback() {
	if(contrast > 0x50)
		contrast = 0x50;
	if(contrast < 0x40)
		contrast = 0x40;
	LCD_Contrast((uint8_t)contrast);
}

static void Menu_AccelRunRaw() {
	double dabl;
	const portTickType xDelay = 100 / portTICK_RATE_MS;
	char buf[14];
	struct lisxxx_data_t accelRaw;

	uint8_t done = 0;
	LisXXX_Init();


	while (done==0) {
		while((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5)) != Bit_SET);
		LisXXX_GetData(&accelRaw);

		dabl = 0.0;
		dabl = 1.0+2.5*8;
		dabl = dabl / 2;
		dabl = dabl - 1;

		LCD_Clear();
		LCD_Str(0, "     Accel    ", 1);
		//sprintf(buf, "x: %.02f", dabl);
		//LCD_Str(1, buf, 0);
		siprintf(buf, "x: %+06d", (int16_t)accelRaw.ax);
		LCD_Str(2, buf, 0);
		siprintf(buf, "y: %+06d", (int16_t)accelRaw.ay);
		LCD_Str(3, buf, 0);
		siprintf(buf, "z: %+06d", (int16_t)accelRaw.az);
		LCD_Str(4, buf, 0);
		LCD_Str(5, "<-            ", 0);
		LCD_Update();

		vTaskDelay(xDelay);
		uint8_t event = Input_GetEvent();
		switch (event) {
			case KEY_LEFT:
				done = 1;
				break;
			default:
				break;
		}

	}
}

static void Menu_AccelRunAligned() {
	const portTickType xDelay = 100 / portTICK_RATE_MS;
	char buf[14];
	struct lisxxx_data_t accelRaw;

	int8_t done = 0;
	
	struct vector_t vect_out;
	struct vector_t vect_meas;

	LisXXX_Init();


	while (done==0) {
		while((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5)) != Bit_SET);
		LisXXX_GetData(&accelRaw);

		vect_meas.coef[0] = (frac)accelRaw.ax;
		vect_meas.coef[1] = (frac)accelRaw.ay;
		vect_meas.coef[2] = (frac)accelRaw.az;

	//	vector_normalize(&vect_meas);
		matrix_mul(&vect_out, &mems_sensors.accel.align, &vect_meas);
		
		LCD_Clear();
		LCD_Str(0, "     Accel    ", 1);
		sprintf(buf, "x: %0.2f", vect_out.coef[0]);
		LCD_Str(2, buf, 0);
		sprintf(buf, "y: %0.2f", vect_out.coef[1]);
		LCD_Str(3, buf, 0);
		sprintf(buf, "z: %0.2f", vect_out.coef[2]);
		LCD_Str(4, buf, 0);
		LCD_Str(5, "<-            ", 0);
		LCD_Update();

		vTaskDelay(xDelay);
		uint8_t event = Input_GetEvent();
		switch (event) {
			case KEY_LEFT:
				done = 1; 
				break;
			default:
				break;
		}
	}
}

static void Menu_AccelAlign() {
	const portTickType xDelay = 100 / portTICK_RATE_MS;
	char buf[14];
	struct lisxxx_data_t accelRaw;

	int8_t iter = 25;
	
	struct vector_t vect_meas;

	LisXXX_Init();


	while (iter>0) {
		while((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5)) != Bit_SET);
		LisXXX_GetData(&accelRaw);

		vect_meas.coef[0] = (frac)accelRaw.ax;
		vect_meas.coef[1] = (frac)accelRaw.ay;
		vect_meas.coef[2] = (frac)accelRaw.az;

		vector_normalize(&vect_meas);
		
		LCD_Clear();
		LCD_Str(0, "     Accel    ", 1);
		siprintf(buf, "iter: %d", iter);
		LCD_Str(1, buf, 0);
		sprintf(buf, "x: %0.2f", vect_meas.coef[0]);
		LCD_Str(2, buf, 0);
		sprintf(buf, "y: %0.2f", vect_meas.coef[1]);
		LCD_Str(3, buf, 0);
		sprintf(buf, "z: %0.2f", vect_meas.coef[2]);
		LCD_Str(4, buf, 0);
		LCD_Str(5, "<-            ", 0);
		LCD_Update();

		vTaskDelay(xDelay);
		uint8_t event = Input_GetEvent();
		switch (event) {
			case KEY_LEFT:
				iter = -1; 
				break;
			default:
				break;
		}

		iter --;
	}

	// test for "cancelation"
	if (iter == 0) {
		struct quaternion_t quat;
		struct vector_t vect_g = { .coef = { 0, 0, -mems_ONE } };
		
		// compute matrix
		quaternion_align(&quat, &vect_g, &vect_meas);
		quaternion_toRotMatrix(&quat, &mems_sensors.accel.align);
	}

}

static void Menu_AccelCalib() {

}

static void Menu_GyroRun() {
	const portTickType xDelay = 100 / portTICK_RATE_MS;
	char buf[14];
	uint16_t rate;
	uint16_t temp;
	uint16_t angle;
	uint16_t status;

	uint8_t done = 0;
	ADIS_Init();


	while (done==0) {
		status = ADIS_ReadReg(STATUS);
		rate = ADIS_ReadReg(GYRO_OUT) & ADIS_DATA;
		temp = ADIS_ReadReg(TEMP_OUT) & ADIS_DATA;
		angle = ADIS_ReadReg(ANGL_OUT)& ADIS_DATA;

		LCD_Clear();
		LCD_Str(0, "     Gyro     ", 1);
		siprintf(buf, "s: %x", status);
//		siprintf(buf, "r: %+06d", (int16_t)rate);
		LCD_Str(1, buf, 0);
		siprintf(buf, "r: %x", rate);
//		siprintf(buf, "r: %+06d", (int16_t)rate);
		LCD_Str(2, buf, 0);
//		siprintf(buf, "t: %+06d", (int16_t)temp);
		siprintf(buf, "t: %x", temp);
		LCD_Str(3, buf, 0);
//		siprintf(buf, "a: %+06d", (int16_t)angle);
		siprintf(buf, "a: %x", angle);
		LCD_Str(4, buf, 0);
		LCD_Str(5, "<-            ", 0);
		LCD_Update();

		vTaskDelay(xDelay);
		uint8_t event = Input_GetEvent();
		switch (event) {
			case KEY_LEFT:
				done = 1;
				break;
			default:
				break;
		}

	}

}
