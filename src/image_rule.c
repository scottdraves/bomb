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

static char *image_rule_c_id = "@(#) $Id: image_rule.c,v 1.1.1.1 2002/12/08 20:49:49 spotspot Exp $";

#include "defs.h"
#include "image.h"
#include "image_db.h"
#include "bomb.h"

void
step_rule_rug_image(int frame, rule_t *p, image8_t *fb) {
   int x,y;	
   board_t *s_heat_board, *d_heat_board;
   board_t *s_image_board;
   u_char *lp;

   s_heat_board=&board[dbuf];
   d_heat_board=&board[1-dbuf];
   s_image_board=&board2[0];   
   dbuf = 1-dbuf;

   for (y=1;y<=YSIZE;y++) {
      lp = fb->p + (fb->stride * (y - 1));
      for (x=1;x<=XSIZE;x++) {
	 int heat, t;
	 heat = ((((*s_heat_board)[x  ][y-1])+
		  ((*s_heat_board)[x-1][y  ]<<1)+
		  ((*s_heat_board)[x  ][y  ]<<1)+
		  ((*s_heat_board)[x+1][y  ]<<1)+
		  ((*s_heat_board)[x  ][y+1])) >> 3);
	 if (heat < 0) {
	    if (p->floor)
	       heat = 0;	 /* hm, just &mask? */
	    else if (p->randomized_underflow)
	       heat = R;
	 }
	 t = (*s_image_board)[x][y];
	 heat += t;
	 heat += p->speed;
	 heat &= p->mask;
	 (*d_heat_board)[x][y]= heat;
	 // if (p->remap)
	    heat = remap[heat];
	 *(lp++) = heat;
#if 0
	 /* some kind of non-linearity could be cool, adjusted to mask,
	    what is the distribution of heat values in a rug? */
	 *(lp++) = 20 * log(1+t);
#endif
      }
   }
}









int current_image = 0;
int image_dir_len;
Image global_images[N_RAM_IMAGES];
Image global_images_small[N_RAM_IMAGES];
Image *global_image;
image8_t small_pattern;
u_char sd_pix[SMALL_SIZE * SMALL_SIZE];

int
gray(Pixel p)
{
   return (p.r + p.g + p.g);
}


void
drive_with_image(int image_num)
{
   int i, j;
   board_t *s_image_board, *d_image_board;
   int w, h, s;
   Pixel *p;
   int scale = (rule.rule == rule_rd) ? 3 : 255;

   grad_state = 0;

   image_num = iclamp(image_num, N_RAM_IMAGES);

   s_image_board=&board2[0];
   d_image_board=&board2[1];

   global_image = &global_images[image_num];

   w = global_image->width;
   h = global_image->height;
   p = global_image->pixels;
   s = global_image->stride;
   for (i = 0; i < YSIZE+2; i++) {
      for (j =  0; j < XSIZE+2; j++) {
	 int g = gray(p[(i * h / (YSIZE+2)) * s + (j * w / (XSIZE+2))]);
	 ((*s_image_board)[j][i]) = g / scale;
	 ((*d_image_board)[j][i]) = g / scale;
      }
   }
}

void
invert_board()
{
   board_t *s_image_board, *d_image_board;
   int i, j;
   s_image_board=&board2[0];
   d_image_board=&board2[1];

   for (i = 0; i < YSIZE+2; i++) {
      for (j =  0; j < XSIZE+2; j++) {
	 (*s_image_board)[j][i] *=  -1;
	 (*d_image_board)[j][i] *=  -1;
      }
   }
}

void rotate_images()
{
   int i;
   Image bt;
   Image st;
   int last = N_RAM_IMAGES - 1;

   bt = global_images[last];
   st = global_images_small[last];

   for (i = last; i > 0; i--) {
      global_images[i] = global_images[i - 1];
      global_images_small[i] = global_images_small[i - 1];
   }
   global_images[0] = bt;
   global_images_small[0] = st;
}

#if win_bomb
char **image_names;
#  define MAXPATHLEN 1030
#else
#if use_suck_dir
#  define MAXPATHLEN 1030
char image_names[max_images][max_image_name];
#else
#  include <dirent.h>
#  include <sys/types.h>
#  include <sys/param.h>
struct dirent **image_dir;
int isimage(const struct dirent *d) {
  // hm are empty filenames legal?
   return d->d_name[0] != '.';
}
#endif
#endif

void file_to_image(int file_num, int image_num)
{
#if 1
   char buf[MAXPATHLEN];
#else
   char buf[PATH_MAX];
#endif
   extern int tiles_ready;
   tiles_ready = 0;
   image_num = iclamp(image_num, N_RAM_IMAGES);
   if (1 > image_dir_len) {
      int x, y;
      Image *im = &global_images[image_num];
      image_allocate(im, 100, 100);
      for (x = 0; x < 100; x++)
	 for (y = 0; y < 100; y++) {
	    int v = (((x>>3) + (y>>3)) & 1) ? 0 : -1;
	    Pixel p;
	    p.r = p.g = p.b = v;
	    im->pixels[im->stride * y + x] = p;
	 }
   } else {
      file_num = iclamp(file_num, image_dir_len);
#if mac_bomb
      sprintf(buf, ":suck:%s", image_names[file_num]);
#elif win_bomb
      sprintf(buf, "%ssuck\\%s", DATA_DIR, image_names[file_num]);
#else
      sprintf(buf, "%ssuck/%s", DATA_DIR, image_dir[file_num]->d_name);
#endif
      if (TCL_ERROR == image_read(global_images + image_num, buf)) {
		 file_to_image(file_num + 1, image_num);
		 return;
      }
   }
   image_allocate(global_images_small + image_num,
		  global_images[image_num].width / SMALL_FACTOR,
		  global_images[image_num].height / SMALL_FACTOR);
   image_filter_down(global_images + image_num,
		     global_images_small + image_num);
}

void random_image_set()
{
   int i;
   for (i = 0; i < N_RAM_IMAGES; i++)
      file_to_image(R, i);
}


void init_images()
{
   int i;
#if !win_bomb
   char buf[1000];
#endif
   small_pattern.width = SMALL_SIZE;
   small_pattern.height = SMALL_SIZE;
   small_pattern.stride = SMALL_SIZE;
   small_pattern.p = sd_pix;
#if win_bomb
   // done in WinInit
#elif use_suck_dir
   image_dir_len = 0;
      sprintf(buf, "%ssuck-dir", DATA_DIR);
      printf("buf=%s\n", buf);
   if (1) {
     FILE *dir = fopen(buf, "r");
    if (NULL == dir) {
        printf("couldnt open suck-dir\n");
		message("open suck-dir failed");
     } else while (1) {
        image_names[image_dir_len][0] = 0;
        fscanf(dir, "%s", image_names[image_dir_len]);

        if ('.' != image_names[image_dir_len][0])
          image_dir_len++;
        else
          break;
     }
   }
#else
   sprintf(buf, "%ssuck", DATA_DIR);
   image_dir_len = scandir(buf, &image_dir, isimage, alphasort);
#endif
   if (1 > image_dir_len) {
      fprintf(stderr, "no images found, using checker-board\n");
      exit(-1);
   }

   // image_dir_len = 0;
   // xxx image loading fucked up

   for (i = 0; i < N_RAM_IMAGES; i++) {
      image_init(&global_images[i]);
      image_init(&global_images_small[i]);
   }

   if (!getenv("quick"))
      random_image_set(); // XXXX
}
