/*
    fractal flame generation package
    Copyright (C) 1992, 1993  Scott Draves <spot@cs.cmu.edu>

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

static char *libifs_c_id = "@(#) $Id: libifs.c,v 1.1.1.1 2002/12/08 20:49:49 spotspot Exp $";

#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "libifs.h"
#include "defs.h"


#define EPS (1e-10)
#  define real double
#  define F(x) x

#define CHOOSE_XFORM_GRAIN 100


/*
 * run the function system described by CP forward N generations.
 * store the n resulting 3 vectors in POINTS.  the initial point is passed
 * in POINTS[0].  ignore the first FUSE iterations.
 */

void iterate(cp, n, fuse, points)
   ifs_control_point *cp;
   int n;
   int fuse;
   ifs_point *points;
{
   int i, j;
   int xform_distrib[CHOOSE_XFORM_GRAIN];
   real p[3], t, r, dr;
   p[0] = points[0][0];
   p[1] = points[0][1];
   p[2] = points[0][2];

   /*
    * first, set up xform, which is an array that converts a uniform random
    * variable into one with the distribution dictated by the density
    * fields 
    */
   dr = 0.0;
   for (i = 0; i < NXFORMS; i++)
      dr += cp->xform[i].density;
   dr = dr / CHOOSE_XFORM_GRAIN;

   j = 0;
   t = cp->xform[0].density;
   r = 0.0;
   /* hm, how much time is spent in here? */
   for (i = 0; i < CHOOSE_XFORM_GRAIN; i++) {
      while (r >= t) {
	 j++;
	 t += cp->xform[j].density;
      }
      xform_distrib[i] = j;
      r += dr;
   }

   for (i = -fuse; i < n; i++) {
      int fn = xform_distrib[R8b % CHOOSE_XFORM_GRAIN];
      real tx, ty, v;


      if (p[0] > 1e10 || p[0] < -1e10 ||
	  p[1] > 1e10 || p[1] < -1e10) {
	 p[0] = (R&0xff) / (double) 0xff;
	 p[1] = (R&0xff) / (double) 0xff;
      }
#if 0
      if (p[0] > 100.0 || p[0] < -100.0 ||
	  p[1] > 100.0 || p[1] < -100.0)
	 count_large++;
      if (p[0] != p[0])
	 count_nan++;
#endif

#define coef   cp->xform[fn].c
#define vari   cp->xform[fn].var

      /* link to here */
      /* first compute the color coord */
      p[2] = (p[2] + cp->xform[fn].color) / 2.0;

      /* printf("xxx1\n"); */
      /* then apply the affine part of the function */
      tx = coef[0][0] * p[0] + coef[1][0] * p[1] + coef[2][0];
      ty = coef[0][1] * p[0] + coef[1][1] * p[1] + coef[2][1];

      p[0] = p[1] = 0.0;

      /* link here */
      /* then add in proportional amounts of each of the variations */
      v = vari[0];
      if (v > 0.0) {
	 /* linear */
	 real nx, ny;
	 nx = tx;
	 ny = ty;
	 p[0] += v * nx;
	 p[1] += v * ny;
      }
      
      v = vari[1];
      if (v > 0.0) {
	 /* sinusoidal */
	 real nx, ny;
	 nx = F(sin)(tx);
	 ny = F(sin)(ty);
	 p[0] += v * nx;
	 p[1] += v * ny;
      }
      
      v = vari[2];
      if (v > 0.0) {
	 /* complex */
	 real nx, ny;
	 real r2 = tx * tx + ty * ty + 1e-6;
	 nx = tx / r2;
	 ny = ty / r2;
	 p[0] += v * nx;
	 p[1] += v * ny;
      }

      v = vari[3];
      if (v > 0.0) {
	 /* swirl */
	 real r2 = tx * tx + ty * ty;  /* /k here is fun */
	 real c1 = F(sin)(r2);
	 real c2 = F(cos)(r2);
	 real nx = c1 * tx - c2 * ty;
	 real ny = c2 * tx + c1 * ty;
	 p[0] += v * nx;
	 p[1] += v * ny;
      }
      
      v = vari[4];
      if (v > 0.0) {
	 /* horseshoe */
	 real a, c1, c2, nx, ny;
	 if (tx < -EPS || tx > EPS ||
	     ty < -EPS || ty > EPS)
	    a = F(atan2)(tx, ty);  /* times k here is fun */
	 else
	    a = 0.0;
	 c1 = F(sin)(a);
	 c2 = F(cos)(a);
	 nx = c1 * tx - c2 * ty;
	 ny = c2 * tx + c1 * ty;
	 p[0] += v * nx;
	 p[1] += v * ny;
      }

      v = vari[5];
      if (v > 0.0) {
	 real nx, ny;
	 if (tx < -EPS || tx > EPS ||
	     ty < -EPS || ty > EPS)
	    nx = F(atan2)(tx, ty) / M_PI;
	 else
	    nx = 0.0;

	 ny = F(sqrt)(tx * tx + ty * ty) - 1.0;
	 p[0] += v * nx;
	 p[1] += v * ny;
      }

      v = vari[6];
      if (v > 0.0) {
	 /* bent */
	 real nx, ny;
	 nx = tx;
	 ny = ty;
	 if (nx < 0.0) nx = nx * 2.0;
	 if (ny < 0.0) ny = ny / 2.0;
	 p[0] += v * nx;
	 p[1] += v * ny;
      }

      
      v = vari[7];
      if (v > 0.0) {
	 /* box */
	 real nx, ny;
	 nx = F(sin)(1/tx);
	 ny = F(cos)(1/ty);
	 p[0] += v * nx;
	 p[1] += v * ny;
      }
      

      /* printf("xxx3\n"); */

      /* if fuse over, store it */
      if (i >= 0) {
	 points[i][0] = p[0];
	 points[i][1] = p[1];
	 points[i][2] = p[2];
      }
   }
#if 0
   if ((count_large > 10 || count_nan > 10)
       && !getenv("PVM_ARCH"))
      fprintf(stderr, "large = %d nan = %d\n", count_large, count_nan);
#endif
}

/* args must be non-overlapping */
void mult_matrix(s1, s2, d)
   real s1[2][2];
   real s2[2][2];
   real d[2][2];
{
   d[0][0] = s1[0][0] * s2[0][0] + s1[1][0] * s2[0][1];
   d[1][0] = s1[0][0] * s2[1][0] + s1[1][0] * s2[1][1];
   d[0][1] = s1[0][1] * s2[0][0] + s1[1][1] * s2[0][1];
   d[1][1] = s1[0][1] * s2[1][0] + s1[1][1] * s2[1][1];
}

real det_matrix(s)
   real s[2][2];
{
   return s[0][0] * s[1][1] - s[0][1] * s[1][0];
}

void flip_matrix(m, h)
   real m[2][2];
   int h;
{
   real s, t;
   if (h) {
      /* flip on horizontal axis */
      s = m[0][0];
      t = m[0][1];
      m[0][0] = m[1][0];
      m[0][1] = m[1][1];
      m[1][0] = s;
      m[1][1] = t;
   } else {
      /* flip on vertical axis */
      s = m[0][0];
      t = m[1][0];
      m[0][0] = m[0][1];
      m[1][0] = m[1][1];
      m[0][1] = s;
      m[1][1] = t;
   }
}

void transpose_matrix(m)
   real m[2][2];
{
   real t;
   t = m[0][1];
   m[0][1] = m[1][0];
   m[1][0] = t;
}

#if 0
real choose_evector(m, r, v)
   real m[3][2], r;
   real v[2];
{
   real b = m[0][1];
   real d = m[1][1];
   real x = r - d;
   if (b > EPS) {
      v[0] = x;
      v[1] = b;
   } else if (b < -EPS) {
      v[0] = -x;
      v[1] = -b;
   } else {
      /* XXX */
      v[0] = 1.0;
      v[1] = 0.0;
   }
}


/* diagonalize the linear part of a 3x2 matrix.  the evalues are returned 
   in r as either reals on the diagonal, or a complex pair.  the evectors
   are returned as a change of coords matrix.  does not handle shearing
   transforms.
   */

void diagonalize_matrix(m, r, v)
   real m[3][2];
   real r[2][2];
   real v[2][2];
{
   real b, c, d;
   real m00, m10, m01, m11;
   m00 = m[0][0];
   m10 = m[1][0];
   m01 = m[0][1];
   m11 = m[1][1];
   b = -m00 - m11;
   c = (m00 * m11) - (m01 * m10);
   d = b * b - 4 * c;
   /* should use better formula, see numerical recipes */
   if (d > EPS) {
      real r0 = (-b + F(sqrt)(d)) / 2.0;
      real r1 = (-b - F(sqrt)(d)) / 2.0;
      r[0][0] = r0;
      r[1][1] = r1;
      r[0][1] = 0.0;
      r[1][0] = 0.0;

      choose_evector(m, r0, v + 0);
      choose_evector(m, r1, v + 1);
   } else if (d < -EPS) {
      real uu = -b / 2.0;
      real vv = F(sqrt)(-d) / 2.0;
      real w1r, w1i, w2r, w2i;
      r[0][0] = uu;
      r[0][1] = vv;
      r[1][0] = -vv;
      r[1][1] = uu;

      if (m01 > EPS) {
	 w1r = uu - m11;
	 w1i = vv;
	 w2r = m01;
	 w2i = 0.0;
      } else if (m01 < -EPS) {
	 w1r = m11 - uu;
	 w1i = -vv;
	 w2r = -m01;
	 w2i = 0.0;
      } else {
	 /* XXX */
	 w1r = 0.0;
	 w1i = 1.0;
	 w2r = 1.0;
	 w2i = 0.0;
      }
      v[0][0] = w1i;
      v[0][1] = w2i;
      v[1][0] = w1r;
      v[1][1] = w2r;

   } else {
      real rr = -b / 2.0;
      r[0][0] = rr;
      r[1][1] = rr;
      r[0][1] = 0.0;
      r[1][0] = 0.0;

      v[0][0] = 1.0;
      v[0][1] = 0.0;
      v[1][0] = 0.0;
      v[1][1] = 1.0;
   }
   /* order the values so that the evector matrix has is positively
      oriented.  this is so that evectors never have to cross when we
      interpolate them. it might mean that the values cross zero when they
      wouldn't have otherwise (if they had different signs) but this is the
      lesser of two evils */
   if (det_matrix(v) < 0.0) {
      flip_matrix(v, 1);
      flip_matrix(r, 0);
      flip_matrix(r, 1);
   }
}


undiagonalize_matrix(r, v, m)
   real r[2][2];
   real v[2][2];
   real m[3][2];
{
   real v_inv[2][2];
   real t1[2][2];
   real t2[2][2];
   real t;
   /* the unfortunate truth is that given we are using row vectors
      the evectors should be stacked horizontally, but the complex
      interpolation functions only work on rows, so we fix things here */
   transpose_matrix(v);
   mult_matrix(r, v, t1);

   t = 1.0 / det_matrix(v);
   v_inv[0][0] = t * v[1][1];
   v_inv[1][1] = t * v[0][0];
   v_inv[1][0] = t * -v[1][0];
   v_inv[0][1] = t * -v[0][1];

   mult_matrix(v_inv, t1, t2);

   /* the unforunate truth is that i have no idea why this is needed. sigh. */
   transpose_matrix(t2);

   /* switch v back to how it was */
   transpose_matrix(v);

   m[0][0] = t2[0][0];
   m[0][1] = t2[0][1];
   m[1][0] = t2[1][0];
   m[1][1] = t2[1][1];
}

#endif

void
interpolate_angle(t, s, v1, v2, v3, tie, cross)
   real t, s;
   real *v1, *v2, *v3;
   int tie;
{
   real x = *v1;
   real y = *v2;
   real d;
   static real lastx, lasty;

   /* take the shorter way around the circle... */
   if (x > y) {
      d = x - y;
      if (d > M_PI + EPS ||
	  (d > M_PI - EPS && tie))
	 y += 2*M_PI;
   } else {
      d = y - x;
      if (d > M_PI + EPS ||
	  (d > M_PI - EPS && tie))
	 x += 2*M_PI;
   }
   /* unless we are supposed to avoid crossing */
   if (cross) {
      if (lastx > x) {
	 if (lasty < y)
	    y -= 2*M_PI;
      } else {
	 if (lasty > y)
	    y += 2*M_PI;
      }
   } else {
      lastx = x;
      lasty = y;
   }

   *v3 = s * x + t * y;
}

void
interpolate_complex(t, s, r1, r2, r3, flip, tie, cross)
   real t, s;
   real r1[2], r2[2], r3[2];
   int flip, tie, cross;
{
   real c1[2], c2[2], c3[2];
   real a1, a2, a3, d1, d2, d3;

   c1[0] = r1[0];
   c1[1] = r1[1];
   c2[0] = r2[0];
   c2[1] = r2[1];
   if (flip) {
      real t = c1[0];
      c1[0] = c1[1];
      c1[1] = t;
      t = c2[0];
      c2[0] = c2[1];
      c2[1] = t;
   }

   /* convert to log space */
   a1 = F(atan2)(c1[1], c1[0]);
   a2 = F(atan2)(c2[1], c2[0]);
   d1 = 0.5 * F(log)(c1[0] * c1[0] + c1[1] * c1[1]);
   d2 = 0.5 * F(log)(c2[0] * c2[0] + c2[1] * c2[1]);

   /* interpolate linearly */
   interpolate_angle(t, s, &a1, &a2, &a3, tie, cross);
   d3 = s * d1 + t * d2;

   /* convert back */
   d3 = F(exp)(d3);
   c3[0] = F(cos)(a3) * d3;
   c3[1] = F(sin)(a3) * d3;

   if (flip) {
      r3[1] = c3[0];
      r3[0] = c3[1];
   } else {
      r3[0] = c3[0];
      r3[1] = c3[1];
   }
}


void interpolate_matrix(t, m1, m2, m3)
   real m1[3][2], m2[3][2], m3[3][2];
   real t;
{
   real s = 1.0 - t;

#if 0
   diagonalize_matrix(m1, r1, v1);
   diagonalize_matrix(m2, r2, v2);

   /* handle the evectors */
   interpolate_complex(t, s, v1 + 0, v2 + 0, v3 + 0, 0, 0, 0);
   interpolate_complex(t, s, v1 + 1, v2 + 1, v3 + 1, 0, 0, 1);

   /* handle the evalues */
   interpolate_complex(t, s, r1 + 0, r2 + 0, r3 + 0, 0, 0, 0);
   interpolate_complex(t, s, r1 + 1, r2 + 1, r3 + 1, 1, 1, 0);

   undiagonalize_matrix(r3, v3, m3);
#endif

   interpolate_complex(t, s, m1 + 0, m2 + 0, m3 + 0, 0, 0, 0);
   interpolate_complex(t, s, m1 + 1, m2 + 1, m3 + 1, 1, 1, 0);

   /* handle the translation part of the xform linearly */
   m3[2][0] = s * m1[2][0] + t * m2[2][0];
   m3[2][1] = s * m1[2][1] + t * m2[2][1];
}

#define INTERP(x)  result->x = c0 * cps[i1].x + c1 * cps[i2].x
#define INTERPI(x)  result->x = (int)(c0 * cps[i1].x + c1 * cps[i2].x)

/*
 * create a control point that interpolates between the control points
 * passed in CPS.  for now just do linear.  in the future, add control
 * point types and other things to the cps.  CPS must be sorted by time.
 */
void interpolate(cps, ncps, time, result)
   ifs_control_point cps[];
   int ncps;
   real time;
   ifs_control_point *result;
{
   int i, j, i1, i2;
   real c0, c1, t;

   if (1 == ncps) {
      *result = cps[0];
      return;
   }
   if (cps[0].time >= time) {
      i1 = 0;
      i2 = 1;
   } else if (cps[ncps - 1].time <= time) {
      i1 = ncps - 2;
      i2 = ncps - 1;
   } else {
      i1 = 0;
      while (cps[i1].time < time)
	 i1++;
      i1--;
      i2 = i1 + 1;
      if (time - cps[i1].time > -1e-7 &&
	  time - cps[i1].time < 1e-7) {
	 *result = cps[i1];
	 return;
      }
   }

   c0 = (cps[i2].time - time) / (cps[i2].time - cps[i1].time);
   c1 = 1.0 - c0;

   result->time = time;

#if have_cmap
   if (cps[i1].cmap_inter) {
     for (i = 0; i < 256; i++) {
       real bright_peak = 2.0;
       real spread = 0.15;
       real d0, d1, e0, e1, c = 2 * M_PI * i / 256.0;
       c = F(cos)(c * cps[i1].cmap_inter) + 4.0 * c1 - 2.0;
       if (c >  spread) c =  spread;
       if (c < -spread) c = -spread;
       d1 = (c + spread) * 0.5 / spread;
       d0 = 1.0 - d1;
       e0 = (d0 < 0.5) ? (d0 * 2) : (d1 * 2);
       e1 = 1.0 - e0;
       for (j = 0; j < 3; j++) {
	 result->cmap[i][j] = (d0 * cps[i1].cmap[i][j] +
			       d1 * cps[i2].cmap[i][j]);
#if 0
	 if (d0 < 0.5)
	   result->cmap[i][j] *= 1.0 + bright_peak * d0;
	 else
	   result->cmap[i][j] *= 1.0 + bright_peak * d1;
#else
	 result->cmap[i][j] = (e1 * result->cmap[i][j] +
			       e0 * 1.0);
#endif
       }
     }
   } else {
     for (i = 0; i < 256; i++) {
       real t[3], s[3];
       rgb2hsv(cps[i1].cmap[i], s);
       rgb2hsv(cps[i2].cmap[i], t);
       for (j = 0; j < 3; j++)
	 t[j] = c0 * s[j] + c1 * t[j];
       hsv2rgb(t, result->cmap[i]);
     }
   }
#endif

   result->cmap_index = -1;
   INTERP(brightness);
   INTERP(contrast);
   INTERP(gamma);
   INTERPI(width);
   INTERPI(height);
   INTERPI(spatial_oversample);
   INTERP(corner[0]);
   INTERP(corner[1]);
   INTERP(pixels_per_unit);
   INTERP(spatial_filter_radius);
   INTERP(sample_density);
   INTERPI(nbatches);
   INTERPI(white_level);
   for (i = 0; i < 2; i++)
      for (j = 0; j < 2; j++) {
	 INTERP(pulse[i][j]);
	 INTERP(wiggle[i][j]);
     }

   for (i = 0; i < NXFORMS; i++) {
      real r;
      INTERP(xform[i].density);
      if (result->xform[i].density > 0)
	 result->xform[i].density = 1.0;
      INTERP(xform[i].color);
      for (j = 0; j < NVARS; j++)
	 INTERP(xform[i].var[j]);
      t = 0.0;
      for (j = 0; j < NVARS; j++)
	 t += result->xform[i].var[j];
      t = 1.0 / t;
      for (j = 0; j < NVARS; j++)
	 result->xform[i].var[j] *= t;

      interpolate_matrix(c1, cps[i1].xform[i].c, cps[i2].xform[i].c,
			 result->xform[i].c);

      /* apply pulse factor. */
      if (1) {
	 real rh_time = time * 2*M_PI / (60.0 * 30.0);
	 r = 1.0;
	 for (j = 0; j < 2; j++)
	    r += result->pulse[j][0] * F(sin)(result->pulse[j][1] * rh_time);
	 for (j = 0; j < 3; j++) {
	    result->xform[i].c[j][0] *= r;
	    result->xform[i].c[j][1] *= r;
	 }

	 /* apply wiggle factor */
	 r = 0.0;
	 for (j = 0; j < 2; j++) {
	    real tt = result->wiggle[j][1] * rh_time;
	    real m = result->wiggle[j][0];
	    result->xform[i].c[0][0] += m *  F(cos)(tt);
	    result->xform[i].c[1][0] += m * -F(sin)(tt);
	    result->xform[i].c[0][1] += m *  F(sin)(tt);
	    result->xform[i].c[1][1] += m *  F(cos)(tt);
	 }
      }
   } /* for i */
}



/*
 * split a string passed in ss into tokens on whitespace.
 * # comments to end of line.  ; terminates the record
 */
void tokenize(ss, argv, argc)
   char **ss;
   char *argv[];
   int *argc;
{
   char *s = *ss;
   int i = 0, state = 0;

   while (*s != ';') {
      char c = *s;
      switch (state) {
       case 0:
	 if ('#' == c)
	    state = 2;
	 else if (!isspace(c)) {
	    argv[i] = s;
	    i++;
	    state = 1;
	 }
       case 1:
	 if (isspace(c)) {
	    *s = 0;
	    state = 0;
	 }
       case 2:
	 if ('\n' == c)
	    state = 0;
      }
      s++;
   }
   *s = 0;
   *ss = s+1;
   *argc = i;
}

int compare_xforms(const void *a0, const void *b0)
{
   real aa[2][2];
   real bb[2][2];
   real ad, bd;
   ifs_xform *a, *b;
   a = (ifs_xform *) a0;
   b = (ifs_xform *) b0;
   aa[0][0] = a->c[0][0];
   aa[0][1] = a->c[0][1];
   aa[1][0] = a->c[1][0];
   aa[1][1] = a->c[1][1];
   bb[0][0] = b->c[0][0];
   bb[0][1] = b->c[0][1];
   bb[1][0] = b->c[1][0];
   bb[1][1] = b->c[1][1];
   ad = det_matrix(aa);
   bd = det_matrix(bb);
   if (ad < bd) return -1;
   if (ad > bd) return 1;
   return 0;
}

#define MAXARGS 1000
#define streql(x,y) (!strcmp(x,y))

/*
 * given a pointer to a string SS, fill fields of a control point CP.
 * return a pointer to the first unused char in SS.  totally barfucious,
 * must integrate with tcl soon...
 */

int parse_control_point(ss, cp) 
   char **ss;
   ifs_control_point *cp;
{
   char *argv[MAXARGS];
   int argc, i, j;
   int set_cm = 0, set_image_size = 0, set_nbatches = 0, set_white_level = 0, set_cmap_inter = 0;
   int set_spatial_oversample = 0;
   real *slot, xf, cm, t, nbatches, white_level, spatial_oversample, cmap_inter;
   real image_size[2];

   slot = &t;

   for (i = 0; i < NXFORMS; i++) {
      cp->xform[i].density = 0.0;
      cp->xform[i].color = (i == 0);
      cp->xform[i].var[0] = 1.0;
      for (j = 1; j < NVARS; j++)
	 cp->xform[i].var[j] = 0.0;
      cp->xform[i].c[0][0] = 1.0;
      cp->xform[i].c[0][1] = 0.0;
      cp->xform[i].c[1][0] = 0.0;
      cp->xform[i].c[1][1] = 1.0;
      cp->xform[i].c[2][0] = 0.0;
      cp->xform[i].c[2][1] = 0.0;
   }
   for (j = 0; j < 2; j++) {
      cp->pulse[j][0] = 0.0;
      cp->pulse[j][1] = 60.0;
      cp->wiggle[j][0] = 0.0;
      cp->wiggle[j][1] = 60.0;
   }
   
   tokenize(ss, argv, &argc);
   for (i = 0; i < argc; i++) {
      if (streql("xform", argv[i]))
	 slot = &xf;
      else if (streql("time", argv[i]))
	 slot = &cp->time;
      else if (streql("brightness", argv[i]))
	 slot = &cp->brightness;
      else if (streql("contrast", argv[i]))
	 slot = &cp->contrast;
      else if (streql("gamma", argv[i]))
	 slot = &cp->gamma;
      else if (streql("image_size", argv[i])) {
	 slot = image_size;
	 set_image_size = 1;
      } else if (streql("corner", argv[i]))
	 slot = cp->corner;
      else if (streql("pulse", argv[i]))
	 slot = cp->pulse[0];
      else if (streql("wiggle", argv[i]))
	 slot = cp->wiggle[0];
      else if (streql("pixels_per_unit", argv[i]))
	 slot = &cp->pixels_per_unit;
      else if (streql("spatial_filter_radius", argv[i]))
	 slot = &cp->spatial_filter_radius;
      else if (streql("sample_density", argv[i]))
	 slot = &cp->sample_density;
      else if (streql("nbatches", argv[i])) {
	 slot = &nbatches;
	 set_nbatches = 1;
      } else if (streql("white_level", argv[i])) {
	 slot = &white_level;
	 set_white_level = 1;
      } else if (streql("spatial_oversample", argv[i])) {
	 slot = &spatial_oversample;
	 set_spatial_oversample = 1;
      } else if (streql("cmap", argv[i])) {
	 slot = &cm;
	 set_cm = 1;
      } else if (streql("density", argv[i]))
	 slot = &cp->xform[(int)xf].density;
      else if (streql("color", argv[i]))
	 slot = &cp->xform[(int)xf].color;
      else if (streql("coefs", argv[i])) {
	 slot = cp->xform[(int)xf].c[0];
	 cp->xform[(int)xf].density = 1.0;
       } else if (streql("var", argv[i]))
	 slot = cp->xform[(int)xf].var;
      else if (streql("cmap_inter", argv[i])) {
	slot = &cmap_inter;
	set_cmap_inter = 1;
      } else
	 *slot++ = atof(argv[i]);
   }
#if have_cmap
   if (set_cm) {
      cp->cmap_index = (int) cm;
      get_cmap(cp->cmap_index, cp->cmap, 256);
   }
#endif
   if (set_image_size) {
      cp->width  = (int) image_size[0];
      cp->height = (int) image_size[1];
   }
   if (set_cmap_inter)
      cp->cmap_inter  = (int) cmap_inter;
   if (set_nbatches)
      cp->nbatches = (int) nbatches;
   if (set_spatial_oversample)
      cp->spatial_oversample = (int) spatial_oversample;
   if (set_white_level)
      cp->white_level = (int) white_level;
   for (i = 0; i < NXFORMS; i++) {
      t = 0.0;
      for (j = 0; j < NVARS; j++)
	 t += cp->xform[i].var[j];
      t = 1.0 / t;
      for (j = 0; j < NVARS; j++)
	 cp->xform[i].var[j] *= t;
   }
   qsort((char *) cp->xform, NXFORMS, sizeof(ifs_xform), compare_xforms);
   return 0;
}

void print_control_point(f, cp, quote)
   FILE *f;
   ifs_control_point *cp;
   int quote;
{
   int i, j;
   char *q = quote ? "# " : "";
   fprintf(f, "%stime %g\n", q, cp->time);
   if (-1 != cp->cmap_index)
      fprintf(f, "%scmap %d\n", q, cp->cmap_index);
   fprintf(f, "%simage_size %d %d corner %g %g pixels_per_unit %g\n",
	   q, cp->width, cp->height, cp->corner[0], cp->corner[1],
	   cp->pixels_per_unit);
   fprintf(f, "%sspatial_oversample %d spatial_filter_radius %g",
	   q, cp->spatial_oversample, cp->spatial_filter_radius);
   fprintf(f, " sample_density %g\n", cp->sample_density);
   fprintf(f, "%snbatches %d white_level %d\n",
	   q, cp->nbatches, cp->white_level);
   fprintf(f, "%sbrightness %g gamma %g cmap_inter %d\n",
	   q, cp->brightness, cp->gamma, cp->cmap_inter);

   for (i = 0; i < NXFORMS; i++)
      if (cp->xform[i].density > 0.0) {
	 fprintf(f, "%sxform %d density %g color %g\n",
		q, i, cp->xform[i].density, cp->xform[i].color);
	 fprintf(f, "%svar", q);
	 for (j = 0; j < NVARS; j++)
	    fprintf(f, " %g", cp->xform[i].var[j]);
	 fprintf(f, "\n%scoefs", q);
	 for (j = 0; j < 3; j++)
	    fprintf(f, " %g %g", cp->xform[i].c[j][0], cp->xform[i].c[j][1]);
	 fprintf(f, "\n");
      }
   fprintf(f, "%s;\n", q);
}


void sprint_control_point(buf, cp, quote)
   char *buf;
   ifs_control_point *cp;
   int quote;
{
  char f[200];
   int i, j;
   char *q = quote ? "# " : "";
   sprintf(f, "%stime %g\n", q, cp->time);
   strcat(buf, f);
   if (-1 != cp->cmap_index) {
      sprintf(f, "%scmap %d\n", q, cp->cmap_index);
      strcat(buf, f);
   }
   sprintf(f, "%simage_size %d %d corner %g %g pixels_per_unit %g\n",
	   q, cp->width, cp->height, cp->corner[0], cp->corner[1],
	   cp->pixels_per_unit);
   strcat(buf, f);
   sprintf(f, "%sspatial_oversample %d spatial_filter_radius %g",
	   q, cp->spatial_oversample, cp->spatial_filter_radius);
   strcat(buf, f);
   sprintf(f, " sample_density %g\n", cp->sample_density);
   strcat(buf, f);
   sprintf(f, "%snbatches %d white_level %d\n",
	   q, cp->nbatches, cp->white_level);
   strcat(buf, f);
   sprintf(f, "%sbrightness %g gamma %g cmap_inter %d\n",
	   q, cp->brightness, cp->gamma, cp->cmap_inter);
   strcat(buf, f);

   for (i = 0; i < NXFORMS; i++)
      if (cp->xform[i].density > 0.0) {
	 sprintf(f, "%sxform %d density %g color %g\n",
		q, i, cp->xform[i].density, cp->xform[i].color);
	 strcat(buf, f);
	 sprintf(f, "%svar", q);
	 strcat(buf, f);
	 for (j = 0; j < NVARS; j++) {
	    sprintf(f, " %g", cp->xform[i].var[j]);
	    strcat(buf, f);
	 }
	 sprintf(f, "\n%scoefs", q);
	 strcat(buf, f);
	 for (j = 0; j < 3; j++) {
	    sprintf(f, " %g %g", cp->xform[i].c[j][0], cp->xform[i].c[j][1]);
	    strcat(buf, f);
	 }
	 sprintf(f, "\n");
	 strcat(buf, f);
      }
   sprintf(f, "%s;\n", q);
   strcat(buf, f);
}

/* returns a uniform variable from 0 to 1 */
real random_uniform01() {
   return (R & 0xffff) / (real) 0xffff;
}

real random_uniform11() {
   return ((R & 0xffff) - 0x7fff) / (real) 0x8000;
}

/* returns a mean 0 variance 1 random variable
   see numerical recipies p 217 */
real random_gaussian() {
   static int iset = 0;
   static real gset;
   real fac, r, v1, v2;

   if (0 == iset) {
      do {
	 v1 = random_uniform11();
	 v2 = random_uniform11();
	 r = v1 * v1 + v2 * v2;
      } while (r >= 1.0 || r == 0.0);
      fac = F(sqrt)(-2.0 * F(log)(r)/r);
      gset = v1 * fac;
      iset = 1;
      return v2 * fac;
   }
   iset = 0;
   return gset;
}

#define random_distrib(v) ((v)[R%alen(v)])

void random_control_point(cp) 
   ifs_control_point *cp;
{
   int i, nxforms, var;
   static int xform_distrib[] = {
      2, 2, 2,
      3, 3, 3,
      4, 4,
      5};
   static int var_distrib[] = {
      -1, -1, -1,
      0, 0, 0, 0,
      1, 1, 1,
      2, 2, 2,
      3, 3,
      4, 4,
      5,
      6,
      7, 7,
   };

   static int mixed_var_distrib[] = {
      0, 0, 0,
      1, 1, 1,
      2, 2, 2,
      3, 3,
      4, 4,
      5, 5,
      6,
      7, 7,
   };

#if have_cmap
   get_cmap(cmap_random, cp->cmap, 256);
#endif
   cp->time = 0.0;
   nxforms = random_distrib(xform_distrib);
   var = random_distrib(var_distrib);
   for (i = 0; i < nxforms; i++) {
      int j, k;
      cp->xform[i].density = 1.0 / nxforms;
      cp->xform[i].color = i == 0;
      for (j = 0; j < 3; j++)
	 for (k = 0; k < 2; k++)
	    cp->xform[i].c[j][k] = random_uniform11();
      for (j = 0; j < NVARS; j++)
	 cp->xform[i].var[j] = 0.0;
      if (var >= 0)
	 cp->xform[i].var[var] = 1.0;
      else
	 cp->xform[i].var[random_distrib(mixed_var_distrib)] = 1.0;
      
   }
   for (; i < NXFORMS; i++)
      cp->xform[i].density = 0.0;
}

/*
 * find a 2d bounding box that does not enclose eps of the fractal density
 * in each compass direction.  works by binary search.
 * this is stupid, it shouldjust use the find nth smallest algorithm.
 */
void estimate_bounding_box(cp, eps, bmin, bmax)
   ifs_control_point *cp;
   real eps;
   real *bmin;
   real *bmax;
{
   int i, j, batch = (int)((eps == 0.0) ? 10000 : 10.0/eps);
   int low_target = (int)(batch * eps);
   int high_target = batch - low_target;
   ifs_point min, max, delta;
   ifs_point *points = (ifs_point *)  malloc(sizeof(ifs_point) * batch);
   iterate(cp, batch, 20, points);

   min[0] = min[1] =  1e10;
   max[0] = max[1] = -1e10;
   
   for (i = 0; i < batch; i++) {
      if (points[i][0] < min[0]) min[0] = points[i][0];
      if (points[i][1] < min[1]) min[1] = points[i][1];
      if (points[i][0] > max[0]) max[0] = points[i][0];
      if (points[i][1] > max[1]) max[1] = points[i][1];
   }

   if (low_target == 0) {
      bmin[0] = min[0];
      bmin[1] = min[1];
      bmax[0] = max[0];
      bmax[1] = max[1];
      return;
   }
   
   delta[0] = (max[0] - min[0]) * 0.25;
   delta[1] = (max[1] - min[1]) * 0.25;

   bmax[0] = bmin[0] = min[0] + 2.0 * delta[0];
   bmax[1] = bmin[1] = min[1] + 2.0 * delta[1];

   for (i = 0; i < 14; i++) {
      int n, s, e, w;
      n = s = e = w = 0;
      for (j = 0; j < batch; j++) {
	 if (points[j][0] < bmin[0]) n++;
	 if (points[j][0] > bmax[0]) s++;
	 if (points[j][1] < bmin[1]) w++;
	 if (points[j][1] > bmax[1]) e++;
      }
      bmin[0] += (n <  low_target) ? delta[0] : -delta[0];
      bmax[0] += (s < high_target) ? delta[0] : -delta[0];
      bmin[1] += (w <  low_target) ? delta[1] : -delta[1];
      bmax[1] += (e < high_target) ? delta[1] : -delta[1];
      delta[0] = delta[0] / 2.0;
      delta[1] = delta[1] / 2.0;
      /*
      fprintf(stderr, "%g %g %g %g\n", bmin[0], bmin[1], bmax[0], bmax[1]);
      */
   }
   /*
   fprintf(stderr, "%g %g %g %g\n", min[0], min[1], max[0], max[1]);
   */
}

/* use hill climberer to find smooth ordering of control points
   this is untested */
   
void sort_control_points(cps, ncps, metric)
   ifs_control_point *cps;
   int ncps;
   real (*metric)();
{
   int niter = ncps * 1000;
   int i, n, m;
   real same, swap;
   for (i = 0; i < niter; i++) {
      /* consider switching points with indexes n and m */
      n = R % ncps;
      m = R % ncps;

      same = (metric(cps + n, cps + (n - 1) % ncps) +
	      metric(cps + n, cps + (n + 1) % ncps) +
	      metric(cps + m, cps + (m - 1) % ncps) +
	      metric(cps + m, cps + (m + 1) % ncps));

      swap = (metric(cps + n, cps + (m - 1) % ncps) +
	      metric(cps + n, cps + (m + 1) % ncps) +
	      metric(cps + m, cps + (n - 1) % ncps) +
	      metric(cps + m, cps + (n + 1) % ncps));

      if (swap < same) {
	 ifs_control_point t;
	 t = cps[n];
	 cps[n] = cps[m];
	 cps[m] = t;
      }
   }
}

/* this has serious flaws in it */

real standard_metric(cp1, cp2)
   ifs_control_point *cp1, *cp2;
{
   int i, j, k;
   real t;
   
   real dist = 0.0;
   for (i = 0; i < NXFORMS; i++) {
      real var_dist = 0.0;
      real coef_dist = 0.0;
      for (j = 0; j < NVARS; j++) {
	 t = cp1->xform[i].var[j] - cp2->xform[i].var[j];
	 var_dist += t * t;
      }
      for (j = 0; j < 3; j++)
	 for (k = 0; k < 2; k++) {
	    t = cp1->xform[i].c[j][k] - cp2->xform[i].c[j][k];
	    coef_dist += t *t;
	 }

      /* weight them equally for now. */
      dist += var_dist + coef_dist;
   }
   return dist;
}

#if 0
void
stat_matrix(f, m)
   FILE *f;
   real m[3][2];
{
   real r[2][2];
   real v[2][2];
   real a;

   diagonalize_matrix(m, r, v);
   fprintf(f, "entries = % 10f % 10f % 10f % 10f\n",
	   m[0][0], m[0][1], m[1][0], m[1][1]);
   fprintf(f, "evalues  = % 10f % 10f % 10f % 10f\n",
	   r[0][0], r[0][1], r[1][0], r[1][1]);
   fprintf(f, "evectors = % 10f % 10f % 10f % 10f\n",
	   v[0][0], v[0][1], v[1][0], v[1][1]);
   a = (v[0][0] * v[1][0] + v[0][1] * v[1][1]) /
      F(sqrt)((v[0][0] * v[0][0] + v[0][1] * v[0][1]) *
	      (v[1][0] * v[1][0] + v[1][1] * v[1][1]));
   fprintf(f, "theta = %g det = %g\n", a,
	   m[0][0] * m[1][1] - m[0][1] * m[1][0]);
}

#endif


#if 0
main()
{
#if 0
   real m1[3][2] = {-0.633344, -0.269064, 0.0676171, 0.590923, 0, 0};
   real m2[3][2] = {-0.844863, 0.0270297, -0.905294, 0.413218, 0, 0};
#endif

#if 0
   real m1[3][2] = {-0.347001, -0.15219, 0.927161, 0.908305, 0, 0};
   real m2[3][2] = {-0.577884, 0.653803, 0.664982, -0.734136, 0, 0};
#endif

#if 0
   real m1[3][2] = {1, 0, 0, 1, 0, 0};
   real m2[3][2] = {0, -1, 1, 0, 0, 0};
#endif

#if 1
   real m1[3][2] = {1, 0, 0, 1, 0, 0};
   real m2[3][2] = {-1, 0, 0, -1, 0, 0};
#endif

   real m3[3][2];
   real t;
   int i = 0;

   for (t = 0.0; t <= 1.0; t += 1.0/15.0) {
      int x, y;
      fprintf(stderr, "%g--\n", t);
      interpolate_matrix(t, m1, m2, m3);
/*       stat_matrix(stderr, m3); */
      x = (i % 4) * 100 + 100;
      y = (i / 4) * 100 + 100;
      printf("newpath ");
      printf("%d %d %d %d %d arc ", x, y, 30, 0, 360);
      printf("%d %d moveto ", x, y);
      printf("%g %g rlineto ", m3[0][0] * 30, m3[0][1] * 30);
      printf("%d %d moveto ", x, y);
      printf("%g %g rlineto ", m3[1][0] * 30, m3[1][1] * 30);
      printf("stroke \n");
      printf("newpath ");
      printf("%g %g %d %d %d arc ", x + m3[0][0] * 30, y + m3[0][1] * 30, 3, 0, 360);
      printf("stroke \n");
      i++;
   }
}
#endif
