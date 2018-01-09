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

static char *image_db_h_id = "@(#) $Id: image_db.h,v 1.1.1.1 2002/12/08 20:49:49 spotspot Exp $";

#define SMALL_FACTOR 5
#define TILE_SIZE 40
#define SMALL_SIZE (TILE_SIZE/SMALL_FACTOR)

#if mac_bomb
#  define use_suck_dir 1
#  define max_image_name 30
#  define max_images 300
extern char image_names[max_images][max_image_name];
#elif win_bomb
#  define use_suck_dir 0
extern char **image_names;
#else
#  define use_suck_dir 0
#include <dirent.h>
extern struct dirent **image_dir;
#endif
extern int image_dir_len;

extern int current_image;
extern Image global_images[N_RAM_IMAGES];
extern Image global_images_small[N_RAM_IMAGES];
extern image8_t small_pattern;
void drive_with_image(int image_num);
void init_images();
void file_to_image(int file_num, int image_num);
void rotate_images();
void invert_board();
void random_image_set();
