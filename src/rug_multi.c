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

static char *rug_multi_c_id = "@(#) $Id: rug_multi.c,v 1.1.1.1 2002/12/08 20:49:51 spotspot Exp $";

#include "defs.h"

#define SHOW_DRIVER 0

void step_rule_rug_multi(int frame, rule_t *p, image8_t *fb)
{
   int x, y;
   board_t *s_heat_board, *d_heat_board;
   board_t *s_game_board, *d_game_board;
   u_char *lp0, *lp1;
   static int ptab[] = {1, 2, 4, 5};
   int nplanes = ptab[iclamp(p->brain_shift, 4)];
   int stripe_height = SMALL_YSIZE / nplanes;
   static int phase = 0;
   int slowdown = nplanes * p->driver_slowdown;
   if (++phase >= nplanes)
      phase = 0;

   dbuf = frame&1;
   s_heat_board=&board[dbuf];
   d_heat_board=&board[1-dbuf];

   dbuf = (frame/slowdown)&1;
   s_game_board=&board2[dbuf];
   d_game_board=&board2[1-dbuf];

   for(y=0;y<=SMALL_YSIZE+1;y++) {
      (*s_game_board)[0][y] = (*s_game_board)[SMALL_XSIZE][y];
      (*s_game_board)[SMALL_XSIZE+1][y] = (*s_game_board)[1][y];
      (*s_heat_board)[0][y] = (*s_heat_board)[SMALL_XSIZE][y];
      (*s_heat_board)[SMALL_XSIZE+1][y] = (*s_heat_board)[1][y];
   }
   for(x=0;x<=SMALL_XSIZE+1;x++) {
      (*s_game_board)[x][0] = (*s_game_board)[x][SMALL_YSIZE];
      (*s_game_board)[x][SMALL_YSIZE+1] = (*s_game_board)[x][1];
      (*s_heat_board)[x][0] = (*s_heat_board)[x][SMALL_YSIZE];
      (*s_heat_board)[x][SMALL_YSIZE+1] = (*s_heat_board)[x][1];
   }

   if (0 == ((frame+1) % slowdown))
      for (y = 1; y<=SMALL_YSIZE; y++) {
	 for (x=1; x<=SMALL_XSIZE; x++) {
	    int ht = (((*s_game_board)[x  ][y  ]) +
		      (*s_game_board)[x+1][y  ] +
		      (*s_game_board)[x-1][y  ] +
		      (*s_game_board)[x  ][y+1] +
		      (*s_game_board)[x  ][y-1])/5;
	    (*d_game_board)[x][y] = (ht - 1) & 471;
	 }
      }

   for (y=1; y<=SMALL_YSIZE; y++) {
      lp0 = fb->p + (fb->stride * (y - 1));
      lp1 = lp0 + fb->stride * SMALL_YSIZE;
      for (x=1; x<=SMALL_XSIZE; x++) {
	 int heat, dh;
	 int driver, hn, hs, he, hw, hc, denom;
	 int gy;

	 driver = 0;
	 for (gy = 1 + ((y-1)/nplanes); gy <= SMALL_YSIZE; gy += stripe_height) {
	    int v;
	    v = (*s_game_board)[x][gy];
	    driver = ((driver<<1) | (v < 20));
	 }

	 hn = (*s_heat_board)[x  ][y-1];
	 hs = (*s_heat_board)[x  ][y+1];
	 he = (*s_heat_board)[x+1][y  ];
	 hw = (*s_heat_board)[x-1][y  ];
	 hc = (*s_heat_board)[x  ][y  ];
	 denom = 5;
	 heat = hn + hs + he + hw + hc;
	 dh = p->speed;
	 if (driver&0x1)   { heat -= hn; denom--;}
	 if (driver&0x2)   { heat -= hs; denom--;}
	 if (driver&0x4)   { heat -= he; denom--;}
	 if (driver&0x8)   { heat -= hw; denom--;}
	 heat = ((heat / denom) + dh) & p->mask;
	 (*d_heat_board)[x][y] = heat;

#if SHOW_DRIVER
	 p1 = iclamp(p1, 2 + nplanes);
	 switch (p1) {
	  case 0:
	    // if (p->remap)
	       heat = remap[heat];
	    break;
	  case 1:
	    heat = driver ^ (driver >> 8) ^ (driver >> 16);
	    break;
	  default:
	    heat = (driver & (1 << (p1 - 2))) ? 0 : 100;
	    break;
	 }
#else
	 // if (p->remap)
	    heat = remap[heat];
#endif

	 lp0[SMALL_XSIZE] = heat;
	 *lp0++ = heat;
	 lp1[SMALL_XSIZE] = heat;
	 *lp1++ = heat;
      }
   }
}
