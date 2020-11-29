# makefile for macrodevice

# version
VERSION_STRING = "\"2.0\""
DEFS += -D VERSION_STRING=$(VERSION_STRING)

# backend selection, comment out following lines to disable backends
use_backend_hidapi = true
use_backend_libevdev = true
use_backend_libusb = true
use_backend_serial = true
use_backend_xindicator = true

# variables
BIN_DIR = /usr/bin
DOC_DIR = /usr/share/doc
MAN_DIR = /usr/share/man/man1
CC = g++
CC_OPTIONS = -Wall -Wextra -O2 
LIBS = -llua -pthread

# change compiler options according to backend selection
ifdef use_backend_hidapi
	DEFS += -D USE_BACKEND_HIDAPI
	BACKEND_OBJ += macrodevice-hidapi.o
	LIBS += -lhidapi-libusb
endif
ifdef use_backend_libevdev
	DEFS += -D USE_BACKEND_LIBEVDEV
	BACKEND_OBJ += macrodevice-libevdev.o
	LIBS += -levdev
endif
ifdef use_backend_libusb
	DEFS += -D USE_BACKEND_LIBUSB
	BACKEND_OBJ += macrodevice-libusb.o
	LIBS += -lusb-1.0
endif
ifdef use_backend_serial
	DEFS += -D USE_BACKEND_SERIAL
	BACKEND_OBJ += macrodevice-serial.o
endif
ifdef use_backend_xindicator
	DEFS += -D USE_BACKEND_XINDICATOR
	BACKEND_OBJ += macrodevice-xindicator.o
	LIBS += -lX11
endif


build: macrodevice-lua.o helpers.o $(BACKEND_OBJ)
	$(CC) *.o -o macrodevice-lua $(LIBS)

clean:
	rm macrodevice-lua *.o

install:
	cp ./macrodevice-lua $(BIN_DIR)/macrodevice-lua
	cp ./macrodevice-lua.1 $(MAN_DIR)
	mkdir $(DOC_DIR)/macrodevice | true
	cp ./example.lua $(DOC_DIR)/macrodevice
	cp ./documentation/backends.md $(DOC_DIR)/macrodevice

uninstall:
	rm -f $(BIN_DIR)/macrodevice-lua
	rm -rf $(DOC_DIR)/macrodevice
	rm -f $(MAN_DIR)/macrodevice-lua.1

# individual .cpp files
macrodevice-lua.o:
	$(CC) -c macrodevice-lua.cpp $(CC_OPTIONS) $(DEFS)

helpers.o:
	$(CC) -c backends/helpers.cpp $(CC_OPTIONS)

macrodevice-hidapi.o:
	$(CC) -c backends/macrodevice-hidapi.cpp $(CC_OPTIONS)

macrodevice-libevdev.o:
	$(CC) -c backends/macrodevice-libevdev.cpp $(CC_OPTIONS)

macrodevice-libusb.o:
	$(CC) -c backends/macrodevice-libusb.cpp $(CC_OPTIONS)

macrodevice-serial.o:
	$(CC) -c backends/macrodevice-serial.cpp $(CC_OPTIONS)

macrodevice-xindicator.o:
	$(CC) -c backends/macrodevice-xindicator.cpp $(CC_OPTIONS)


