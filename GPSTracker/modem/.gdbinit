
file FLASH_RUN/stm32_modem.elf
target remote localhost:3333

# set the target-specific info
set remote hardware-watchpoint-limit 4
set remote hardware-breakpoint-limit 6
monitor gdb_breakpoint_override hard

monitor soft_reset_halt

# enable debug mode
break main
continue
#set variable DEBUG_ON = 1
#del 1

