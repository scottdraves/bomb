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

static char *shade_c_id = "@(#) $Id: shade.c,v 1.1.1.1 2002/12/08 20:49:51 spotspot Exp $";

/* by nix@cs.cmu.edu
   integrated by spot Jan 17 1995
   aka wisp

   todo:
   plumes generally too small
   vary turn chance?
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

void step_rule_shade(int frame, rule_t *p, image8_t *fb)
{
   int x,y,i,j;
   board_t *sboard, *dboard;
   u_char *lp;
   int uflow = !p->randomized_underflow;
   int angular;
   static int atime = 0;

   if (atime) {
      angular = 1;
      atime--;
   } else {
      angular = 0;
      if (!(R%200))
	 atime = 20;
   }

   sboard = &board[dbuf];
   dboard = &board[1-dbuf];
   dbuf = 1-dbuf;

   for(y=1;y<=YSIZE;y++) {
      lp = fb->p + (fb->stride * (y - 1));
      for(x=1;x<=XSIZE;x++) {
	 int t;
	 t = ((((*sboard)[x  ][y-1]) +
	       ((*sboard)[x-1][y  ]<<1) +
	       ((*sboard)[x  ][y  ]<<1) +
	       ((*sboard)[x+1][y  ]<<1) +
	       ((*sboard)[x  ][y+1])
	       + p->speed) >> 3);
	 if (uflow && t<0) t = 0;
	 t &= p->mask;
	 (*dboard)[x][y]= t;
	 // if (p->remap)
	    t = remap[t];
	 *(lp++)=t;
      }
   }

   for (i=0; i<SPOTS; i++) {
      Spot *s = spots + i;
      for (j = 0; j < s->steps; j++) {
	 if (3 > i && angular) {
	    if (atime < 10)
	       s->cx += s->speed * s->r;
	    else
	       s->cy += s->speed * s->r;
	 } else {
	    s->a += s->speed;
	 }

	 if (1) {
	   double scale;
	   scale = 1.0;
	   /*
	   scale = ((p->drift_speed-15)/30.0 + 1);
	   if (scale > 2.0) scale = 2.0;
	   else if (scale < 0.2) scale = 0.2;
	   */

	   /* scale = 1 + 0.5 * sin(frame/5.0); */
	   s->x = (int) ((double)s->r * scale * cos(s->a) + (double)s->cx);
	   s->y = (int) ((double)s->r * scale * sin(s->a) + (double)s->cy);
	 }

	 if (s->x > 0 && s->x < XSIZE
	     && s->y > 0 && s->y < YSIZE) {
	    int *pressure = &(*dboard)[(int)s->x][(int)s->y];
	    *pressure = p->mask;
	 }
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
	 if (ncx > s->r/3 && ncx < (XSIZE - s->r/3)
	     && ncy > s->r/3 && ncy < (YSIZE-s->r/3)) {
	    s->cx = ncx;
	    s->cy = ncy;
	    s->r = (int) sqrt((s->x - s->cx)*(s->x - s->cx) +
			(s->y - s->cy)*(s->y - s->cy));

	    s->a = (int) atan2(s->y - s->cy, s->x - s->cx);
	    /* could make speed = c/radius to get constant velocity */
	    s->speed = -s->speed;
	 }

	 /*
	    printf("spot %d: x:%f y:%f cx:%f cy:%f r:%f a:%f da:%f\n",
	    i, s->x, s->y, s->cx, s->cy, s->r, s->a, s->speed);
	    */
      }

#if 0
      /* figure out where spot should move according to the
       * pressure value */

      spots[i].x += spots[i].dx;

      if (spots[i].x >= XSIZE || spots[i].x < 0) {
	 spots[i].dx = -spots[i].dx;
	 spots[i].x += spots[i].dx;
      }
      spots[i].y += spots[i].dy;
      if (spots[i].y >= YSIZE || spots[i].y < 0) {
	 spots[i].dy = -spots[i].dy;
	 spots[i].y += spots[i].dy;
      }
#endif

   }
}

void init_shade()
{
   int i;
   for(i=0;i<SPOTS;i++) {
      Spot *s = spots + i;
      s->x = R%XSIZE;
      s->y = R%YSIZE;
      s->cx = R%60 - 30 + s->x;
      s->cy = R%60 - 30 + s->y;
      s->r = sqrt((s->x - s->cx)*(s->x - s->cx) +
		  (s->y - s->cy)*(s->y - s->cy));
      s->a = atan2(s->y - s->cy, s->x - s->cx);
      s->speed = 0.0125;
      s->steps = 8;
   }
}
