/* nsfinfo : get/set nsf file info.
 *
 * by benjamin gerard <ben@sashipa.com> 2003/04/18
 *
 * This program supports nsf playing time calculation extension.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU Library General 
 * Public License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Library General Public License for more details.  To obtain a 
 * copy of the GNU Library General Public License, write to the Free 
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Any permitted reproduction of these routines, in whole or in part,
 * must bear this legend.
 */

/*
	This file has been modified by Matthew Strait so that it can be
	used to do playing time calculation without using ben's idea of
	changing the NSF file format.  He wrote this as a stand-alone
	program.  I have modified it so that the parts I want are called
	from nosefart.  I also modified the calculation algorithm 
	somewhat.
	
	For this to work, the changes he made to other files in this source
	tree have been left alone.  Therefore, there may be references to 
	the "NSF2" file format in the code, even though this version of
	nosefart does not use it.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "nes6502.h"
#include "types.h"
#include "nsf.h"
#include "config.h"

static int quiet = 0;

#ifdef NSF_PLAYER
#define  NES6502_4KBANKS
#endif

#ifdef NES6502_4KBANKS
#define  NES6502_NUMBANKS  16
#define  NES6502_BANKSHIFT 12
#else
#define  NES6502_NUMBANKS  8
#define  NES6502_BANKSHIFT 13
#endif

extern uint8 *acc_nes6502_banks[NES6502_NUMBANKS];
extern int max_access[NES6502_NUMBANKS];

static void msg(const char *fmt, ...)
{
  va_list list;
  if (quiet) 
    return;
  
  va_start(list, fmt);
  vfprintf(stderr, fmt, list);
  va_end(list);
}

static void lpoke(uint8 *p, int v)
{
  p[0] = v;
  p[1] = v >> 8;
  p[2] = v >> 16;
  p[3] = v >> 24;
}


static int nsf_playback_rate(nsf_t * nsf)
{
  uint8 * p;
  unsigned int def, v;
  
  
  if (nsf->pal_ntsc_bits & NSF_DEDICATED_PAL) {
    p = (uint8*)&nsf->pal_speed;
    def = 50;
  } else {
    p = (uint8*)&nsf->ntsc_speed;
    def = 60;
  }
  v = p[0] | (p[1]<<8);
  return v ? 1000000 / v : def;
}

static int nsf_inited = 0;

static unsigned int nsf_calc_time(nsf_t * src,
  int len,  int track,  unsigned int frame_frag, int force)
{
  unsigned int full_time, introless_time;
  nsf_t * nsf = NULL;
  float sec; 
  unsigned int playback_rate = nsf_playback_rate(src);
  int err;
  unsigned int max_frag;

  // trouble with zelda2:7?
  int default_frag_size = 20 * playback_rate; // 2 * 60 * playback_rate; 

  if (track < 0 || track > src->num_songs) {
    fprintf(stderr, "nsfinfo : calc time, track #%d out of range.\n", track);
  }
  /* always called with frame_frag == 0 --matt s. */
  frame_frag = frame_frag ? frame_frag : default_frag_size;
  max_frag = 60 * 60 * playback_rate;

  if (!nsf_inited) {
    //msg("nsfinfo : init nosefart engine...\n");
    if (nsf_init() == -1) {
      fprintf(stderr, "nsfinfo :  init failed.\n");
      goto error;
    }
    nsf_inited = 1;
    //msg("nsfinfo : nosefart engine initialized\n");
  }

  //msg("nsfinfo : loading nsf...\n");
  nsf = nsf_load(NULL, src, len);
  if (!nsf) {
    fprintf(stderr,"nsfinfo: load failed\n");
    goto error;
  }
  if (!force && nsf->song_frames && nsf->song_frames[track]) {
    full_time = nsf->song_frames[track];
    goto error; /* Not en error :) */
  }

  //msg("nsfinfo : init [%s] #%d...\n", nsf->song_name, track);
  /* ben : Don't care about sound format here,
   * since it will not be rendered.
   */
  err = nsf_playtrack(nsf, track, 8000, 8, 1);
  if (err != track) {
    if (err == -1) {
      /* $$$ ben : Becoz nsf_playtrack() kicks nsf ass :( */
      nsf = NULL;
    }
    fprintf(stderr,"nsfinfo: track %d not initialized\n", track);
    goto error;
  }

  /* the way this works is that it finds the last place that new memory 
     is accessed.  The time it takes to do this is the length of the song.
     Well, sorta.  It's the time after which no new material is played.
     If the song has an intro which is not repeated, it counts that as part
     of the length.  (And if a song goes A B C B B B B ..., it will be the
     length of A B C.  I don't think I've encountered this, however, 
     although I think it could happen.)
     -matt s. 
  */
  {
    int done = 0;
    uint32 last_accessed_frame = 0, prev_frag = 0, starting_frame = 0;

    //msg("nsfinfo : Emulating up to %u frames (%d hz)\n", frame_frag, playback_rate);

    while (!done) 
    {
      nsf_frame(nsf); /* advance one frame. -matt s. */

      //msg("%d ", nsf->cur_frame);
      if (nes6502_mem_access)
      {
        //msg("!");
	last_accessed_frame = nsf->cur_frame;
      }
      //msg("\n");

      if (nsf->cur_frame > frame_frag) 
      {
        if (last_accessed_frame > prev_frag) 
        {
	  prev_frag = nsf->cur_frame;
	  frame_frag += default_frag_size;
	  //msg("nsfinfo : memory was accessed, enlarging search to next %u frames\n",
	  //    default_frag_size);
	  
          if (frame_frag >= max_frag) 
          {
	    msg("\nnsfinfo : unable to find end of music within %u frames, giving up!\n", max_frag);
	    goto error;
	  }
	} 
        else 
	  done = 1;
      }
    }
    full_time = last_accessed_frame + 16 /* fudge room */;
    sec = (float)(full_time + nsf->playback_rate - 1) / (float)nsf->playback_rate;
    //printf("track %d with intro is %u frames, %.2f seconds\n",
	//track, full_time, sec);

    // doesn't make a difference
    //nsf->cur_frame = last_accessed_frame;
  }

  /* clear out the memory access information.  This is a kludge
     because I, matt s, don't totally understand what ben is doing! 
     max_access information is collected in src/cpu/nes6502/nes6502.c */
  {
    int a;
    for(a = 0; a < NES6502_NUMBANKS; a++)
    {
	//msg("max_access[%d] = %d\n", a, max_access[a]);
  	memset(acc_nes6502_banks[a], 0, max_access[a]);
    }
  }

  //msg("starting second run\n");

  /* This finds the length of the song _without_ the intro. -matt s */
  {
    /* don't want to count what we've already looked at */
    int starting_frame = nsf->cur_frame; 

    int done = 0;
    uint32 last_accessed_frame = 0, prev_frag = 0;

    //msg("nsfinfo : Emulating up to %u frames (%d hz)\n", frame_frag, playback_rate);

    while (!done) 
    {
      nsf_frame(nsf); /* advance one frame. -matt s. */

      //msg("%d ", nsf->cur_frame - starting_frame);
      if (nes6502_mem_access)
      {
        //msg("!");
	last_accessed_frame = nsf->cur_frame;
      }
      //msg("\n");

      if (nsf->cur_frame > frame_frag) 
      {
        if (last_accessed_frame > prev_frag) 
        {
	  prev_frag = nsf->cur_frame;
	  frame_frag += default_frag_size;
	  //msg("nsfinfo : memory was accessed, enlarging search to next %u frames\n",
	  //    default_frag_size);
	  
          if (frame_frag >= max_frag) 
          {
	    msg("\nnsfinfo : unable to find end of music within %u frames\n\tgiving up!", max_frag);
	    goto error;
	  }
	} 
        else 
	  done = 1;
      }
    }
    introless_time = last_accessed_frame - starting_frame + 16 /* fudge room */;
    //sec = (float)(introless_time + nsf->playback_rate - 1) / (float)nsf->playback_rate;
    //printf("track %d without intro is %u frames, %.2f seconds\n",
//	track, introless_time, sec);
  }

  /* Want to get both results back to nosefart.  Neither should be 
     larger than 2^16, so let's shove them together into one int.
     the high order bits are full_time and the lower order
     bits are introless_time */
	return (full_time * 0x1000 + introless_time);

 error:
  nsf_free(&nsf);
  fprintf(stderr, "Error with time calculation, bailing out!\n");
  return 0x00000001; /* something small, but non-zero (zero means unlimited) */
}


static char * clean_string(char *d, const char *s, int max)
{
  int i;

  --max;
  for (i=0; i<max && s[i]; ++i) {
    d[i] = s[i];
  }
  for (; i<=max; ++i) {
    d[i] = 0;
  }

  return (char*)d;
}

static int get_integer(const char * arg, const char * opt, int * pv)
{
  char * end;
  long v;

  if (strstr(arg,opt) != arg) {
    return 1;
  }
  arg += strlen(opt);
  end = 0;
  v = strtol(arg, &end, 0);
  if (!end || *end) {
    return -1;
  }
  *pv = v;
  return 0;
}

static int get_string(const char * arg, const char * opt, char * v, int max)
{
  if (strstr(arg,opt) != arg) {
    return 1;
  }
  clean_string(v, arg + strlen(opt), max);
  return 0;
}

static int track_number(char **ps, int max)
{
  int n;
  if (!isdigit(**ps)) {
    return -1;
  }
  n = strtol(*ps, ps, 10);
  if (n<0 || n>max) {
    fprintf(stderr,"nsfinfo : track #%d out of range\n", n);
    return -1;
  } else if (n==0) {
    n = max;
  }
  return n;
}

static int read_track_list(char **trackList, int max, int *from, int *to)
{
  int fromTrack, toTrack;
  char *t = *trackList;

  if (t) {
    /* Skip comma ',' */
    while(*t == ',') {
      ++t;
    }
  }

  /* Done with this list. */
  if (!t || !*t) {
/*     msg("track list finish here\n"); */
    *trackList = 0;
    return 0;
  }

/*   msg("parse:%s\n", t); */
  *trackList = t;
  fromTrack = track_number(trackList, max);

/*   msg("-> %d [%s]\n", fromTrack, *trackList); */

  if (fromTrack < 0) {
    return -1;
  }

  switch(**trackList) {
  case ',': case 0:
    toTrack = fromTrack;
    break;
  case '-':
    (*trackList)++;
    toTrack = track_number(trackList, max);
    if (toTrack < 0) {
      return -2;
    }
    break;
  default:
    return -1;
  }

  *from = fromTrack;
  *to   = toTrack;

/*   msg("from:%d, to:%d [%s]\n", fromTrack, toTrack, *trackList); */

  return 1;
}

int time_info(const char * iname, int track) {
  char * buffer = 0;
  int len = 0;
  nsf_t * nsf;

  static unsigned int times[256];

  //msg("Loading [%s] file.\n", iname);
  /* Load whole file. */
  {
    FILE * f = fopen(iname, "rb");
    if (!f) {
      perror(iname);
      return 2;
    }

    if (fseek(f,0,SEEK_END) == -1) {
      perror(iname);
    } else {
      len = ftell(f);
      if (len == -1) {
        perror(iname);
      } else if (len < 128) {
        fprintf(stderr, "nsfinfo : %s not an nsf file (too small).\n", iname);
      } else {
	char tmp_hd[128];
	//msg("File length is %d bytes.\n", len);
	int err = fseek(f,0,SEEK_SET);
	if (err != -1) {
	  err = fread(tmp_hd, 1, 128, f);
	}
	if (err != 128) {
	  perror(iname);
	} else if (memcmp(tmp_hd,NSF_MAGIC,5)) {
	  fprintf(stderr, "nsfinfo : %s not an nsf file (invalid magic).\n",
		  iname);
	} else {
	  //msg("Looks like a valid NSF file, loading data ...\n");
	  buffer = malloc(len);
	  if (!buffer) {
	    perror(iname);
	  } else {
	    memcpy(buffer, tmp_hd, 128);
	    err = fread(buffer+128, 1, len-128, f);
	    if (err != len-128) {
	      perror(iname);
	      free(buffer);
	      buffer = 0;
	    }
	  }
	}
      }
    }
    fclose(f);
  }
  if (!buffer) {
    return 3;
  }
  //msg("Successfully loaded [%s].\n", iname);

  nsf = (nsf_t *)buffer;
  clean_string((char*)nsf->song_name, (char*)nsf->song_name, sizeof(nsf->song_name));
  clean_string((char*)nsf->artist_name, (char*)nsf->artist_name, sizeof(nsf->artist_name));
  clean_string((char*)nsf->copyright, (char*)nsf->copyright, sizeof(nsf->copyright));
  memset(times,0,sizeof(times));

  return nsf_calc_time(nsf, len, track, 0, 1);
}
