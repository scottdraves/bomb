/*
    bomb - automatic interactive visual stimulation
    Copyright (C) 1994  Scott Draves <spot@cs.cmu.edu>

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

static char *static_c_id = "@(#) $Id: static.c,v 1.1.1.1 2002/12/08 20:49:54 spotspot Exp $";

#include "defs.h"

#define imprint_test   1

#define numer          5
#define denom          8
#define lost           (256 - ((256 * numer) / denom))

void
step_rule_static(int frame, rule_t *p, image8_t *fb)
{
   int x,y;
   int r=0,s;
   u_char *lp;
   board_t *s_image_board;
   s_image_board=&board2[0];   

   for (y = 1; y <= YSIZE; y++) {
      lp = fb->p + (fb->stride * (y - 1));
      for (x = 1; x <= XSIZE; x++) {
	 /* fuckers must only give us 31 bits.  it's nuts
	    to use fully random #s here anyway */
#if win_bomb
	 r = rand();
#else
	 r = (x&1) ? (r >> 8) : R;
#endif
	 s = r&255;
#if win_bomb
		if (0 == s) s = 1;
		if (255==s) s = 254;
#endif
#if imprint_test
	 if ((*s_image_board)[x][y]) {
	    *(lp++) = (s * numer) / denom + lost/2;
	 } else
#endif
	    *(lp++) = s;
      }
   }
}
