#ifndef FLASH_h_
#define FLASH_h_


#define FLSH_StoreAddress 0x0800FC00
//#define FLSH_StoreAddress 0x08007C00

uint8_t FLSH_UpdateStorage(uint8_t *buf, uint16_t len);
uint8_t FLSH_ReadStorage(uint8_t *buf, uint16_t len);

#endif

