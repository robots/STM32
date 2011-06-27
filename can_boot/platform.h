/*
 * Platform specific file
 *
 * 2010 Michal Demin
 *
 */
#ifndef PLATFORM_H_
#define PLATFORM_H_

#define HAVE_BUTTON          1
#define HAVE_MAGIC           1
#define HAVE_LED             1

#define APP_ADDR             0x08002000
#define PAGE_SIZE            1024
#define STRING_ID            "BOOT"

// node ID
#define NODE_ID              127

// magic
#define MAGIC_ADDR           0x20002000
#define MAGIC_KEY            0xDEADBEEF

// button
#define BUTTON_PIN           GPIO_Pin_4
#define BUTTON_GPIO          GPIOB
#define BUTTON_EXPECT        0
#define RCC_EARLY_APB2       RCC_APB2Periph_GPIOB

// led
#define LED_GPIO             GPIOA
#define LED_PIN              GPIO_Pin_13
#define LED_ON               Bit_RESET
#define RCC_LED_APB2         RCC_APB2Periph_GPIOA


#endif

