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

static char *match_c_id = "@(#) $Id: match.c,v 1.1.1.1 2002/12/08 20:49:50 spotspot Exp $";

#include <math.h>

#include "defs.h"
#include "image.h"
#include "image_db.h"
#include "match.h"


#define FR R8b

/* dot product of two sub-rectangles.  the image8_t is 8 bit per pixel and
   the Image is 32, so we compare only to SIG_CHAN */

#define SIG_CHAN 1

int
image8_compare(Image *image1, image8_t *image2)
{
  int t, i, j;
  int diff = 0;
  Image im1 = *image1;
  image8_t im2 = *image2;

  for (i = 0; i < im1.height; i++)
     for (j = 0; j < im1.width; j++) {
	u_char *p1 = (u_char *)(im1.pixels + (i * im1.stride + j));
	u_char *p2 = (u_char *)(im2.p + (i * im2.stride + j));
	u_char  g = p1[SIG_CHAN];
	t = g - *p2;
	diff += t * t;

     }
  return diff;
}

int
image8_climb(image8_t *base, Image *tile)
{
   Image try;
   int t, current = image8_compare(tile, base);
   if (0 == current)
      return 1;
      
   try = *tile;
   if (try.origin_y + try.height < try.parent->height) {
      try.pixels += try.stride;
      try.origin_y++;
      t = image8_compare(&try, base);
      if (t < current) {
	 *tile = try;
	 current = t;
      }
      try.pixels -= try.stride;
      try.origin_y--;
   }
   if (try.origin_y > 0) {
      try.pixels -= try.stride;
      try.origin_y--;
      t = image8_compare(&try, base);
      if (t < current) {
	 *tile = try;
	 current = t;
      }
      try.pixels += try.stride;
      try.origin_y++;
   }
   if (try.origin_x + try.width < try.parent->width) {
      try.origin_x++;
      try.pixels += 1;
      t = image8_compare(&try, base);
      if (t < current) {
	 *tile = try;
	 current = t;
      }
      try.origin_x--;
      try.pixels -= 1;
   }
   if (try.origin_x > 0) {
      try.pixels -= 1;
      try.origin_x--;
      t = image8_compare(&try, base);
      if (t < current) {
	 *tile = try;
	 current = t;
      }
      try.pixels += 1;
      try.origin_x++;
   }
   return 0;
}

void
image8_match_init(image8_t *base,
		  Image *images, int nimages,
		  int nbest, Image *best, int *best_diffs, int *origins)
{
   int i;
   int w = base->width;
   int h = base->height;
   for (i = 0; i < nbest; i++) {
      int ii = R % nimages;
      Image *img = &images[ii];
      origins[i] = ii;
      if (0 >  (img->width - w)) {
	printf("pping\n");
	exit(1);
      }
      if (0 >  (img->height - h)) {
	printf("zing\n");
	exit(1);
      }
      (void) image_subimage(best + i, img,
			    R % (img->width - w + 1),
			    R % (img->height - h + 1),
			    w, h);
      best_diffs[i] = image8_compare(best + i, base);
   }
}

/* BIG_IMAGES and SMALL_IMAGES are NIMAGES length arrays of images.  the
   small ones are filtered down (by SMALL_FACTOR) versions of the big ones.
   search for PATTERN and return the best match found in MATCH.  it tries
   NSMALL matches in each of the small images, and the NBIG best of those
   are matched against the big images. */

#define MAX_NBIG 300

void
image8_match2(image8_t *pattern, Image *match,
	      Image *big_images, Image *small_images, int nimages,
	      int nbig, int nsmall)
{
   int i;
   Image big_match;
   static Image small_matches[MAX_NBIG];
   static int diffs[MAX_NBIG];
   static int origins[MAX_NBIG];
   int best;

   if (nbig > MAX_NBIG) {
      nbig = MAX_NBIG;
      fprintf(stderr, "nbig truncated from %d to %d\n",
	      nbig, MAX_NBIG);
   }

   image8_filter_down(pattern, &small_pattern);

   image8_match_init(&small_pattern, small_images, nimages,
		     nbig, small_matches, diffs, origins);

   for (i = 0; i < nimages; i++)
      image8_match(&small_pattern, &small_images[i],
		   nsmall, nbig, small_matches, diffs, origins, i);

   best = 1<<31;
   for (i = 0; i < nbig; i++) {
      Image *small_match = &small_matches[i];
      int match_i, diff;
      match_i = origins[i];
      image_subimage(&big_match, &big_images[match_i],
		     small_match->origin_x * SMALL_FACTOR,
		     small_match->origin_y * SMALL_FACTOR,
		     pattern->width, pattern->height);
      diff = image8_compare(&big_match, pattern);
      if (0 == i || diff < best) {
	 best = diff;
	 *match = big_match;
      }
   }
}



int
image8_find_largest(int n, int *diffs)
{
   int i;
   int j = 0;
   int v = diffs[j];

   for (i = 1; i < n; i++)
      if (diffs[i] > v) {
	 v = diffs[i];
	 j = i;
      }
   return j;
}
   

/* match NTRIES rectangles in IMAGE against BASE.  return the NBEST matches
   in BEST, and how good each match is in BEST_DIFFS.  the BEST and
   BEST_DIFFS arrays must be initialized on entry.  should use a heap
   instead of an array */

void
image8_match(image8_t *base,
	     Image *image, int ntries,
	     int nbest, Image *best, int *best_diffs,
	     int *origins, int index)
{
   int w = base->width;
   int h = base->height;
   int y, j;
   int n_per_row, d_row;
   int worst_of_best;
   int worst_index;
   Image try;

   if (ntries > (image->height - h)) {
      n_per_row = (int) ceil(ntries / (double)(image->height - h + 1));
      d_row = 1;
   } else {
      n_per_row = 1;
      d_row = (int) ceil((image->height - h + 1) / (double)ntries);
   }

   worst_index = image8_find_largest(nbest, best_diffs);
   worst_of_best = best_diffs[worst_index];

   image_subimage(&try, image, 0, 0, w, h);
			    
   for (y = 0; y < image->height - h; y += d_row) {
      Pixel *row_base = image->pixels + y * image->stride;
      try.origin_y = y;
      for (j = 0; j < n_per_row; j++) {
	 int diff;
	 try.origin_x = FR % (image->width - w + 1);
	 try.pixels = row_base + try.origin_x;
	 diff = image8_compare(&try, base);
	 if (diff < worst_of_best) {
	    origins[worst_index] = index;
	    best_diffs[worst_index] = diff;
	    best[worst_index] = try;
	    worst_index = image8_find_largest(nbest, best_diffs);
	    worst_of_best = best_diffs[worst_index];
	 }
      }
   }
}

void
image8_fill(image8_t *image, int v)
{
   int w = image->width;
   int h = image->height;
   u_char *p = image->p;
   int s = image->stride;
   int i, j;
   
   for (i = 0; i < h; i++)
      for (j = 0; j < w; j++)
	 p[i * s + j] = v;
}


void
image8_filter_down(image8_t *from, image8_t *to)
{
   image8_t f = *from;
   image8_t t = *to;
   int h_scale = f.height / t.height;
   int w_scale = f.width / t.width;
   int i, j;
   int npix;
   npix = h_scale * w_scale;

   for (i = 0; i < t.height; i++)
      for (j = 0; j < t.width; j++) {
	 int ii, jj;
	 int tot = 0;
	 for (ii = 0; ii < h_scale; ii++)
	    for (jj = 0; jj < w_scale; jj++) {
	       tot += f.p[f.stride * (i * h_scale + ii) +
			  (j * w_scale + jj)];
	    }
	 t.p[t.stride * i + j] = tot / npix;
      }
}

/* 24->8, just uses the red channel */
void
image8_blit(Image *from, image8_t *to)
{
   Image f = *from;
   int stride = to->stride;
   u_char *tp = to->p;
   int i, j;

#if 1
   if (f.height > to->height)
      fprintf(stderr, "f.height > to->height (%d > %d)\n",
	      f.height, to->height);
   if (f.width > to->width)
      fprintf(stderr, "f.width > to->width (%d > %d)\n",
	      f.width, to->width);
#endif

   for (i = 0; i < f.height; i++)
      for (j = 0; j < f.width; j++) {
	 tp[stride * i + j] =
	    f.pixels[f.stride * i + j].r;
      }
}

void
image8_blit8(image8_t *from, image8_t *to)
{
   int w = from->width;
   int h = from->height;
   u_char *fp = from->p;
   int fstride = from->stride;
   int tstride = to->stride;
   u_char *tp = to->p;
   int i, j;

   for (i = 0; i < h; i++)
      for (j = 0; j < w; j++)
	 tp[tstride * i + j] = fp[fstride * i + j];
}
