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

static char *match_h_id = "@(#) $Id: match.h,v 1.1.1.1 2002/12/08 20:49:50 spotspot Exp $";

void image8_match2(image8_t *pattern, Image *match,
		   Image *big_images, Image *small_images, int nimages,
		   int nbig_tries, int nsmall_tries);


void image8_match(image8_t *base,
		  Image *image, int ntries,
		  int nbest, Image *best, int *best_diffs,
		  int *origins, int index);
int image8_compare(Image *image1, image8_t *image2);
void image8_filter_down(image8_t *from, image8_t *to);
void image8_blit(Image *from, image8_t *to);
void image8_blit8(image8_t *from, image8_t *to);
void image8_fill(image8_t *image, int v);
int image8_climb(image8_t *base, Image *tile);
