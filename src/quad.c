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

static char *quad_c_id = "@(#) $Id: quad.c,v 1.1.1.1 2002/12/08 20:49:50 spotspot Exp $";

#include "defs.h"

void
step_rule_quad(int frame, rule_t *p, image8_t *fb) {
   int x,y;	
   board_t *sboard, *dboard;
   int bm = p->brain & 31;
   u_char *lp, *lp1;
   int speed = (p->speed >>2);

   bm = 408 + ((bm>>2) | ((bm&3)<<3));

   sboard = &board[dbuf];
   dboard = &board[1-dbuf];
   dbuf = 1-dbuf;


   for(y=0;y<=YSIZE+1;y++) {
      (*sboard)[0][y] = (*sboard)[SMALL_XSIZE][y];
      (*sboard)[SMALL_XSIZE+1][y] = (*sboard)[1][y];
   }
   for(x=0;x<=SMALL_XSIZE+1;x++) {
      (*sboard)[x][0] = (*sboard)[x][YSIZE];
      (*sboard)[x][YSIZE+1] = (*sboard)[x][1];
   }

   for (y=1;y<=YSIZE;y++) {
      /* why not an additional -1 for x? */
      lp = fb->p + (fb->stride * (y - 1));
      lp1 = fb->p + (fb->stride * (y - 1)) + XSIZE-2;
      for (x=1;x<=SMALL_XSIZE;x++) {
	 int t;
	 int c = (*sboard)[x][y];
	 t = c;
	 t = ((t>>1) * (65535 - t))>>8;
	 t = (bm * t) >> 7;
	 t = (t>>6)&65535;
	 t = (((t*10) + speed +
	       ((*sboard)[x-1][y  ])+
	       (((*sboard)[x  ][y-1])<<1)+
	       (((*sboard)[x+1][y  ]))+
	       (((*sboard)[x  ][y+1])<<1)) >> 4);
	 if (t < 0) t = 0;
	 (*dboard)[x][y] = t;
	 *lp1-- = t;
	 *(lp++) = t;
      }
   }
}
