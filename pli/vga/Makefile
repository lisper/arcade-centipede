#

#INCS = -I../../cver/gplcver-2.12a.src/pli_incs
INCS = -I/files/code/cpus/caddr/cver/gplcver-2.12a.src/pli_incs

CFLAGS= -fPIC -Wall -g $(INCS) -D__CVER__
LFLAGS= -G -shared -export-dynamic -lSDL -lpthread

all: vga.so vga.vpi

vga.o: vga.c 
	$(CC) $(CFLAGS) -c vga.c

vga.so: vga.o Makefile
	$(LD) $(LFLAGS) vga.o -o vga.so

vga.vpi: vga.o Makefile
	$(LD) $(LFLAGS) vga.o -o vga.vpi

clean:
	rm -f *.o *.so *.vpi
