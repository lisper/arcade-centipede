/*
 * display.c: X display interface for Centipede simulator
 *
 * Copyright 1994 Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/display.c,v 1.11 1994/08/21 03:08:33 eric Exp eric $
 */

#undef COLOR_DEBUG
#undef COLOR_DEBUG_2


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "misc.h"
#include "display.h"
#include "memory.h"
#include "sim6502.h"  /* for stepflag and traceflag */
#include "game.h"


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


Display *mydisplay;
int myscreen;
int mydepth;
Window mywindow;
GC mygc;
Colormap mycmap;


#define MAX_STAMP_COLORS 16
#define MAX_MOB_COLORS 16
#define MAX_NUM_PLANES 8
#define MAX_COLOR_TABLE_SIZE 32


int stamp_colors;
int mob_colors;
int stamp_planes;
int mob_planes;
int num_planes;
int color_table_size;
int min_stamp_color_table_index;
int max_stamp_color_table_index;
int min_mob_color_table_index;
int max_mob_color_table_index;


unsigned long stamp_pixel [MAX_STAMP_COLORS];
unsigned long stamp_plane_mask;
GC stamp_gc;

unsigned long mob_pixel [MAX_MOB_COLORS];
unsigned long mob_plane_mask;
GC mob_erase_gc;
GC mob_mask_gc;
GC mob_draw_gc [MAX_MOB_COLORS];


#define STAMP_ROWS 32
#define STAMP_COLUMNS 30


#define STAMP_WIDTH  ( 8 * x_scale)
#define STAMP_HEIGHT ( 8 * y_scale)
#define MOB_WIDTH    (16 * x_scale)
#define MOB_HEIGHT   ( 8 * y_scale)


Pixmap stamp_pixmap [256];

Pixmap mob_bitmap [MAX_MOB_COLORS][256];

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

XColor color_value [16];


#define MAX_KEY 256  /* KeyCode is defined as an unsigned char */
int *keymap [MAX_KEY];
int escape_code;

void setup_keyboard (void)
{
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
}


#ifdef COLOR_DEBUG_2
void MyStoreColor (Display *display, Colormap cmap, XColor *color)
{
  printf ("set cmap %d pixel %08x = #%04x%04x%04x\n", cmap, color->pixel,
	  color->red, color->green, color->blue);
  XStoreColor (display, cmap, color);
  XFlush (display);
}
#else
#define MyStoreColor XStoreColor
#endif


void write_color_table (int addr, unsigned char data)
{
  int i;
  XColor color;

#ifdef COLOR_DEBUG_2
  printf ("write_color_table (0x%02x, 0x%02x)\n", addr, data);
#endif

  color.flags = DoRed | DoGreen | DoBlue;

  if (game == MILLIPEDE)
    {
      color.red   = ((data & 0xe0) ^ 0xe0) << 8;
      color.green = ((data & 0x1c) ^ 0x1c) << 11;
      color.blue  = ((data & 0x03) ^ 0x03) << 14;
    }
  else
    color = color_value [data & 0x0f];

  if ((addr >= min_stamp_color_table_index) &&
      (addr <= max_stamp_color_table_index))
    {
      color.pixel = stamp_pixel [addr & (stamp_colors - 1)];
      MyStoreColor (mydisplay, mycmap, & color);
    }
  else if ((addr >= min_mob_color_table_index) &&
	   (addr <= max_mob_color_table_index))
    {
      for (i = 0; i < stamp_colors; i++)
	{
	  color.pixel = mob_pixel [addr & (mob_colors - 1)] | stamp_pixel [i];
	  MyStoreColor (mydisplay, mycmap, & color);
	}
    }
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
	  write_color_table (i, c);
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
	if (c != old_video_mem [a])
	  {
	    XCopyArea (mydisplay, stamp_pixmap [c],
		       mywindow, stamp_gc, 0, 0, STAMP_WIDTH, STAMP_HEIGHT,
		       x * STAMP_WIDTH,
		       ((STAMP_ROWS - 1) - y) * STAMP_HEIGHT);
	    old_video_mem [a] = c;
	  }
      }
}


void erase_mobs (void)
{
  int i;

#if ERASE_MOB_PLANE
  XFillRectangle (mydisplay, mywindow, mob_mask_gc,
		  0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
#else
  for (i = 0; i < 16; i++)
    {
      XFillRectangle (mydisplay, mywindow, mob_erase_gc,
		      last_mob_horizontal [i], last_mob_vertical [i],
		      MOB_WIDTH, MOB_HEIGHT);
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

#ifndef ERASE_MOB_PLANE
      last_mob_horizontal [i] = horizontal;
      last_mob_vertical   [i] = vertical;
#endif

      if (vertical >= (248 * y_scale))
	continue;

      /* first punch a hole */
      XCopyPlane (mydisplay, mob_bitmap [0][picture],
		  mywindow, mob_mask_gc,
		  0, 0, MOB_WIDTH, MOB_HEIGHT, horizontal, vertical, 1);

      /* now draw the mob */
      XCopyPlane (mydisplay, mob_bitmap [1][picture],
		  mywindow, mob_draw_gc [(color >> 4) & 0x03], 
		  0, 0, MOB_WIDTH, MOB_HEIGHT, horizontal, vertical, 1);

      XCopyPlane (mydisplay, mob_bitmap [2][picture],
		  mywindow, mob_draw_gc [(color >> 2) & 0x03],
		  0, 0, MOB_WIDTH, MOB_HEIGHT, horizontal, vertical, 1);

      XCopyPlane (mydisplay, mob_bitmap [3][picture],
		  mywindow, mob_draw_gc [color & 0x03],
		  0, 0, MOB_WIDTH, MOB_HEIGHT, horizontal, vertical, 1);
    }
}


int log2 (int x)
{
  int i;

  for (i = 0; x > 1; i++)
    x >>= 1;
  return (i);
}


void init_colors (void)
{
  int i, j;
  int result;
  int flags = { DoRed | DoGreen | DoBlue };
  unsigned long base_pixel;
  unsigned long plane_mask [MAX_NUM_PLANES];
  XSetWindowAttributes attributes;

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

  stamp_planes = log2 (stamp_colors);
  mob_planes   = log2 (mob_colors);

  num_planes = stamp_planes + mob_planes;

  mycmap = DefaultColormap (mydisplay, myscreen);

  result = XAllocColorCells (mydisplay, mycmap, 0 /* contig */, plane_mask,
			     num_planes, & base_pixel, 1);

  if (! result)
    {
      mycmap = XCreateColormap (mydisplay, mywindow, 
				DefaultVisual (mydisplay, myscreen),
				AllocNone);
      if (! mycmap)
	fatal_error (2, "Can't allocate color cells in default color map, and can't create private color map\n");

      result = XAllocColorCells (mydisplay, mycmap, 0 /* contig */, plane_mask,
				 num_planes, & base_pixel, 1);
      if (! result)
	fatal_error (2, "Can't allocate color cells in private color map\n");
      XSetWindowColormap (mydisplay, mywindow, mycmap);
    }

#ifdef COLOR_DEBUG
  printf ("base_pixel = 0x%08x\n", base_pixel);
  for (i = 0; i < num_planes; i++)
    printf ("plane_mask [%d] = 0x%08x\n", i, plane_mask [i]);
#endif

  for (i = 0; i < stamp_colors; i++)
    {
      stamp_pixel [i] = base_pixel;
      for (j = 0; j < stamp_planes; j++)
	if (i & (1 << j))
	  stamp_pixel [i] |= plane_mask [j];
    }

  stamp_plane_mask == 0;
  for (i = 0; i < stamp_planes; i++)
    stamp_plane_mask |= plane_mask [i];

#ifdef COLOR_DEBUG
  for (i = 0; i < stamp_colors; i++)
    printf ("stamp_pixel [%d] = 0x%08x\n", i, stamp_pixel [i]);
  printf ("stamp_plane_mask = 0x%08x\n", stamp_plane_mask);
#endif

  for (i = 0; i < mob_colors; i++)
    {
      mob_pixel [i] = base_pixel;
      for (j = 0; j < mob_planes; j++)
	if (i & (1 << j))
	  mob_pixel [i] |= plane_mask [stamp_planes + j];
    }

  mob_plane_mask = 0;
  for (i = 0; i < mob_planes; i++)
    mob_plane_mask |= plane_mask [stamp_planes + i];
  
#ifdef COLOR_DEBUG
  for (i = 0; i < mob_colors; i++)
    printf ("mob_pixel [%d] = 0x%08x\n", i, mob_pixel [i]);
  printf ("mob_plane_mask = 0x%08x\n", mob_plane_mask);
#endif

  for (i = 0; i < 16; i++)
    {
      if (! XParseColor (mydisplay, mycmap, color_name [i], & color_value [i]))
	fatal_error (2, "XParseColor of %s failed\n", color_name [i]);
#ifdef COLOR_DEBUG
      printf ("  { \"%s\", \"#%04x%04x%04x\" }, \n", color_name [i],
	      color_value [i].red,
	      color_value [i].green,
	      color_value [i].blue);
#endif
    }

  stamp_gc = XCreateGC (mydisplay, mywindow, 0, 0);
  if (! stamp_gc)
    fatal_error (2, "error creating stamp_gc\n");
  XSetForeground (mydisplay, stamp_gc, stamp_pixel [3]);
  XSetBackground (mydisplay, stamp_gc, stamp_pixel [0]);
  XSetFunction   (mydisplay, stamp_gc, GXcopy);
  XSetPlaneMask  (mydisplay, stamp_gc, stamp_plane_mask);

  mob_erase_gc = XCreateGC (mydisplay, mywindow, 0, 0);
  if (! mob_erase_gc)
    fatal_error (2, "error creating mob_erase_gc\n");
  XSetFunction   (mydisplay, mob_erase_gc, GXclear);
  XSetPlaneMask  (mydisplay, mob_erase_gc, mob_plane_mask);

  mob_mask_gc = XCreateGC (mydisplay, mywindow, 0, 0);
  if (! mob_mask_gc)
    fatal_error (2, "error creating mob_mask_gc\n");
  XSetForeground (mydisplay, mob_mask_gc, mob_pixel [3]);
  XSetBackground (mydisplay, mob_mask_gc, mob_pixel [0]);
  XSetFunction   (mydisplay, mob_mask_gc, GXand);
  XSetPlaneMask  (mydisplay, mob_mask_gc, mob_plane_mask);

  for (i = 0; i < mob_colors; i++)
    {
      mob_draw_gc [i] = XCreateGC (mydisplay, mywindow, 0, 0);
      if (! mob_draw_gc [i])
	fatal_error (2, "error creating mob_draw_gc [%d]\n", i);
      XSetForeground (mydisplay, mob_draw_gc [i], mob_pixel [i]);
      XSetBackground (mydisplay, mob_draw_gc [i], mob_pixel [0]);
      XSetFunction   (mydisplay, mob_draw_gc [i], GXor);
      XSetPlaneMask  (mydisplay, mob_draw_gc [i], mob_plane_mask);
    }

  attributes.background_pixel = stamp_pixel [0];
  attributes.colormap = mycmap;
  XChangeWindowAttributes (mydisplay, mywindow, CWBackPixel | CWColormap,
			   & attributes);
  XClearWindow (mydisplay, mywindow);

}


void init_graphics (int argc, char *argv[])
{
  XSizeHints myhint;
  char *name;
  unsigned long black, white;  /* pixel values */

  mydisplay = XOpenDisplay ("");
  if (!mydisplay)
    fatal_error (2, "Can't init X\n");

  myscreen = DefaultScreen (mydisplay);
  mydepth = DefaultDepth (mydisplay, myscreen);
  white = WhitePixel (mydisplay, myscreen);
  black = BlackPixel (mydisplay, myscreen);

  myhint.x = 50;
  myhint.y = 50;
  myhint.width = WINDOW_WIDTH;
  myhint.height = WINDOW_HEIGHT;
  myhint.flags = PPosition | PSize;

  mywindow = XCreateSimpleWindow
    (mydisplay,
     DefaultRootWindow (mydisplay),
     myhint.x, myhint.y, myhint.width, myhint.height,
     0 /* border width */,
     white, black);

  name = game_name (game);

  XSetStandardProperties (mydisplay, mywindow, name, name,
			  None, argv, argc, &myhint);

  mygc = XCreateGC (mydisplay, mywindow, 0, 0);
  XSetBackground (mydisplay, mygc, black);
  XSetForeground (mydisplay, mygc, white);

  XSetLineAttributes (mydisplay, mygc, 0, LineSolid, CapButt, JoinMiter);

  XMapRaised (mydisplay, mywindow);
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

void kludge (void)
{
  int i;
  for (i = 0; i < 16000; i++)
    {
      /* XClearWindow (mydisplay, mywindow); */
      XDrawLine (mydisplay, mywindow, mygc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
      XFlush (mydisplay);
    }
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


void TestPixmap (Pixmap pixmap, int req_depth)
{
  Window root;
  int x, y;
  unsigned int width, height, border_width, depth;
  Status status;

  status = XGetGeometry (mydisplay, pixmap, & root, & x, & y,
			 & width, & height, & border_width, & depth);
  if (! status )
    fatal_error (2, "XGetGeometry failed\n");
  if (depth != req_depth)
    fatal_error (2, "pixmap_error\n");
}


void create_stamp_pixmaps (void)
{
  int c, x, y;
  XImage *image;
  image = XCreateImage (mydisplay, DefaultVisual (mydisplay, myscreen),
			mydepth, ZPixmap, 0, 0, STAMP_WIDTH, STAMP_HEIGHT,
			(mydepth > 8) ? 32 : 8,
			0);
  if (! image)
    fatal_error (2, "XCreateImage failed\n");
  image->data = (char *) malloc (image->bytes_per_line * STAMP_HEIGHT);
  if (! image->data)
    fatal_error (2, "malloc for image failed\n");

  for (c = 0; c < 256; c++)
    {
      stamp_pixmap [c] = XCreatePixmap (mydisplay, mywindow,
					STAMP_WIDTH, STAMP_HEIGHT, mydepth);
      /* TestPixmap (stamp_pixmap [c], mydepth); */
      for (y = 0; y < STAMP_HEIGHT; y++)
	for (x = 0; x < STAMP_WIDTH; x++)
	  {
	    if (game == MILLIPEDE)
	      {
		if (! XPutPixel (image, x, y, stamp_pixel [get_stamp_pixel (c, x, y) | (c & 0xc0) >> 4]))
		  fatal_error (2, "XPutPixel failed\n");
	      }
	    else
	      {
		if (! XPutPixel (image, x, y, stamp_pixel [get_stamp_pixel (c, x, y)]))
		  fatal_error (2, "XPutPixel failed\n");
	      }
	  }
      XPutImage (mydisplay, stamp_pixmap [c], mygc, image, 0, 0, 0, 0, STAMP_WIDTH, STAMP_HEIGHT);
    }

  free (image->data);
  image->data = NULL;
  XFree (image);
}


void set_bitmap (char *data, int row_size, int x, int y)
{
  data += y * row_size + (x / 8);
  *data |= (1 << (x & 7));
}


void create_mob_bitmaps (void)
{
  int c, x, y;
  int i;
  int row_size;
  char *bitmap_data [MAX_MOB_COLORS];

  row_size = (MOB_WIDTH + 7) / 8;
  /* printf ("mob bitmap data row size %d\n", row_size); */

  for (i = 0; i < mob_colors; i++)
    {
      bitmap_data [i] = (char *) malloc (row_size * MOB_HEIGHT);
      if (! bitmap_data [i])
	fatal_error (2, "can't malloc bitmap data for mobs\n");
    }

  for (c = 0; c < 256; c++)
    {
      for (i = 0; i < mob_colors; i++)
	memset (bitmap_data [i], 0, row_size * MOB_HEIGHT);

      for (x = 0; x < MOB_WIDTH; x++)
	for (y = 0; y < MOB_HEIGHT; y++)
	  set_bitmap (bitmap_data [get_mob_pixel (c, x, y)], row_size, x, y);

      for (i = 0; i < mob_colors; i++)
	{
	  mob_bitmap [i][c] = XCreateBitmapFromData (mydisplay, mywindow,
						     bitmap_data [i],
						     MOB_WIDTH, MOB_HEIGHT);
	  TestPixmap (mob_bitmap [i][c], 1);
	}
    }

  for (i = 0; i < 4; i++)
    free (bitmap_data [i]);
}


void handle_events (void)
{
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
}


static int m = 0;


void update_display (void)
{
  int i;
  char *argv [2] = { "sim", NULL };

  if (! initialized)
    {
      read_charset_roms ();

      init_graphics (1, argv);

      setup_keyboard ();

      if (need_kludge)
	kludge ();

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
    }
}
