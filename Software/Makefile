CC=vc +kick13
CFLAGS=-I$(NDK_INC) -c99 -DNDEBUG -lamiga -lauto

all: FlashKickstart

FlashKickstart: FlashKickstart.c FlashDeviceSST39.c Helpers.c

	$(CC) $(CFLAGS) $^ -lamiga -o FK
