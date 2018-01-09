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

static char *rug_rug_c_id = "@(#) $Id: rug_rug.c,v 1.1.1.1 2002/12/08 20:49:51 spotspot Exp $";

#include "defs.h"


/* to eliminate blockiness, consider annealing the results of the first rug
   before driving the second.  or just figure out how to run a high
   resolution rug that boils at low spatial frequencies, perhapse by
   increasing the speed of light. */

static int slow_frames = 0;

void step_rule_rug_rug(int frame, rule_t *p, image8_t *fb)
{
   int x, y;
   // static int debt = 0;
   board_t *s_heat_board, *d_heat_board;
   board_t *s_game_board, *d_game_board;
   u_char *lp;

   dbuf = frame&1;
   s_heat_board=&board[dbuf];
   d_heat_board=&board[1-dbuf];
   dbuf = slow_frames&1;
   s_game_board=&board2[dbuf];
   d_game_board=&board2[1-dbuf];

   if (-1 == p->driver_slowdown ||
	   ((p->driver_slowdown > 0) &&
	    0 == (frame % p->driver_slowdown))) {
	   slow_frames++;

   /* torus */
   for(y=0;y<=VSMALL_YSIZE+1;y++) {
      (*s_game_board)[0][y] = (*s_game_board)[VSMALL_XSIZE][y];
      (*s_game_board)[VSMALL_XSIZE+1][y] = (*s_game_board)[1][y];
   }
   for(x=0;x<=VSMALL_XSIZE+1;x++) {
      (*s_game_board)[x][0] = (*s_game_board)[x][VSMALL_YSIZE];
      (*s_game_board)[x][VSMALL_YSIZE+1] = (*s_game_board)[x][1];
   }

      for(y=1;y<=VSMALL_YSIZE;y++) {
	 for(x=1;x<=VSMALL_XSIZE;x++) {
	    int heat;
	    heat = ((((*s_game_board)[x  ][y-1]) +
		     ((*s_game_board)[x-1][y  ]<<1) +
		     ((*s_game_board)[x  ][y  ]<<1) +
		     ((*s_game_board)[x+1][y  ]<<1) +
		     ((*s_game_board)[x  ][y+1])) >> 3);
	    heat--;
	    heat &= 471;
	    (*d_game_board)[x][y]= heat;
	 }
      }
	  }

   for(y=0;y<=YSIZE+1;y++) {
      (*s_heat_board)[0][y] = (*s_heat_board)[XSIZE][y];
      (*s_heat_board)[XSIZE+1][y] = (*s_heat_board)[1][y];
   }
   for(x=0;x<=XSIZE+1;x++) {
      (*s_heat_board)[x][0] = (*s_heat_board)[x][YSIZE];
      (*s_heat_board)[x][YSIZE+1] = (*s_heat_board)[x][1];
   }

   for (y=1;y<=YSIZE;y++) {
      lp = fb->p + (fb->stride * (y - 1));
      for (x=1;x<=XSIZE;x++) {
	 int heat, t;
	 heat = ((((*s_heat_board)[x  ][y-1]<<1) +
		  ((*s_heat_board)[x-1][y  ]) +
		  ((*s_heat_board)[x  ][y  ]<<1) +
		  ((*s_heat_board)[x+1][y  ]) +
		  ((*s_heat_board)[x  ][y+1]<<1)) >> 3);
	 /* just >> rounds wrong, but close enough */
	 t = (*s_game_board)[x>>3][y>>3];
	 t = 20 < t;
	 if (t)
	    heat += p->speed;
	 else
	    heat -= p->speed;

	 heat &= p->mask;
	 
	 (*d_heat_board)[x][y]= heat;
	// if (p->remap)
	    heat = remap[heat];
	 /* heat = t ? 0 : 100; */
	 *(lp++) = heat;
      }
   }
}
