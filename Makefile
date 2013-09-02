# Cross toolchain variables
# If these are not in your path, you can make them absolute.
XT_PRG_PREFIX = mipsel-elf-
CC = $(XT_PRG_PREFIX)gcc
LD = $(XT_PRG_PREFIX)ld

# uMPS2-related paths

# Simplistic search for the umps2 install. prefix. If you have umps2
# installed on some weird locaiton, set UMPS2_DIR_PREFIX by hand.
ifneq ($(wildcard /usr/bin/umps2),)
    UMPS2_DIR_PREFIX = /usr
else
    UMPS2_DIR_PREFIX = /usr/local
endif

UMPS2_DATA_DIR = $(UMPS2_DIR_PREFIX)/share/umps2
UMPS2_INCLUDE_DIR = $(UMPS2_DIR_PREFIX)/include/umps2

# Compiler options
CFLAGS_LANG = -ffreestanding -ansi
CFLAGS_MIPS = -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic
CFLAGS = $(CFLAGS_LANG) $(CFLAGS_MIPS) -I$(UMPS2_INCLUDE_DIR) -Wall -O0

# Linker options
LDFLAGS = -G 0 -nostdlib -T $(UMPS2_DATA_DIR)/umpscore.ldscript

# Add the location of crt*.S to the search path
VPATH = $(UMPS2_DATA_DIR)

.PHONY : all clean

all: kernel.core.umps 

kernel.core.umps: kernel
	umps2-elf2umps -k kernel

kernel: scheduler.o interrupts.o handler.o main.o print.o utils.o pcb.o asl.o p2test.o 
	mipsel-linux-ld -T /usr/share/umps2/elf32ltsmip.h.umpscore.x /usr/lib/umps2/crtso.o main.o scheduler.o interrupts.o handler.o asl.o pcb.o print.o utils.o p2test.o /usr/lib/umps2/libumps.o -o kernel

main.o: main.c main.h scheduler.h interrupts.h pcb.e asl.e const13.h const13_customized.h uMPStypes.h types13.h libumps.h
	mipsel-linux-gcc -pedantic -Wall -c main.c

scheduler.o: scheduler.c scheduler.h const13.h
	mipsel-linux-gcc -pedantic -Wall -c scheduler.c

interrupts.o: interrupts.c interrupts.h 
	mipsel-linux-gcc -pedantic -Wall -c interrupts.c

handler.o: handler.c handler.h
	mipsel-linux-gcc -pedantic -Wall -c handler.c

p2test.o: p2test.c
	mipsel-linux-gcc -pedantic -Wall -c p2test.c

asl.o: asl.c asl.e utils.o types13.h uMPStypes.h const13.h const13_customized.h libumps.h
	mipsel-linux-gcc -pedantic -Wall -c asl.c

pcb.o: pcb.c pcb.e utils.o types13.h uMPStypes.h const13.h const13_customized.h libumps.h
	mipsel-linux-gcc -pedantic -Wall -c pcb.c

print.o: print.c const13.h uMPStypes.h types13.h libumps.h
	mipsel-linux-gcc -pedantic -Wall -c print.c

utils.o: utils.c utils.h
	mipsel-linux-gcc -pedantic -Wall -c utils.c

clean:
	rm -f *.o term*.umps kernel

distclean: clean
	-rm kernel.*.umps

# Pattern rule for assembly modules
%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<
