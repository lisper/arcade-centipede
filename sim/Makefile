
all: rom sim

ROM_BASE = ../roms/src/centiped3/

ROMS = $(ROM_BASE)/136001-307.d1 $(ROM_BASE)/136001-308.e1 $(ROM_BASE)/136001-309.fh1 $(ROM_BASE)/136001-310.j1

rom: $(ROMS)
	cat $(ROMS) >rom

sim: fake6502.c sim.c
	cc -o sim -g fake6502.c sim.c
