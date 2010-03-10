#ifndef PLATFORM_H_
#define PLATFORM_H_

#define LED_PIN            GPIO_Pin_12
#define LED_GPIO           GPIOC

#define LCD_E_PIN          GPIO_Pin_10
#define LCD_E_GPIO         GPIOC

#define LCD_RESET_PIN      GPIO_Pin_7
#define LCD_RESET_GPIO     GPIOC

#define LCD_DC_PIN         GPIO_Pin_2
#define LCD_DC_GPIO        GPIOB

#define TAMP_PIN           GPIO_Pin_13
#define TAMP_GPIO          GPIOC

#define WAKE_PIN           GPIO_Pin_0
#define WAKE_GPIO          GPIOA

#define JOY_CENTER_PIN     GPIO_Pin_6
#define JOY_CENTER_GPIO    GPIOC

#define NRF_CS_PIN         GPIO_Pin_4
#define NRF_CS_GPIO        GPIOA

#define NRF_CE_PIN         GPIO_Pin_8
#define NRF_CE_GPIO        GPIOC

#define NRF_IRQ_PIN        GPIO_Pin_9
#define NRF_IRQ_GPIO       GPIOC

#define ADIS_CS_PIN        GPIO_Pin_10
#define ADIS_CS_GPIO       GPIOB

#define ADIS_RESET_PIN     GPIO_Pin_3
#define ADIS_RESET_GPIO    GPIOA

#define LED_WRITE(x)       GPIO_WriteBit(LED_GPIO, LED_PIN, x)

#define LCD_DC_WRITE(x)    GPIO_WriteBit(LCD_DC_GPIO, LCD_DC_PIN, x)
#define LCD_E_WRITE(x)     GPIO_WriteBit(LCD_E_GPIO, LCD_E_PIN, x)
#define LCD_RESET_WRITE(x) GPIO_WriteBit(LCD_RESET_GPIO, LCD_RESET_PIN, x)

#define TAMP_READ          GPIO_ReadInputDataBit(TAMP_GPIO, TAMP_PIN)
#define WAKE_READ          GPIO_ReadInputDataBit(WAKE_GPIO, WAKE_PIN)
#define JOY_CENTER_READ    GPIO_ReadInputDataBit(JOY_CENTER_GPIO, JOY_CENTER_PIN)

#define NRF_CE_WRITE(x)    GPIO_WriteBit(NRF_CE_GPIO, NRF_CE_PIN, x)
#define NRF_CS_WRITE(x)    GPIO_WriteBit(NRF_CS_GPIO, NRF_CS_PIN, x)
#define NRF_IRQ_READ       GPIO_ReadInputDataBit(MRF_IRQ_GPIO, NRF_IRQ_PIN)

#define ADIS_CS_WRITE(x)   GPIO_WriteBit(ADIS_CS_GPIO, ADIS_CS_PIN, x)
#define ADIS_RESET_WRITE(x) GPIO_WriteBit(ADIS_RESET_GPIO, ADIS_RESET_PIN, x)
#endif

