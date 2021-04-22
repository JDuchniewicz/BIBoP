# For now makefile will be in this file, we will change it later
PROJECT_DIR       = $(shell dirname $(shell dirname $(shell pwd)))

ARDMK_DIR         = $(PROJECT_DIR)/Arduino-Makefile
ARDUINO_PACKAGE_DIR = $(HOME)/.arduino15/packages
ARDUINO_DIR       = /usr/share/arduino
USER_LIB_PATH     :=  $(realpath $(PROJECT_DIR)/lib)
BOARD_TAG         = nano_33_iot
MONITOR_PORT      = /dev/ttyACM*
MONITOR_BAUDRATE  = 115200
AVR_TOOLS_DIR     = /usr
AVRDUDE           = /usr/bin/avrdude
CFLAGS_STD        = -std=gnu11
CXXFLAGS_STD      = -std=gnu++17
CXXFLAGS         += -pedantic -Wall -Wextra
LDFLAGS          += -fdiagnostics-color

### OBJDIR
### Don't touch this!
### This is were you put the binaries you just compile using 'make'
CURRENT_DIR       = $(shell basename $(CURDIR))
OBJDIR            = $(PROJECT_DIR)/build/$(CURRENT_DIR)/$(BOARD_TAG)

include $(ARDMK_DIR)/Sam.mk

