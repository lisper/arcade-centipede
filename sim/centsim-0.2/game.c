/*
 * game.c: Atari game definitions & setup functions
 *
 * Copyright 1991-1994 Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/game.c,v 1.5 1994/08/20 04:53:11 eric Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "display.h"
#include "memory.h"
#include "game.h"
#include "misc.h"

int use_nmi;

int game = 0;

typedef struct { int show; char *kw1; char *kw2; char *name; } game_info;

game_info game_names [] =
{
  { 0, "unknown",    "??",    "Unknown" },
  { 1, "centipede",  "c",     "Centipede" },
  { 1, "millipede",  "m",     "Millipede" },
  { 0, "football",   "f",     "Football" },
  { 0, "football4",  "f4",    "Football 4" },
  { 0, "soccer",     "s",     "Soccer" },
  { 0, "basketball", "b",     "Basketball" }
};


int pick_game (char *name)
{
  int i;

  for (i = FIRST_GAME; i <= LAST_GAME; i++)
    if ((my_stricmp (name, game_names [i].kw1) == 0) ||
	(my_stricmp (name, game_names [i].kw2) == 0))
      {
	return (i);
      }

  fprintf (stderr, "ERROR: Unknown game \"%s\"\n", name);
  exit (1);
}


void show_games (void)
{
  int i;

  for (i = FIRST_GAME; i <= LAST_GAME; i++)
    if (game_names [i].show)
      fprintf (stderr, "    %-10s    %s\n", game_names [i].kw1, game_names [i].name);
}


char *game_name (int game)
{
  return (game_names [game].name);
}


rom_info centipede_roms [] =
{
  { "roms/136001.307", 0x2000, 0x0800, 0 },
  { "roms/136001.308", 0x2800, 0x0800, 0 },
  { "roms/136001.309", 0x3000, 0x0800, 0 },
  { "roms/136001.310", 0x3800, 0x0800, 0 },
  { NULL,   0,      0,      0 }
};

tag_info centipede_tags [] =
{
  { 0x0000, 0x0200, RD | WR, MEMORY }, /* RAM */
  { 0x0400, 0x0400, RD | WR, MEMORY }, /* video RAM */

  { 0x0800,      1, RD,      OPTSW1 },
  { 0x0801,      1, RD,      OPTSW2 },

  { 0x0c00,      1, RD,      CENTIPEDE_TRACKBALL_HORIZ },
  { 0x0c01,      1, RD,      CENTIPEDE_SWITCH1 },
  { 0x0c02,      1, RD,      CENTIPEDE_TRACKBALL_VERT },
  { 0x0c03,      1, RD,      CENTIPEDE_JOYSTICK },

  { 0x1000, 0x0010, RD | WR, POKEY1 },

  { 0x1400, 0x0010, RD | WR, MEMORY }, /* color RAM */

  { 0x1600,   0x40,      WR, EAROMWR },
  { 0x1680,      1,      WR, EAROMCON },
  { 0x1700,   0x40, RD,      EAROMRD },

  { 0x1800,      1,      WR, INTACK },

  { 0x1c00,      8,      WR, CENTIPEDE_OUT1 },

  { 0x2000,      1,      WR, WDCLR },

  { 0x2400,      1,      WR, TRACKBALL_CLR },

  { 0,           0, 0,       0 }
};


rom_info millipede_roms [] =
{
  { "roms/136013.104", 0x4000, 0x1000, 0 },
  { "roms/136013.103", 0x5000, 0x1000, 0 },
  { "roms/136013.102", 0x6000, 0x1000, 0 },
  { "roms/136013.101", 0x7000, 0x1000, 0 },
  { NULL,   0,      0,      0 }
};

tag_info millipede_tags [] =
{
  { 0x0000, 0x0400, RD | WR, MEMORY }, /* RAM */

  { 0x0400, 0x0010, RD | WR, POKEY1 },
  { 0x0800, 0x0010, RD | WR, POKEY2 },

  { 0x1000, 0x0400, RD | WR, MEMORY }, /* video RAM */

  { 0x0800,      1, RD,      OPTSW1 },
  { 0x0801,      1, RD,      OPTSW2 },

  { 0x2000,      1, RD,      MILLIPEDE_TRACKBALL_HORIZ },
  { 0x2001,      1, RD,      MILLIPEDE_TRACKBALL_VERT },
  { 0x2010,      1, RD,      MILLIPEDE_SWITCH1 },
  { 0x2011,      1, RD,      MILLIPEDE_SWITCH2 },

  { 0x2030,   0x40, RD,      EAROMRD },

  { 0x2480, 0x0020, RD | WR, MEMORY }, /* color RAM */

  { 0x2500,      8,      WR, MILLIPEDE_OUT1 },

  { 0x2600,      1,      WR, INTACK },
  { 0x2680,      1,      WR, WDCLR },
  { 0x2700,      1,      WR, EAROMCON },

  { 0x2780,   0x40,      WR, EAROMWR },

  { 0,           0, 0,       0 }
};


void setup_game (void)
{
  tag_area (0x0000, 0x10000, RD | WR, UNKNOWN);

  switch (game)
    {
    case CENTIPEDE:
      setup_roms_and_tags (centipede_roms, centipede_tags);

      copy_rom (0x3ffa, 0xfffa, 6);

      video_base_address = 0x0400;
      color_base_address = 0x1400;

      optionreg1 = 0x54;  /* switch N9, 8..1, off = 1, on = 0 */
      /* 1 credit minimum, easy, 12000 point bonus, 3 lives, English */

      optionreg2 = 0x00;  /* switch N8, 8..1, off = 1, on = 0 */
      /* free play */

      break;

    case MILLIPEDE:
      setup_roms_and_tags (millipede_roms, millipede_tags);

      copy_rom (0x7ffa, 0xfffa, 6);

      video_base_address = 0x1000;
      color_base_address = 0x2480;

      optionreg1 = 0xff;  /* switch D4, 1..8, off = 0, on = 1 */
      optionreg2 = 0xff;  /* switch B4, 1..8, off = 0, on = 1 */

      break;

    default:
      fprintf (stderr, "ERROR: Unknown game\n");
      exit (1);
    }
}
