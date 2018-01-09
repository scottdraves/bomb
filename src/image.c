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

static char *image_c_id = "@(#) $Id: image.c,v 1.1.1.1 2002/12/08 20:49:49 spotspot Exp $";

#include "defs.h"
#include "image.h"

void
image_fill(Image *image, Pixel v)
{
   int w = image->width;
   int h = image->height;
   Pixel *p = image->pixels;
   int s = image->stride;
   int i, j;
   
   for (i = 0; i < h; i++)
      for (j = 0; j < w; j++)
	 p[i * s + j] = v;
}

void
image_random(Image *image)
{
   int w = image->width;
   int h = image->height;
   Pixel *p = image->pixels;
   int s = image->stride;
   int i, j;
   
   for (i = 0; i < h; i++)
      for (j = 0; j < w; j++) {
	 Pixel x;
	 x.r = R; x.g = R; x.b = R;
	 x.a = 255;
	 p[i * s + j] = x;
      }
}


/* next two functions stolen from readppm.c from photo widget.
   shouldn't the dll of graphics formats have something for this,
   then we can handle jpg too... */

static char *
ppm_fgets(line, len, fp)
    char *line;
    int len;
    FILE *fp;
{
    int s, i = 0;
    for(;;){
        if (i == len - 1) {
          line[i] = 0;
          break;
        }
	if( -1 == (s = fgetc(fp))) {
	    return NULL;
	 }
	 if (s == '\n' || s == '\r') {
	    line[i] = 0;
		if (-1 == (s = fgetc(fp))) {
			break;
		}
		if (s == '\n' || s == '\r') {
			break;
		} else {
			ungetc(s, fp);
		}
	    break;
	  }
	  line[i++] = s;
	  
    }
    if ('#' == line[0])
       return ppm_fgets(line, len, fp);
    return line;
}

static int
read_hdr(fp, wp, hp, channelsp)
   FILE *fp;
   int *wp, *hp, *channelsp;
{
    int pmax;
    char line[64];
    if (0 == fp) {
       printf("bad file\n");
       return 0;
    }
    if( ppm_fgets(line, sizeof(line), fp) == NULL )
      return 0;
    
    if ( strcmp(line, "P6") == 0 ) {
      *channelsp = 3;
    } else if ( strcmp(line, "P5") == 0 ) {
      *channelsp = 1;
    } else {
	return 0;
    }

    if( ppm_fgets(line, sizeof(line), fp) == NULL
       || sscanf(line, "%d%d", wp, hp) != 2 ) {
	return 0;
     }
    if( ppm_fgets(line, sizeof(line), fp) == NULL
       || sscanf(line, "%d", &pmax) != 1
       || pmax != 255 ) {
	return 0;
      }
    return 1;
}

#define file_type_unknown 0
#define file_type_ppm     1
#define file_type_gif     2
#define file_type_jpeg    3
#define file_type_tiff    4

int
file_type(FILE *fp)
{
   int r = file_type_unknown;
   char buf[4];

   if (4 != fread(buf, 1, 4, fp))
      goto done;
   if (!strncmp("GIF", buf, 3))
      r = file_type_gif;
   else if (!strncmp("P6", buf, 2) || !strncmp("P5", buf, 2))
      r = file_type_ppm;
   else if (!strncmp("II", buf, 2) ||
	    !strncmp("JJ", buf, 2))
      r = file_type_tiff;
   else
      r = file_type_jpeg; /* just guessing now */
 done:
   fseek(fp, 0, SEEK_SET);
   return r;
}

   
/* should support jpeg */

int
image_read(Image *image, char *file)
{
   FILE *fp;
   int width, height, channels, nrd;
   int total_bytes;
   int gifLoadBomb(Image *image, char *filename);

   fp = fopen(file, "rb");
   if (fp == NULL)
      return TCL_ERROR;
   switch (file_type(fp)) {

    case file_type_gif:
      fclose(fp);
      return gifLoadBomb(image, file);

    case file_type_ppm:
      if (!read_hdr(fp, &width, &height, &channels)) {
         printf("bad header %s\n", file);
	     goto fail;
	  }
      /* note ppm stores 1 or 3 bytes per pixel, we store 4.
	     so after we read it in, we have to insert gaps
	     for the alpha */
      total_bytes = width * height * channels;
      image = image_allocate(image, width, height);
      if (NULL == image)
         goto fail;
      if (total_bytes != (nrd = fread(image->pixels, 1, total_bytes, fp))) {
         printf("short file %s %d %d\n", file, nrd);
	     goto fail;
      }
      fclose(fp);
      
      /* insert gaps, be careful at last pixel */
      if (1) {
	 char *p = (char *) image->pixels;
	 int scan_ppm = total_bytes;
	 int scan_rgba = width * height * sizeof(Pixel);
	 if (3 == channels) {
	   while (scan_ppm) {
	     p[scan_rgba - 1] = (char) alpha_solid;
	     p[scan_rgba - 2] = p[scan_ppm - 1];
	     p[scan_rgba - 3] = p[scan_ppm - 2];
	     p[scan_rgba - 4] = p[scan_ppm - 3];
	     scan_rgba -= 4;
	     scan_ppm -= 3;
	   }
	 } else {
	   while (scan_ppm) {
	     char q = p[scan_ppm - 1];
	     p[scan_rgba - 1] = (char) alpha_solid;
	     p[scan_rgba - 2] = q;
	     p[scan_rgba - 3] = q;
	     p[scan_rgba - 4] = q;
	     scan_rgba -= 4;
	     scan_ppm -= 1;
	   }
	 }
      }

      break;
	default:
		goto fail;
   }
      
   return TCL_OK;
 fail:
   fclose(fp);
   fprintf(stderr, "unable to read image: %s\n", file);
   return TCL_ERROR;
}

void
image_destroy(Image* image)
{
   if (NULL != image->pixels &&
       NULL == image->parent)
      free(image->pixels);
   free(image);
}

void
image_init(Image *image)
{
   image->width =
      image->height = 0;
   image->pixels = 0;
   image->parent = 0;
}

/* recycle passed in image and its storage if possible.
   never returns a subimage */
Image *
image_allocate(Image *image, int w, int h)
{
   int total_bytes = (sizeof(Pixel)) * w * h;
   if (NULL == image) {
      image = (Image *) malloc(sizeof(Image));
      image->pixels = (Pixel *) malloc(total_bytes);
   } else if (w != image->width ||
	      h != image->height ||
	      NULL != image->parent) {
      /* ugh, complex XXX */
      if (NULL != image->pixels &&
	  NULL == image->parent)
	 free(image->pixels);
      image->pixels = (Pixel *) malloc(total_bytes);
   }
   if (NULL == image->pixels)
     return NULL;
   image->width = w;
   image->height = h;
   image->stride = w;
   image->origin_x = 0;
   image->origin_y = 0;
   return image;
}

Image *
image_subimage(Image *image, Image *of, int x, int y, int w, int h)
{
   if (NULL == image)
      image = (Image *) malloc(sizeof(Image));
   image->width = w;
   image->height = h;
   image->stride = of->stride;
   image->pixels = of->pixels + of->stride * y + x;
   /* eliminate hierarchy */
   if (NULL == of->parent) {
      image->origin_x = x;
      image->origin_y = y;
      image->parent = of;
   } else {
      image->origin_x = of->origin_x + x;
      image->origin_y = of->origin_y + y;
      image->parent = of->parent;
   }
#if 1
   if (x + w > of->width)
      fprintf(stderr, "x + w > of->width (%d + %d > %d)\n",
	      x, w, of->width);
   if (y + h > of->height)
      fprintf(stderr, "y + h > of->height (%d + %d > %d)\n",
	      y, h, of->height);
#endif
   return image;
}


/* if the requested size is larger than the image itself, then
   the tile is silently truncated to fit */

Image *
image_random_tile(Image *image, Image *of, int size)
{
   int x, y;
   if (size >= of->width) {
      size = of->width;
      x = 0;
   } else
      x = R % (of->width - size);
   if (size >= of->height) {
      size = of->height;
      y = 0;
   } else
      y = R % (of->height - size);
   if (NULL == image)
      image = (Image *) malloc(sizeof(Image));
   image->width = size;
   image->height = size;
   image->stride = of->stride;
   image->pixels = of->pixels + of->stride * y + x;
   /* eliminate hierarchy */
   if (NULL == of->parent) {
      image->origin_x = x;
      image->origin_y = y;
      image->parent = of;
   } else {
      image->origin_x = of->origin_x + x;
      image->origin_y = of->origin_y + y;
      image->parent = of->parent;
   }
   return image;
}

void
image_mean_pixel(Image *img, Pixel *pix)
{
   int r, g, b, a;
   Pixel p;
   unsigned char *bytes = (unsigned char *) &p;
   int i, j;
   int npix;

   a = r = g = b = 0;
   for (i = 0; i < img->height; i++)
      for (j = 0; j < img->width; j++) {
	 p = img->pixels[img->stride * i + j];
	 r += bytes[0];
	 g += bytes[1];
	 b += bytes[2];
	 a += bytes[3];
      }

   npix = img->width * img->height;

   bytes[0] = r / npix;
   bytes[1] = g / npix;
   bytes[2] = b / npix;
   bytes[3] = a / npix;

   *pix = p;
}
   
   

void
image_filter_down(Image *from, Image *to)
{
   int h_scale = from->height / to->height;
   int w_scale = from->width / to->width;
   int i, j;
   Pixel p;
   int npix;

   npix = h_scale * w_scale;

   for (i = 0; i < to->height; i++)
      for (j = 0; j < to->width; j++) {
	 int ii, jj;
	 int r, g, b, a;
	 a = r = g = b = 0;
	 for (ii = 0; ii < h_scale; ii++)
	    for (jj = 0; jj < w_scale; jj++) {
	       p = from->pixels[from->stride * (i * h_scale + ii) +
				(j * w_scale + jj)];
	       a += p.a;
	       r += p.r;
	       g += p.g;
	       b += p.b;
	    }
	 p.a = a / npix;
	 p.r = r / npix;
	 p.g = g / npix;
	 p.b = b / npix;
	 to->pixels[to->stride * i + j] = p;
      }
}
  

/* and more complex arithmetic versions, with alpha */
/* no overlap */
void
image_blit(Image *from, Image *to)
{
   Image f = *from;
   Image t = *to;
   int i, j;
   for (i = 0; i < f.height; i++)
      for (j = 0; j < f.width; j++)
	 t.pixels[t.stride * i + j] =
	    f.pixels[f.stride * i + j];
}
