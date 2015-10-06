/*
 * debugger.h: debugger for Atari game simulator
 *
 * Copyright 1991-1994 Hedley Rainnie and Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/debugger.h,v 1.2 1994/08/12 05:23:28 eric Exp $
 */


#define INTBREAK 0x01
#define BREAKPT  0x02
#define BREAK	 0x03
#define STEP	 0x04
#define ERRORBRK 0x05

void dodebugger (int type);
