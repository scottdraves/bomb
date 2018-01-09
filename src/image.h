/*
    bomb - automatic interactive visual stimulation
    Copyright (C) 1995  Scott Draves <spot@cs.cmu.edu>

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

static char *image_h_id = "@(#) $Id: image.h,v 1.1.1.1 2002/12/08 20:49:49 spotspot Exp $";

/* simple image package.
   images are uniformly chunky 32 bit.
   subimages are handled (basically instead of rect struct)
   allocation is bizarre, redo
   */
 
typedef struct {unsigned char r, g, b, a;} Pixel;

#define alpha_mask   0xff000000
#define rgb_mask     0x00ffffff
#define alpha_solid  0xff
#define alpha_clear  0x00

typedef struct Image {
  Pixel *pixels; /* only a freeable pointer if NULL==parent */
  int width, height, stride;
  int origin_x, origin_y; /* vector from origin of parent to this image */
  struct Image *parent;
} Image;

#define TCL_ERROR 0
#define TCL_OK    1

/* memory mgt */
extern void image_init(Image *);
extern Image *image_allocate(Image *, int, int);
extern Image *image_subimage(Image *image, Image *of, int x, int y, int w, int h);
extern void image_destroy(Image *);

extern Image *image_random_tile(Image *image, Image *of, int size);
extern void image_mean_pixel(Image *img, Pixel *pix);

/* io */
extern int image_read(Image *, char *);

/* ops */
extern void image_filter_down(Image *from, Image *to);
extern void image_blit(Image *from, Image *to);
extern void image_blit_masked(Image *from, Image *to);
extern void image_fill(Image *, Pixel);
extern void image_random(Image *);

/* associative search */
extern void image_match_fast(Image *base, Image *image,
			     Image *best, int ntries, double *best_diff);
