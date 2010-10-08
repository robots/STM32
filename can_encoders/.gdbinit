

file FLASH_RUN/stm32_can_encoders.elf

target remote localhost:3333

monitor soft_reset_halt

break main
continue
set variable DEBUG_ON = 1

define can_enable
	set variable SYS_InterruptEnable = 0x0000
	set variable CANController_Control = 1
	call CANController_ControlHandle ()
	set variable CANController_Timing = { 0x03, 0x115 }
	call CANController_TimingHandle()
	set variable CANController_Control = 0
	call CANController_ControlHandle ()
	set variable CANController_TXBuffer = {0x3, 0x80, {12,23,43 , 12,23,23,23,23}}
	set variable SYS_InterruptEnable = 0x000f
end
document can_enable
	Sets CAN timing to 1Mbit
end

