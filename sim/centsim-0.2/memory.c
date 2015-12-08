/*
 * memory.c: memory and I/O functions for Atari game simulator
 *
 * Copyright 1991-1994 Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/memory.c,v 1.6 1994/08/20 04:53:11 eric Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "display.h"
#include "game.h"
#include "misc.h"
#include "sim6502.h"

int breakflag = 0;
int flagrandom = 0;
int force_random = 0;
int force_val = 0;

byte optionreg1=0xff;
byte optionreg2=0xff;

joy joystick [2];

int self_test = 0;

/* input switch counters */
int cslot_left = 0;
int cslot_right = 0;
int cslot_util = 0;

int slam = 0;
int start1 = 0;
int start2 = 0;

int debug_hw_read = 0;
int debug_hw_write = 0;

char *pokey_rregs[]={
"POT0", 
"POT1", 
"POT2", 
"POT3", 
"POT4", 
"POT5", 
"POT6", 
"POT7", 
"ALLPOT", 
"KBCODE", 
"RANDOM", 
"unused0xB", 
"unused0xC", 
"unused0xD", 
"IRQSTAT", 
"SKSTAT"
};

char *pokey_wregs[]={
"AUDF1", 
"AUDC1", 
"AUDF2", 
"AUDC2", 
"AUDF3", 
"AUDC3", 
"AUDF4", 
"AUDC4", 
"AUDCTL", 
"STIMER", 
"SKRES", 
"POTGO", 
"unused0xC", 
"SEROUT", 
"IRQEN", 
"SKCTLS" 
};

/*
 * This used to decrement the switch variable if it was non-zero, so that
 * they would automatically release.  This has been changed to increment
 * it if less than zero, so switches set by the debugger will release, but
 * to leave it alone if it is greater than zero, for keyboard handling.
 */
int check_switch_decr (int *sw)
{
  if ((*sw) < 0)
    {
      (*sw)++;
      if ((*sw) == 0)
	printf ("switch released\n");
    }
  return ((*sw) != 0);
}

elem *mem;

byte MEMRD(unsigned addr, int PC, int totcycles)
{
  register byte tag,result=0;

  if(!(tag=mem[addr].tagr)) 
    return(mem[addr].cell);
  
  if(tag & BREAKTAG)
    {
      breakflag = 1;
    }

  switch(tag & 0x3f) 
    {
    case MEMORY:
      result = mem[addr].cell;
      break;
    case EAROMRD:
      result = 0x3f;
      break;
    case OPTSW1:
      result = optionreg1;
      if (debug_hw_read) printf("read optsw1 %x\n", result);
      break;
    case OPTSW2:
      result = optionreg2;
      if (debug_hw_read) printf("read optsw2 %x\n", result);
      break;
    case POKEY1:
      if((addr & 0x0f) == 0x0a)
	{
	  if (mem [addr | 0xf].cell & 0x03)
	    result = (rand() >> 12) & 0xff;
	  if (flagrandom)
	    {
	      if (force_random)
		{
		  result = force_val;
		}
	      if (debug_hw_read) printf ("r0 %x @pc %x\n", result, PC);
	    }
	}
      else result = mem[addr].cell;
      break;
    case POKEY2:
      if((addr & 0x0f) == 0x0a)
	{
	  if (mem [addr | 0xf].cell & 0x03)
	    result = (rand() >> 12) & 0xff;
	  if (flagrandom)
		  if (debug_hw_read) printf ("r0 %x @pc %x\n", result, PC);
	}
      else result = mem[addr].cell;
      break;
    case CENTIPEDE_TRACKBALL_HORIZ:
      result = ((! self_test) << 5) | 
	(((totcycles & 0x00007000) == 0x00007000) << 6);
      if (debug_hw_read) printf("read tb h %x\n", result);
      break;
    case CENTIPEDE_TRACKBALL_VERT:
      result = 0x00;
      break;
    case CENTIPEDE_SWITCH1:
      result = ((cslot_right       == 0) << 7) |
	       ((cslot_util        == 0) << 6) |
	       ((cslot_left        == 0) << 5) |
	       ((slam              == 0) << 4) |
	       ((joystick [1].fire == 0) << 3) |
	       ((joystick [0].fire == 0) << 2) |
	       ((start2            == 0) << 1) |
	       ((start1            == 0) << 0);
      if (debug_hw_read) printf("read sw1 %x\n", result);
      break;
    case CENTIPEDE_JOYSTICK:
      result = ((joystick [0].right == 0) << 7) |
	       ((joystick [0].left  == 0) << 6) |
	       ((joystick [0].down  == 0) << 5) |
	       ((joystick [0].up    == 0) << 4) |
	       ((joystick [1].right == 0) << 3) |
	       ((joystick [1].left  == 0) << 2) |
	       ((joystick [1].down  == 0) << 1) |
	       ((joystick [1].up    == 0) << 0);
      if (debug_hw_read) printf("read js %x\n", result);
      break;
    case MILLIPEDE_TRACKBALL_HORIZ:
      result = (((totcycles & 0x00007000) == 0x00007000) << 6) |
	       ((start1             == 0) << 5) |
               ((joystick [0].fire  == 0) << 4);
      break;
    case MILLIPEDE_TRACKBALL_VERT:
      result = ((start2             == 0) << 5) |
               ((joystick [1].fire  == 0) << 4);
      break;
    case MILLIPEDE_SWITCH1:
      result = ((cslot_right        == 0) << 7) |
	       ((cslot_util         == 0) << 4) |
	       ((cslot_left         == 0) << 4) |
	       ((slam               == 0) << 4) |
	       ((joystick [0].up    == 0) << 3) |
	       ((joystick [0].down  == 0) << 2) |
	       ((joystick [0].left  == 0) << 1) |
	       ((joystick [0].right == 0) << 0);
      break;
    case MILLIPEDE_SWITCH2:
      result = 0xff;
      break;
    case UNKNOWN:
      breakflag = 1;
      printf("@%x Unknown rd addr %x data %02x tag %02x\n",
	     PC, addr,mem[addr].cell, mem[addr].tagr);
      result = 0xff;
      break;
    default:
      breakflag = 1;
      printf("@%x Why are we here rd addr %x data %02x tag %02x\n",
	     PC, addr,mem[addr].cell, mem[addr].tagr);
      result = 0xff;
      break;
    }

  if(tag & BREAKTAG)
    {
      printf ("@%x Breakpoint read %x, data %x\n", PC, addr, result);
    }

  return(result);
}


void MEMWR(unsigned addr, int val, int PC, int totcycles)
{
  register byte tag;

  if (debug_hw_write)
	  if (addr >= 0x400 && addr <= 0x800) {
		  printf("wr %04x <- %02x (%02x)\n", addr, val, (addr-0x400)/4);
	  }

  if(!(tag=mem[addr].tagw)) mem[addr].cell = val;
  else 
    {
      if(tag & BREAKTAG) 
	{
	  breakflag = 1;
	  printf ("@%x Breakpoint write %x, data %x\n", PC, addr, val);
	}

      switch(tag & 0x3f) 
	{
	case MEMORY:
	  mem[addr].cell = val;
	  break;
	case INTACK:
	  irq_cycle = totcycles + 6144;
	  break;
	case WDCLR:
	case EAROMCON:
	case EAROMWR:
	  /* none of these are implemented yet, but they're OK. */
	  break;
	case POKEY1:
	  mem[addr].cell = val;
	  break;
	case POKEY2:
	  mem[addr].cell = val;
	  break;
	case TRACKBALL_CLR:
	  break;  /* not yet implemented */
	case CENTIPEDE_OUT1:
	  break;  /* not yet implemented */
	case MILLIPEDE_OUT1:
	  break;  /* not yet implemented */
	case IGNWRT:
	  break;
	case ROMWRT:
	  printf("@%x ROM write addr %x val %x data %02x tag %02x\n",
		 PC, addr, val, mem[addr].cell, mem[addr].tagw);
	  break;
	case UNKNOWN:
	  printf("@%x Unknown wr addr %x val %x data %02x tag %02x\n",
		 PC, addr, val, mem[addr].cell, mem[addr].tagw);
	  breakflag = 1;
	  break;
	default:
	  printf("@%x Why are we here wr addr %x val %x data %02x tag %02x\n",
		 PC, addr, val, mem[addr].cell, mem[addr].tagw);
	  breakflag = 1;
	  break;
	}
    }
}


void read_rom_image (char *fn, unsigned faddr, unsigned len, unsigned offset)
{
  FILE *fp;
  byte core[4096];
  unsigned j;

#ifdef ROM_READ_DEBUG
  fprintf(stderr, "Reading %s at addr %x\n", fn ,faddr);
#endif
  fp = fopen(fn, "rb");
  if(!fp) 
    {
      fprintf(stderr, "Can't open %s\n", fn);
      exit(1);
    }
  if(fseek(fp, offset, 0) != 0)
    {
      fprintf(stderr, "Can't seek to %d in %s\n", offset, fn);
      exit(1);
    }
  if(fread(core,1,len,fp) != len)
    {
      fprintf(stderr, "Read of %s fails\n", fn);
      exit(1);
    }
  fclose (fp);
  for(j=0; j < len; j++) 
    {
      mem[faddr].cell = core[j];
      mem[faddr].tagr = 0;
      mem[faddr].tagw = ROMWRT;
      faddr++;
    }
}


void tag_area (unsigned addr, unsigned len, int dir, int tag)
{
  unsigned i;

  for (i = 0; i < len; i++)
    {
      if (dir & RD)
	mem [addr].tagr = tag;
      if (dir & WR)
	mem [addr].tagw = tag;
      addr++;
    }
}


void setup_roms_and_tags (rom_info *rom_list, tag_info *tag_list)
{
  while (rom_list->name != NULL)
    {
      read_rom_image (rom_list->name, rom_list->addr, rom_list->len, rom_list->offset);
      rom_list++;
    }
  while (tag_list->len != 0)
    {
      tag_area (tag_list->addr, tag_list->len, tag_list->dir, tag_list->tag);
      tag_list++;
    }
}


void copy_rom (unsigned source, unsigned dest, unsigned len)
{
  unsigned i;

  for(i = 0; i < len; i++)
    {
      mem[dest].cell = mem[source].cell;
      mem[dest].tagr = mem[source].tagr;
      mem[dest].tagw = mem[source].tagw;
      dest++;
      source++;
    }

}


