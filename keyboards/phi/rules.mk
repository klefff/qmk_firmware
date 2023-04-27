# This file intentionally not left blank
RAW_ENABLE = yes 						# Enable RAW HID communication to PC
#CONSOLE_ENABLE = yes
Link_Time_Optimisation = yes			# Reduce size of firmware by optimizing at link time (probably not needed as this code was originally written for OLED)

EXTRAFLAGS += -flto						# Helps with firmware size reduction

# Encoder
ENCODER_ENABLE = yes
#ENCODER_MAP_ENABLE = yes

F_CPU = 16000000
ARCH = AVR8
F_USB = $(F_CPU)

SRC += spi_master.c

#OPT_DEFS += -DINTERRUPT_CONTROL_ENDPOINT
#OPT_DEFS += -DBOOTLOADER_SIZE=4096
