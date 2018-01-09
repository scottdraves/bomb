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

static char *fuse_c_id = "@(#) $Id: fuse.c,v 1.1.1.1 2002/12/08 20:49:48 spotspot Exp $";

#include "defs.h"
#include "image.h"
#include "image_db.h"
#include "match.h"

#define MAX_ORDER 1000

#define FR R8b

#define nclimbers 200
struct {
   Image src;
   image8_t dst;
   int score;
} tiles[nclimbers];
int tiles_ready = 0;
int current_climber = 0;

void step_rule_fuse(int frame, rule_t *p, image8_t *fb)
{
   int i;
   image8_t dest = *fb;
   static int order = 0;
   int bsize;
   extern int scramble;
   int dr = iclamp(p->drift, fuse_ndrifts);

   if (scramble) {
     dr = 0;
     scramble--;
   }

   switch (dr) {
    case 0:
      bsize = p->bsize;
      /* bsize = TILE_SIZE; */
      dest.width = bsize;
      dest.height = bsize;

      for (i = 0; i < 100; i++) {
	 int dest_x = FR%(fb->width - bsize);
	 int dest_y = FR%(fb->height - bsize);
	 Image *src_img =
	    &global_images[iclamp(R, p->image_window_size)];
	 Image src_rect;
	 image_random_tile(&src_rect, src_img, bsize);
	 dest.p = fb->p + fb->stride * dest_y + dest_x;
	 image8_blit(&src_rect, &dest);
      }
      order = (int)(0.8 * order);
      break;
    case 1:
      if (1) {
	int ntries;
	int tuner;
	int nb;
	
	bsize = TILE_SIZE;
	dest.width = bsize;
	dest.height = bsize;

	tuner = p->search_time;
	if (tuner < 0)
	  tuner = 0;
	else if (tuner > 10)
	  tuner = 10;
	p->search_time = tuner;

	ntries = 1 + (int)(order/10 * pow(1.5, tuner));
	nb = 500/ntries;
	if (nb < 1) nb = 1;
#if 0
	printf("ntries=%d nb=%d\n", ntries, nb);
#endif
	for (i = 0; i < nb; i++) {
	  int dest_x, dest_y;
	  Image match;
	  order++;
	  if (order > MAX_ORDER)
	    order = MAX_ORDER;

	  dest_x = FR%(fb->width - bsize);
	  dest_y = FR%(fb->height - bsize);
	  dest.p = fb->p + dest_x + dest_y * fb->stride;

	  image8_match2(&dest, &match,
			global_images, global_images_small,
			p->image_window_size,
			1 + ntries/(SMALL_FACTOR * SMALL_FACTOR), ntries);
	  image8_blit(&match, &dest);
	}
      }
      break;

      /*
    case 2:
      bsize = p->bsize;
      if (!tiles_ready) {
	 int i;
	 for (i = 0; i < nclimbers; i++) {
	    int dest_x, dest_y;
	    Image *src_img =
	       &global_images[iclamp(R, p->image_window_size)];
	    image_random_tile(&tiles[i].src, src_img, bsize);

	    dest_x = FR%(fb->width - bsize);
	    dest_y = FR%(fb->height - bsize);
	    tiles[i].dst.stride = fb->stride;
	    tiles[i].dst.width = bsize;
	    tiles[i].dst.height = bsize;
	    tiles[i].dst.p = fb->p + fb->stride * dest_y + dest_x;
	 }
	 tiles_ready = 1;
      }
      if (1) {
	 int i;
	 int last = current_climber + 2;
	 for (i = current_climber; i < last; i++) {
	    int j = i%nclimbers;
	    if (image8_climb(&tiles[j].dst, &tiles[j].src)) {
	       int dest_x, dest_y;
	       dest_x = FR%(fb->width - bsize);
	       dest_y = FR%(fb->height - bsize);
	       dest.p = fb->p + dest_x + dest_y * fb->stride;
	    }
	 }
	 for (i = current_climber; i < last; i++) {
	    int j = i%nclimbers;
	    image8_blit(&tiles[j].src, &tiles[j].dst);
	 }
	 current_climber = last;
      }
      break;
      */
    case 2:
      for (i = 0; i < 5; i++) {
	 static int track = 0;
	 Image src_img =
	    global_images[iclamp(track++, p->image_window_size)];
	 image8_t dst = *fb;
	 if (dst.width < src_img.width) {
	    src_img.pixels += (src_img.width - dst.width) / 2;
	    src_img.width = dst.width;
	 } else {
	    dst.p += (dst.width - src_img.width) / 2;
	    dst.width = src_img.width;
	 }
	 if (dst.height < src_img.height) {
	    src_img.pixels += src_img.stride * ((src_img.height - dst.height) / 2);
	    src_img.height = dst.height;
	 } else {
	    dst.p += dst.stride * ((dst.height - src_img.height) / 2);
	    dst.height = src_img.height;
	 }
	 image8_blit(&src_img, &dst);
	 order = MAX_ORDER;
      }
      break;
    case 3:
      for (i = 0; i < 5; i++) {
	 static int track = 0;
	 Image src_img =
	    global_images[iclamp(track++, p->image_window_size)];
	 image8_t dst = *fb;
	 if (dst.width < src_img.width) {
	    src_img.pixels += (src_img.width - dst.width) / 2;
	    src_img.width = dst.width;
	 } else if (dst.width == src_img.width) {
	    src_img.width = dst.width;
	 } else {
	    dst.p += FR%((dst.width - src_img.width));
	    dst.width = src_img.width;
	 }
	 if (dst.height < src_img.height) {
	    src_img.pixels += src_img.stride * ((src_img.height - dst.height) / 2);
	    src_img.height = dst.height;
	 } else if (dst.height == src_img.height) {
	   dst.height = src_img.height;
	 } else {
	    dst.p += dst.stride * (FR%(dst.height - src_img.height));
	    dst.height = src_img.height;
	 }
	 image8_blit(&src_img, &dst);
	 order = MAX_ORDER;
      }
      break;
   }
}
