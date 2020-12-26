CC=m68k-amigaos-gcc
CFLAGS=-mcrt=nix13 -DNDEBUG

all: FlashKickstart

FlashKickstart: FlashKickstart.c FlashDeviceSST39.c Helpers.c RomInfo.c

	$(CC) $(CFLAGS) $^ -o FK
