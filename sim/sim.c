#include <stdio.h>
#include "fcntl.h"

#include "fake6502.h"

unsigned char rom[8192];
unsigned char ram[1024];
unsigned char pfram[1024];

unsigned char mop[16];
unsigned char mov[16];
unsigned char moh[16];
unsigned char moc[16];

unsigned char pfcram[8];
unsigned char mocram[8];

unsigned char tb;
unsigned char coin;
unsigned char vblank;
unsigned char selftest;

/*
map
0000-03ff ram
0400-07bf pf ram
07c0-07cf mo v
07e0-07ef mo h
07f0-07ff mo c
0800      sw 1
0801      sw 2
0c00	  tb v
0c01	  coin
0c02	  tb h
0c03	  joystick
1000	pokey
1404 pf color ram
140c mo color ram
1600 ea addr & data
1680 ea control
1700 ea read data
1800 irq ack
1c00 l coin ctr
1c01 c coin ctr
1c02 r coin ctr
1c03 led1
1c04 led2
1c07 tb flip
2000 wd
2400 clear track ctrs
2000-3fff rom
*/

int reporting;
int report_ram;

void report(int force)
{
	extern uint16_t pc;
	extern uint8_t sp, a, x, y, status;

	if (pc == 0x3a19) { reporting = 1; report_ram = 1; }
	if (pc == 0x3c0a) reporting = 1;

	if (reporting || force)
		printf("pc %04x sp %04x a=%02x x=%02x\n", pc, sp, a, x);
}

unsigned char old_c00, old_c01;

uint8_t read6502(uint16_t address)
{
	unsigned char v;

	if (address >= 0x2000 && address <= 0x3fff) {
		if (0) printf("read %x -> %x\n", address, rom[address - 0x2000]);
		return rom[address - 0x2000];
	}

	if (address >= 0xe000 && address <= 0xffff) {
		if (0) printf("read %x -> %x\n", address, rom[address - 0xe000]);
		return rom[address - 0xe000];
	}

	if (address < 0x400) {
		if (report_ram) printf("read %x -> %x\n", address, ram[address]);
		return ram[address];
	}
	if (address >= 0x400 && address < 0x7c0) {
		return pfram[address - 0x400];
	}
	if (address >= 0x7c0 && address < 0x800) {
		if (((address >> 4) & 0xf) == 0xc)
			v = mop[address - 0x7c0];
		if (((address >> 4) & 0xf) == 0xd)
			v = mov[address - 0x7d0];
		if (((address >> 4) & 0xf) == 0xe)
			v = moh[address - 0x7e0];
		if (((address >> 4) & 0xf) == 0xf)
			v = moc[address - 0x7f0];
		printf("mo read %x -> %x\n", address, v);
		return v;
	}

	switch (address) {
	case 0xc00:
		v = tb | (vblank ? 0x40 : 0) | (selftest ? 0 : 0x20);
		if (v != old_c00) {
			printf("read %x -> %x\n", address, v);
			old_c00 = v;
		}
		return v;
	case 0xc01:
		v = coin;
		if (v != old_c01) {
			printf("read %x -> %x\n", address, v);
			old_c01 = v;
		}
		return v;
	case 0xc02:
		v = 0;
		return v;


	case 0x100a: /* pokey */
		return 0x10;

	case 0x1c00: /* l coin ctr */
		return 0x00;
	case 0x1c01: /* c coin ctr */
	case 0x1c02: /* r coin ctr */
		return 0;
	case 0x1800:
		return 0;
	}

	printf("read %x -> unmapped\n", address);
	report(1);
	return 0;
}

void write6502(uint16_t address, uint8_t value)
{
	if (address < 0x400) {
		ram[address] = value;
		return;
	}

	if (address >= 0x400 && address <= 0x7bf) {
		if (value != 0) {
			printf("pf write %x <- %x (%x)\n", address, value, (address-0x400)/4);
			fflush(stdout);
		}
		pfram[address - 0x400] = value;
		return;
	}

	if (address >= 0x7c0 && address < 0x800) {
		printf("mo write %x <- %x\n", address, value);
		if (((address >> 4) & 0xf) == 0xc)
			mop[address - 0x7c0] = value;
		if (((address >> 4) & 0xf) == 0xd)
			mov[address - 0x7d0] = value;
		if (((address >> 4) & 0xf) == 0xe)
			moh[address - 0x7e0] = value;
		if (((address >> 4) & 0xf) == 0xf)
			moc[address - 0x7f0] = value;
		return;
	}

	switch (address) {
		/* pokey */
	case 0x1000:
	case 0x1001:
	case 0x1002:
	case 0x1003:
	case 0x1005:
	case 0x1007:
	case 0x1008:
		return;
	case 0x100a:
		return;
	case 0x100f:
		return;
	case 0x1404:
	case 0x1405:
	case 0x1406:
	case 0x1407:
	case 0x1408:
	case 0x1409:
	case 0x140a:
	case 0x140b:
		printf("pf cram: write %x <- %x\n", address, value);
		pfcram[address - 0x1404] = value;
		return;
	case 0x140c:
	case 0x140d:
	case 0x140e:
	case 0x140f:
	case 0x1410:
	case 0x1411:
	case 0x1412:
	case 0x1413:
		printf("mo cram: write %x <- %x\n", address, value);
		mocram[address - 0x140c] = value;
		return;
	case 0x1c07:
		return;
	case 0x1800:
		return;
	case 0x2000:
		return;
	case 0x2400:
		/* clr tb ctrs */
		return;
	}

	printf("write %x <- %x unmapped\n", address, value);
	report(1);
}

int read_roms(void)
{
	int f, r;

	f = open("rom", O_RDONLY);
	if (f < 0)
		perror("rom");
	r = read(f, rom, 8192);
	if (r != 8192)
		return -1;
	close(f);
	return 0;
}

int vcount;
int ccount;
int icount;
unsigned long long t;

void advance(void)
{
	extern uint16_t pc;

	t++;
	vcount++;
	if (vcount > 90000 && vcount < 99999) {
		if (vblank == 0) printf("%lld: vblank on\n", t);
		vblank = 1;
	}
	if (vcount > 99999) {
		printf("%lld: vblank off\n", t);
		vcount = 0;
		vblank = 0;
	}

	ccount++;
	if (t > 2000000 && t < 2001000) {
		if (coin == 0) printf("%lld: pc=%x coin on\n", t, pc);
		coin = 0x01;
	}
	if (t > 2001000) {
		if (coin) printf("%lld: pc=%x coin off\n", t, pc);
		coin = 0x0;
	}


	icount++;
	if (icount > 100000) {
		icount = 0;
		printf("%lld: pc=%x irq\n", t, pc);
		irq6502();
	}
}

int setup(void)
{
	if (read_roms())
		return -1;
	return 0;
}

main()
{
	unsigned long cycles;

	setup();

	reset6502();
	cycles = 0;

	while (1) {
		report(0);
		step6502();
		cycles++;
//		if (cycles > 50000000)
//			break;
		advance();
	}
}
