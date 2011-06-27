#include "stm32f10x.h"
#include "platform.h"

#include "commands.h"
#include "can.h"

#include <string.h>

typedef void (*reset_t)(void);
extern uint32_t _isr_vectorsflash_offs;

void main(void) __attribute__ ((noreturn));
static void bootloader();

#if ((HAVE_BUTTON) || (HAVE_LED))
GPIO_InitTypeDef GPIO_InitStructure = {
	.GPIO_Speed = GPIO_Speed_50MHz,
	.GPIO_Pin = GPIO_Pin_4,
	.GPIO_Mode = GPIO_Mode_IPU
};
#endif

#define BOOT_ID (0x700 | NODE_ID)
#define TX_ID   (0x180 | NODE_ID)
#define RX_ID   (0x200 | NODE_ID)

// should be enough for whole page + blabla
#define CMD_SIZE 2048
static uint8_t cmd[CMD_SIZE];
static uint16_t cmd_len;

// should be enough for whole page
#define RSP_SIZE CMD_SIZE
static uint8_t rsp[RSP_SIZE];
static uint16_t rsp_len;
static uint16_t rsp_idx;

const char *boot_id = STRING_ID;


void main(void)
{
#if (HAVE_BUTTON)
	RCC_APB2PeriphClockCmd(RCC_EARLY_APB2, ENABLE);

	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	AFIO->MAPR |= 0x01000000; 
#endif

#if (HAVE_MAGIC)
	if ((*(volatile uint32_t*)MAGIC_ADDR) != MAGIC_KEY) {
#endif
#if (HAVE_BUTTON)
		if ((BUTTON_GPIO->IDR & BUTTON_PIN) != BUTTON_EXPECT) {
#endif
			// Test user stack whether programmed correctly
			if (((*(volatile uint32_t*)APP_ADDR) & 0x2FFE0000 ) == 0x20000000) {
				reset_t app_reset;
				
				// Initialize stack pointer of user app
				__set_MSP(*(volatile uint32_t*) APP_ADDR);

				// load reset vector address
				app_reset = (reset_t) (*(volatile uint32_t*) (APP_ADDR + 4));

				// execute
				app_reset();
			}
#if (HAVE_BUTTON)
		}
#endif
#if (HAVE_MAGIC)
	}
	// reset magic word
	*(volatile uint32_t*)MAGIC_ADDR = 0xFFFFFFFF;
#endif

	// initialize clock
	SystemInit();

#if (HAVE_LED)
	// enable led periph
	RCC_APB2PeriphClockCmd(RCC_LED_APB2, ENABLE);

	// Configure LED pin
	GPIO_InitStructure.GPIO_Pin = LED_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(LED_GPIO, (GPIO_InitTypeDef *)&GPIO_InitStructure);

	// turn on led
	GPIO_WriteBit(LED_GPIO, LED_PIN, LED_ON);
#endif

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, (uint32_t)&_isr_vectorsflash_offs);

	bootloader();

	while (1) {
	}
}

uint8_t chksum(uint8_t *buf, uint16_t len)
{
	uint8_t sum = 0;
	uint16_t i;

	for (i = 0; i < len; i++) {
		sum -= buf[i];
	}

	return sum;
}

static void bootloader()
{
	struct can_message_t msg_rx;
	struct can_message_t msg_tx;

	uint32_t address;
	uint16_t size;
	uint8_t sum;
	uint8_t *p;


	int i;

	// initialize can
	can_init("1M");

	// clear filter store (this does not apply the change)
	can_filter_clear();

	// set filter to receive only our messages
	can_filter_addmask(RX_ID, 0x7FF, 0);

	// filters should have been added by Canfestival, let's apply changes
	can_filter_apply();

	msg_tx.flags = 1 | CAN_MSG_SIZE;
	msg_tx.id = BOOT_ID;
	msg_tx.data[0] = 0x00;

	// notify other party that we are alive
	can_send(&msg_tx);

	cmd_len = 0;

	rsp_len = 0;
	rsp_idx = 0;

	while (1) {
		// send if there is something in the buffer
		if (rsp_len > 0) {
			uint8_t len;

			len = (rsp_len - rsp_idx > 8) ? 8 : rsp_len - rsp_idx;

			msg_tx.id = TX_ID;
			msg_tx.flags = len | CAN_MSG_SIZE;
			memcpy(msg_tx.data, rsp + rsp_idx, len);

			// if message was sent 
			if (can_send(&msg_tx) != 0xff) {
				rsp_idx += len;
				if (rsp_idx >= rsp_len) {
					rsp_len = 0;
					rsp_idx = 0;
				}
			}
		}

		if (can_receive(&msg_rx)) {
			uint8_t len;

			// should not be necessaty as CAN hw does filtering for us
			if (msg_rx.id == RX_ID) {
				// recv pieces and put them in buffer
				len = msg_rx.flags & CAN_MSG_SIZE;
				memcpy(cmd + cmd_len, msg_rx.data, len);
				cmd_len += len;
			}
		}

		// parse cmd buffer
		switch (cmd[0]) {
			case CMD_ID:
				cmd_len = 0;
				memcpy(rsp, boot_id, sizeof(STRING_ID));
				rsp_len = sizeof(STRING_ID);
				break;

			case CMD_READPROG:
				// wait for the header
				if (cmd_len < 1+4+2) break;
				cmd_len = 0;

				address = cmd[1] | cmd[2] << 8 | cmd[3] << 16 | cmd[4] << 24;
				size = cmd[5] | cmd[6] << 8;

				// prepare answer
				rsp[0] = CMD_READPROG;
				memcpy(rsp + 2, (void *)address, size);
				rsp[1] = chksum(rsp + 2, size);
				rsp_len = 2 + size;

				break;

			case CMD_WRITEPROG:
				// wait for the header
				if (cmd_len < 1+4+2+1) break;

				address = cmd[1] | cmd[2] << 8 | cmd[3] << 16 | cmd[4] << 24;
				size = cmd[5] | cmd[6] << 8;
				sum = cmd[7];				

				// wait for the rest of data
				if (cmd_len < 1+4+2+1+size) break;
				cmd_len = 0;

				rsp_len = 2;
				rsp[0] = CMD_WRITEPROG;

				// test address on page boundary
				if ((address % PAGE_SIZE) || (address < APP_ADDR)) {
					rsp[1] = ERR_ADDR;
					break;
				}

				// test size
				if ((size > PAGE_SIZE) || ((size % 4) != 0)) {
					rsp[1] = ERR_SIZE;
					break;
				}

				if (chksum(&cmd[8], size) != sum) {
					// bad checksum
					rsp[1] = ERR_CHKSUM;
					break;
				}

				// program flash
				FLASH_Unlock();

				p = &cmd[1+4+2+1];
				for (i = 0; i < size; i += 4) {
					uint32_t word;

					word = p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
					if (FLASH_ProgramWord(address, word) != FLASH_COMPLETE) {
						rsp[1] = ERR_FLASH;
						break;
					}
					address += 4;
				}

				rsp[1] = ERR_OK;

				break;

			case CMD_ERASEPROG:
				// wait for the header
				if (cmd_len < 1+4) break;

				address = cmd[1] | cmd[2] << 8 | cmd[3] << 16 | cmd[4] << 24;

				cmd_len = 0;
				rsp_len = 2;
				rsp[0] = CMD_ERASEPROG;

				// test address on page boundary
				if ((address % PAGE_SIZE) || (address < APP_ADDR)) {
					rsp[1] = ERR_ADDR;
					break;
				}

				FLASH_Unlock();
				if (FLASH_ErasePage(address) != FLASH_COMPLETE) {
					rsp[1] = ERR_FLASH;
					break;
				}

				rsp[1] = ERR_OK;
				break;

			case CMD_RESET:
				NVIC_SystemReset();
				break;
	
			case CMD_CHKSUM:

			case CMD_READCONF:

			case CMD_WRITECONF:

			case CMD_WRITERAM:
			case CMD_READRAM:
			case CMD_JMP:
			default:
				// unknown command
				rsp_len = 2;
				rsp[0] = cmd[0];
				rsp[1] = ERR_BAD_CMD;
				cmd_len = 0;
				break;
		}
	}
}

