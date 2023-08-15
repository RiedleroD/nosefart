/*
** NsfInfo (c) 2003  benjamin gerard <ben@sashipa.com> and Matthew Strait
** <quadong@users.sf.net>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General 
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

/* Pretty simple.  Give it a file name and a track and it returns the
time (in frames) of the track with its intro in the first 2 bytes and
the time without intro in the last 2 bytes. It's mostly accurate and
pretty fast. */

#include "nsf.h"

typedef struct playback_time_s{
	uint32 intro_frames;
	uint32 loop_frames;
	double intro_seconds;
	double loop_seconds;
} playback_time_t;

playback_time_t* nsf_calc_time(int track, nsf_t* nsf, unsigned int frame_frag);

