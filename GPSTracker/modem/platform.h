#ifndef PLATFORM_H_
#define PLATFORM_H_

#define LED_ON Bit_RESET
#define LED_OFF Bit_SET

#define LED_RED_PIN        GPIO_Pin_0
#define LED_RED_GPIO       GPIOB
#define LED_GREEN_PIN      GPIO_Pin_1
#define LED_GREEN_GPIO     GPIOB
#define LED_YELLOW_PIN     GPIO_Pin_2
#define LED_YELLOW_GPIO    GPIOB


#define RFM_CS_PIN         GPIO_Pin_4
#define RFM_CS_GPIO        GPIOA

#define RFM_IRQ_PIN        GPIO_Pin_1
#define RFM_IRQ_GPIO       GPIOA

#define RFM_RST_PIN        GPIO_Pin_2
#define RFM_RST_GPIO       GPIOA

#define RFM_INT_PIN        GPIO_Pin_3
#define RFM_INT_GPIO       GPIOA

#define USB_DISCONNECT_PIN GPIO_Pin_9
#define USB_DISCONNECT     GPIOA

#define LED_RED(x)         GPIO_WriteBit(LED_RED_GPIO, LED_RED_PIN, x)
#define LED_GREEN(x)       GPIO_WriteBit(LED_GREEN_GPIO, LED_GREEN_PIN, x)
#define LED_YELLOW(x)      GPIO_WriteBit(LED_YELLOW_GPIO, LED_YELLOW_PIN, x)

#define RFM_CS(x)          GPIO_WriteBit(RFM_CS_GPIO, RFM_CS_PIN, x)
#define RFM_RST(x)         GPIO_WriteBit(RFM_RST_GPIO, RFM_RST_PIN, x)
#define RFM_IRQ_READ()     GPIO_ReadInputDataBit(RFM_IRQ_GPIO, RFM_IRQ_PIN)

#endif

