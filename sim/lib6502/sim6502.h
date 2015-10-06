/*
 * sim6502.h: 6502 simulator for Atari game simulator
 *
 * Copyright 1991-1993 Hedley Rainnie and Eric Smith
 *
 * Macros for addressing modes and instructions
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/sim6502.h,v 1.2 1994/08/12 05:23:28 eric Exp $
 */


#ifdef FIFO
struct _fifo {
    unsigned PC;
    unsigned SP;
    byte A;
    byte X;
    byte Y;
    byte flags;
};
extern struct _fifo fifo[0x10000];
extern unsigned short pcpos;
#endif

extern long icount;

extern int stepflag;
extern int traceflag;

extern byte save_A;
extern byte save_X;
extern byte save_Y;
extern byte save_flags;
extern word save_PC;
extern long save_totcycles;

extern byte _SP;

extern long irq_cycle;
#ifdef COUNT_INTERRUPTS
extern long int_count;
extern long int_quit;
#endif


void sim_6502 (void);


#define N_BIT 0x80
#define V_BIT 0x40
#define B_BIT 0x10
#define D_BIT 0x08
#define I_BIT 0x04
#define Z_BIT 0x02
#define C_BIT 0x01
