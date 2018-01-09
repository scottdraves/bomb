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

static char *acidlife_c_id = "@(#) $Id: acidlife.c,v 1.1.1.1 2002/12/08 20:49:37 spotspot Exp $";

#include "defs.h"
#include "bomb.h"

#define nbrains 3

int
high_growth_game(rule_t *p)
{
   return ((iclamp(p->brain,nbrains) == 2) &&
	   ((p->rule == rule_acidlife1) ||
	    (p->rule == rule_acidlife2)));
}
     

/* should simplify use of lp? */

void step_rule_acidlife1(int frame, rule_t *p, image8_t *fb)
{
   int x, y, q1;
   board_t *s_heat_board, *d_heat_board;
   board_t *s_game_board, *d_game_board;
   u_char *lp0, *lp1;
   int br = iclamp(p->brain,nbrains);

   s_heat_board=&board[dbuf];
   d_heat_board=&board[1-dbuf];
   s_game_board=&board2[dbuf];
   d_game_board=&board2[1-dbuf];
   dbuf = 1-dbuf;

   switch (br) {
    case 0:
      for (y=0;y<=SMALL_YSIZE;y++) {
	 (*s_game_board)[0][y] = R;
	 x = R;
	 x = (x & 1) && (x & 2);
	 (*s_game_board)[SMALL_XSIZE+1][y] = x;
      }
      for (x=0;x<=SMALL_XSIZE;x++) {
	 (*s_game_board)[x][0] = R;
	 (*s_game_board)[x][SMALL_YSIZE+1] = R;
      }
      break;
    case 1:
      break;
    case 2:
      for (y=0;y<=SMALL_YSIZE;y++) {
	 (*s_game_board)[0][y] = 0;
	 (*s_game_board)[SMALL_XSIZE+1][y] = 0;
	 (*s_heat_board)[0][y] = 0;
	 (*s_heat_board)[SMALL_XSIZE+1][y] = 0;
      }
      for (x=0;x<=SMALL_XSIZE;x++) {
	 (*s_game_board)[x][0] = 0;
	 (*s_game_board)[x][SMALL_YSIZE+1] = 0;
	 (*s_heat_board)[x][0] = 0;
	 (*s_heat_board)[x][SMALL_YSIZE+1] = 0;
      }
      break;
   }

   for (y=1;y<=SMALL_YSIZE;y++) {
      lp0 = fb->p + (fb->stride * 2 * (y - 1));
      lp1 = lp0 + XSIZE - 1;
      for (x=1;x<=SMALL_XSIZE;x++) {
	 int heat, t;
	 heat = ((((*s_heat_board)[x  ][y-1])+
		  ((*s_heat_board)[x-1][y  ])+
		  ((*s_heat_board)[x  ][y  ])+
		  ((*s_heat_board)[x+1][y  ])+
		  ((*s_heat_board)[x  ][y+1])
		  + p->speed)
		 / 5) & p->mask;
	 t = (((*s_game_board)[x+1][y+1]&LIVE_BIT)+
	      ((*s_game_board)[x+1][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y+1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x  ][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y  ]&LIVE_BIT)+
	      ((*s_game_board)[x+1][y  ]&LIVE_BIT)+
	      ((*s_game_board)[x  ][y+1]&LIVE_BIT));
	 q1 = (*s_game_board)[x][y]&LIVE_BIT;
	 switch (br) {
	  case 0:
	    /* adding || (heat > 200) interesting? */
	    if (q1) {
	       t = (t == 2 || t == 3);
	    } else {
	       t = (t == 3);
	    }
	    if (t) heat = p->mask;
	    break;
	  case 1:
	    t += q1;
	    /* 0001010111 */
	    t = ((t > 6) || (t == 3) || (t == 5));
	    if (t) heat = p->mask>>1;
	    break;
	  case 2:
	    t += q1;
	    t = t&1;
	    if (t) heat = p->mask;
	    break;
	 }
	 t = ((*s_game_board)[x][y] << 1) | t;
	 (*d_game_board)[x][y]= t;
	 (*d_heat_board)[x][y]= heat;
	 t = (heat > 100) ? (t|128) : (heat&127);
#if 0
	 t = heat;
#endif
	 if (0==t) t = 1;
	 else if (255==t) t = 254;
	 /* use stride+-1 for effect, nix idea */
	 lp0[fb->stride] = t;
	 *lp0++ = t;
	 lp1[fb->stride] = t;
	 *lp1-- = t;
      }
   }
}

#define feedback 0
#define flevel 20

void step_rule_acidlife2(int frame, rule_t *p, image8_t *fb)
{
   int x, y;
   board_t *s_heat_board, *d_heat_board;
   board_t *s_game_board, *d_game_board;
   u_char *lp0, *lp1, *lp2, *lp3;
   int br = iclamp(p->brain,nbrains);
   int q0, q1;

   s_heat_board=&board[dbuf];
   d_heat_board=&board[1-dbuf];
   s_game_board=&board2[dbuf];
   d_game_board=&board2[1-dbuf];
   dbuf = 1-dbuf;

   switch (br) {
    case 0:
      for (y=0;y<=SMALL_YSIZE;y++) {
	 int t;
	 (*s_game_board)[0][y] = R;
	 t = R;
	 (*s_game_board)[SMALL_XSIZE+1][y] = t;
      }
      for (x=0;x<=SMALL_XSIZE;x++) {
	 (*s_game_board)[x][0] = R;
	 (*s_game_board)[x][SMALL_YSIZE+1] = R;
      }
      break;
    case 1:
    case 2:
      for (y=0;y<=SMALL_YSIZE;y++) {
	 (*s_game_board)[0][y] = 0;
	 (*s_game_board)[SMALL_XSIZE+1][y] = 0;
	 (*s_heat_board)[0][y] = 0;
	 (*s_heat_board)[SMALL_XSIZE+1][y] = 0;
      }
      for (x=0;x<=SMALL_XSIZE;x++) {
	 (*s_game_board)[x][0] = 0;
	 (*s_game_board)[x][SMALL_YSIZE+1] = 0;
	 (*s_heat_board)[x][0] = 0;
	 (*s_heat_board)[x][SMALL_YSIZE+1] = 0;
      }
      break;
   }

   for(y=1;y<=SMALL_YSIZE;y++) {
      lp0 = fb->p + (fb->stride * (y - 1));
      lp1 = fb->p + (fb->stride * (YSIZE - y - 1));
      lp2 = fb->p + (fb->stride * (y - 1)) + XSIZE - 1;
      lp3 = fb->p + (fb->stride * (YSIZE - y - 1)) + XSIZE - 1;
      for(x=1;x<=SMALL_XSIZE;x++) {
	 int heat, t;
	 heat = ((((*s_heat_board)[x  ][y-1])+
		  ((*s_heat_board)[x-1][y  ])+
		  ((*s_heat_board)[x  ][y  ])+
		  ((*s_heat_board)[x+1][y  ])+
		  ((*s_heat_board)[x  ][y+1])
		  + p->speed)/5)&p->mask;
	 t = (((*s_game_board)[x+1][y+1]&LIVE_BIT)+
	      ((*s_game_board)[x+1][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y+1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x  ][y-1]&LIVE_BIT)+
	      ((*s_game_board)[x-1][y  ]&LIVE_BIT)+
	      ((*s_game_board)[x+1][y  ]&LIVE_BIT)+
	      ((*s_game_board)[x  ][y+1]&LIVE_BIT));
	 q1 = (*s_game_board)[x][y]&LIVE_BIT;
	 switch (br) {
	  case 0:
	    if (q1) {
	       t = (t == 2 || t == 3);
	    } else {
	       t = (t == 3);
	    }
	    if (t) heat = 100;
	    break;
	  case 1:
	    t += q1;
	    t = ((t > 6) || (t == 3) || (t == 5));
	    if (t) heat -= 2;
	    break;
	  case 2:
	    t += q1;
	    t = t&1;
	    if (t) heat = p->mask;
	    break;
	 }
	 
	 q0 = t;
	 t = ((*s_game_board)[x][y] << 1) | t;
	 (*d_game_board)[x][y]= t;
	 (*d_heat_board)[x][y]= heat;
	 if (0==heat) heat = 1;
	 else if (255==heat) heat = 254;
	 *lp0++ = heat;
	 *lp1++ = heat;
	 *lp2-- = heat;
	 *lp3-- = heat;
      }
   }
}

