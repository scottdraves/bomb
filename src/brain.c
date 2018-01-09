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

static char *brain_c_id = "@(#) $Id: brain.c,v 1.1.1.1 2002/12/08 20:49:40 spotspot Exp $";

#include "defs.h"

typedef struct {
   int subrule;
   int mask;
} sub_brain_t;

/* ordering could be much better */
static sub_brain_t sub_brain_table[] =
#ifdef wbomb
{
 {0, 0x0070}, /* 18 */
 {0, 0x4004}, /* 5  */
 {2, 0},      /* 19 */
 {0, 0x8080}, /* 9  */
 {3, 0x800},  /* 12 */
/* {3, 0x200},   37 */
 {5, 0x06},   /* 10 */
 {3, 0x010},  /* 25 */
};
#else
{{6, 0},
 {0, 0x2008},
 {5, 0x12},
 {0, 0x0808},
 {0, 0x0208},
 {0, 0x4004},
 {4, 0},
 {0, 0x8008},
 {5, 0x12},
 {0, 0x8080},
 {5, 0x06},
 {0, 0x8800},
 {3, 0x800},
 {0, 0x6000},
 {4, 0},
 {0, 0x0600},
 {0, 0x0060},
 {5, 0x12},
 {0, 0x0070},
 {2, 0},
 {0, 0x0090},
 {3, 0x004},
 {0, 0x0900},
 {3, 0x008},
 {0, 0x9000},
 {3, 0x010},
 {0, 0x5000},
 {5, 0xa0},
 {3, 0x020},
 {0, 0x0014},
 {3, 0x040},
 {0, 0x0500},
 {4, 0},
 {0, 0x0050},
 {0, 0x0014},
 {3, 0x080},
 {0, 0x0104},
 {3, 0x200},
 {0, 0x1040},
 {0, 0x10400},
 {0, 0x104000},
 {1, 0},
 {5, 0x06},
 {4, 0},
 };
#endif

/* p1 toggles direct display of brain (bypass heat), good for debugging */
#define SHOW_DRIVER 0



void step_rule_rug_brain(int frame, rule_t *p, image8_t *fb)
{
   int x,y;
   board_t *s_heat_board, *d_heat_board;
   board_t *s_game_board, *d_game_board;
   u_char *lp0, *lp1;
   int brain = iclamp(p->brain, alen(sub_brain_table));
   int mask = sub_brain_table[brain].mask;
   int shift_bits = iclamp(p->brain_shift, 26);
   int live_bit = 1<<(shift_bits);
   int phase_mask = (1<<(shift_bits+1))-1;
   static int phase = 0;
   if (++phase > shift_bits)
      phase = 0;

   s_heat_board=&board[dbuf];
   d_heat_board=&board[1-dbuf];
   s_game_board=&board2[dbuf];
   d_game_board=&board2[1-dbuf];
   dbuf = 1-dbuf;

   // sprintf(status_string+20, "%d %d %d %d", p->mask, p->speed, brain, shift_bits);


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

   for (y=1;y<=SMALL_YSIZE;y++) {
      lp0 = fb->p + (fb->stride * (y - 1));
      lp1 = lp0 + fb->stride * SMALL_YSIZE;
      for (x=1;x<=SMALL_XSIZE;x++) {
	 int heat, t, c, dh;
	 int nneigh;
	 int centerhist;
	 int newbit;
	 int driver;
	 int hn, hs, he, hw, hc;
	 int denom;
	 nneigh =
	    (((*s_game_board)[x+1][y+1]&live_bit)+
	     ((*s_game_board)[x+1][y-1]&live_bit)+
	     ((*s_game_board)[x-1][y+1]&live_bit)+
	     ((*s_game_board)[x-1][y-1]&live_bit)+
	     ((*s_game_board)[x  ][y-1]&live_bit)+
	     ((*s_game_board)[x-1][y  ]&live_bit)+
	     ((*s_game_board)[x+1][y  ]&live_bit)+
	     ((*s_game_board)[x  ][y+1]&live_bit)) >> shift_bits;

	 centerhist = (*s_game_board)[x][y];
	 driver = (centerhist >> phase) & phase_mask;

	 hn = (*s_heat_board)[x  ][y-1];
	 hs = (*s_heat_board)[x  ][y+1];
	 he = (*s_heat_board)[x+1][y  ];
	 hw = (*s_heat_board)[x-1][y  ];
	 hc = (*s_heat_board)[x  ][y  ];
	 denom = 5;
	 heat = hn + hs + he + hw + hc + p->speed;
	 dh = 0;
	 driver = driver | (driver >> 8);
	 if (driver&0x1)     dh += 10;
	 if (driver&0x2)     dh -= 5;
	 if (driver&0x4)   { heat -= hn; denom--;}
	 if (driver&0x8)   { heat -= hs; denom--;}
	 if (driver&0x10)    dh += 20;
	 if (driver&0x20)  { heat -= he; denom--;}
	 if (driver&0x40)  { heat -= hw; denom--;}
	 if (driver&0x80 && heat < 0) heat = 0;
	 heat = (heat / denom) & p->mask;
	 heat += dh;


	 c = centerhist;
	 t = nneigh;
	 switch (sub_brain_table[brain].subrule) {
	 default:
	  case 0:
	    newbit = (t == 3) || ((t > 3) && (((c & mask) == 0)));
	    break;
	  case 1:
	    newbit = (t == 3) || ((t > 3) && ((((c & 0x007) == 0) ^
					       ((c & 0x060) == 0)) |
					      ((c & 0x700) == 0)));
	    break;
	  case 2:
	    newbit = (t == 3) || ((t > 2) && (((c&6) == 0) ^ ((c&0x60) == 0)));
	    break;
	  case 3:
	    newbit = ((t == 2) || (t > 3)) && ((c&mask) == 0);
	    break;
	  case 4:
	    newbit = (t == 2) && ((c&6) == 0);
	    break;
	  case 5:
	    newbit = (t > 1) && (t < 6) && ((c&mask) == 0);
	    break;
	  case 6:
	    newbit = (t == 1) || ((t > 3) && (((c & 0x8008) == 0)));
	    break;
	 }
	 centerhist = (centerhist << 1) | newbit;

	 (*d_game_board)[x][y] = centerhist;
	 (*d_heat_board)[x][y] = heat;

#if SHOW_DRIVER
	 p1 = iclamp(p1, 3 + shift_bits);
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

#if 0
/* this this development code and notes, pasted down here to make the above
   more readable */
#if 1
	 t = (t == 1) || ((t > 3) && (((c & 0x8008) == 0)));
#elif 0
	 /* 0x700 0x70 0x38 */
	 /* 0x8010 0x8008 */
	 /* any two bit pattern (except 0x0c melts away, others?)  */
	 /* 0x4004 wavey blobs, but die out slowly, usually */
	 t = (t == 3) || ((t > 3) && (((c & 0x8008) == 0)));
#elif 0
	 t = (t == 3) || ((t > 3) &&
			  (((c & 0x007) == 0) ^
			   ((c & 0x060) == 0) |
			   ((c & 0x700) == 0)));
#elif 0
	 /* noise with floating holes */
	 t = (t == 3) || ((t > 2) && (((c&6) == 0) ^ ((c&0x60) == 0)));
#elif 0
	 /* all: rough edges, bits of vert/horiz stability
	    in the lower areas, near circular (octagonal?) shape
	    2 almost noise
	    4 near-static rectilinear mosaic
	    8,10 swirly texture
	    20 unstable z-reaction
	    40 semi-stable z
	    80 stable z, one eventually dominates
	    400 time tunnel, horns less apparent
	    4000 same, even thicker
	    */
	 t = ((t == 2) || (t > 3)) && ((c&0x20) == 0);
#elif 0
	 /* any two bit constant works */
	 /* diamond shape, quickly stabilizes */
	 /* 6 stable, no large features
	    30 fair sized features
	    c0 only a few left
	    300 often fails
	    1 bit gap (5/a/14 etc) interesting, otherwise
	    lower bit not very important.  can effect stripe-width
	    */
	 t = (t > 1) && (t < 6) && ((c&0x12) == 0);
#else
	 /* like brian's brain.  files' namesake */
	 t = (t == 2) && ((c&6) == 0);
#endif

#endif
