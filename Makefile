# Needs devkitARM and libctru (devkitPro). Build: make    Another server: make REGISTER_URL=https://host/path
ifeq ($(strip $(DEVKITARM)),)
$(error Set DEVKITARM, for example to /opt/devkitpro/devkitARM)
endif

DEVKITPRO ?= /opt/devkitpro
CTRULIB   ?= $(DEVKITPRO)/libctru
PATH      := $(DEVKITPRO)/tools/bin:$(DEVKITARM)/bin:$(PATH)

TARGET := ProtariumConnect
CC     := arm-none-eabi-gcc
ARCH   := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS  := -O2 -Wall -Wextra -ffunction-sections -mword-relocations $(ARCH) -D__3DS__ -I$(CTRULIB)/include
LDFLAGS := -specs=3dsx.specs $(ARCH) -L$(CTRULIB)/lib -lctru -lm
ifdef REGISTER_URL
CFLAGS += -DREGISTER_URL='"$(REGISTER_URL)"'
endif

SOURCES := $(wildcard source/*.c)
OBJECTS := $(SOURCES:source/%.c=build/%.o)

all: $(TARGET).3dsx

build/%.o: source/%.c $(wildcard source/*.h) | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

$(TARGET).elf: $(OBJECTS)
	$(CC) $^ $(LDFLAGS) -o $@

$(TARGET).smdh: icon.png
	smdhtool --create "Protarium Connect" "Links this console to Protarium" "Protarium Network" icon.png $@

$(TARGET).3dsx: $(TARGET).elf $(TARGET).smdh
	3dsxtool $< $@ --smdh=$(TARGET).smdh

clean:
	rm -rf build $(TARGET).elf $(TARGET).smdh $(TARGET).3dsx

.PHONY: all clean
