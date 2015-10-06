
CODE_ROMS_SRC = \
	../src/centiped3/136001-307.d1 \
	../src/centiped3/136001-308.e1 \
	../src/centiped3/136001-309.fh1 \
	../src/centiped3/136001-310.j1

PROM_SRC = ../src/centiped1/136001-113.p4

PF_ROM_SRC = \
	../src/centiped1/136001-111.f7 \
	../src/centiped1/136001-112.hj7

#	rom_j1.v rom_fh1.v rom_e1.v rom_s1.v \


all: \
	rom_code_case.v \
	rom_f7_case.v rom_hj7_case.v \
	prom_p4.v

prom_p4.v: dump2p
	./dump2p <$(PROM_SRC) >$@

rom_f7_case.v: dumppf
	./dumppf -c <../src/centiped1/136001-111.f7 >$@

rom_hj7_case.v: dumppf
	./dumppf -c <../src/centiped1/136001-112.hj7 >$@

rom_code_case.v: dumpcode
	./dumpcode -c $(CODE_ROMS_SRC) >$@

clean:
	rm -f *.v
	rm -f dump2p
