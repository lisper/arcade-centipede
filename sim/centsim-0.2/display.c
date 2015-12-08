/*
 * display.c: SDL display interface for Centipede simulator
 *
 * Original 1994 Eric Smith
 * Mods 2015 Brad Parker
 *
 * $Header$
 */

#undef COLOR_DEBUG
#undef COLOR_DEBUG_2


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <SDL/SDL.h>

#include "misc.h"
#include "display.h"
#include "memory.h"
#include "sim6502.h"  /* for stepflag and traceflag */
#include "game.h"

static SDL_Surface *screen;
static int rows, cols;

int need_kludge = 0;
int x_scale = 1;
int y_scale = 1;
int ints_per_frame = 16;


unsigned short color_base_address;
unsigned short video_base_address;


int initialized = 0;


#define ROMSIZE 2048
char charset1 [ROMSIZE];
char charset2 [ROMSIZE];


#define MAX_STAMP_COLORS 16
#define MAX_MOB_COLORS 16
#define MAX_COLOR_TABLE_SIZE 32


int stamp_colors;
int mob_colors;
int color_table_size;
int min_stamp_color_table_index;
int max_stamp_color_table_index;
int min_mob_color_table_index;
int max_mob_color_table_index;

#define STAMP_ROWS 32
#define STAMP_COLUMNS 30


#define STAMP_WIDTH  ( 8 * x_scale)
#define STAMP_HEIGHT ( 8 * y_scale)
#define MOB_WIDTH    (16 * x_scale)
#define MOB_HEIGHT   ( 8 * y_scale)

typedef struct {
	int h, w;
	unsigned char pix[32*32];
} pixmap;

void putpixel(pixmap *pm, int x, int y, int p)
{
	pm->pix[ (y*pm->w) + x ] = p;
}

pixmap stamp_pixmap [256];
pixmap mob_pixmap [256];

#define WINDOW_WIDTH (STAMP_COLUMNS * STAMP_WIDTH)
#define WINDOW_HEIGHT (STAMP_ROWS * STAMP_HEIGHT)


char *color_name [16] =
{
  "#ff80ff", /* pale yellowgreen */
  "#0080ff", /* light green */
  "#ff0080", /* deep rose */
  "#000080", /* navy blue */

  "#ff8000", /* orange */
  "#008000", /* dark green */
  "#ff0000", /* red */
  "#000000", /* black */

  "#ffffff", /* white */
  "#00ffff", /* light blue */
  "#ff00ff", /* purple */
  "#0000ff", /* royal blue */

  "#ffff00", /* deep yellow */
  "#00ff00", /* lime green */
  "#ff0000", /* red */
  "#000000", /* black */
};

void sdl_clear_rect(int h, int v, int hsize, int vsize)
{
  int hh, vv, offset;
  unsigned char *ps;

  if (h < 0 || v < 0)
	  return;

  ps = (unsigned char *)screen->pixels;

  for (vv = 0; vv < vsize; vv++) {
    for (hh = 0; hh < vsize; hh++) {
      offset = ((v+vv) * cols) + h+hh;
      ps[offset] = 0;
    }
  }

  SDL_UpdateRect(screen, h, v, hsize, vsize);
}

void sdl_blit_pixmap(pixmap *pm, int pm_x, int pm_y, int pm_w, int pm_h, int window_x, int window_y)
{
  int h, v, hh, vv, offset;
  unsigned char *ps, p;

  if (window_x < 0 || window_y < 0)
	  return;

  ps = (unsigned char *)screen->pixels;

  for (vv = 0; vv < pm_h; vv++) {
    for (hh = 0; hh < pm_w; hh++) {
      offset = ((window_y+vv) * cols) + window_x+hh;
      p = pm->pix[ ((pm_y+vv)*pm->w) + (pm_x+hh) ];
//      p = old_color_map[p];
      switch (p) {
      case 0: p = 0x00; break;
      case 1: p = 0x07; break;
      case 2: p = 0x18; break;
      case 3: p = 0xe0; break;
      }
      ps[offset] = p;
    }
  }

  SDL_UpdateRect(screen, window_x, window_y, pm_w, pm_h);
}


#define MAX_KEY 256  /* KeyCode is defined as an unsigned char */
int *keymap [MAX_KEY];
int escape_code;

void setup_keyboard (void)
{
#if 0
  int i;

  for (i = 0; i < MAX_KEY; i++)
    keymap [i] = NULL;

  keymap [XKeysymToKeycode (mydisplay, XK_1)] = & start1;
  keymap [XKeysymToKeycode (mydisplay, XK_2)] = & start2;

  keymap [XKeysymToKeycode (mydisplay, XK_0)] = & self_test;

  keymap [XKeysymToKeycode (mydisplay, XK_Left)]  = & joystick [0].left;
  keymap [XKeysymToKeycode (mydisplay, XK_Right)] = & joystick [0].right;
  keymap [XKeysymToKeycode (mydisplay, XK_Up)]    = & joystick [0].up;
  keymap [XKeysymToKeycode (mydisplay, XK_Down)]  = & joystick [0].down;
  keymap [XKeysymToKeycode (mydisplay, XK_space)] = & joystick [0].fire;

  escape_code = XKeysymToKeycode (mydisplay, XK_Escape);

  XSelectInput (mydisplay, mywindow,
		ExposureMask | KeyPressMask | KeyReleaseMask);
#endif
}


#define MOB_PICTURE_OFFSET 0x3c0
#define MOB_HORIZONTAL_OFFSET 0x3d0
#define MOB_VERTICAL_OFFSET 0x3e0
#define MOB_COLOR_OFFSET 0x3f0
/*
 * The horizontal & vertical designations are reversed with respect to the
 * hardware documentation, because the monitor in the game is used in a
 * "sideways" (portrait) orientation.
 */

int old_color_map [MAX_COLOR_TABLE_SIZE];
int old_video_mem [0x400];

int last_mob_horizontal [16];
int last_mob_vertical   [16];


void update_colors (void)
{
  int i, c;

  for (i = 0; i < color_table_size; i++)
    {
      c = mem [color_base_address + i].cell;
      if (c != old_color_map [i])
	{
//	  write_color_table (i, c);
	  old_color_map [i] = c;
	}
    }
}


void force_color_table_update (void)
{
  int i;

  for (i = 0; i < color_table_size; i++)
    old_color_map [i] = -1;
}


void force_stamp_redraw (void)
{
  int i;

  for (i = 0; i < 0x400; i++)
    old_video_mem [i] = -1;
}


void update_stamps (void)
{
  int x, y, a, c;

  for (y = 0; y < STAMP_ROWS; y++)
    for (x = 0; x < STAMP_COLUMNS; x++)
      {
	a = x * STAMP_ROWS + y;
	c = mem [video_base_address + a].cell;
	if (c != old_video_mem [a] || 1)
	  {
	    sdl_blit_pixmap(&stamp_pixmap[c], 0, 0, STAMP_WIDTH, STAMP_HEIGHT,
			    x * STAMP_WIDTH, ((STAMP_ROWS - 1) - y) * STAMP_HEIGHT);

	    old_video_mem [a] = c;
	  }
      }
}

#define ERASE_MOB_PLANE

void erase_mobs (void)
{
  int i;

#ifdef ERASE_MOB_PLANE
  sdl_clear_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
#else
  int h, v, hh, vv, offset;
  unsigned char *ps;
  for (i = 0; i < 16; i++) {
      h = last_mob_horizontal [i];
      v = last_mob_vertical [i];
      sdl_clear_rect(h, v, MOB_HEIGHT, MOB_WIDTH);
  }
#endif
}


void draw_mobs (void)
{
  int i;
  int picture, horizontal, vertical, color;

  for (i = 0; i < 16; i++)
    {
      picture    = mem [video_base_address + MOB_PICTURE_OFFSET + i].cell;
      color      = mem [video_base_address + MOB_COLOR_OFFSET + i].cell;

      horizontal = ((255 - mem [video_base_address + MOB_HORIZONTAL_OFFSET + i].cell) - 16) * x_scale;
      vertical   = (((255 - mem [video_base_address + MOB_VERTICAL_OFFSET + i].cell) - 8) & 0xff) * y_scale;

      if (0) printf("stamp %d: %02x%02x%02x%02x\n", i, color&0xff, vertical&0xff, horizontal&0xff, picture&0xff);

#ifndef ERASE_MOB_PLANE
      last_mob_horizontal [i] = horizontal;
      last_mob_vertical   [i] = vertical;
#endif

      if (vertical >= (248 * y_scale))
	continue;

      /* first punch a hole */
      sdl_blit_pixmap(&mob_pixmap[picture], 0, 0, MOB_WIDTH, MOB_HEIGHT, horizontal, vertical);
    }
}


void init_colors (void)
{
  if (game == MILLIPEDE)
    {
      stamp_colors = 16;
      mob_colors = 16;
      color_table_size = 32;
      min_stamp_color_table_index = 0x00;
      max_stamp_color_table_index = 0x0f;
      min_mob_color_table_index = 0x11;
      max_mob_color_table_index = 0x1f;
    }
  else
    {
      stamp_colors = 4;
      mob_colors = 4;
      color_table_size = 16;
      min_stamp_color_table_index = 0x04;
      max_stamp_color_table_index = 0x07;
      min_mob_color_table_index = 0x0d;
      max_mob_color_table_index = 0x0f;
    }
}


void init_graphics (int h, int v)
{
    int flags;

    cols = h;
    rows = v;

    printf("Initialize display %dx%d\n", cols, rows);

    flags = SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE;

    if (SDL_Init(flags)) {
        printf("SDL initialization failed\n");
        return;
    }

    /* NOTE: we still want Ctrl-C to work - undo the SDL redirections*/
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);

    flags = SDL_HWSURFACE|SDL_ASYNCBLIT|SDL_HWACCEL;

    screen = SDL_SetVideoMode(cols, rows+8, 8, flags);

    if (!screen) {
        printf("Could not open SDL display\n");
        return;
    }

    SDL_WM_SetCaption("Centipede", "Centipede");
}


int read_file (char *name, int len, char *buffer)
{
  FILE *f;

  f = fopen (name, "rb");
  if (! f)
    return (0);

  if (len != fread (buffer, 1, len, f))
    return (0);

  fclose (f);

  return (1);
}

void read_charset_roms (void)
{
  int result1, result2;

  if (game == MILLIPEDE)
    {
      result1 = read_file ("roms/136013.106", ROMSIZE, & charset1 [0]);
      result2 = read_file ("roms/136013.107", ROMSIZE, & charset2 [0]);
    }
  else
    {
      result1 = read_file ("roms/136001.201", ROMSIZE, & charset1 [0]);
      result2 = read_file ("roms/136001.202", ROMSIZE, & charset2 [0]);
    }
  if (! (result1 && result2))
    fatal_error (2, "can't read character set\n");
}

int get_stamp_pixel (int c, int x, int y)
{
  int offset;
  int mask;
  int result = 0;

  if (game != MILLIPEDE)
    {
      if (c & 0x80)
	x = (STAMP_WIDTH - 1) - x;
      if (c & 0x40)
	y = (STAMP_HEIGHT - 1) - y;
      c &= 0x3f;
    }

  x /= x_scale;
  y /= y_scale;

  offset = ((c & 0x40) << 4) | 0x200 | ((c & 0x3f) << 3) | x;
  mask = (1 << y);  
  
  if (charset1 [offset] & mask)
    result |= 1;
  if (charset2 [offset] & mask)
    result |= 2;
  return (result);
}


int get_mob_pixel (int c, int x, int y)
{
  int offset;
  int mask;
  int result = 0;

  if (c & 0x80)
    x = (MOB_WIDTH - 1) - x;
  if (c & 0x40)
    y = (MOB_HEIGHT - 1) - y;
  c &= 0x3f;

  x /= x_scale;
  y /= y_scale;

  offset = ((c & 1) << 10) | ((c & 0x3e) << 3) | x;
  mask = (1 << y);

  if (charset1 [offset] & mask)
    result |= 1;
  if (charset2 [offset] & mask)
    result |= 2;
  return (result);
}


void create_stamp_pixmaps (void)
{
  int c, x, y;

  for (c = 0; c < 256; c++)
    {
      stamp_pixmap[c].w = STAMP_WIDTH;
      stamp_pixmap[c].h = STAMP_HEIGHT;

      for (y = 0; y < STAMP_HEIGHT; y++)
	for (x = 0; x < STAMP_WIDTH; x++)
	  {
	    if (game == MILLIPEDE) {
	      putpixel(&stamp_pixmap[c], x, y, get_stamp_pixel (c, x, y) | (c & 0xc0) >> 4);
	    } else {
	      putpixel(&stamp_pixmap[c], x, y, get_stamp_pixel (c, x, y));
	    }
	  }
    }
}


void set_bitmap (char *data, int row_size, int x, int y)
{
  data += y * row_size + (x / 8);
  *data |= (1 << (x & 7));
}


void create_mob_bitmaps (void)
{
  int c, x, y;

  for (c = 0; c < 256; c++)
    {
      mob_pixmap[c].w = MOB_WIDTH;
      mob_pixmap[c].h = MOB_HEIGHT;

      for (x = 0; x < MOB_WIDTH; x++)
	for (y = 0; y < MOB_HEIGHT; y++)
	  putpixel(&mob_pixmap[c], x, y, get_mob_pixel(c, x, y));
    }
}


void handle_events (void)
{
#if 0
  XEvent event;
  int mode = QueuedAfterFlush;

  while (XEventsQueued (mydisplay, mode) != 0)
    {
      mode = QueuedAlready;  /* only get new events once per call */
      XNextEvent (mydisplay, & event);
      switch (event.type)
	{
	case Expose:
	  if (event.xexpose.count == 0)
	    force_stamp_redraw ();
	  break;
	case KeyPress:
	case KeyRelease:
	  if (event.xkey.keycode == escape_code)
	    {
	      stepflag = 1;
	      traceflag = 0;
	    }
	  else if (keymap [event.xkey.keycode])
	    {
	      *(keymap[event.xkey.keycode]) = (event.type == KeyPress);
	    }
	  break;
	default:
	  break;
	}
    }
#endif
}


static int m = 0;


void update_display (void)
{
  int i;

  if (! initialized)
    {
      read_charset_roms ();

      init_graphics (WINDOW_WIDTH, WINDOW_HEIGHT);

      setup_keyboard ();

      init_colors ();

      create_stamp_pixmaps ();
      create_mob_bitmaps ();

      force_color_table_update ();
      force_stamp_redraw ();

      initialized = 1;
    }

  if (++m == ints_per_frame)
    {
      m = 0;
      erase_mobs ();
      update_colors ();
      update_stamps ();
      draw_mobs ();
      handle_events ();
usleep(50000);
    }
}

void dump_display_ram(void)
{
	FILE *f;
	f = fopen("dump.txt", "w");
	if (f) {
		int a, c;
		for (a = 0x400; a < 0x7c0; a++) {
			c = mem [a].cell;
			fprintf(f, "%x %x\n", a, c);
		}
		fclose(f);
	}
}

