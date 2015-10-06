
all:
	(cd 6502Disassembler; cc -o dis6502 dis6502.c)
	cat ../src/centiped3/136001-307.d1 ../src/centiped3/136001-308.e1 ../src/centiped3/136001-309.fh1 ../src/centiped3/136001-310.j1 >rom
	./6502Disassembler/dis6502 rom >rom.dis.txt
