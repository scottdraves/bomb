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

static char *wave_c_id = "@(#) $Id: wave.c,v 1.1.1.1 2002/12/08 20:49:54 spotspot Exp $";

/* by nix@cs.cmu.edu

 wave simulations

cell attributes: 
 value
 velocity
 stiffness is needed to do lenses
parameters: 
 global stiffness - fixed for now
 damping - use "speed"

 */

#include "defs.h"

#define SPOTS 16

typedef struct {
    float x,y;
    float cx,cy;	/* center */
    float a;		/* current angle from center */
    float r;		/* radius */
    float speed;	/* radians/sec */
    int steps;		/* how many steps (of the given speed) each
			   frame - move the spot faster without strobing */
    int value;
} Spot;

static Spot spots[SPOTS];

void step_rule_wave(int frame, rule_t *p, image8_t *fb)
{
   int x,y,i,j;
   board_t *sboard, *dboard;
   board_t *svboard, *dvboard;
   u_char *lp0, *lp1;
   int stiffness = 64;

   sboard = &board[dbuf];
   dboard = &board[1-dbuf];
   svboard = &board2[dbuf];
   dvboard = &board2[1-dbuf];
   dbuf = 1-dbuf;

   /* torus */
   for (y=0;y<=SMALL_YSIZE+1;y++) {
      (*sboard)[0][y] = (*sboard)[SMALL_XSIZE][y];
      (*sboard)[SMALL_XSIZE+1][y] = (*sboard)[1][y];
      (*sboard)[0][y] = (*sboard)[SMALL_XSIZE][y];
      (*sboard)[SMALL_XSIZE+1][y] = (*sboard)[1][y];
      (*svboard)[0][y] = (*svboard)[SMALL_XSIZE][y];
      (*svboard)[SMALL_XSIZE+1][y] = (*svboard)[1][y];
      (*svboard)[0][y] = (*svboard)[SMALL_XSIZE][y];
      (*svboard)[SMALL_XSIZE+1][y] = (*svboard)[1][y];
   }
   for (x=0;x<=SMALL_XSIZE+1;x++) {
      (*sboard)[x][0] = (*sboard)[x][SMALL_YSIZE];
      (*sboard)[x][SMALL_YSIZE+1] = (*sboard)[x][1];
      (*sboard)[x][0] = (*sboard)[x][SMALL_YSIZE];
      (*sboard)[x][SMALL_YSIZE+1] = (*sboard)[x][1];
      (*svboard)[x][0] = (*svboard)[x][SMALL_YSIZE];
      (*svboard)[x][SMALL_YSIZE+1] = (*svboard)[x][1];
      (*svboard)[x][0] = (*svboard)[x][SMALL_YSIZE];
      (*svboard)[x][SMALL_YSIZE+1] = (*svboard)[x][1];
   }

   for(y=1;y<=SMALL_YSIZE;y++) {
      lp0 = fb->p + (fb->stride * (y - 1));
      lp1 = lp0 + fb->stride * SMALL_YSIZE;
      for(x=1;x<=SMALL_XSIZE;x++) {
	 int dt, t;
	 /* target is average of nearest neighbors */
	 dt = ((((*sboard)[x  ][y-1]) +
	        ((*sboard)[x-1][y  ]) +
	        ((*sboard)[x  ][y+1]) +
	        ((*sboard)[x+1][y  ])) >> 2)
	     - (*sboard)[x][y];

	 t = (*svboard)[x][y] + ((stiffness * dt) >> 8);
	 t += (p->speed * t) >> 8;
	 if (t > p->mask)
	    t = p->mask;
	 else if (t < -p->mask)
	    t = -p->mask;

	 (*dvboard)[x][y] = t;

	 t = (*sboard)[x][y] + (*dvboard)[x][y];
	 (*dboard)[x][y] = t + ((p->speed * t) >> 8);

	 t += (p->mask >> 2);

/*
	 if (t<0) t = 0;
	 if (t > 255) t = 255;
 */
	 /* 
	 t &= p->mask;
	 if (p->remap)
	    t = remap[t];
          */
	 t = remap[t & p->mask];

	 lp0[SMALL_XSIZE] = t;
	 *lp0++ = t;
	 lp1[SMALL_XSIZE] = t;
	 *lp1++ = t;
      }
   }

   for (i=0; i<SPOTS; i++) {
      Spot *s = spots + i;
      for (j = 0; j < s->steps; j++) {
	 s->a += s->speed;
	 s->x = s->r * cos(s->a) + s->cx;
	 s->y = s->r * sin(s->a) + s->cy;

#if 0
	 if (s->x > 0 && s->x < SMALL_XSIZE
	     && s->y > 0 && s->y < SMALL_YSIZE) {
	    int *pressure = &(*dboard)[(int)s->x][(int)s->y];
	    *pressure = p->mask;
	 }
#else
         {
	     int tx = ((int)s->x) % SMALL_XSIZE;
	     int ty = ((int)s->y) % SMALL_YSIZE;
	     if (tx < 0) tx += SMALL_XSIZE;
	     if (ty < 0) tx += SMALL_YSIZE;
	     (*dboard)[tx][ty] = p->mask;
	 }
#endif
      }
      if (0 == R%20) {
	 float ncx, ncy;
	 /*
	    printf("spot %d: x:%f y:%f cx:%f cy:%f r:%f a:%f da:%f\n",
	    i, s->x, s->y, s->cx, s->cy, s->r, s->a, s->speed);
	    */
	 /* pick a new center */
	 /* make it be colinear with cx,cy - x,y for G2 continuity */
	 /* this keeps the same radius */
	 ncx = s->x - s->cx + s->x;
	 ncy = s->y - s->cy + s->y;
#if 0
	 if (ncx > s->r/3 && ncx < (SMALL_XSIZE - s->r/3)
	     && ncy > s->r/3 && ncy < (SMALL_YSIZE-s->r/3)) {
#endif
	    s->cx = ncx;
	    s->cy = ncy;
	    s->r = sqrt((s->x - s->cx)*(s->x - s->cx) +
			(s->y - s->cy)*(s->y - s->cy));

	    s->a = atan2(s->y - s->cy, s->x - s->cx);
	    /* could make speed = c/radius to get constant velocity */
	    s->speed = -s->speed;
#if 0
	 }
#endif

	 /*
	    printf("spot %d: x:%f y:%f cx:%f cy:%f r:%f a:%f da:%f\n",
	    i, s->x, s->y, s->cx, s->cy, s->r, s->a, s->speed);
	    */
      }

   }
}

void init_wave()
{
   int i;
   for(i=0;i<SPOTS;i++) {
      Spot *s = spots + i;
      s->x = R%SMALL_XSIZE;
      s->y = R%SMALL_YSIZE;
      s->cx = R%60 - 30 + s->x;
      s->cy = R%60 - 30 + s->y;
      s->r = sqrt((s->x - s->cx)*(s->x - s->cx) +
		  (s->y - s->cy)*(s->y - s->cy));
      s->a = atan2(s->y - s->cy, s->x - s->cx);
      s->speed = 0.0125;
      s->steps = 8;
   }
}
