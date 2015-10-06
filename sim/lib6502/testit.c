#include <stdio.h>
#include <stdlib.h>

typedef unsigned char byte;
typedef unsigned short word;


unsigned char code[] = {
	0xea,
	0xea,
	0
};

void memwr(unsigned addr, int v, int PC, int totcycles)
{
}

byte memrd(unsigned addr, int PC, int totcycles)
{
	printf("memrd %x\n", addr);

	if (addr < 10) 
		return code[addr];
	return 0;
}

byte memrdwd(unsigned addr, int PC, int totcycles)
{
	return (memrd(addr,PC,totcycles) | (memrd(addr+1,PC,totcycles) << 8));
}

main()
{
	int i;
	int pc, sp, sr, a, x, y;

	reset_6502();
	for (i = 0; i < 4; i++) {
		get_6502_regs(&pc, &sp, &sr, &a, &x, &y);
		printf("pc=%04x sp=%04x sr=%02x; a %02x x %02x y %02x\n",
		       pc, sp, sr, a, x, y);
		sim_6502();
	}
	exit(0);
}
