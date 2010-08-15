/*
 * Fault handlers, stolen from FreeRTOS web (www.FreeRTOS.org)
 *
 * 2009-2010 Michal Demin
 *
 */
#include <stdint.h>

void MemManage_Handler(void)  __attribute__((naked));
void BusFault_Handler(void) __attribute__ ((naked));
void UsageFault_Handler(void) __attribute__ ((naked));

struct stack_t {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t psr;
};

void halt_faulty(struct stack_t *faulty_stack) {
	(void)faulty_stack;
	/* Inspect faulty_stack->pc to locate the offending instruction. */
	while(1);
}


void MemManage_Handler(void) {
		__asm volatile
		(
			" tst lr, #4                     \n"
			" ite eq                         \n"
			" mrseq r0, msp                  \n"
			" mrsne r0, psp                  \n"
			" ldr r1, [r0, #24]              \n"
			" ldr r2, mem_handler_const      \n"
			" bx r2                          \n"
			" mem_handler_const: .word halt_faulty\n"
		);
}

void BusFault_Handler(void) {
		__asm volatile
		(
			" tst lr, #4                     \n"
			" ite eq                         \n"
			" mrseq r0, msp                  \n"
			" mrsne r0, psp                  \n"
			" ldr r1, [r0, #24]              \n"
			" ldr r2, bus_handler_const      \n"
			" bx r2                          \n"
			" bus_handler_const: .word halt_faulty\n"
		);

}

void UsageFault_Handler(void) {
		__asm volatile
		(
			" tst lr, #4                     \n"
			" ite eq                         \n"
			" mrseq r0, msp                  \n"
			" mrsne r0, psp                  \n"
			" ldr r1, [r0, #24]              \n"
			" ldr r2, usage_handler_const     \n"
			" bx r2                          \n"
			" usage_handler_const: .word halt_faulty\n"
		);

}

