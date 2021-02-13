CC=m68k-amigaos-gcc
CFLAGS=-mcrt=nix13 -DNDEBUG -Wall -Ireqtools

all: FK FlashKickstart

FK: FlashKickstart.c FlashDeviceSST39.c Helpers.c RomInfo.c
	$(CC) $(CFLAGS) $^ -o FK

FlashKickstart: gui.c RomInfo.c Helpers.c FlashDeviceSST39.c reqtoolsstub.c reqtoolsglue.c libreqtools.a
	$(CC) $(CFLAGS) $^ -o FlashKickstart
