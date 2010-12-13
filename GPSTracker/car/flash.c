#include "platform.h"
#include "stm32f10x.h"

#include "flash.h"

uint8_t FLSH_UpdateStorage(uint8_t *buf, uint16_t len)
{
	uint32_t tmp;
	uint16_t i;

	CRC_ResetDR();
	FLASH_Unlock();
	FLASH_ErasePage(FLSH_StoreAddress);

	for (i = 0; i < (len + 3) / 4; i++ ) {
		memcpy(&tmp, &buf[4*i], 4);
		FLASH_ProgramWord(FLSH_StoreAddress + 4 + i * 4, tmp);
		CRC_CalcCRC(tmp);
	}
	tmp = CRC_ResetDR() & 0xffff0000 | len & 0x0000ffff;
	FLASH_ProgramWord(FLSH_StoreAddress, tmp);
	FLASH_Lock();

	return 0;
}

uint8_t FLSH_ReadStorage(uint8_t *buf, uint16_t len)
{
	uint8_t i;

	if ((*(volatile uint32_t *)FLSH_StoreAddress) & 0x0000ffff != len) {
		return 1;
	}

	CRC_ResetDR();
	for (i = 0; i < (len + 3) / 4; i++) {
		CRC_CalcCRC(FLSH_StoreAddress + 4 + i * 4);
	}

	if ((*(volatile uint32_t *)FLSH_StoreAddress) & 0xffff0000 != CRC_GetCRC()) {
		return 1;
	}

	memcpy(&buf, FLSH_StoreAddress, len);
	return 0;
}
