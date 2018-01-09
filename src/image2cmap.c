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

#include "defs.h"
#include "image.h"

/* simple hill climber, circular topology */

int dist(Pixel p0, Pixel p1) {
   int d;
   int r;
   d = p0.r - p1.r;
   r = d * d;
   d = p0.g - p1.g;
   r += d * d;
   d = p0.b - p1.b;
   r += d * d;
   return r;
}

double total_length(Pixel *cmap, int n) {
   int i;
   double t = dist(cmap[0], cmap[n-1]);
   for (i = 1; i < n; i++)
      t += dist(cmap[i-1], cmap[i]);
   return t / 1000.0;
}

void improve_cmap_order(Pixel *cmap, int n) {
   int i, i0, i1, niters;
   int total_d = 0;
   Pixel t;

   niters = (int)(argd("niters", 100) * 200000);
   for (i = 0; i < niters; i++) {
      int as_is, swapped;
      i0 = R%n;
      i1 = R%n;
      as_is = (dist(cmap[(i0-1)%n], cmap[i0]) +
	       dist(cmap[i0], cmap[(i0+1)%n]) +
	       dist(cmap[(i1-1)%n], cmap[i1]) +
	       dist(cmap[i1], cmap[(i1+1)%n]));
      swapped = (dist(cmap[(i0-1)%n], cmap[i1]) +
		 dist(cmap[i1], cmap[(i0+1)%n]) +
		 dist(cmap[(i1-1)%n], cmap[i0]) +
		 dist(cmap[i0], cmap[(i1+1)%n]));
      if (swapped < as_is) {
	 total_d += as_is - swapped;
	 if (total_d > 20000) {
	    total_d = 0;
	    fprintf(stderr, ".");
	 }
	 t = cmap[i0];
	 cmap[i0] = cmap[i1];
	 cmap[i1] = t;
      }
   }
   fprintf(stderr, "\n");

}

int main(int argc, char **argv) {
   char *fname = args("in", "in.ppm");
   int nentries = argd("nentries", 256);
   int box_size = argd("box_size", 4);
   Pixel *cmap = malloc(sizeof(Pixel) * nentries);
   int t, i;
   Image in;
   Image box;

   image_init(&in);
   image_init(&box);

   if (NULL == cmap) {
      fprintf(stderr, "could not allocate %d entry cmap\n", nentries);
      exit(1);
   }

   if (TCL_ERROR == image_read(&in, fname))
      exit(1);

   t = time(NULL);
   srandom(argd("seed", t));
   
   for (i = 0; i < nentries; i++) {
      Pixel p;      
      image_random_tile(&box, &in, box_size);
      image_mean_pixel(&box, &p);
      cmap[i] = p;
   }

   fprintf(stderr, "length = %g\n", total_length(cmap, nentries));
   improve_cmap_order(cmap, nentries);
   fprintf(stderr, "length = %g\n", total_length(cmap, nentries));

   printf("(comment %s)\n(cmap\n", fname);
   for (i = 0; i < nentries; i++) {
      printf(" (%d %d %d)", cmap[i].r, cmap[i].g, cmap[i].b);
      if (3 == i%4)
	 printf("\n");
   }
   printf(")\n");
}
   
