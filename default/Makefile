###############################################################################
# Makefile for the project DuckHunt
###############################################################################

## General Flags
PROJECT = DuckHunt
GAME = DuckHunt
MCU = atmega644
TARGET = $(GAME).elf
CC = avr-gcc
INFO=../gameinfo.properties
UZEBIN_DIR = ../../../bin

## Kernel settings
KERNEL_DIR = ../../../kernel

## Escape spaces in mixer path (due to custom sounds.inc)
EMPTY:=
SPACE:= $(EMPTY) $(EMPTY)
SPACE_ESC:= \\$(SPACE)
MIX_PATH:= $(realpath ../data/sounds.inc)
MIX_PATH_ESC:= $(subst $(SPACE),$(SPACE_ESC),$(MIX_PATH))

KERNEL_OPTIONS  = -DVIDEO_MODE=3 -DRESOLUTION_EXT=1 -DVRAM_TILES_H=32 -DSCREEN_TILES_V=25 -DFIRST_RENDER_LINE=36
KERNEL_OPTIONS += -DMAX_SPRITES=0 -DRAM_TILES_COUNT=42 -DINTRO_LOGO=0 -DTRANSLUCENT_COLOR=0xC7 -DTRUE_RANDOM_GEN=1
KERNEL_OPTIONS += -DRTLIST_ENABLE=0 -DSPRITES_AUTO_PROCESS=0 -DSPRITES_VSYNC_PROCESS=0 -DSPRITES_ENABLED=0
KERNEL_OPTIONS += -DMUSIC_ENGINE=STREAM -DSOUND_MIXER=1 -DSOUND_CHANNEL_5_ENABLE=1 -DMIXER_WAVES=\"$(MIX_PATH_ESC)\"

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -gdwarf-2 -std=gnu99 -DF_CPU=28636360UL -Os -fsigned-char -ffunction-sections -fno-toplevel-reorder
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d 
CFLAGS += $(KERNEL_OPTIONS)


## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-gdwarf2

## Linker flags
LDFLAGS = $(COMMON)
LDFLAGS += -Wl,-Map=$(GAME).map 
LDFLAGS += -Wl,-gc-sections 


## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings


## Objects that must be built in order to link
OBJECTS = uzeboxVideoEngineCore.o asmFunctions.o gui.o uzeboxCore.o uzeboxSoundEngine.o uzeboxSoundEngineCore.o uzeboxVideoEngine.o $(GAME).o 

## Objects explicitly added by the user
LINKONLYOBJECTS = 

## Include Directories
INCLUDES = -I"$(KERNEL_DIR)" 

## Resources
RESOURCES = ../data/title.inc ../data/tiles.inc ../data/sprites1.inc ../data/sprites2.inc
RESOURCES+= ../data/music-uncompressed/gameover.inc ../data/music-uncompressed/gotduck.inc
RESOURCES+= ../data/music-uncompressed/intro.inc ../data/music-uncompressed/lose.inc
RESOURCES+= ../data/music-uncompressed/perfect.inc ../data/music-uncompressed/roundclear.inc
RESOURCES+= ../data/music-uncompressed/title.inc ../data/music-compressed.inc

## Build
all: $(RESOURCES) $(TARGET) $(GAME).hex $(GAME).eep $(GAME).lss $(GAME).uze size

## Rebuild graphics resources
../data/title.inc: ../data/png/title.png ../data/title.xml
	$(UZEBIN_DIR)/gconvert ../data/title.xml

../data/tiles.inc: ../data/png/tiles.png ../data/tiles.xml
	$(UZEBIN_DIR)/gconvert ../data/tiles.xml

../data/sprites1.inc: ../data/png/sprites1.png ../data/sprites1.xml
	$(UZEBIN_DIR)/gconvert ../data/sprites1.xml

../data/sprites2.inc: ../data/png/sprites2-16.png ../data/sprites2.xml
	$(UZEBIN_DIR)/gconvert ../data/sprites2.xml

## Rebuild uncompressed music resources
../data/music-uncompressed/gameover.inc: ../data/midi/gameover.mid
	$(UZEBIN_DIR)/midiconv -f 120 ../data/midi/gameover.mid ../data/music-uncompressed/gameover.inc -vmus_game_over

../data/music-uncompressed/gotduck.inc: ../data/midi/gotduck.mid
	$(UZEBIN_DIR)/midiconv -f 120 ../data/midi/gotduck.mid ../data/music-uncompressed/gotduck.inc -vmus_got_duck

../data/music-uncompressed/intro.inc: ../data/midi/intro.mid
	$(UZEBIN_DIR)/midiconv -f 120 ../data/midi/intro.mid ../data/music-uncompressed/intro.inc -vmus_intro

../data/music-uncompressed/lose.inc: ../data/midi/lose.mid
	$(UZEBIN_DIR)/midiconv -f 120 ../data/midi/lose.mid ../data/music-uncompressed/lose.inc -vmus_lose

../data/music-uncompressed/perfect.inc: ../data/midi/perfect.mid
	$(UZEBIN_DIR)/midiconv -f 120 ../data/midi/perfect.mid ../data/music-uncompressed/perfect.inc -vmus_perfect

../data/music-uncompressed/roundclear.inc: ../data/midi/roundclear.mid
	$(UZEBIN_DIR)/midiconv -f 120 ../data/midi/roundclear.mid ../data/music-uncompressed/roundclear.inc -vmus_round_clear

../data/music-uncompressed/title.inc: ../data/midi/title.mid
	$(UZEBIN_DIR)/midiconv -f 120 ../data/midi/title.mid ../data/music-uncompressed/title.inc -vmus_title

MIDI_UNCOMPRESSED_LIST = ../data/music-uncompressed/gameover.inc ../data/music-uncompressed/gotduck.inc
MIDI_UNCOMPRESSED_LIST+= ../data/music-uncompressed/intro.inc ../data/music-uncompressed/lose.inc
MIDI_UNCOMPRESSED_LIST+= ../data/music-uncompressed/perfect.inc ../data/music-uncompressed/roundclear.inc
MIDI_UNCOMPRESSED_LIST+= ../data/music-uncompressed/title.inc

##Rebuild compressed music resources
../data/music-compressed.inc: $(MIDI_UNCOMPRESSED_LIST) ../data/music-compressed.inc
	$(UZEBIN_DIR)/mconvert ../data/music-compression.cfg

## Compile Kernel files
uzeboxVideoEngineCore.o: $(KERNEL_DIR)/uzeboxVideoEngineCore.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

uzeboxSoundEngineCore.o: $(KERNEL_DIR)/uzeboxSoundEngineCore.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

uzeboxCore.o: $(KERNEL_DIR)/uzeboxCore.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uzeboxSoundEngine.o: $(KERNEL_DIR)/uzeboxSoundEngine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

uzeboxVideoEngine.o: $(KERNEL_DIR)/uzeboxVideoEngine.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

gui.o: $(KERNEL_DIR)/gui.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  $<

## Compile game sources
asmFunctions.o: ../asmFunctions.s
	$(CC) $(INCLUDES) $(ASMFLAGS) -c  $<

$(GAME).o: ../DuckHunt.c
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

%.uze: $(TARGET)
	-$(UZEBIN_DIR)/packrom $(GAME).hex $@ $(INFO)

UNAME := $(shell sh -c 'uname -s 2>/dev/null || echo not')
AVRSIZEFLAGS := -A ${TARGET}
ifneq (,$(findstring MINGW,$(UNAME)))
AVRSIZEFLAGS := -C --mcu=${MCU} ${TARGET}
endif

size: ${TARGET}
	@echo
	@avr-size -C --mcu=${MCU} ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(GAME).* dep/* *.uze $(MIDI_UNCOMPRESSED_LIST)


## Other dependencies
-include $(shell mkdir dep 2>/dev/null) $(wildcard dep/*)

