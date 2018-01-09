/*
    bomb - automatic interactive visual stimulation
    Copyright (C) 1995  Scott Draves <spot@cs.cmu.edu>

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

static char *slip_c_id = "@(#) $Id: slip.c,v 1.1.1.1 2002/12/08 20:49:51 spotspot Exp $";

#include "defs.h"
#include "bomb.h"

void
blit(image8_t *fb, int x, int y, int dx, int dy, int bsize) {
  u_char *line_base, *p;
  int blit_offset, ix, iy, i, j;
	 
  blit_offset = fb->stride * dy + dx;
  line_base = fb->p + x + (y * fb->stride);

  if (dx > 0) {
    ix = -1;
    line_base += bsize - 1;
  } else
    ix = 1;
  if (dy > 0) {
    iy = -fb->stride;
    line_base += fb->stride * (bsize - 1);
  } else
    iy = fb->stride;
  /* gcc is not compiling this code well. */
  if (p1&1)
    for (j = 0; j < bsize; j++) {
      p = line_base;
      for (i = 0; i < bsize; i++) {
	p[blit_offset] = *p+1;
	p += ix;
      }
      line_base += iy;
    }
  else
    for (j = 0; j < bsize; j++) {
      p = line_base;
      for (i = 0; i < bsize; i++) {
	p[blit_offset] = *p;
	p += ix;
      }
      line_base += iy;
    }
}

void
step_rule_slip(int frame, rule_t *p, image8_t *fb) {
   int ib;
   int field = iclamp(p->brain, 6);
   int bsize = 30;
   int max_delta = 5;
#if 0
   bsize = p->drift_speed*3;
   if (bsize < 10) bsize = 10;
   else if (bsize > 64) bsize = 64;
#endif
   switch (field) {
    case 4:
      spiral(&board3[0], frame * (6.28 / 560), frame%20, 20);
      if (1) {
	/* creeping edges */
	int b = bsize/2;
	int w = (fb->width-b);
	int h = (fb->height-b);
	switch (R%4) {
	case 0: blit(fb, 0, R%h,  1, 0, b); break;
	case 1: blit(fb, w, R%h, -1, 0, b); break;
	case 2: blit(fb, R%w, 0, 0,  1, b); break;
	case 3: blit(fb, R%w, h, 0, -1, b); break;
	}
      }
      break;
    case 5:
      bsize = 20;
      if (grad_state++ < 20)
	 image2grad(&board2[dbuf], &board3[0], 15, frame%20, 20);
      break;
   }

   /* adjust this to even out frame rate */
   for (ib = 0; ib < 70; ib++) {
      int dx, dy;
      int x = max_delta + R%(fb->width  - (bsize + 2*max_delta));
      int y = max_delta + R%(fb->height - (bsize + 2*max_delta));
      double t, tx, ty, s1, s2;

      switch (field) {
      default:
       case 0:
	 dx = R8b%3 - 1;
	 dy = R8b%3 - 1;
	 break;
       case 1:
	 tx = 2 * x / (double) fb->width - 1;
	 ty = 2 * y / (double) fb->height - 1;
	 s1 = -ty;
	 s2 = tx;
	 dx = quantize(s1);
	 dy = quantize(s2);
	 break;
       case 2:
	 tx = 2 * x / (double) fb->width - 1;
	 ty = 2 * y / (double) fb->height - 1;

	 if (ty < 0) {
	    ty = ty + 0.05;
	    if (ty > 0)
	       ty = 0.0;
	 }
	 if (ty > 0) {
	    ty = ty - 0.05;
	    if (ty < 0)
	       ty = 0.0;
	 }
      
	 t = tx * tx + ty * ty + 1e-5;
	 s1 = 2 * tx * tx / t - 1;
	 s2 = 2 * tx * ty / t;
	 if (0) {
	   double scale = ((p->drift_speed - 10)/20.0 + 1);
	   if (scale > 5.0) scale = 5.0;
	   if (scale < 0.2) scale = 0.2;

	   s1 *= 2 * scale;
	   s2 *= 2 * scale;
	 }
	 dx = quantize(s1);
	 dy = quantize(s2);
	 break;
       case 3:
	 tx = 2 * x / (double) fb->width - 1;
	 ty = 2 * y / (double) fb->height - 1;
	 s1 = tx;
	 s2 = ty;
	 dx = quantize(s1);
	 dy = quantize(s2);
	 break;
       case 4:
	 if (1) {
	    int dxdy = board3[0][x][y];
	    s1 = ((dxdy >> 8) - 128) / 64.0;
	    s2 = ((dxdy & 255) - 128) / 64.0;
	 }
	 dx = quantize(s1);
	 dy = quantize(s2);
	 break;
       case 5:
	 if (1) {
	    int dxdy = board3[0][x][y];
	    int xi = ((dxdy >> 8) - 128);
	    int yi = ((dxdy & 255) - 128);
	    int p = (xi * xi + yi * yi);
	    if (p) {
	       double r = 4.0 / sqrt(p);
	       dx = quantize(-yi * r);
	       dy = quantize(xi * r);
	    } else {
	       dx = R8b%3 - 1;
	       dy = R8b%3 - 1;
	    }
	 }
	 break;
      }

      blit(fb, x, y, dx, dy, bsize);
   }
}
