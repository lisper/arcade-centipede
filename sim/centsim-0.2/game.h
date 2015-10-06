/*
 * game.h: Atari game definitions & setup functions
 *
 * Copyright 1991-1994 Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/game.h,v 1.1 1994/08/08 07:30:16 eric Exp $
 */

extern int game;

/* color */
#define CENTIPEDE 1
#define MILLIPEDE 2

/* B&W */
#define FOOTBALL 3
#define FOOTBALL4 4
#define SOCCER 5
#define BASKETBALL 6

#define FIRST_GAME CENTIPEDE
#define LAST_GAME MILLIPEDE

extern int use_nmi;  /* set true to generate NMI instead of IRQ */

int pick_game (char *name);
void show_games (void);
char *game_name (int game);
void setup_game (void);
