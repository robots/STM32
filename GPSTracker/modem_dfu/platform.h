#ifndef PLATFORM_H_
#define PLATFORM_H_

#define USB_DISCONNECT                  GPIOA
#define USB_DISCONNECT_PIN              GPIO_Pin_9

#define RCC_APB2Periph_GPIO_DISCONNECT  RCC_APB2Periph_GPIOB

#define LED_WRITE(x)       GPIO_WriteBit(LED_GPIO, LED_PIN, x)

#define LCD_RESET_WRITE(x) GPIO_WriteBit(LCD_RESET_GPIO, LCD_RESET_PIN, x)

#define JOY_CENTER_READ    GPIO_ReadInputDataBit(JOY_CENTER_GPIO, JOY_CENTER_PIN)

#endif

