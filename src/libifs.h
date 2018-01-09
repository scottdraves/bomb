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


#ifndef libifs_included
#define libifs_included

static char *libifs_h_id = "@(#) $Id: libifs.h,v 1.1.1.1 2002/12/08 20:49:49 spotspot Exp $";

#include <stdio.h>

#define NVARS   8
#define NXFORMS 6

typedef double ifs_point[3];

typedef struct {
   double var[NVARS];   /* normalized interp coefs between variations */
   double c[3][2];      /* the coefs to the affine part of the function */
   double density;      /* prob is this function is chosen. 0 - 1 */
   double color;        /* color coord for this function. 0 - 1 */
} ifs_xform;

#define have_cmap 0

typedef struct {
   ifs_xform xform[NXFORMS];
#if have_cmap
   cmap cmap;
#endif
   double time;
   int  cmap_index;
   double brightness;           /* 1.0 = normal */
   double contrast;             /* 1.0 = normal */
   double gamma;
   int  width, height;        /* of the final image */
   int  spatial_oversample;
   double corner[2];            /* camera corner */
   double pixels_per_unit;      /* and scale */
   double spatial_filter_radius; /* variance of gaussian */
   double sample_density;        /* samples per pixel (not bucket) */
   /* in order to motion blur more accurately we compute the logs of the 
      sample density many times and average the results.  we interplate
      only this many times. */
   int nbatches;
   /* this much color resolution.  but making it too high induces clipping */
   int white_level;
   int cmap_inter; /* if this is true, then color map interpolates one entry
		      at a time with a bright edge */
   double pulse[2][2]; /* [i][0]=magnitute [i][1]=frequency */
   double wiggle[2][2]; /* frequency is /minute, assuming 30 frames/s */
} ifs_control_point;


#if __STDC__
# define P(x)x
#else
# define P(x)()
#endif



extern void iterate P((ifs_control_point *cp, int n, int fuse, ifs_point points[]));
extern void interpolate P((ifs_control_point cps[], int ncps, double time, ifs_control_point *result));
extern void tokenize P((char **ss, char *argv[], int *argc));
extern void print_control_point P((FILE *f, ifs_control_point *cp, int quote));
extern void sprint_control_point P((char *f, ifs_control_point *cp, int quote));
extern void random_control_point P((ifs_control_point *cp));
extern void estimate_bounding_box P((ifs_control_point *cp, double eps, double *bmin, double *bmax));
extern void sort_control_points P((ifs_control_point *cps, int ncps, double (*metric)()));
extern double standard_metric P((ifs_control_point *cp1, ifs_control_point *cp2));
extern double random_uniform01 P(());
extern double random_uniform11 P(());
extern double random_gaussian P(());
extern void mult_matrix P((double s1[2][2], double s2[2][2], double d[2][2]));
extern int parse_control_point();
#endif
