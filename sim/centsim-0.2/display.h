/*
 * display.h: X display interface for Centipede simulator
 *
 * Copyright 1994 Eric Smith
 *
 * $Header: /home/marble/eric/vg/atari/centsim/RCS/display.h,v 1.3 1994/08/10 05:41:32 eric Exp $
 */

extern int need_kludge;  /* workaround for bug in X386-SGCS & Xaccel servers */
extern int x_scale;
extern int y_scale;
extern int ints_per_frame;

extern unsigned short color_base_address;
extern unsigned short video_base_address;

void update_display (void);
