# Needs devkitARM + libctru (devkitPro) and the 3dstools (3dsxtool, smdhtool) on PATH.
ifeq ($(strip $(DEVKITARM)),)
$(error Set DEVKITARM, e.g. /opt/devkitpro/devkitARM)
endif
DEVKITPRO ?= /opt/devkitpro
CTRULIB   ?= $(DEVKITPRO)/libctru

TARGET := ProtariumConnect
ARCH   := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS := -g -Wall -O2 -mword-relocations -ffunction-sections $(ARCH) -D__3DS__ -Iinclude -I$(CTRULIB)/include
ifdef REGISTER_URL
CFLAGS += -DREGISTER_URL=\"$(REGISTER_URL)\"
endif
LDFLAGS := -specs=3dsx.specs -g $(ARCH) -L$(CTRULIB)/lib -lctru -lm

CC := $(DEVKITARM)/bin/arm-none-eabi-gcc

all: $(TARGET).3dsx

$(TARGET).elf: source/main.c include/root_ca.h
	$(CC) $(CFLAGS) source/main.c $(LDFLAGS) -o $@

$(TARGET).smdh: icon.png
	smdhtool --create "Protarium Connect" "Register your Pretendo NEX account with Protarium" "Protarium Network" icon.png $@

$(TARGET).3dsx: $(TARGET).elf $(TARGET).smdh
	3dsxtool $< $@ --smdh=$(TARGET).smdh

clean:
	rm -f $(TARGET).elf $(TARGET).smdh $(TARGET).3dsx

.PHONY: all clean
