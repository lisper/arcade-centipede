/*
 * misc.h: misc. definitions for Atari game simulator
 *
 * Copyright 1991-1994 Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/misc.h,v 1.3 1994/08/12 05:23:28 eric Exp $
 */


void fatal_error (int n, char *fmt, ...);


/*
 * I'm sick of dealing with machines that don't define stricmp, or define
 * it wrong, or give it some other stupid name
 */
int my_stricmp (char *s1, char *s2);
int my_stricmpn (char *s1, char *s2, int n);
