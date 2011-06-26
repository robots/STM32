#ifndef ADC_h_
#define ADC_h_

extern uint16_t AD_Data;

enum {
	AD_DIS, // disable watchdog
	AD_LOW, // enable, interrupt on low
	AD_HIGH,// enable, interrupt on high
};

void ADC_init();
void ADC_watch(uint8_t a);
void ADC_start();

#endif

