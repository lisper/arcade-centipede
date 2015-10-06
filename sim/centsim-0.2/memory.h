/*
 * memory.h: memory and I/O functions for Atari game simulator
 *
 * Copyright 1991-1994 Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/memory.h,v 1.3 1994/08/12 05:23:28 eric Exp $
 */

extern int breakflag;
extern int flagrandom;
extern int force_random;
extern int force_val;

typedef unsigned char byte;
typedef unsigned short word;

typedef struct _elem {
    byte cell;
    byte tagr;
    byte tagw;
#ifdef MAGIC_PC
    byte magic;  /* flag indicating interrupt OK here */
#else
    byte pad;
#endif
} elem;

extern elem *mem;

/* types of access */
#define RD 1
#define WR 2

/* tags */
#define MEMORY   0 /* memory, no special processing */
#define ROMWRT   1 /* ROM write, just print a message */
#define IGNWRT   2 /* spurious write that we don't care about */
#define UNKNOWN  3 /* don't know what it is */

#define WDCLR	 4
#define INTACK	 5

#define POKEY1   6 /* Pokey */
#define POKEY2   7 /* Pokey */

#define OPTSW1         8
#define OPTSW2         9

#define EAROMCON      10
#define EAROMWR       11
#define EAROMRD       12

#define TRACKBALL_CLR   13

#define CENTIPEDE_TRACKBALL_HORIZ 14
#define CENTIPEDE_TRACKBALL_VERT  15
#define CENTIPEDE_SWITCH1         16
#define CENTIPEDE_JOYSTICK        17
#define CENTIPEDE_OUT1            18

#define MILLIPEDE_TRACKBALL_HORIZ 19
#define MILLIPEDE_TRACKBALL_VERT  20
#define MILLIPEDE_SWITCH1         21
#define MILLIPEDE_SWITCH2         22
#define MILLIPEDE_OUT1            23

#define BREAKTAG  0x80

#define memrd(addr,PC,cyc) (mem[addr].tagr?MEMRD(addr,PC,cyc):mem[addr].cell)

byte MEMRD(unsigned addr, int PC, int totcycles);

#define memrdwd(addr,PC,cyc) ((mem[addr].tagr||mem[(addr)+1].tagr) ? \
			      (MEMRD(addr,PC,cyc) | (MEMRD((addr)+1,PC,cyc) << 8)) : \
			      (mem[addr].cell|(mem[(addr)+1].cell<<8)))

//#define memwr(addr,val,PC,cyc) if (mem[addr].tagw) MEMWR(addr,val,PC,cyc); else mem[addr].cell = val
#define memwr(addr,val,PC,cyc) MEMWR(addr,val,PC,cyc);

void MEMWR(unsigned addr, int val, int PC, int totcycles);

typedef struct
{
  int left;
  int right;
  int up;
  int down;
  int fire;
} joy;

extern joy joystick [2];

extern byte optionreg1, optionreg2;
extern int self_test;

/* input switch counters */
extern int cslot_left, cslot_right, cslot_util;
extern int slam, start1, start2;

typedef struct
{
  char *name;
  unsigned addr;
  unsigned len;
  unsigned offset;
} rom_info;


typedef struct
{
  unsigned addr;
  unsigned len;
  int dir;
  int tag;
} tag_info;

void read_rom_image (char *fn, unsigned faddr, unsigned len, unsigned offset);
void tag_area (unsigned addr, unsigned len, int dir, int tag);
void setup_roms_and_tags (rom_info *rom_list, tag_info *tag_list);
void copy_rom (unsigned source, unsigned dest, unsigned len);

