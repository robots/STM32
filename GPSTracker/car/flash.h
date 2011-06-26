#ifndef FLASH_h_
#define FLASH_h_


#define FLSH_StoreAddress 0x0800FC00 // end of 64kB flash
//#define FLSH_StoreAddress8 0x08007C00 // end of 32kB flash

struct flash_config_t {
	uint16_t dev_id;
	uint8_t log_inteval;
	uint16_t adc_warn_level;
} __attribute__ ((packed));


uint8_t FLSH_UpdateStorage(uint8_t *buf, uint16_t len);
uint8_t FLSH_ReadStorage(uint8_t *buf, uint16_t len);

#endif

