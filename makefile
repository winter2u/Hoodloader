#
#             LUFA Library
#     Copyright (C) Dean Camera, 2014.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Run "make help" for target help.

# This work for the 16u2 only
MCU = atmega16u2
MCU_AVRDUDE = atmega16u2
MCU_DFU = atmega16u2

F_CPU = 16000000
BOARD  = USER


#Vendor ID from lufa       0x03EB
#Product ID created my own
#HOODLOADER                0x6E68
#HOODLOADER-Lite           0x4E48

#You can also use the native Arduino VID and PID
#VendorID  Arduino      0x2341
#ProductID Arduino Uno  0x0001
#ProductID Arduino Mega 0x0010

ARDUNIOHID_OPTS  = -DHOODLOADER_VID=0x03EB
ARDUNIOHID_OPTS += -DHOODLOADER_PID=0x6E68

#Version in BCD:
ARDUNIOHID_OPTS += -DHOODLOADER_V1=0
ARDUNIOHID_OPTS += -DHOODLOADER_V2=1
ARDUNIOHID_OPTS += -DHOODLOADER_V3=7


ARCH         = AVR8
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = Hoodloader
SRC          = $(TARGET).c Descriptors.c lufa-LUFA-140302/LUFA/Drivers/Peripheral/AVR8/Serial_AVR8.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH    = ./lufa-LUFA-140302/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -IConfig/ $(ARDUNIOHID_OPTS)
LD_FLAGS     =

# Default target
all:

# Include LUFA build script makefiles
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk
