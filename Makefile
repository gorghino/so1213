#main target
all: kernel.core.umps 

kernel.core.umps: kernel
	umps2-elf2umps -k kernel

kernel: p1test.o asl.o pcb.o utils.o
	mipsel-linux-ld -T /usr/local/share/umps2/elf32ltsmip.h.umpscore.x /usr/local/lib/umps2/crtso.o p1test.o asl.o pcb.o utils.o /usr/local/lib/umps2/libumps.o -o 		kernel

p1test.o: p1test.c const13.h uMPStypes.h types13.h asl.e pcb.e libumps.h
	mipsel-linux-gcc -pedantic -Wall -c p1test.c
 
asl.o: asl.c asl.e utils.o types13.h uMPStypes.h const13.h libumps.h
	mipsel-linux-gcc -pedantic -Wall -c asl.c

pcb.o: pcb.c pcb.e utils.o types13.h uMPStypes.h const13.h libumps.h
	mipsel-linux-gcc -pedantic -Wall -c pcb.c

utils.o: utils.c utils.h
	mipsel-linux-gcc -pedantic -Wall -c utils.c
###################################################################################
###################################################################################
	
clean:
	rm -f *.o term*.umps kernel


distclean: clean
	-rm kernel.*.umps
