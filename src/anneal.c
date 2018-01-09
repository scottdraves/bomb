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

static char *anneal_c_id = "@(#) $Id: anneal.c,v 1.1.1.1 2002/12/08 20:49:37 spotspot Exp $";

#include "defs.h"


void step_rule_rug_anneal(int frame, rule_t *p, image8_t *fb)
{
   int x, y;
   static int debt = 0;
   board_t *s_heat_board, *d_heat_board;
   board_t *s_game_board, *d_game_board;
   u_char *lp0, *lp1;

   s_heat_board=&board[dbuf];
   d_heat_board=&board[1-dbuf];
   s_game_board=&board2[dbuf];
   d_game_board=&board2[1-dbuf];
   dbuf = 1-dbuf;

   /* torus */
   for (y=0;y<=SMALL_YSIZE+1;y++) {
      (*s_game_board)[0][y] = (*s_game_board)[SMALL_XSIZE][y];
      (*s_game_board)[SMALL_XSIZE+1][y] = (*s_game_board)[1][y];
      (*s_heat_board)[0][y] = (*s_heat_board)[SMALL_XSIZE][y];
      (*s_heat_board)[SMALL_XSIZE+1][y] = (*s_heat_board)[1][y];
   }
   for (x=0;x<=SMALL_XSIZE+1;x++) {
      (*s_game_board)[x][0] = (*s_game_board)[x][SMALL_YSIZE];
      (*s_game_board)[x][SMALL_YSIZE+1] = (*s_game_board)[x][1];
      (*s_heat_board)[x][0] = (*s_heat_board)[x][SMALL_YSIZE];
      (*s_heat_board)[x][SMALL_YSIZE+1] = (*s_heat_board)[x][1];
   }

   for (y=1;y<=SMALL_YSIZE;y++) {
      lp0 = fb->p + (fb->stride * (y - 1));
      lp1 = lp0 + fb->stride * SMALL_YSIZE;
      for (x=1;x<=SMALL_XSIZE;x++) {
	 int heat, t;
	 heat = ((((*s_heat_board)[x  ][y-1]) +
		  ((*s_heat_board)[x-1][y  ]) +
		  ((*s_heat_board)[x  ][y  ]) +
		  ((*s_heat_board)[x+1][y  ]) +
		  ((*s_heat_board)[x  ][y+1])) / 5);
	 heat &= p->mask;
	 
	 /* ugh, only need '&' cuz of randomized fill pattern */
	 t = (((*s_game_board)[x+1][y+1]&LIVE_BIT)+
	      ((*s_game_board)[x+1][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y+1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x  ][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y  ]&LIVE_BIT)+
	      ((*s_game_board)[x+1][y  ]&LIVE_BIT)+
	      ((*s_game_board)[x  ][y+1]&LIVE_BIT));
	 /* what is this anyway?
	 if (t < 3) t = 0;
	 else if (t > 4) t = 1;
	 else t = 4 - t;
	 */
	 /* just made this up at random */
	 t = (t - debt) > 3;
	 if ((t ^ (*s_game_board)[x][y])&LIVE_BIT) {
	    if (t) {
	       debt++;
	    } else {
	       debt--;
	    }
	 }
	 if (t)
	    heat += p->speed;
	 else
	    heat -= p->speed;
	 (*d_game_board)[x][y]= t;
	 (*d_heat_board)[x][y]= heat;
	 // if (p->remap)
	    heat = remap[heat];

	 lp0[SMALL_XSIZE] = heat;
	 *lp0++ = heat;
	 lp1[SMALL_XSIZE] = heat;
	 *lp1++ = heat;
      }
   }
}

void step_rule_rug_anneal2(int frame, rule_t *p, image8_t *fb)
{
   int x, y;
   static int debt = 0;
   board_t *s_heat_board, *d_heat_board;
   board_t *s_game_board, *d_game_board;
   board_t *save_board;
   u_char *lp0, *lp1;

   s_heat_board=&board[dbuf];
   d_heat_board=&board[1-dbuf];
   s_game_board=&board2[dbuf];
   d_game_board=&board2[1-dbuf];
   save_board=&board3[0];
   dbuf = 1-dbuf;

   /* torus */
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

   /* should be able to do something more interesting here */
   if (0==(frame%40)) {
      for(y=1;y<=SMALL_YSIZE;y++)
	 for(x=1;x<=SMALL_XSIZE;x++)
	    (*save_board)[x][y] = (*s_game_board)[x][y];

      for(y=1;y<=SMALL_YSIZE;y++) {
	 for(x=1;x<=SMALL_XSIZE;x++) {
	    (*s_game_board)[x][y] = R&1;
	 }
      }
   }

   for(y=1;y<=SMALL_YSIZE;y++) {
      lp0 = fb->p + (fb->stride * (y - 1));
      lp1 = lp0 + fb->stride * SMALL_YSIZE;
      for(x=1;x<=SMALL_XSIZE;x++) {
	 int heat, t;
	 heat = ((((*s_heat_board)[x  ][y-1]) +
		  ((*s_heat_board)[x-1][y  ]) +
		  ((*s_heat_board)[x  ][y  ]) +
		  ((*s_heat_board)[x+1][y  ]) +
		  ((*s_heat_board)[x  ][y+1])) / 5);
	 if ((*save_board)[x][y])
	    heat += p->speed;
	 else
	    heat -= p->speed;
	 heat &= p->mask;
	 
	 /* ugh, only need '&' cuz of randomized fill pattern */
	 t = (((*s_game_board)[x+1][y+1]&LIVE_BIT)+
	      ((*s_game_board)[x+1][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y+1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x  ][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y  ]&LIVE_BIT)+
	      ((*s_game_board)[x+1][y  ]&LIVE_BIT)+
	      ((*s_game_board)[x  ][y+1]&LIVE_BIT));

	 /* just made this up at random */
	 t = (t - debt) > 3;
	 if ((t ^ (*s_game_board)[x][y])&LIVE_BIT) {
	    if (t) {
	       debt++;
	    } else {
	       debt--;
	    }
	 }
	 (*d_game_board)[x][y]= t;
	 (*d_heat_board)[x][y]= heat;
	// if (p->remap)
	    heat = remap[heat];
	 /* heat = (*save_board)[x][y] ? 0 : 100; */

	 lp0[SMALL_XSIZE] = heat;
	 *lp0++ = heat;
	 lp1[SMALL_XSIZE] = heat;
	 *lp1++ = heat;
      }
   }
}
