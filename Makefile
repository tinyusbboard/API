#######################################################################################

# environment variable of the current user to locate the AVR8 toolchain
AVRPATH = $(AVR8TOOLCHAINBINDIR)

# the type of avr microcontroller
DEVICE = atmega8

# the frequency the microcontroller is clocked with
F_CPU = 16000000

#tinyUSBboard Rev3 and newer
DEFINES += -DVUSB_CFG_IOPORTNAME=D -DVUSB_CFG_DMINUS_BIT=7 -DVUSB_CFG_DPLUS_BIT=2
DEFINES += -DVUSB_CFG_HASNO_PULLUP_IOPORTNAME -DVUSB_CFG_HASNO_PULLUP_BIT
DEFINES += -DBUTTON_PROG=D,6

# where the firmware should be located within the flashmemory (in case you trampoline)
FLASHADDRESS = 0x0000

# (not important for compiling) - the device transporting firmware into the controller
PROGRAMMER = -c usbasp
#######################################################################################



# Tools:
ECHO=@echo
GCC=gcc
MAKE=@make
RM=@rm -f

CC=$(AVRPATH)avr-gcc
OBC=@$(AVRPATH)avr-objcopy
OBD=@$(AVRPATH)avr-objdump
SIZ=@$(AVRPATH)avr-size

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)


MYCFLAGS = -Wall -Os -g3 -ggdb -fno-move-loop-invariants -fno-tree-scev-cprop -fno-inline-small-functions -I. -Iusbdrv -mmcu=$(DEVICE) -DF_CPU=$(F_CPU) $(CFLAGS)   $(DEFINES)
MYLDFLAGS = -Wl,--relax,--gc-sections $(LDFLAGS)


FLASHPREAMBLEDEFINE = 
ifneq ($(FLASHADDRESS), 0)
ifneq ($(FLASHADDRESS), 00)
ifneq ($(FLASHADDRESS), 000)
ifneq ($(FLASHADDRESS), 0000)
ifneq ($(FLASHADDRESS), 00000)
ifneq ($(FLASHADDRESS), 0x0)
ifneq ($(FLASHADDRESS), 0x00)
ifneq ($(FLASHADDRESS), 0x000)
ifneq ($(FLASHADDRESS), 0x0000)
ifneq ($(FLASHADDRESS), 0x00000)
FLASHPREAMBLE = 0x0000
FLASHPREAMBLEDEFINE = -DFLASHPREAMBLE=$(FLASHPREAMBLE)
MYLDFLAGS += -Wl,--section-start=.text=$(FLASHADDRESS)
endif
endif
endif
endif
endif
endif
endif
endif
endif
endif




STDDEP	 = *.h
EXTRADEP = Makefile


all:  example.hex example.raw example.asm

apipage.o: apipage.c $(STDDEP) $(EXTRADEP)
	$(CC) apipage.c -c -o apipage.o $(MYCFLAGS)

example.o: example.c $(STDDEP) $(EXTRADEP)
	$(CC) example.c -c -o example.o $(MYCFLAGS)

example.elf: example.o apipage.o $(STDDEP) $(EXTRADEP)
	$(CC) example.o apipage.o -o example.elf $(MYCFLAGS) -Wl,-Map,example.map $(MYLDFLAGS)
	$(ECHO) "."
	$(SIZ) example.elf
	$(ECHO) "."

example.hex: example.elf $(STDDEP) $(EXTRADEP)
	$(OBC) -j .text -j .data -O ihex example.elf example.hex

example.raw: example.elf $(STDDEP) $(EXTRADEP)
	$(OBC) -j .text -j .data -O binary example.elf example.raw

example.asm: example.elf $(STDDEP) $(EXTRADEP)
	$(OBD) -d example.elf > example.asm

disasm: example.elf $(STDDEP) $(EXTRADEP)
	$(OBD) -d example.elf

flash: all
	$(ECHO) "."
	$(AVRDUDE) -U flash:w:example.hex:i
	$(ECHO) "."

deepclean: clean
	$(RM) *~

clean:
	$(RM) *.o
	$(RM) example.hex
	$(RM) example.raw
	$(RM) example.asm
	$(RM) example.map
	$(RM) example.elf
