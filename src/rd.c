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

static char *rd_c_id = "@(#) $Id: rd.c,v 1.1.1.1 2002/12/08 20:49:50 spotspot Exp $";

#include "defs.h"
#include "bomb.h"

/* rd = reaction/diffusion */
/* replacing R with R8b only buys 2% on P5 */
#define pick(c0,c1,d0,d1) return (((R%c0) > c1) ? d0 : d1)

 int dxdy2dither(int dx, int dy) {
   if (dx > 0) {
      if (dy > 0) {
	 if (dx > dy)
	    pick(dx, dy, 1, -1);
	 else
	    pick(dy, dx, 2, -1);
      } else if (dy < 0) {
	 dy = -dy;
	 if (dx > dy)
	    pick(dx, dy, 1, -2);
	 else
	    pick(dy, dx, 2, -2);
      } else {
	 return 1;
      }
   } else if (dx < 0) {
      dx = -dx;
      if (dy > 0) {
	 if (dx > dy)
	    pick(dx, dy, 1, -2);
	 else
	    pick(dy, dx, 2, -2);
      } else if (dy < 0) {
	 dy = -dy;
	 if (dx > dy)
	    pick(dx, dy, 1, -1);
	 else
	    pick(dy, dx, 2, -1);
      } else {
	 return 1;
      }
   } else if (0 == dy) {
      return 0;
   } else {
      return 2;
   }
}

int smooth_checkers;

void step_rule_rd(int frame, rule_t *p, image8_t *fb)
{
   int x,y;
   board_t *s_heat_board, *d_heat_board;
   board_t *s_image_board;
   u_char *lp0, *lp1;
   int reaction = iclamp(p->brain, 4);
   int diffusion = iclamp(p->brain_shift, 12);

   switch (diffusion) {
    case 7:
      spiral(&board3[0], frame * (6.28 / 3000), frame%30, 30);
      break;
    case 8:
      if (grad_state++ < 40)
	 /* why dbuf instead of 0? */
	 image2grad(&board2[dbuf], &board3[0], 15, frame%40, 40);
      break;
    case 9:
      if (1) {
	 static int pat[][6] = {{0, 1, 2, 0, 1, 2},
				{0, 2, 2, 0, 1, 1},
				{0, 0, 2, 1, 1, 2},
				{0, 1, 0, 1, 0, 1},
				{0, 2, 3, 3, 2, 0},
				{3, 0, 3, 1, 3, 2},
				{2, 0, 1, 3, 1, 0}};
	 /* i hate to use p1 here */
	 hex_tile(&board3[0], frame * (6.28 / 4000), pat[iclamp(p1,6)], 6, frame%30, 30);
      }
      break;      
   }

   s_heat_board=&board[dbuf];
   d_heat_board=&board[1-dbuf];
   s_image_board=&board2[0];
   dbuf = 1-dbuf;

   for(y=0;y<=SMALL_YSIZE+1;y++) {
      (*s_heat_board)[0][y] = (*s_heat_board)[SMALL_XSIZE][y];
      (*s_heat_board)[SMALL_XSIZE+1][y] = (*s_heat_board)[1][y];
   }
   for(x=0;x<=SMALL_XSIZE+1;x++) {
      (*s_heat_board)[x][0] = (*s_heat_board)[x][SMALL_YSIZE];
      (*s_heat_board)[x][SMALL_YSIZE+1] = (*s_heat_board)[x][1];
   }

   for (y=1;y<=SMALL_YSIZE;y++) {
      lp0 = fb->p + (fb->stride * (y - 1));
      lp1 = lp0 + fb->stride * SMALL_YSIZE;
      for (x=1;x<=SMALL_XSIZE;x++) {
	 int heat, t, uvv;
	 int hn, hs, he, hw, hc;
	 int r1, r2;
	 int d1, d2;

	 /* see rdbs in bomb.c, label dependency */
	 if (diffusion < 3)
	    t = diffusion;
	 else switch (diffusion) {
	  case 3:
	    t = (((x + y) ^ (x - y)) & 32)
	       ? 1 : 2;
	    break;
	  case 4:
	    t = ((((200 * x) / 320 + y) % 100) > 50) ? 1 : 2;
	    break;
	  case 5:
	    t = (*s_image_board)[(2*x + 160)%320][(2*y+100)%200] ? 1 : 2;
	    break;
	  case 6:
	    t = dxdy2dither(x - 80, y - 50);
	    break;
	  case 9:
	  case 7:
	    if (1) {
	       int dxdy = board3[0][(2*x + 160)%320][(2*y+100)%200];
	       int dx = (dxdy >> 8) - 128;
	       int dy = (dxdy & 255) - 128;
	       t = dxdy2dither(dx, dy);
	    }
	    break;
	  case 8:
	    if (1) {
	       int dxdy = board3[0][(2*x + 160)%320][(2*y+100)%200];
	       int dx = (dxdy >> 8) - 128;
	       int dy = (dxdy & 255) - 128;
	       t = dxdy2dither(-dy, dx);
	    }
	    break;
	  default:
		  t = 0;
		  break;
	 }

	 if (t < 0 && !(smooth_checkers)) {
	    /* rotate pi/2 clockwise.  note this divides board into
               even and odd partitions */
	    t = -t;
	    hn = (*s_heat_board)[x+1][y-1];
	    hs = (*s_heat_board)[x-1][y+1];
	    he = (*s_heat_board)[x+1][y+1];
	    hw = (*s_heat_board)[x-1][y-1];
	 } else {
	    hn = (*s_heat_board)[x  ][y-1];
	    hs = (*s_heat_board)[x  ][y+1];
	    he = (*s_heat_board)[x+1][y  ];
	    hw = (*s_heat_board)[x-1][y  ];
	 }
	 hc = (*s_heat_board)[x  ][y  ];

#define mx 0xffff
#define bits 16
	 switch (t) {
	  default:
	  case 0:
	    r1 = (hc&mx) + (hn&mx) + (hs&mx) + (he&mx) + (hw&mx);
	    r1 = r1 / 5;
	    r2 = (hc>>13) + (hn>>16) + (hs>>16) + (he>>16) + (hw>>16);
	    r2 = r2 / 12;
	    break;
	  case 1:
	    r1 = 2*(hc&mx) + 2*(hn&mx) + 2*(hs&mx) + (he&mx) + (hw&mx);
	    r1 = r1 / 8;
	    r2 = (hc>>13) + (hn>>16) + (hs>>16) + (he>>16) + (hw>>16);
	    r2 = r2 / 12;
	    break;
	  case 2:
	    r1 = 2*(hc&mx) + (hn&mx) + (hs&mx) + 2*(he&mx) + 2*(hw&mx);
	    r1 = r1 / 8;
	    r2 = (hc>>13) + (hn>>16) + (hs>>16) + (he>>16) + (hw>>16);
	    r2 = r2 / 12;
	    break;
	 }

	 uvv = (((r1 * r2) >> bits) * r2) >> bits;

	 switch (reaction) {
	 default:
	  case 0:
	    d1 = 2 * (((28 * (mx-r1)) >> 10) - uvv);
	    d2 = 3 * (uvv - ((80 * r2) >> 10));
	    break;
	  case 1:
	    d1 = 4 * (((28 * (mx-r1)) >> 10) - uvv);
	    d2 = 4 * (uvv - ((80 * r2) >> 10));
	    break;
	  case 2:
	    d1 = 3 * (((27 * (mx-r1)) >> 10) - uvv);
	    d2 = 3 * (uvv - ((80 * r2) >> 10));
	    break;
	  case 3:
	    d1 = 2 * (((27 * (mx-r1)) >> 10) - uvv);
	    d2 = 2 * (uvv - ((80 * r2) >> 10));
	    break;
	 }

	 r1 += d1;
	 r2 += d2;
	 if (r1 > mx) r1 = mx;
	 if (r2 > mx) r2 = mx;
	 if (r1 < 0)  r1 = 0;
	 if (r2 < 0)  r2 = 0;

	 (*d_heat_board)[x][y] = r1 | (r2<<bits);

#if 0
	 switch (p1%4)  {
	  case 0:
	    heat = r1>>8;
	    break;
	  case 1:
	    heat = (*s_image_board)[x][y] / 2;
	    break;
	  case 2:
	    heat = (board3[0][(2*x + 160)%320][(2*y+100)%200]) >> 8;
	    heat = (board3[0][x][y]) >> 8;
	    break;
	  case 3:
	    heat = (board3[0][(2*x + 160)%320][(2*y+100)%200]) & 255;
	    break;
	 }
#else
	 heat = (r1>>8);
#if win_bomb
	 if (0 == heat) heat = 1;
	 else if (255 == heat) heat = 254;
#endif
#endif

	 lp0[SMALL_XSIZE] = heat;
	 *lp0++ = heat;
	 lp1[SMALL_XSIZE] = heat;
	 *lp1++ = heat;
      }
   }
   smooth_checkers = 0;
}

void step_rule_rd2(int frame, rule_t *p, image8_t *fb)
{
   int x,y;
   board_t *s_heat_board, *d_heat_board;
   board_t *s_image_board;
   u_char *lp0, *lp1;
   int reaction = iclamp(p->brain, 4);

   s_heat_board=&board[dbuf];
   d_heat_board=&board[1-dbuf];
   s_image_board=&board2[0];
   dbuf = 1-dbuf;

   for(y=0;y<=SMALL_YSIZE+1;y++) {
      (*s_heat_board)[0][y] = (*s_heat_board)[SMALL_XSIZE][y];
      (*s_heat_board)[1][y] = (*s_heat_board)[SMALL_XSIZE+1][y];
      (*s_heat_board)[SMALL_XSIZE+2][y] = (*s_heat_board)[2][y];
      (*s_heat_board)[SMALL_XSIZE+3][y] = (*s_heat_board)[3][y];

    }
   for(x=0;x<=SMALL_XSIZE+1;x++) {
      (*s_heat_board)[x][0] = (*s_heat_board)[x][SMALL_YSIZE];
      (*s_heat_board)[x][1] = (*s_heat_board)[x][SMALL_YSIZE+1];
      (*s_heat_board)[x][SMALL_YSIZE+2] = (*s_heat_board)[x][2];
      (*s_heat_board)[x][SMALL_YSIZE+3] = (*s_heat_board)[x][3];
   }

/* signed vs unsigned shift right */
#undef mx
#undef bits
#define mx 0x7fff
#define bits 15


   for (y=2;y<SMALL_YSIZE+2;y++) {
      lp0 = fb->p + (fb->stride * (y - 2));
      lp1 = lp0 + fb->stride * SMALL_YSIZE;
      for (x=2;x<SMALL_XSIZE+2;x++) {
	 int heat;
	 int hn, hs, he, hw, hc;
	 int r1, r2;
	 int d1, d2;

	    hn = (*s_heat_board)[x  ][y-1];
	    hs = (*s_heat_board)[x  ][y+1];
	    he = (*s_heat_board)[x+1][y  ];
	    hw = (*s_heat_board)[x-1][y  ];
	    hc = (*s_heat_board)[x  ][y  ];

	    r1 = hc&mx;
	    r1 += hn&mx;
	    r1 += hs&mx;
	    r1 += he&mx;
	    r1 += hw&mx;
	    r1 += (*s_heat_board)[x  ][y-2]&mx;
	    r1 += (*s_heat_board)[x  ][y+2]&mx;
	    r1 += (*s_heat_board)[x+2][y  ]&mx;
	    r1 += (*s_heat_board)[x-2][y  ]&mx;

	    r1 = r1/9;

	    r2 = (((hc>>bits)&mx)<<3) + ((hn>>bits)&mx) +
	      ((hs>>bits)&mx) + ((he>>bits)&mx) + ((hw>>bits)&mx);
	    r2 = r2 / 12;

	switch (reaction) {
	default:
	case 0:
	 d1 = (r1 < r2) ? 50 : -50;
	 d2 = d1*3;
	 break;
	case 1:
	 d1 = (r1 < r2 - 2500) ? 100 : -100;
	 d2 = d1*2;
	 break;
	case 2:
	 d1 = (r1 < r2) ? 50 : -50;
	 d2 = d1*5;
	 break;
	case 3:
	 d1 = (r1 < r2 - 500) ? 50 : -50;
	 d2 = 5*d1;

	 break;
	}

	 r1 += d1;
	 if (r1 > mx) r1 = mx;
	 if (r1 < 0)  r1 = 0;

	 r2 += d2;
	 if (r2 > mx) r2 = mx;
	 if (r2 < 0)  r2 = 0;

	 (*d_heat_board)[x][y] = r1 | (r2<<bits);

#if 0
	 switch (p1%4)  {
	  case 0:
	    heat = r1>>8;
	    break;
	  case 1:
	    heat = d1;
	    break;
	  case 2:
	    heat = r2>>8;
	    break;
	  case 3:
	    heat = (*s_image_board)[x][y] / 2;
	    break;
	 }
#else
	 heat = (r1>>7);
#if win_bomb
	 if (0 == heat) heat = 1;
	 else if (255 == heat) heat = 254;
#endif
#endif

	 lp0[SMALL_XSIZE] = heat;
	 *lp0++ = heat;
	 lp1[SMALL_XSIZE] = heat;
	 *lp1++ = heat;
      }
   }
}
