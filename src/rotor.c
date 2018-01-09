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

static char *rotor_c_id = "@(#) $Id: rotor.c,v 1.1.1.1 2002/12/08 20:49:50 spotspot Exp $";

#include "defs.h"
#include "bomb.h"
#include "libifs.h"
#include "sound.h"

// double cc[NXFORMS][3][2];

ifs_control_point cc_direction;

#define nsamples 1000
#define nbatches 10
#define bufsize (nsamples/nbatches)
ifs_point p_buf[bufsize];


/*
 * a rotor is the sum of spinners (n=~3).  each spinner's speed&radius
 * wanders piecewise linear over a range, uniform dist.  originally from
 * Tom Lawrence (tcl@cs.brown.edu: 'flight' simulator, now at SGI), put
 * into xlock (by Steve Zellers, zellers@sun.com), maintained there by
 * Patrick J. Naughton.  
 */

#define MAX_SPINNERS 10

typedef struct {
   int radius_steps, speed_steps;
   double radius, dradius;
   double speed, dspeed;
} spinner;

double spinner_time = 0.0;
int nspinners;
spinner spinners[MAX_SPINNERS];


int nspin2;
spinner spin2[MAX_SPINNERS];


void init_spinner(int *nspin, spinner *spin)
{
   int i;
   int n = (R%2) ? 3 : ((R%3)?2:4);
   if (n > MAX_SPINNERS)
      n = MAX_SPINNERS;
   *nspin = n;
#define KRS 30000
   for (i = 0; i < n; i++) {
      spin[i].radius_steps = KRS;
      spin[i].speed_steps = KRS;
      spin[i].radius = (R%100-25)/50.0;
      spin[i].speed  = 0.1 + (R%100)/20.0;
      spin[i].dradius = (R%100-50)/(50.0*KRS);
      spin[i].dspeed  = (R%100-50)/(20.0*KRS);
   }
}

void init_rotate()
{
   init_spinner(&nspinners, spinners);
   init_spinner(&nspin2, spin2);
}


int nice_ratio0(double r) {
  if (0.1 > abs(r - floor(r))) return 1;
  if (0.05 > abs(2*r - floor(2*r))) return 1;
  if (0.033 > abs(3*r - floor(3*r))) return 1;
  if (0.025 > abs(4*r - floor(4*r))) return 1;
  return 0;
}

int nice_ratio(double r) {
	return nice_ratio0(r) || nice_ratio0(1.0/r);
}

void rotate(int *nspin, spinner *spin, double *xy)
{
   double x, y;
   int i;
   int n = *nspin;
    int found_good_ratio = 0;
	int search_steps = 0;
#define SI spin[i]

   while (search_steps++ < 50 && !found_good_ratio) {
     for (i = 0; i < n; i++) {
        SI.radius += SI.dradius;
        SI.speed += SI.dspeed;
 		if (!nice_ratio(SI.speed / spin[0].speed)) {
			spinner_time += 0.04;
			break;
		}
	 }
	 found_good_ratio = (i == n);
   }


   x = y = 0.0;
   for (i = 0; i < n; i++) {
      x += SI.radius * cos(SI.speed * spinner_time);
      y += SI.radius * sin(SI.speed * spinner_time);
      if (0 == SI.radius_steps--) {
	 double goal = (R%100)/100.0+(R%100)/100.0;
	 SI.radius_steps = KRS;
	 SI.dradius = (goal - SI.radius) / KRS;
      }
      if (0 == SI.speed_steps--) {
	 double goal = 0.1 + (R%100)/20.0;
	 SI.speed_steps = KRS;
	 SI.dspeed = (goal - SI.speed) / KRS;
      }
   }
   x = x / n;
   y = y / n;

   xy[0] = x;
   xy[1] = y;
}


void rotate_int(int *nspin, spinner *spin, int *ixy)
{
   double xy[2];
   rotate(nspin, spin, xy);
   ixy[0] = (xy[0]+1) * (XSIZE/2);
   ixy[1] = (xy[1]+1) * (YSIZE/2);
}




/* flame support code */

void normalize_liss_coefs()
{
   int i, j, k;
   double r;
   for (i = 0; i < NXFORMS; i++) {
      r = 1e-6;
      for (j = 0; j < 3; j++)
	 for (k = 0; k < 2; k++) {
	   double t;
	    t = cc_direction.xform[j].c[k][i];
	    r += t * t;
	 }
      r = 0.015 / sqrt(r);
      for (j = 0; j < 3; j++)
	 for (k = 0; k < 2; k++)
	    cc_direction.xform[j].c[k][i] *= r;
   }
}
void pick_liss_coefs()
{
   int i, j, k;
   double r;
   for (i = 0; i < NXFORMS; i++) {
      r = 1e-6;
      for (j = 0; j < 3; j++)
	 for (k = 0; k < 2; k++) {
	    cc_direction.xform[j].c[k][i] = random_uniform11();
	 }
   }
   normalize_liss_coefs();
}

int nflames;
ifs_control_point flames[MAXFLAMES];

void init_rotor()
{
   char sbuffer[4000];
   char *ss = sbuffer;
   int c, i, n = 0;
   FILE *f;

   if (getenv("quick"))
      goto quick;

   sprintf(sbuffer, "%sflame-data", DATA_DIR);
   f = fopen(sbuffer, "r");

   if (NULL == f)
      goto fail;
   while (1) {
      i = 0;
      ss = sbuffer;
      do {
	 c = fgetc(f);
	 if (EOF == c)
	    goto done_reading;
	 sbuffer[i++] = c;
      } while (';' != c);
      parse_control_point(&ss, flames + n);
      n++;
      if (n == MAXFLAMES)
	 goto done_reading;
   }
 done_reading:
   nflames = n;
   if (n > 0)
      return;
 fail:
   fprintf(stderr, "flame-data not read\n");
 quick:
   nflames = 1;
   random_control_point(flames);
}


void
rotcp(rule_t *p, double by)
{
   int i;
   for (i = 0; i < NXFORMS; i++) {
      double r[2][2];
      double T[2][2];
      double U[2][2];
      double dtheta = by * 8.0 * M_PI / (1000.0 * nbatches);

      r[1][1] = r[0][0] = cos(dtheta);
      r[0][1] = sin(dtheta);
      r[1][0] = -r[0][1];
      T[0][0] = p->flame_cp.xform[i].c[0][0];
      T[1][0] = p->flame_cp.xform[i].c[1][0];
      T[0][1] = p->flame_cp.xform[i].c[0][1];
      T[1][1] = p->flame_cp.xform[i].c[1][1];
      mult_matrix(r, T, U);
      p->flame_cp.xform[i].c[0][0] = U[0][0];
      p->flame_cp.xform[i].c[1][0] = U[1][0];
      p->flame_cp.xform[i].c[0][1] = U[0][1];
      p->flame_cp.xform[i].c[1][1] = U[1][1];
   }
}

void
rot_point(rule_t *p, double by)
{
   int i;
   for (i = 0; i < NXFORMS; i++) {
      double s[2];
      double r[2][2];
      double dtheta = by * 8.0 * M_PI / (1000.0 * nbatches);
      r[1][1] = r[0][0] = cos(dtheta);
      r[0][1] = sin(dtheta);
      r[1][0] = -r[0][1];
      s[0] = p->flame_cp.xform[i].c[2][0];
      s[1] = p->flame_cp.xform[i].c[2][1];
      
	 
      p->flame_cp.xform[i].c[2][0] =
	 s[0] * r[0][0] + s[1] * r[0][1];
      p->flame_cp.xform[i].c[2][1] =
	 s[0] * r[1][0] + s[1] * r[1][1];
   }
}

int gong = 0;

void step_rule_rotorug(int frame, rule_t *p, image8_t *fb)
{
   int x,y,bx,i;
   board_t *sboard, *dboard;
   u_char *lp;
   int drift_type = iclamp(p->drift, 9);
   int bsize = p->bsize/15;
#ifndef wbomb
   if (bsize < 1) bsize = 1;
   else
#endif
   if (bsize > 8) bsize /= 2;
   if (bsize > 8) bsize /= 2;

   if (sound_present && p->speed_beat_size) {
     bsize = (p->drift_speed - 12) / 2;
     if (bsize < 0) bsize = 0;
     bsize = 1+sqrt(bsize);
   }

   sboard = &board[dbuf];
   dboard = &board[1-dbuf];
   dbuf = 1-dbuf;

   if (2 == drift_type) {
     if (gong > 0)
       gong--;
     for (i = 0; i < NXFORMS; i++) {
       double r;
       r = (p->flame_cp.xform[i].c[0][0] *
	    p->flame_cp.xform[i].c[1][1]) -
	 (p->flame_cp.xform[i].c[0][1] *
	  p->flame_cp.xform[i].c[1][0]);
       if (r > 3 || r < -3)
	 gong += 10;
       if (0)
	 printf("%.3g(%.3g) ", r, p->flame_cp.xform[i].density);
     }
     if (0 /* gong > 800 */) {
       gong = 0;
       random_control_point(&rule.flame_cp);
       pick_liss_coefs();
     }
     if (0) printf("%d\n", gong);
   }


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
	 if (t<0) t = 0;
	 t &= p->mask;
	 (*dboard)[x][y]= t;
	// if (p->remap)
	    t = remap[t];
	 *(lp++)=t;
      }
   }
   if (0) {
     /* show waveform */
#if 0
     int start;
     int best = 100000;
     int n;
     double r;
     n = sound_buf_last + 0;
     if (n >= sound_buf_end)
       n = sound_buf_end-1;
     start = sound_buf_last;
     for (i = sound_buf_last; i < n; i++) {
       int samp = (int)bomb_sound_buf[i] - 128;
       samp *= samp;
       if (samp < best) {
	 best = samp;
	 start = i;
       }
     }
     if (0) {
       /* polar */
       r = (R&255)/256.0*6.28;
       for (i = 0; i < 200; i++) {
	 int s = (int)bomb_sound_buf[i+start] - 128;
	 pen(dboard, p->mask,
	     160 + s * cos(r+6.28*i/200.0),
	     100 + s * sin(r+6.28*i/200.0),
	     bsize);
       }
     } else
       /* linear */
       for (i = 0; i < 200; i++) {
	 pen(dboard, p->mask, 34 + bomb_sound_buf[i+start], i, bsize);
     }
#endif
   } else if (1 >= drift_type) {
      for (i = 0; i < 200; i++)  {
	 int xy[2];
	 spinner_time += 0.04;
	 rotate_int(&nspinners, spinners, xy);
	 x = xy[0];
	 switch (drift_type) {
	  case 0:
	    /* circular (original) */
	    y = xy[1];
	    break;
#if 0
	  case 1:
	    /* rectangular */
	    rotate_int(&nspin2, spin2, xy);
	    y = xy[1];
	    break;
	  case 3:
	    /* pulsing circular */
	    if (1) {
	       double t = frame * 0.05;
	       double s = (sin(t * p->rhythm[0]) *
			   sin(t * p->rhythm[1]));
	       s = 1 - s * s;
	       x = (xy[0] - (XSIZE/2)) * s + (XSIZE/2);
	       y = (xy[1] - (YSIZE/2)) * s + (YSIZE/2);
	    }
	    break;
#endif
	  case 1:
	    /* vertical */
	    /* y = (int)(0.987 * spinner_time * YSIZE) % YSIZE; */
	    y = (1+sin(spinner_time)) * (YSIZE/2);
	    break;
	 }
	 pen(dboard, p->mask, x, y, bsize);
      }
   } else {
      for (bx = 0; bx < nbatches; bx++) {
	 int j, k;
	 double time = adjust_speed(p->drift_speed) * (bx/(double)nbatches) + p->drift_time;

	 switch (drift_type) {
	 case 2:
	   for (i = 0; i < NXFORMS; i++) {
	     for (j = 0; j < 3; j++) {
	       for (k = 0; k < 2; k++) {
		 double x;
		 x = p->flame_cp.xform[i].c[j][k] +=
		   adjust_speed(cc_direction.xform[i].c[j][k]*0.1);
		 if (x > 1) cc_direction.xform[i].c[j][k] += -1e-3;
		 else if (x < -1) cc_direction.xform[i].c[j][k] += 1e-3;
		 else cc_direction.xform[i].c[j][k] += random_uniform11()*1e-3;
	       }
	     }
	   }
	     
	   break;
	 case 4:
	    for (i = 0; i < NXFORMS; i++)
	       for (j = 0; j < 3; j++)
		  for (k = 0; k < 2; k++)
		     p->flame_cp.xform[i].c[j][k] = sin(time * cc_direction.xform[i].c[j][k]);
	    break;
	  case 3:
	  case 5:
	    rotcp(p, adjust_speed(5.0 + p->drift_speed/20.0));
	    rot_point(p, adjust_speed(0.1));
	    break;
	  case 6:
	    rotcp(p, adjust_speed(0.03 + p->drift_speed/100.0));
	    break;
	  case 7:
	  case 8:
	    if (0) {
	      int i, j;
	      for (j = 0; j < flame_nspan; j++) {
		p->flame_span[j].time = (double) j;
		for (i = 0; i < 2; i++) {
		  p->flame_span[j].pulse[i][0] = 0.0;
		  p->flame_span[j].pulse[i][1] = 60.0;
		  p->flame_span[j].wiggle[i][0] = 0.0;
		  p->flame_span[j].wiggle[i][1] = 60.0;
		}
	      }
	      interpolate(p->flame_span, flame_nspan, 1.1/*+0.02*sin(time)*/, &p->flame_cp);
	    }
	     
	    if (1) {
	       int c;
	       c = p->rhythm[0];
	       
	       rotcp(p, 20*adjust_speed(0.05 + sin(c*time/480.0)));
	       rot_point(p, 10*adjust_speed(sin(time/60.0)));
#if 0
	       if (frame%c < c/2)
		  rotcp(p, adjust_speed(1.0));
	       else
		  rotcp(p, adjust_speed(-1.05));

	       c = p->rhythm[1]/2;
	       if (frame%c < c/2)
		  rot_point(p, adjust_speed(0.5));
	       else
		  rot_point(p, adjust_speed(-0.5));
#endif
	    }
	    break;
	  
	 }

	 /* overhead of fuse could get in the way as bufsize drops, don't
	    really need it.  on fast rhythm mach, how small can batches go?
	    they are 100 now */
	 
	 iterate(&p->flame_cp, bufsize, 5, p_buf);
	 for (i = 0; i < bufsize; i++)  {
	    double px, py;
	    px = p_buf[i][0];
	    py = p_buf[i][1];
	    if (px < 1.0 && py < 1.0 && px > -1.0 && py > -1.0) {
	       int x, y;
	       x = (px + 1.0) * XSIZE/2;
	       y = (py + 1.0) * YSIZE/2;
	       pen(dboard, p->mask, x, y, bsize);
	    }
	 }
      }
   }
#if 0
   for (i = 0; i < nspinners; i++) {
      printf("%d r=%g s=%g\n", i, SI.radius, SI.speed);
   }
#endif
}

