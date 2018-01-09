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

static char *rug_c_id = "@(#) $Id: rug.c,v 1.1.1.1 2002/12/08 20:49:51 spotspot Exp $";

#include "defs.h"

void
step_rule_rug(int frame, rule_t *p, image8_t *fb) {
   int x,y;	
   board_t *sboard, *dboard;
   u_char *lp;
   int speed = p->speed;

   sboard = &board[dbuf];
   dboard = &board[1-dbuf];
   dbuf = 1-dbuf;

   for (y=1;y<=YSIZE;y++) {
      /* why not an additional -1 for x? */
      lp = fb->p + (fb->stride * (y - 1));
      for (x=1;x<=XSIZE;x++) {
	 int t;
	 t = ((((*sboard)[x  ][y-1])+
	       ((*sboard)[x-1][y  ]<<1)+
	       ((*sboard)[x  ][y  ]<<1)+
	       ((*sboard)[x+1][y  ]<<1)+
	       ((*sboard)[x  ][y+1])
	       + speed) >> 3);
	 if (t < 0) {
	    if (p->floor)
	       t = 0;	 /* hm, just &mask? */
	    else if (p->randomized_underflow)
	       t = R;
	 }
	 t &= p->mask;
	 (*dboard)[x][y]= t;
	 // if (p->remap)
	    t = remap[t];
	 *(lp++) = t;
#if 0
	 /* some kind of non-linearity could be cool, adjusted to mask,
	    what is the distribution of heat values in a rug? */
	 *(lp++) = 20 * log(1+t);
#endif
      }
   }
}

void
step_rule_rug2(int frame, rule_t *p, image8_t *fb) {
   int x, y;
   board_t *sboard, *dboard;
   u_char *lp;
   int bcolor;

   sboard = &board[dbuf];
   dboard = &board[1-dbuf];
   dbuf = 1-dbuf;

   for (y=1;y<=YSIZE;y++) {
      lp = fb->p + (fb->stride * (y - 1));
      bcolor = 0;
      for (x=1;x<=XSIZE;x++) {
	 int t, flat;
	 int m, n, s, e, w;
	 flat = 1;
	 m = (*sboard)[x  ][y  ];
	 n = (*sboard)[x  ][y-1];
	 if (n != m) flat = 0;
	 s = (*sboard)[x  ][y+1];
	 if (s != m) flat = 0;
	 e = (*sboard)[x+1][y  ];
	 if (e != m) flat = 0;
	 w = (*sboard)[x-1][y  ];
	 if (w != m) flat = 0;
	 t = ((n + s + ((e + w + m) << 1) + p->speed) >> 3);
	 if (t < 0) {
	    if (p->floor)
	       t = 0;
	    else if (p->randomized_underflow)
	       t = R;
	 }
	 t &= p->mask;
	 (*dboard)[x][y] = t;
	 // if (p->remap)
	    t = remap[t];
	 if (flat) {
	    t = bcolor;
	 } else if (p->cycle_bkg)
	    bcolor++;
	 *(lp++) = t;
      }
   }
}


void
step_rule_rug3(int frame, rule_t *p, image8_t *fb) {
   int x,y;	
   board_t *sboard, *dboard;
   u_char *lp;
   int speed;
   double k = p->speed * 300.0 / (YSIZE * YSIZE + XSIZE * XSIZE);

   sboard = &board[dbuf];
   dboard = &board[1-dbuf];
   dbuf = 1-dbuf;

   for (y=1;y<=YSIZE;y++) {
     int rbyte = random_byte();
      /* why not an additional -1 for x? */
      lp = fb->p + (fb->stride * (y - 1));
      for (x=1;x<=XSIZE;x++) {
	 int t;
	 double xs = (x - (XSIZE/2));
	 double ys = (y - (YSIZE/2));
	 double sp = k * (xs * xs + ys * ys);
	 int thresh;
	 speed = (int) sp;
	 thresh = (int) ((sp - speed) * 255);
#if 0
	 if (0 && 10 == y && !(x & 15))
	   printf("x=%d ps=%d sp=%g xs=%g ys=%g, speed=%d, thresh=%d, XSIZE=%d k=%g\n",
		  x, p->speed, sp, xs, ys, speed, thresh, XSIZE, k);
#endif
	 t = ((((*sboard)[x  ][y-1])+
	       ((*sboard)[x-1][y  ]<<1)+
	       ((*sboard)[x  ][y  ]<<1)+
	       ((*sboard)[x+1][y  ]<<1)+
	       ((*sboard)[x  ][y+1])
	       + speed + (rbyte > thresh) ) >> 3);
	 if (t < 0) {
	    if (p->floor)
	       t = 0;	 /* hm, just &mask? */
	    else if (p->randomized_underflow)
	       t = R;
	 }
	 t &= p->mask;
	 (*dboard)[x][y]= t;
	 // if (p->remap)
	    t = remap[t];
	 *(lp++) = t;
#if 0
	 /* some kind of non-linearity could be cool, adjusted to mask,
	    what is the distribution of heat values in a rug? */
	 *(lp++) = 20 * log(1+t);
#endif
      }
   }
}

