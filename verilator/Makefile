#
#
#

#V = verilator
#V = /usr/local/bin/verilator
V = /usr/local/src/verilator-3.876/bin/verilator

TOP = --top-module centipede_verilator
RTL = ../rtl
CPU = ../6502/bc
ROM = ../roms/v3
INC = +incdir+$(RTL) +incdir+$(CPU) +incdir+$(ROM)

DEFINE = +define+debug=1 +define+SIMULATION=1
LDFLAGS = "-L../../sim/lib6502 -lSDL -lpthread -l6502"
EXE = ./tmp/Vcentipede_verilator
OPT = -O2 
CFLAGS = "-O -DCOSIM=1 -I../../emu/famec"

V_SRC = \
	centipede_verilator.v \
	$(RTL)/centipede.v $(RTL)/p6502.v $(RTL)/pokey.v \
	$(RTL)/ram.v $(RTL)/rom.v \
	$(RTL)/color_ram.v $(RTL)/pf_ram.v  $(RTL)/pf_rom.v \
	$(RTL)/vprom.v $(RTL)/hs_ram.v

V_DEP = \
	$(CPU)/bc6502.v

C_SRC = \
	centipede_verilator.cpp vga.cpp cosim.cpp

VOUT = tmp/Vcentipede_verilator.cpp

all: $(EXE)

$(VOUT): $(V_SRC) $(V_DEP) Makefile
	$V -cc $(OPT) -LDFLAGS $(LDFLAGS) -exe --trace --Mdir ./tmp $(DEFINE) $(INC) $(TOP) -CFLAGS $(CFLAGS) $(V_SRC) $(C_SRC)

$(EXE): $(VOUT) $(C_SRC)
	(cd tmp; make -f Vcentipede_verilator.mk)

clean:
	rm -f tmp/*
