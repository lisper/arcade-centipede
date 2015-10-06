/*
 * sim6502.c: 6502 simulator for Atari game simulator
 *
 * Copyright 1991-1994 Eric Smith and Hedley Rainnie
 *
 * 6502 simulator and debugger by Hedley Rainnie, Doug Neubauer, and Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/sim6502.c,v 1.2 1994/08/12 05:23:28 eric Exp $
 */

#ifndef __GNUC__
#define inline
#endif

#include <stdio.h>

typedef unsigned char byte;
typedef unsigned short word;


//#include "memory.h"
//#include "game.h"
//#include "misc.h"
#include "sim6502.h"
#include "macro6502.h"
//#include "debugger.h"

#define STEP 1
#define BREAKPT 2
void dodebugger (int arg)
{
}

#define dobreak(arg) do { \
			   save_A = A; \
			   save_X = X; \
			   save_Y = Y; \
			   save_flags = flags_to_byte; \
                           save_PC = PC; \
                           save_totcycles = totcycles; \
			   dodebugger (arg); \
			   A = save_A; \
			   X = save_X; \
			   Y = save_Y; \
			   byte_to_flags (save_flags); \
			   PC = save_PC; \
                         } while (0)


int stepflag = 0;
int traceflag = 0;


byte save_A;
byte save_X;
byte save_Y;
byte save_flags;
word save_PC;
long save_totcycles;

byte _SP;

long irq_cycle = 0;
#ifdef COUNT_INTERRUPTS
long int_count = 0;
long int_quit = 0;
#endif


#if 0
#define dopush(val) memwr(0x100+(_SP--),(val))
#define dopop() memrd(0x100+(_SP=(++_SP)))
/* #define dopop() memrd(0x100+(_SP=(_SP+1)&0xff)) */
#else
void dopush(byte val, unsigned short PC)
{
  word addr;
  addr = _SP + 0x100;
  _SP--;
  _SP &= 0xff;
  memwr (addr, val, PC, 0);
}
byte dopop(unsigned short PC)
{
  word addr;
  _SP++;
  _SP &= 0xff;
  addr = _SP + 0x100;
  return (memrd(addr, PC, 0));
}
#endif


#ifdef FIFO
struct _fifo fifo[0x10000];
unsigned short pcpos=0;
#endif


long icount=0;
long totcycles;
int use_nmi;
int self_test;
int breakflag;

int PC;
int opcode;
int addr;
int A;
int X;
int Y;
int CC;

int irq_posted;

void get_6502_regs(int *ppc, int *psp, int *psr, int *pa, int *px, int *py)
{
	*ppc = PC & 0xffff;
	*psp = _SP & 0xffff;
	*psr = CC & 0xff;
	*pa = A & 0xff;
	*px = X & 0xff;
	*py = Y & 0xff;
}

void set_6502_a(int a)
{
	A = a;
}

void set_6502_irq(int on)
{
	if (on) {
		//printf("***POST IRQ\n");
		irq_posted = 1;
	} else {
		//printf("***UNPOST IRQ\n");
		irq_posted = 0;
	}
}

void reset_6502(void)
{
      save_PC = (memrd(0xfffd,0,0) << 8) | memrd(0xfffc,0,0);
      save_A = 0;
      save_X = 0;
      save_Y = 0;
      save_flags = I_BIT | Z_BIT;
      save_totcycles = 0;
      irq_cycle = 8192;

      A = save_A;
      X = save_X;
      Y = save_Y;
      CC = save_flags;
      PC = save_PC;
      totcycles = 0;
}

void sim_6502 (void)
{
#ifdef CYCLE_COUNT_EXACT
  int oldaddr;
#endif

//  A = save_A;
//  X = save_X;
//  Y = save_Y;
//  byte_to_flags (save_flags);
//  PC = save_PC;
//  totcycles = save_totcycles;

test_interrupt:
  if (/*(totcycles > irq_cycle) || */irq_posted)
    {
      if (use_nmi)
	{
	  if (! self_test)
	    {
	      /* do NMI */
#ifdef COUNT_INTERRUPTS
	      int_count += 1;
	      if ((int_quit) && (int_count >= int_quit))
		exit (0);
#endif
	      dopush(PC>>8, PC);
	      dopush(PC&0xff, PC);
	      dopush(flags_to_byte, PC);
	      SET_I;
	      PC = memrdwd (0xfffa, PC, totcycles);
	      totcycles += 7;
	      irq_cycle += 6144;
//	      update_display ();
	    }
	}
      else
	{
	  if (! TST_I)
	    {
//printf("***SERVICE IRQ %d\n", irq_posted);
	      irq_posted = 0;
	      /* do IRQ */
#ifdef COUNT_INTERRUPTS
	      int_count += 1;
	      if ((int_quit) && (int_count >= int_quit))
		exit (0);
#endif
	      dopush(PC>>8, PC);
	      dopush(PC&0xff, PC);
	      dopush(flags_to_byte, PC);
	      SET_I;
	      PC = memrdwd (0xfffe, PC, totcycles);
	      totcycles += 7;
	      irq_cycle += 0x7fffffff;
//	      update_display ();
	      goto skip_cycle;
	    }
	}
    }

//  while(1) 
//    {
#if 1
      if (stepflag) 
	dobreak(STEP);
      else if (breakflag) 
	dobreak(BREAKPT);
#endif

#if 0
      if((icount & 0x1ffff) == 0) printf("%ld @%x 1%x\n", icount, PC, _SP);
#endif

#ifdef FIFO
      fifo[pcpos & 0xffff].PC = PC;
      fifo[pcpos & 0xffff].A = A;
      fifo[pcpos & 0xffff].X = X;
      fifo[pcpos & 0xffff].Y = Y;
      fifo[pcpos & 0xffff].flags = flags_to_byte;
      fifo[pcpos++ & 0xffff].SP = _SP;
#endif

#ifdef INST_COUNT
      icount++;
#endif

#if 0
      {
	      static int ic = 0;
	      ic++;
	      if (ic == 1000) {
		      ic = 0;
		      printf("pc=%04x sp=%0x a %02x x %02x y %02x\n", PC, _SP, A, X, Y);
	      }
      }
#endif

  opcode = memrd (PC, PC, totcycles); PC++;

      switch(opcode) 			/* execute opcode */
	{
	case 0x69:  /* ADC */  EA_IMM    DO_ADC   C( 2)  break;
	case 0x6d:  /* ADC */  EA_ABS    DO_ADC   C( 3)  break;
	case 0x65:  /* ADC */  EA_ZP     DO_ADC   C( 4)  break;
	case 0x61:  /* ADC */  EA_IND_X  DO_ADC   C( 6)  break;
	case 0x71:  /* ADC */  EA_IND_Y  DO_ADC   C( 5)  break;
	case 0x75:  /* ADC */  EA_ZP_X   DO_ADC   C( 4)  break;
	case 0x7d:  /* ADC */  EA_ABS_X  DO_ADC   C( 4)  break;
	case 0x79:  /* ADC */  EA_ABS_Y  DO_ADC   C( 4)  break;

	case 0x29:  /* AND */  EA_IMM    DO_AND   C( 2)  break;
	case 0x2d:  /* AND */  EA_ABS    DO_AND   C( 4)  break;
	case 0x25:  /* AND */  EA_ZP     DO_AND   C( 3)  break;
	case 0x21:  /* AND */  EA_IND_X  DO_AND   C( 6)  break;
	case 0x31:  /* AND */  EA_IND_Y  DO_AND   C( 5)  break;
	case 0x35:  /* AND */  EA_ZP_X   DO_AND   C( 4)  break;
	case 0x39:  /* AND */  EA_ABS_Y  DO_AND   C( 4)  break;
	case 0x3d:  /* AND */  EA_ABS_X  DO_AND   C( 4)  break;

	case 0x0e:  /* ASL */  EA_ABS    DO_ASL   C( 6)  break;
	case 0x06:  /* ASL */  EA_ZP     DO_ASL   C( 5)  break;
	case 0x0a:  /* ASL */            DO_ASLA  C( 2)  break;
	case 0x16:  /* ASL */  EA_ZP_X   DO_ASL   C( 6)  break;
	case 0x1e:  /* ASL */  EA_ABS_X  DO_ASL   C( 7)  break;

	case 0x90:  /* BCC */		 DO_BCC   C( 2)  /*goto test_interrupt;*/break;
	case 0xb0:  /* BCS */		 DO_BCS   C( 2)  /*goto test_interrupt;*/break;
	case 0xf0:  /* BEQ */		 DO_BEQ   C( 2)  /*goto test_interrupt;*/break;
	case 0x30:  /* BMI */		 DO_BMI   C( 2)  /*goto test_interrupt;*/break;
	case 0xd0:  /* BNE */		 DO_BNE   C( 2)  /*goto test_interrupt;*/break;
	case 0x10:  /* BPL */		 DO_BPL   C( 2)  /*goto test_interrupt;*/break;
	case 0x50:  /* BVC */		 DO_BVC   C( 2)  /*goto test_interrupt;*/break;
	case 0x70:  /* BVS */		 DO_BVS   C( 2)  /*goto test_interrupt;*/break;

	case 0x2c:  /* BIT */  EA_ABS    DO_BIT   C( 4)  break;
	case 0x24:  /* BIT */  EA_ZP     DO_BIT   C( 3)  break;

#if 0
	case 0x00:  /* BRK */            DO_BRK   C( 7)  break;
#endif

	case 0x18:  /* CLC */            DO_CLC   C( 2)  break;
	case 0xd8:  /* CLD */            DO_CLD   C( 2)  break;
	case 0x58:  /* CLI */            DO_CLI   C( 2)  break;
	case 0xb8:  /* CLV */            DO_CLV   C( 2)  break;

	case 0xc9:  /* CMP */  EA_IMM    DO_CMP   C( 2)  break;
	case 0xcd:  /* CMP */  EA_ABS    DO_CMP   C( 4)  break;
	case 0xc5:  /* CMP */  EA_ZP     DO_CMP   C( 3)  break;
	case 0xc1:  /* CMP */  EA_IND_X  DO_CMP   C( 6)  break;
	case 0xd1:  /* CMP */  EA_IND_Y  DO_CMP   C( 5)  break;
	case 0xd5:  /* CMP */  EA_ZP_X   DO_CMP   C( 4)  break;
	case 0xd9:  /* CMP */  EA_ABS_Y  DO_CMP   C( 4)  break;
	case 0xdd:  /* CMP */  EA_ABS_X  DO_CMP   C( 4)  break;

	case 0xe0:  /* CPX */  EA_IMM    DO_CPX   C( 2)  break;
	case 0xec:  /* CPX */  EA_ABS    DO_CPX   C( 4)  break;
	case 0xe4:  /* CPX */  EA_ZP     DO_CPX   C( 3)  break;

	case 0xc0:  /* CPY */  EA_IMM    DO_CPY   C( 2)  break;
	case 0xcc:  /* CPY */  EA_ABS    DO_CPY   C( 4)  break;
	case 0xc4:  /* CPY */  EA_ZP     DO_CPY   C( 3)  break;

	case 0xce:  /* DEC */  EA_ABS    DO_DEC   C( 6)  break;
	case 0xc6:  /* DEC */  EA_ZP     DO_DEC   C( 5)  break;
	case 0xd6:  /* DEC */  EA_ZP_X   DO_DEC   C( 6)  break;
	case 0xde:  /* DEC */  EA_ABS_X  DO_DEC   C( 7)  break;

	case 0xca:  /* DEX */            DO_DEX   C( 2)  break;
	case 0x88:  /* DEY */            DO_DEY   C( 2)  break;

	case 0x49:  /* EOR */  EA_IMM    DO_EOR   C( 2)  break;
	case 0x4d:  /* EOR */  EA_ABS    DO_EOR   C( 4)  break;
	case 0x45:  /* EOR */  EA_ZP     DO_EOR   C( 3)  break;
	case 0x41:  /* EOR */  EA_IND_X  DO_EOR   C( 6)  break;
	case 0x51:  /* EOR */  EA_IND_Y  DO_EOR   C( 5)  break;
	case 0x55:  /* EOR */  EA_ZP_X   DO_EOR   C( 4)  break;
	case 0x59:  /* EOR */  EA_ABS_Y  DO_EOR   C( 4)  break;
	case 0x5d:  /* EOR */  EA_ABS_X  DO_EOR   C( 4)  break;

	case 0xee:  /* INC */  EA_ABS    DO_INC   C( 6)  break;
	case 0xe6:  /* INC */  EA_ZP     DO_INC   C( 5)  break;
	case 0xf6:  /* INC */  EA_ZP_X   DO_INC   C( 6)  break;
	case 0xfe:  /* INC */  EA_ABS_X  DO_INC   C( 7)  break;

	case 0xe8:  /* INX */            DO_INX   C( 2)  break;
	case 0xc8:  /* INY */            DO_INY   C( 2)  break;

	case 0x4c:  /* JMP */  EA_ABS    DO_JMP   C( 3)  /*goto test_interrupt;*/break;
	case 0x6c:  /* JMP */  EA_IND    DO_JMP   C( 5)  /*goto test_interrupt;*/break;

	case 0x20:  /* JSR */  EA_ABS    DO_JSR   C( 6)  /*goto test_interrupt;*/break;

	case 0xa9:  /* LDA */  EA_IMM    DO_LDA   C( 2)  break;
	case 0xad:  /* LDA */  EA_ABS    DO_LDA   C( 4)  break;
	case 0xa5:  /* LDA */  EA_ZP     DO_LDA   C( 3)  break;
	case 0xa1:  /* LDA */  EA_IND_X  DO_LDA   C( 6)  break;
	case 0xb1:  /* LDA */  EA_IND_Y  DO_LDA   C( 5)  break;
	case 0xb5:  /* LDA */  EA_ZP_X   DO_LDA   C( 4)  break;
	case 0xb9:  /* LDA */  EA_ABS_Y  DO_LDA   C( 4)  break;
	case 0xbd:  /* LDA */  EA_ABS_X  DO_LDA   C( 4)  break;

	case 0xa2:  /* LDX */  EA_IMM    DO_LDX   C( 2)  break;
	case 0xae:  /* LDX */  EA_ABS    DO_LDX   C( 4)  break;
	case 0xa6:  /* LDX */  EA_ZP     DO_LDX   C( 3)  break;
	case 0xbe:  /* LDX */  EA_ABS_Y  DO_LDX   C( 4)  break;
	case 0xb6:  /* LDX */  EA_ZP_Y   DO_LDX   C( 4)  break;

	case 0xa0:  /* LDY */  EA_IMM    DO_LDY   C( 2)  break;
	case 0xac:  /* LDY */  EA_ABS    DO_LDY   C( 4)  break;
	case 0xa4:  /* LDY */  EA_ZP     DO_LDY   C( 3)  break;
	case 0xb4:  /* LDY */  EA_ZP_X   DO_LDY   C( 4)  break;
	case 0xbc:  /* LDY */  EA_ABS_X  DO_LDY   C( 4)  break;

	case 0x4e:  /* LSR */  EA_ABS    DO_LSR   C( 6)  break;
	case 0x46:  /* LSR */  EA_ZP     DO_LSR   C( 5)  break;
	case 0x4a:  /* LSR */            DO_LSRA  C( 2)  break;
	case 0x56:  /* LSR */  EA_ZP_X   DO_LSR   C( 6)  break;
	case 0x5e:  /* LSR */  EA_ABS_X  DO_LSR   C( 7)  break;

	case 0xea:  /* NOP */                     C( 2)  break;

	case 0x09:  /* ORA */  EA_IMM    DO_ORA   C( 2)  break;
	case 0x0d:  /* ORA */  EA_ABS    DO_ORA   C( 4)  break;
	case 0x05:  /* ORA */  EA_ZP     DO_ORA   C( 3)  break;
	case 0x01:  /* ORA */  EA_IND_X  DO_ORA   C( 6)  break;
	case 0x11:  /* ORA */  EA_IND_Y  DO_ORA   C( 5)  break;
	case 0x15:  /* ORA */  EA_ZP_X   DO_ORA   C( 4)  break;
	case 0x19:  /* ORA */  EA_ABS_Y  DO_ORA   C( 4)  break;
	case 0x1d:  /* ORA */  EA_ABS_X  DO_ORA   C( 4)  break;

	case 0x48:  /* PHA */            DO_PHA   C( 3)  break;
	case 0x08:  /* PHP */            DO_PHP   C( 3)  break;
	case 0x68:  /* PLA */            DO_PLA   C( 4)  break;
	case 0x28:  /* PLP */            DO_PLP   C( 4)  break;

	case 0x2e:  /* ROL */  EA_ABS    DO_ROL   C( 6)  break;
	case 0x26:  /* ROL */  EA_ZP     DO_ROL   C( 5)  break;
	case 0x2a:  /* ROL */            DO_ROLA  C( 2)  break;
	case 0x36:  /* ROL */  EA_ZP_X   DO_ROL   C( 6)  break;
	case 0x3e:  /* ROL */  EA_ABS_X  DO_ROL   C( 7)  break;

	case 0x6e:  /* ROR */  EA_ABS    DO_ROR   C( 6)  break;
	case 0x66:  /* ROR */  EA_ZP     DO_ROR   C( 5)  break;
	case 0x6a:  /* ROR */            DO_RORA  C( 2)  break;
	case 0x76:  /* ROR */  EA_ZP_X   DO_ROR   C( 6)  break;
	case 0x7e:  /* ROR */  EA_ABS_X  DO_ROR   C( 7)  break;

	case 0x40:  /* RTI */            DO_RTI   C( 6)  /*goto test_interrupt;*/break;
	case 0x60:  /* RTS */            DO_RTS   C( 6)  /*goto test_interrupt;*/break;

	case 0xe9:  /* SBC */  EA_IMM    DO_SBC   C( 2)  break;
	case 0xed:  /* SBC */  EA_ABS    DO_SBC   C( 4)  break;
	case 0xe5:  /* SBC */  EA_ZP     DO_SBC   C( 3)  break;
	case 0xe1:  /* SBC */  EA_IND_X  DO_SBC   C( 6)  break;
	case 0xf1:  /* SBC */  EA_IND_Y  DO_SBC   C( 5)  break;
	case 0xf5:  /* SBC */  EA_ZP_X   DO_SBC   C( 4)  break;
	case 0xf9:  /* SBC */  EA_ABS_Y  DO_SBC   C( 4)  break;
	case 0xfd:  /* SBC */  EA_ABS_X  DO_SBC   C( 4)  break;

	case 0x38:  /* SEC */            DO_SEC   C( 2)  break;
	case 0xf8:  /* SED */            DO_SED   C( 2)  break;
	case 0x78:  /* SEI */            DO_SEI   C( 2)  break;

	case 0x8d:  /* STA */  EA_ABS    DO_STA   C( 4)  break;
	case 0x85:  /* STA */  EA_ZP     DO_STA   C( 3)  break;
	case 0x81:  /* STA */  EA_IND_X  DO_STA   C( 6)  break;
	case 0x91:  /* STA */  EA_IND_Y  DO_STA   C( 6)  break;
	case 0x95:  /* STA */  EA_ZP_X   DO_STA   C( 4)  break;
	case 0x99:  /* STA */  EA_ABS_Y  DO_STA   C( 5)  break;
	case 0x9d:  /* STA */  EA_ABS_X  DO_STA   C( 5)  break;

	case 0x8e:  /* STX */  EA_ABS    DO_STX   C( 4)  break;
	case 0x86:  /* STX */  EA_ZP     DO_STX   C( 3)  break;
	case 0x96:  /* STX */  EA_ZP_Y   DO_STX   C( 4)  break;

	case 0x8c:  /* STY */  EA_ABS    DO_STY   C( 4)  break;
	case 0x84:  /* STY */  EA_ZP     DO_STY   C( 3)  break;
	case 0x94:  /* STY */  EA_ZP_X   DO_STY   C( 4)  break;

	case 0xaa:  /* TAX */            DO_TAX   C( 2)  break;
	case 0xa8:  /* TAY */            DO_TAY   C( 2)  break;
	case 0x98:  /* TYA */            DO_TYA   C( 2)  break;
	case 0xba:  /* TSX */            DO_TSX   C( 2)  break;
	case 0x8a:  /* TXA */            DO_TXA   C( 2)  break;
	case 0x9a:  /* TXS */            DO_TXS   C( 2)  break;

	default:
	  printf ("@%x Illegal opcode %2x\n", PC, opcode);
	  breakflag = 1;
	  break;
	}
//    }
//  dobreak(INTBREAK);
skip_cycle:
      ;
}
