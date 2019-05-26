CC=vc +kick13
CFLAGS=-I$(NDK_INC) -DNDEBUG

all: FlashKickstart

FlashKickstart: FlashKickstart.c FlashDeviceSST39.c Helpers.c

	$(CC) $(CFLAGS) $^ -lamiga -o FK
