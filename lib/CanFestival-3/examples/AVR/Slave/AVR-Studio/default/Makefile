###############################################################################
# Makefile for the project SlaveAVR
###############################################################################

## General Flags
PROJECT = SlaveAVR
MCU = at90can128
TARGET = SlaveAVR.elf
CC = avr-gcc.exe

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2                              -Os -fsigned-char -fpack-struct
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=SlaveAVR.map


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Include Directories
INCLUDES = -I"I:\Entwicklung\Firmware\CAN\CanFestival-3\examples\AVR\Slave\AVR-Studio\..\..\..\..\include" -I"I:\Entwicklung\Firmware\CAN\CanFestival-3\examples\AVR\Slave\AVR-Studio\..\..\..\..\include\AVR" -I"I:\Entwicklung\Firmware\CAN\CanFestival-3\examples\AVR\Slave\AVR-Studio\." 

## Objects that must be built in order to link
OBJECTS = can_AVR.o dcf.o timer.o emcy.o lifegrd.o lss.o nmtMaster.o nmtSlave.o objacces.o pdo.o sdo.o states.o sync.o timer_AVR.o ObjDict.o main.o ds401.o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Build
all: $(TARGET) SlaveAVR.hex SlaveAVR.eep SlaveAVR.lss size

## Compile
can_AVR.o: ../../../../../drivers/AVR/can_AVR.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

dcf.o: ../../../../../src/dcf.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

timer.o: ../../../../../src/timer.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

emcy.o: ../../../../../src/emcy.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

lifegrd.o: ../../../../../src/lifegrd.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

lss.o: ../../../../../src/lss.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

nmtMaster.o: ../../../../../src/nmtMaster.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

nmtSlave.o: ../../../../../src/nmtSlave.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

objacces.o: ../../../../../src/objacces.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

pdo.o: ../../../../../src/pdo.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

sdo.o: ../../../../../src/sdo.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

states.o: ../../../../../src/states.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

sync.o: ../../../../../src/sync.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

timer_AVR.o: ../../../../../drivers/AVR/timer_AVR.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ObjDict.o: ../../ObjDict.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

main.o: ../../main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

ds401.o: ../../ds401.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	-avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) SlaveAVR.elf dep/* SlaveAVR.hex SlaveAVR.eep SlaveAVR.lss SlaveAVR.map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

