SILENT = @

# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# OBJECTS ...... The object files created from your source files. This list is
#                usually the same as the list of source files with suffix ".o".
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected.
# FUSES ........ Parameters for avrdude to flash the fuses appropriately.

CLOCK      = 8000000
PROGRAMMER = -c avrispmkii -P usb

# # int. rc osc @ 8mhz [cksel=0010, sut=10]
# # do not divide clock by 8
# # bodlevel=111
# # spi enabled
# DEVICE = atmega328p
# FUSES  = -U lfuse:w:0xE2:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m

# int. rc osc @ 8mhz [cksel=0010, sut=10]
# do not divide clock by 8
# bodlevel=111
# spi enabled
DEVICE = atmega168
FUSES  = -U lfuse:w:0xE2:m -U hfuse:w:0xDF:m -U efuse:w:0xF9:m

## -- ok, that should be the last of the stuff generally defined by the user

C_SRC     = $(foreach dir,. src,$(wildcard $(dir)/*.c))
ALL_OBJS  = $(C_SRC:.c=.o)
DEP_FILES = $(ALL_OBJS:.o=.d)

-include $(DEP_FILES)

# compiler and binutils commands =============================================
CC = avr-gcc
CXX = avr-g++
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
AVRDUDE = avrdude 

# compiler options and *FLAGS ================================================
TARGET_ARCH = -mmcu=$(DEVICE)

CPPFLAGS = -DF_CPU=$(CLOCK) -Isrc
# LDFLAGS  = 
CFLAGS   = --std=c99 -Wall -Werror -fdiagnostics-show-option -Os -g
ARFLAGS  = rs
# add "v" for verbose

DEVICE_SPECIFIC_LIB = libtwi_$(DEVICE).a

# choose minimal printf implementation
# http://www.nongnu.org/avr-libc/user-manual/group__avr__stdio.html#gaa3b98c0d17b35642c0f3e4649092b9f1
# LDFLAGS += -Wl,-u,vfprintf -lprintf_min # … um, program size GROWS when I do that??

# and now for some rules =====================================================
.SUFFIXES: .elf .hex _size _dis _dump _flash

# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.
.elf.hex:
	@echo "creating $<"
	$(SILENT) $(OBJCOPY) -j .text -j .data -O ihex $< $@

# override built-in rule to generate dependency lines with "gcc -M …"
# default: $(COMPILE.c) $(OUTPUT_OPTION) $<
.c.o:
	@echo "compiling $<"
	$(SILENT) $(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) \
		-M -MF $(subst .o,.d,$@) -MT "$@ $(subst .o,.d,$@)" $<
	$(SILENT) $(COMPILE.c) $(OUTPUT_OPTION) $<
   
# just messing around here; runs the C preprocessor
.c.S:
	$(SILENT) $(COMPILE.c) -E $<

%.elf: %.o
	@echo "linking $@"
	$(SILENT) $(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.a:
	$(SILENT) $(AR) $(ARFLAGS) $@ $?

# Targets for code debugging and analysis

# show size
.elf_size:
	$(SILENT) $(SIZE) --mcu=$(DEVICE) --format=avr $<

# disassemble
.elf_dis:
	$(SILENT) $(OBJDUMP) -d $<

# http://www.nongnu.org/avr-libc/user-manual/group__demo__project.html#demo_project_obj
# requires "-g"
.elf_dump:
	$(SILENT) $(OBJDUMP) -h -S $<

.hex_flash:
	$(SILENT) $(AVRDUDE) $(PROGRAMMER) -p $(DEVICE) -U flash:w:$<:i

# symbolic targets:
.PHONY: all
all: device-specific-lib

.PHONY: fuse
fuse:
	$(SILENT) $(AVRDUDE) $(PROGRAMMER) -p $(DEVICE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
# .PHONY: install
#install: fuse main_flash 

.PHONY: clean clean_all clean_objs
clean: clean_all

clean_all:
	@echo "cleaning all"
	$(SILENT) rm -f *.hex *.elf lib*.a $(ALL_OBJS) $(DEP_FILES)

clean_objs:
	@echo "cleaning objects"
	$(SILENT) rm -f $(ALL_OBJS) $(DEP_FILES)

# file targets:
libtwi.a: src/twi_master.o

device-specific-lib: clean_objs libtwi.a
	@echo "renaming libtwi.a to $(DEVICE_SPECIFIC_LIB)"
	@mv libtwi.a $(DEVICE_SPECIFIC_LIB)
