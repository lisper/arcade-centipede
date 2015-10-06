/*
 * main.c: Atari game simulator
 *
 * Copyright 1991-1994 Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/main.c,v 1.8 1994/08/21 03:08:33 eric Exp eric $
 */


#undef SINGLE_GAME
#define DEFAULT_GAME "Centipede"


#ifndef __GNUC__
#define inline
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "memory.h"
#include "game.h"
#include "display.h"
#include "misc.h"
#include "sim6502.h"


int getpid(void);


void foo()
{
    stepflag = 1;
    traceflag = 0;
    signal(SIGINT, foo);
}



void show_usage_message (char *av0)
{
#ifdef SINGLE_GAME
  fprintf (stderr, "Usage: %s [<options>]\n", av0);
#else
  fprintf (stderr, "Usage: %s <game> [<options>]\n", av0);
#endif
  fprintf (stderr, 
           "   -x<n>   scale pixels up horizontally by <n>\n"
           "   -y<n>   scale pixels up vertically by <n>\n"
           "   -i<n>   update display every <n>th interrupt\n"
	   "   -st     self test mode\n"
	   "   -load <file>  specify a memory dump file to load\n"
	   "   -brk    break into debugger before starting\n");
#ifndef SINGLE_GAME
  fprintf( stderr,
	  "\nThe following games are supported:\n"
	  "    keyword       title\n"
	  "    ----------    ------------------------------\n");
  show_games ();
#endif
}


int main(int argc, char *argv[])
{
  int ac;
  char **av;

  int show_usage = 1;
  char *reload_file = NULL;

  mem = (elem *) calloc(65536L,sizeof(elem));
  if(!mem)
    fatal_error (2, "Cannot get memory for simulation\n");

  ac = argc;
  av = argv;

  while (argc > 1)
    {
      argc--; argv++;
      if (*argv[0] == '-')
	{
	  if (my_stricmp (argv [0], "-st") == 0)
	    self_test = 1;
	  else if (my_stricmp (argv [0], "-load") == 0)
	    {
	      argc--; argv++;
	      if (reload_file == NULL)
		reload_file = argv [0];
	      else
		{
		  fprintf (stderr, "Only one load file may be specified\n");
		  goto quit;
		}
	    }
	  else if (my_stricmp (argv [0], "-brk") == 0)
	    stepflag = 1;
	  else if (my_stricmp (argv [0], "-k") == 0)
	    need_kludge = 1;
          else if (my_stricmpn (argv [0], "-x", 2) == 0)
	    x_scale = atoi (argv [0] + 2);
          else if (my_stricmpn (argv [0], "-y", 2) == 0)
	    y_scale = atoi (argv [0] + 2);
          else if (my_stricmpn (argv [0], "-i", 2) == 0)
	    ints_per_frame = atoi (argv [0] + 2);
	  else if (my_stricmp (argv [0], "-help") == 0)
	    goto quit;
	  else if (my_stricmp (argv [0], "-?") == 0)
	    goto quit;
#ifdef COUNT_INTERRUPTS
	  else if (my_stricmpn (argv [0], "-intquit", 8) == 0)
	    int_quit = atoi (argv [0] + 8);
#endif
	  else
	    {
	      fprintf (stderr, "Unrecognized option \"%s\"\n", argv[0]);
	      goto quit;
	    }
	}
      else
	{
#ifdef SINGLE_GAME
	  fprintf (stderr, "Unrecognized option \"%s\"\n", argv[0]);
	  goto quit;
#else
	  if (game == 0)
	    game = pick_game (argv [0]);
	  else
	    {
	      fprintf (stderr, "Only one game may be specified\n");
	      goto quit;
	    }
#endif
	}
    }

#ifdef SINGLE_GAME
  game = pick_game (SINGLE_GAME);
#endif

#ifdef DEFAULT_GAME
  if (game == 0)
    game = pick_game (DEFAULT_GAME);
#endif

  if (game == 0)
    {
      fprintf (stderr, "A game must be specified\n");
      goto quit;
    }

  show_usage = 0;

  signal(SIGINT, foo);

  srand(getpid());

  if (reload_file != NULL)
    reload (reload_file);
  else
    {
      setup_game ();
      save_PC = (memrd(0xfffd,0,0) << 8) | memrd(0xfffc,0,0);
      save_A = 0;
      save_X = 0;
      save_Y = 0;
      save_flags = I_BIT | Z_BIT;
      save_totcycles = 0;
      irq_cycle = 8192;
    }

  sim_6502 ();

 quit:
  if (show_usage)
    {
      show_usage_message (av [0]);
      return 1;
    }
  return 0;
}
