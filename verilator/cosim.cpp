#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>

#include "Vcentipede_verilator.h"

extern "C" {
	void reset_6502(void);
	void get_6502_regs(int *pc, int *sp, int *sr, int *a, int *x, int *y);
	void set_6502_a(int a);
	void sim_6502(void);
	void set_6502_irq(int on);

static int has_init;
static int mismatch_count;

static Vcentipede_verilator *_top;

static int rtl_irq_event;
static unsigned int rtl_irq_pc;

static unsigned char ram[8192];
static unsigned char rom[8192];

int read_roms(void)
{
	int f, r;

	f = open("../sim/rom", O_RDONLY);
	if (f < 0)
		perror("rom");
	r = read(f, rom, 8192);
	if (r != 8192)
		return -1;
	close(f);
	return 0;
}

void cosim_6502(Vcentipede_verilator *top,
		unsigned long long rtl_time,
		int rtl_reset,
		unsigned int rtl_pc, unsigned int rtl_sp, unsigned int rtl_sr,
		unsigned int rtl_a, unsigned int rtl_x, unsigned int rtl_y)
{
	if (0) printf("cosim_6502; time %lld; rtl_reset=%d rtl_pc=%x\n",
		      rtl_time, rtl_reset, rtl_pc);

	if (!has_init) {
		has_init = 1;
		_top = top;
		read_roms();
		reset_6502();
		return;
	}

	if (rtl_reset == 0 && rtl_pc != 0) {
		int pc, sp, sr, a, x, y;
		int mismatch;

		sim_6502();
		get_6502_regs(&pc, &sp, &sr, &a, &x, &y);
		sr |= 0x20;

		if (0)
			printf("sim_pc=%04x sim_sp=%04x sim_sr=%02x; sim_a %02x sim_x %02x sim_y %02x\n",
			       pc, sp, sr, a, x, y);

		mismatch = 0;
		if (pc != rtl_pc)
			mismatch |= 0x01;
		if (sp != rtl_sp)
			mismatch |= 0x02;
		if (sr != rtl_sr)
			mismatch |= 0x04;
		if (a != rtl_a)
			mismatch |= 0x08;
		if (x != rtl_x)
			mismatch |= 0x08;
		if (y != rtl_y)
			mismatch |= 0x08;

		if (mismatch) {
			mismatch_count++;
			if (mismatch_count > 3) exit(0);
		} else
			mismatch_count = 0;

		if (mismatch) printf("\n");

		if (mismatch & 1)
			printf("cosim mismatch: %llu: pc rtl %x, sim %x\n", rtl_time, rtl_pc, pc);
		if (mismatch & 2)
			printf("cosim mismatch: %llu: sp rtl %x, sim %x\n", rtl_time, rtl_sp, sp);
		if (mismatch & 4)
			printf("cosim mismatch: %llu: sr rtl %x, sim %x\n", rtl_time, rtl_sr, sr);
		if (mismatch & 8)
			printf("cosim mismatch: %llu: a %x %x, x %x %x, y %x %x\n",
			       rtl_time, rtl_a, a, rtl_x, x, rtl_y, y);

		if (mismatch) {
			printf("sim: pc=%04x sp=%04x sr=%02x; a %02x x %02x y %02x\n",
			       pc, sp, sr, a, x, y);
			printf("rtl: pc=%04x sp=%04x sr=%02x; a %02x x %02x y %02x\n",
			       rtl_pc, rtl_sp, rtl_sr, rtl_a, rtl_x, rtl_y);
		}
		else {
			if (0) printf("ok : pc=%04x sp=%04x sr=%02x; a %02x x %02x y %02x\n",
			       rtl_pc, rtl_sp, rtl_sr, rtl_a, rtl_x, rtl_y);
		}

	}
}

void cosim_int_event(unsigned int pc, int on)
{
	if (on) {
		rtl_irq_pc = pc;
		rtl_irq_event++;
		//printf("sim: irq event %d\n", rtl_irq_event, pc);
		set_6502_irq(1);
	} else {
		rtl_irq_event = 0;
		set_6502_irq(0);
	}
}

void memwr(unsigned addr, int v, int PC, int totcycles)
{
	if (addr < 0x3ff) {
		if (0) printf("memwr ram %x <- %x\n", addr, v);
		ram[addr] = v;
		return;
	}

	printf("memwr ? %x <- %x\n", addr, v);
}

unsigned char memrd(unsigned addr, int PC, int totcycles)
{
	unsigned char v;

	if (addr < 0x3ff) {
		if (0) printf("memrd ram %x -> %x (pc %x)\n", addr, ram[addr], PC);
		return ram[addr];
	}

	if (addr >= 0x2000 && addr < 0x4000) {
		if (0) printf("memrd rom %x -> %x\n", addr, rom[addr-0x2000]);
		return rom[addr-0x2000];
	}

	if (addr >= 0xe000 && addr < 0x10000) {
		if (0) printf("memrd rom %x -> %x\n", addr, rom[addr-0xe000]);
		return rom[addr-0xe000];
	}

	switch (addr) {
	case 0x800:
		return _top->v__DOT__sw1;
	case 0x801:
		return _top->v__DOT__sw2;
	case 0xc00:
		v = _top->v__DOT__uut__DOT__vblankd ? 0x70 : 0x30;
		return v;
	case 0xc01:
		v = 0xff; /* need to grab last coin rd on rtl */
		return v;
	case 0x100a:
		v = _top->v__DOT__uut__DOT__last_pokey_rd;
		return v;
	}

	printf("memrd ? %x\n", addr);
	return 0;
}

unsigned int memrdwd(unsigned addr, int PC, int totcycles)
{
	return (memrd(addr,PC,totcycles) | (memrd(addr+1,PC,totcycles) << 8));
}

};
