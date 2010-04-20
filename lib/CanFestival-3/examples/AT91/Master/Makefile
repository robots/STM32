###############################################################################
# Makefile for the project MasterAT91
###############################################################################

## General Flags
PROJECT = MasterAT91
MCU = arm7tdmi
TARGET = AT91
CC = arm-gcc.exe
SRC = ../../../src
DRV = ../../../drivers/AT91
INC = ../../../include

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -Os -fsigned-char -fpack-struct
#CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS +=  -Wl,-Map=$(PROJECT).map

## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Include Directories
INCLUDES = -I$(INC) -I$(INC)/AT91

## Objects that must be built in order to link
OBJECTS = 	$(DRV)/can_AT91.o\
		$(DRV)/timer_AT91.o\
		$(SRC)/dcf.o\
		$(SRC)/timer.o\
		$(SRC)/emcy.o\
		$(SRC)/lifegrd.o\
		$(SRC)/lss.o\
		$(SRC)/nmtMaster.o\
		$(SRC)/nmtSlave.o\
		$(SRC)/objacces.o\
		$(SRC)/pdo.o\
		$(SRC)/sdo.o\
		$(SRC)/states.o\
		$(SRC)/sync.o\
		ObjDict.o\
		interrupt_timer.o\
		main.o

## Build
all: $(PROJECT).elf $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss size

## Compile
%.o: %.c
#	@echo " "
	@echo "---------------------------------------------------------------------------"
	@echo "**Compiling $< -> $@"
#	@echo "*********************************************"
	$(CC) $(INCLUDES) $(CFLAGS) -c $<
#	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $< 


##Link
$(PROJECT).elf: $(OBJECTS)
#	@echo " "
	@echo "---------------------------------------------------------------------------"
	@echo "**Linking :  $@"
#	@echo "*********************************************"
	$(CC) $(LDFLAGS) $(LIBDIRS) $(LIBS) $(^F) -o $@

%.hex: $(PROJECT).elf
	arm-elf-objcopy.exe -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(PROJECT).elf
	-arm-elf-objcopy.exe $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(PROJECT).elf
	arm-elf-objdump.exe -h -S $< > $@

size: $(PROJECT).elf
	@echo
	@arm-elf-size -C --mcu=${MCU} $(PROJECT).elf

## Clean target
.PHONY: clean
clean:
	-rm -rf *.o $(PROJECT).elf dep/* $(PROJECT).hex $(PROJECT).eep $(PROJECT).lss $(PROJECT).map


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

