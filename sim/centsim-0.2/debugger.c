/*
 * debugger.c: debugger for Atari game simulator
 *
 * Copyright 1991-1994 Hedley Rainnie and Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/debugger.c,v 1.4 1994/08/20 04:53:11 eric Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "debugger.h"
#include "sim6502.h"
#include "dis6502.h"
#include "game.h"
#include "display.h"
#include "misc.h"


static void dumprange (word addr1, word addr2)
{
  word addr;
  byte data;
  printf ("%x %x\n", addr1, addr2);
  breakflag = 0;
  for (; addr1 <= addr2; addr1 += 16)
    {
      printf ("%04x: ", addr1);
      for (addr = addr1; addr < (addr1 + 16); addr++)
	{
	  if (addr <= addr2)
	    {
	      data = memrd (addr, 0, 0);
	      if (breakflag)
		{
		  addr2 = addr - 1;
		  printf ("   ");
		}
	      else
		printf ("%02x ", data);
	    }
	  else
	    printf ("   ");
	}
      for (addr = addr1; addr < (addr1 + 16); addr++)
	{
	  if (addr <= addr2)
	    {
	      data = memrd (addr, 0, 0);
	      if (breakflag)
		{
		  addr2 = addr - 1;
		  printf (" ");
		}
	      else if ((data >= 0x20) && (data <= 0x7e))
		printf ("%c", data);
	      else
		printf (".");
	    }
	  else
	    printf (" ");
	}
      printf ("\n");
    }
}


static void dummemloc(word addr)
{
  byte val;
  val = memrd(addr, 0, 0);
  printf("mem[%04x]: %02x, tagr: %02x, tagw: %02x\n",addr,val,mem[addr].tagr, mem[addr].tagw);
}

static void prthelp()
{
  printf ("\n .... HELP MENU ....\n");
  printf (" c,g:    cont (run at PC)\n");
  printf (" s:      single step\n");
  printf (" t:      trace\n");
  printf (" r addr[,addr]: read memory loc\n");
  printf (" w addr val: write memory loc\n");
  printf (" b addr: set breakpoint\n");
  printf (" l:      list breakpoints\n");
  printf (" d addr: disassemble memory locs\n");
#ifdef FIFO
  printf (" f:      fifo dump\n");
#endif
  printf (" o r,val: set option register r to val\n");
  printf (" R:      register dump\n");
  printf (" q:      exit simulator\n");
}

struct hdr 
{
  unsigned short pc;
  unsigned short a;
  unsigned short x;
  unsigned short y;
  unsigned short sp;
  unsigned short flags;
  long totcycles;
  long irq_cycle;
  long icount;
};

static void dumpworld(s)
     char *s;
{
  FILE *fp;
  struct hdr h;
  byte tagr, tagw;
  byte val;
  long i;
  
  fp = fopen(s, "wb");
  if(!fp) 
    {
      printf("Cannot save to dumpfile %s\n", s);
      return;
    }
  h.pc = save_PC - 1;
  h.a = save_A;
  h.x = save_X;
  h.y = save_Y;
  h.sp = _SP;
  h.flags = save_flags;
  h.icount = icount;
  h.totcycles = save_totcycles;
  h.irq_cycle = irq_cycle;
  fwrite(&h, sizeof(h), 1, fp);
    
  for(i=0;i < 65536;i++) 
    {
      tagr = mem[i].tagr;
      tagw = mem[i].tagw;
      val = memrd(i, 0, 0);
      fwrite(&tagr, 1, 1, fp);
      fwrite(&tagw, 1, 1, fp);
      fwrite(&val, 1, 1, fp);
    }
  fclose(fp);
}

void reload(s)
     char *s;
{
  FILE *fp;
  struct hdr h;
  byte tagr, tagw;
  byte val;
  long i;
    
  fp = fopen(s, "rb");
  if(!fp) 
    {
      printf("Cannot read dumpfile %s\n", s);
      return;
    }
  fread(&h, sizeof(h), 1, fp);
  save_PC = h.pc;
  save_A = h.a;
  save_X = h.x;
  save_Y = h.y;
  _SP = h.sp;
  save_flags = h.flags;
  icount = h.icount;
  irq_cycle = h.irq_cycle;
  save_totcycles = h.totcycles;
    
  for(i=0;i < 65536;i++) 
    {
      fread(&tagr, 1, 1, fp);
      fread(&tagw, 1, 1, fp);
      fread(&val, 1, 1, fp);
      mem[i].tagr = tagr;
      mem[i].tagw = tagw;
      memwr(i, val, 0, 0);
    }
  fclose(fp);
}



static void formatdump(byte a, byte x, byte y, byte p, byte s)
{
  char flagstr[9];
  
  flagstr[0] = (p & N_BIT) ? 'N' : 'n';
  flagstr[1] = (p & V_BIT) ? 'V' : 'v';
  flagstr[2] = (p & B_BIT) ? 'B' : 'b';
  flagstr[3] = (p & D_BIT) ? 'D' : 'd';
  flagstr[4] = (p & I_BIT) ? 'I' : 'i';
  flagstr[5] = (p & Z_BIT) ? 'Z' : 'z';
  flagstr[6] = (p & C_BIT) ? 'C' : 'c';
  flagstr[7] = 0;
  printf("A:%02x X:%02x Y:%02x %s SP:1%02x", a, x, y, flagstr, s);
}


static void dumpregs(void)
{
    disasm_6502 (save_PC);
    formatdump(save_A, save_X, save_Y, save_flags, _SP);
    printf ("  cyc: %lu\n", save_totcycles);
}


static void xyzzy()
{
  printf ("Nothing happens here.\n");
}


static void debugger_handle_inputs (char *s)
{
  int player = 0;
  int newstate = 1;
  while (*s != '\0')
    {
      switch (*s)
	{
	case ' ':
	  break;
	case '1':
	  player = 0;
	  break;
	case '2':
	  player = 1;
	  break;
	case '-':
	  newstate = 0;
	  break;
	case '+':
	  newstate = 1;
	  break;
#if 0
	case 'l':
	case 'L':
	  switches [player].left = newstate;
	  break;
	case 'r':
	case 'R':
	  switches [player].right = newstate;
	  break;
	case 'f':
	case 'F':
	  switches [player].fire = newstate;
	  break;
	case 't':
	case 'T':
	  switches [player].thrust = newstate;
	  break;
	case 'h':
	case 'H':
	  switches [player].hyper = newstate;
	  break;
	case 's':
	case 'S':
	  switches [player].shield = newstate;
	  break;
	case 'a':
	case 'A':
	  switches [player].abort = newstate;
	  break;
#endif
	default:
	  printf ("unknown input '%c'\n", *s);
	  break;
	}
      s++;
    }
}


/* ***************** debugger ***************** */
/* ***************** debugger ***************** */
/* ***************** debugger ***************** */


void dodebugger(int type)
{
  int i, j;
  int c;
  word addr, addr2;
  byte val;
  int runflag;
  int t1,t2;
  char c1;
  char *ip;
  int rw_mode;
#ifdef FIFO
  unsigned short pos;
#endif
  int n;
  char inbuf [300];

  runflag = 0;

  if (type != STEP)
    traceflag = 0;

  breakflag = 0;

#ifdef COUNT_INTERRUPTS
  printf ("Interrupts: %d\n", int_count);
#endif

  switch(type) 
    {
    case INTBREAK:
      printf("User break @PC %x\n",save_PC);
      break;
    case ERRORBRK:
      printf("Error @PC %x\n",save_PC);
      break;
    case BREAKPT:
      printf("breakpoint @PC %x\n",save_PC);
      break;
    case BREAK:
      printf("break instruction! @PC %x\n",save_PC);
      break;
    case STEP:
      break;
    default:
      printf("badbrkpt\n");
      break;
    }

  dumpregs();

  while((! traceflag) && (!runflag)) 
    {
      printf("> ");
      gets(inbuf);
      c = inbuf[0];
      switch(c) 
	{
	case 'b':
	  ip = inbuf + 1;
	  if (*ip == 'r')
	    {
	      rw_mode = RD;
	      ip++;
	    }
	  else if (*ip == 'w')
	    {
	      rw_mode = WR;
	      ip++;
	    }
	  else
	    rw_mode = RD | WR;
	  sscanf(ip,"%x",&t1);
	  addr = t1;
	  if (rw_mode & RD)
	    mem[addr].tagr ^= BREAKTAG;
	  if (rw_mode & WR)
	    mem[addr].tagw ^= BREAKTAG;
	  dummemloc(addr);
	  break;
	case 'l':
	  for(i=0;i < 65536;i++) 
	    {
	      if(((mem [i].tagr & BREAKTAG) == BREAKTAG) ||
		 ((mem [i].tagw & BREAKTAG) == BREAKTAG))
		{
		  printf ("@%x", i);
		  if (mem [i].tagr & BREAKTAG)
		    printf (" RD");
		  if (mem [i].tagw & BREAKTAG)
		    printf (" RD");
		  printf ("\n");
		}
	    }
	  break;
	case 'd': /* dis memory */
	  sscanf(inbuf+1,"%x",&t1);
	  addr = t1;
	  for (i=0; i < 16; i++)
	    {
	      addr += disasm_6502 (addr);
	      printf("\n");
	    }
	  break;
	case '$':
	  c1 = 'U';
	  t2 = -1;
	  sscanf(inbuf+1, "%c,%d", &c1, &t2);
	  switch (c1)
	    {
	    case 'L':
	    case 'l':
	      if (t2 == -1)
		cslot_left = -100;
	      else
		cslot_left = -t2;
	      break;
	    case 'R':
	    case 'r':
	      if (t2 == -1)
		cslot_right = -100;
	      else
		cslot_right = -t2;
	      break;
	    case 'U':
	    case 'u':
	      if (t2 == -1)
		cslot_util = -100;
	      else
		cslot_util = -t2;
	      break;
	    case 'S':
	    case 's':
	      if (t2 == -1)
		slam = -100;
	      else
		slam = -t2;
	      break;
	    case '1':
	      if (t2 == -1)
		start1 = -100;
	      else
		start1 = -t2;
	      break;
	    case '2':
	      if (t2 == -2)
		start2 = -100;
	      else
		start2 = -t2;
	      break;
	    default:
	      printf ("Syntax is \"$s,v\"\n");
	      printf ("s is L, R, U, for left, right, utility coin slots\n");
	      printf ("s is 1 or 2 for 1 or 2 player start\n");
	      printf ("v is number of reads to assert switch\n");
	    }
	  break;
	case 'M': /* Diff ZP */
	  sscanf(inbuf+1, "%x", &i);
	  addr = i;
	  for(i=0;i < 16;i++) 
	    {
	      printf("%04x: ", addr);
	      for(j=0;j < 16;j++, addr++) 
		{
		  val = memrd(addr, 0, 0);
		  printf("%02x ", val);
		}
	      printf("\n");
	    }
	  break;
	case '#':
	  flagrandom ^= 1;
	  if(sscanf(inbuf+1, "%d",&n) == 1) 
	    {
	      force_random = 1;
	      force_val = n;
	    }
	  break;
#ifdef FIFO
	case 'f':
	  sscanf(inbuf+1, "%d",&n);
	  pos = (pcpos-1) & 0xffff;
	  for(i=0;i < n;i++) 
	    {
	      addr = fifo[pos].PC;
	      disasm_6502 (addr);
	      printf(" ");
	      formatdump(fifo[pos].A, fifo[pos].X, fifo[pos].Y, fifo[pos].flags, fifo[pos].SP); 
	      printf ("\n");
	      pos = (pos - 1) & 0xffff;
	    }
	  break;
#endif
	case 'h':
	  prthelp();
	  break;

	case 'c':  /* Continue */
	case 'g':  /* Go */
	  if (sscanf(inbuf+1, "%x", &t1) == 1)
	    {
	      printf ("set PC to %04x\n", t1);
	      save_PC = t1;
	    }
	  stepflag = 0;
	  traceflag = 0;
	  runflag = 1;
	  break;
	case 's': /* Single step */
	  stepflag = 1;
	  traceflag = 0;
	  runflag = 1;
	  break;
	case 't': /* Trace */
	  stepflag = 1;
	  traceflag = 1;
	  runflag = 1;
	  break;
	case 'o':
	  sscanf(inbuf+1, "%d,%x",&t1,&t2);
	  if (t1 == 1)
	    optionreg1 = t2;
	  else if (t1 == 2)
	    optionreg2 = t2;
	  else
	    printf ("No option register %d\n", t1);
	  break;
	case 'i':
	  debugger_handle_inputs (inbuf+1);
	  break;
	case 'C':
	  for(i=0;i < 0x100;i++) 
	    {
	      if((mem[0x100+i].tagw & BREAKTAG)) 
		{
		  mem[0x100+i].tagw &= ~BREAKTAG;
		}
	    }
	  break;
	case 'R':
	  dumpregs();
	  break;
	case 'r':
	  sscanf(inbuf+1,"%x",&t1);
	  addr = t1;
	  dummemloc(addr);
	  break;
	case 'z':
	  sscanf(inbuf+1, "%x %x", &t1, & t2);
	  addr = t1;
	  addr2 = t2;
	  dumprange (addr, addr2);
	  break;
	case 'w':
	  sscanf(inbuf+1,"%x %x",&t1,&t2);
	  addr = t1;
	  val = t2;
	  memwr(addr,val, 0, 0);
	  dummemloc(addr);
	  break;
	case 'W':
	  sscanf(inbuf+1,"%x %x",&t1,&t2);
	  addr = t1;
	  val = t2;
	  mem [addr].cell = val;
	  dummemloc(addr);
	  break;
	case 'X':
	  xyzzy();
	  break;
	case '\n':
	case 0:
	  prthelp();
	  break;
	case 'q':
	  exit(0);
	  break;
	default:
	  printf("Bad command %c, (%s)\n",c,inbuf);
	  prthelp();
	}
    }
  breakflag = 0;
}

