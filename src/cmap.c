/*
    screen hacks
    Copyright (C) 1992  Scott Draves <spot@cs.cmu.edu>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

static char *cmap_c_id = "@(#) $Id: cmap.c,v 1.1.1.1 2002/12/08 20:49:45 spotspot Exp $";

#include <stdio.h>
#include "defs.h"
#include "cmap.h"

int ncmaps;
int the_cmaps[maxcmaps][256][3];

int
next_contrasting_color(int c, int i, int d)
{
   int nchecked = 0;
   int j = i;
   int dd;
   int max_i = 0;
   int max = 0;
   while (nchecked++ < 250) {
      int t;
      j = (j+1)%256;

      t = (the_cmaps[c][i][0] - the_cmaps[c][j][0]);
      dd = t * t;
      t = (the_cmaps[c][i][1] - the_cmaps[c][j][1]);
      dd += t * t;
      t = (the_cmaps[c][i][2] - the_cmaps[c][j][2]);
      dd += t * t;

      if (dd > max) {
	 max_i = j;
	 max = dd;
      }
      if (dd >= d)
	 return j;
   }
   return max_i;
}

void
init_cmaps()
{
   FILE *in;
   char tag[10];
   char buf[1000];
   ncmaps = 0;

   if (getenv("quick"))
      goto quick;

   sprintf(buf, "%scmap-data", DATA_DIR);
   in = fopen(buf, "r");
   if (NULL == in) {
     message("in is NULL");
   }
   while (1 == fscanf(in, " ( %s9", tag)) {
      int i;
      if (!strcmp(tag, "comment")) {
	 while (')' != fgetc(in));
      } else if (!strcmp(tag, "cmap")) {
	 for (i = 0; i < 256; i++) {
	    int *entry = &the_cmaps[ncmaps][i][0];
	    fscanf(in, " ( %d %d %d )", entry, entry+1, entry+2);
	 }
	 fscanf(in, " )");
	 ncmaps++;
	 if (ncmaps == maxcmaps) {
	    fprintf(stderr, "maxcmaps=%d exceeded, truncating\n", maxcmaps);
	    return;
	 }
      } else
	 fprintf(stderr, "bad tag: %s\n", tag);
   }
 quick:
   if (0 == ncmaps) {
      int i;
      fprintf(stderr, "no cmaps found, using graymap\n");
      for (i = 0; i < 256; i++) {
	 the_cmaps[0][i][0] = i;
	 the_cmaps[0][i][1] = i;
	 the_cmaps[0][i][2] = i;
      }
      ncmaps = 1;
   }
}
