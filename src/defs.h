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

static char *defs_h_id = "@(#) $Id: defs.h,v 1.1.1.1 2002/12/08 20:49:45 spotspot Exp $";


#define mac_bomb 0
#define vga_bomb 0
#define xws_bomb 0
#define ogl_bomb 0
#define win_bomb 1

#define use_guile 0
#define use_esd   0
#define use_quicktime 0
#define use_mpeg  0




#if use_guile
#include <gh.h>
#endif

#if use_mpeg
#include "mpeg2/src/mpeg2enc/mpeg.h"
#endif

#if use_quicktime
#include "quicktime.h"
extern quicktime_t *qt_file;
extern int use_jpeg;
#endif

#if mac_bomb || win_bomb
// during development use this
// #  define DATA_DIR "..\\src\\"
// for release use this
#  define DATA_DIR ""
typedef unsigned char u_char;
#  define M_PI 3.141592654
#else
#  include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>


#define enable_logging 0
extern FILE *log_fp;

#if vga_bomb
#  include <vga.h>
#endif

#include "cmap.h"
#include "libifs.h"


#ifdef __cplusplus
extern "C" {
#endif

#define sym_one          0
#define sym_tile4        1
#define sym_mirror4      2
#define sym_mirror2      3
#define sym_frame_buffer 4
#define sym_tile4_stack2 5

#define distrib_original   0
#define distrib_new        1
#define distrib_continuous 2
#define distrib_rule_lock  4

#define grad_none -1


void begin_timer();
double end_timer();

extern int display_fps;

#define alen(x) (sizeof(x)/sizeof((x)[0]))

#if mac_bomb || win_bomb
#  define R (rand()|(rand()<<16))
#else
#  define R   (random())
#endif

#define R8b (random_byte())

#define fuse_ndrifts 4

int iclamp(int p, int n);




/* REZ is not used by some rules */

/* REZ chooses the grid size.  
   0 = 160x100
   1 = 160x200
   2 = 320x200 */
#define REZ 2

#if REZ==0
#  define XSIZE 160
#  define XMUL 2
#  define YSIZE 100
#  define YMUL 2
#elif REZ==1
#  define XSIZE 160
#  define XMUL 2
#  define YSIZE 200
#  define YMUL 1
#elif REZ==2
#  define XSIZE 320
#  define XMUL 1
#  define YSIZE 200
#  define YMUL 1
#endif

#define SMALL_XSIZE (XSIZE/2)
#define SMALL_YSIZE (YSIZE/2)
#define SMALL_XMUL  (XMUL*2)
#define SMALL_YMUL  (YMUL*2)

#define VSMALL_XSIZE (XSIZE/8)
#define VSMALL_YSIZE (YSIZE/8)
#define VSMALL_XMUL  (XMUL*8)
#define VSMALL_YMUL  (YMUL*8)


typedef int board_t[XSIZE+2][YSIZE+2];

typedef struct {
   int fill;
} fill_t;

#define MAXRHYTHM 2

#define flame_nspan 3

typedef struct {
   int rule;
   int speed;     /* computed from speed_*    */
   int mask;
   int randomized_underflow;
   int cycle_bkg;
   int remap;
   int floor;           /* computed from frame, hot, cold */
   int driver_slowdown;
   int brain;
   int brain_shift;
   int drift;
   int drift_speed;
   int rhythm[MAXRHYTHM];
   double drift_time;
   ifs_control_point flame_cp;
   ifs_control_point flame_span[flame_nspan];
   int hot;
   int cool;
   int image_window_size;
   int search_time;
   int bsize;
   int speed_base;
   int speed_beat_speed;
   int speed_beat_size;
   int seq[2];
   int fastflip_rate;
} rule_t;

typedef struct {
   int cmap;
   int index;
   int index2;
} cmap_t;

extern board_t board[2];
extern board_t board2[2];
extern board_t board3[2];
extern int dbuf;
extern int remap[256 * 256];
extern int p1;
#define MAXFLAMES 500
extern ifs_control_point flames[MAXFLAMES];
extern int nflames;

extern int nosound;


void distrib(int dist, rule_t *rule, cmap_t *cmap, fill_t *fill);
void init_rotate();

#define LIVE_BIT 1
#define HIST_BIT 2

/* a place to write 8-bit pixels.  should have a cmap in here */
typedef struct {
   u_char *p;
   int width;
   int height;
   int stride;
} image8_t;

extern image8_t fb;

#define max_heat (0x10000)

#define N_RAM_IMAGES 10

int seq_next_image(int *seq_p);
void init_shade();
void init_wave();
void seq_start(int *seq);
void init_rotor();
void init_seq();
void pick_liss_coefs();

#define status_string_length 80
extern char status_string[status_string_length];
extern int display_fps;

int random_byte();
int quantize(double d);


/* instead of argv (XXX) */
#define argd(s,d)   ((hidden_ai = getenv(s)) ? atof(hidden_ai) : (d))
#define argi(s,d)   ((hidden_ai = getenv(s)) ? atoi(hidden_ai) : (d))
#define args(s,d)   ((hidden_ai = getenv(s)) ? hidden_ai : (d))
#define arge(s,e)   ((hidden_ai = getenv(s)) && (!strcmp(e,hidden_ai)))

extern char *hidden_ai;

extern int high_growth_game(rule_t *p);

extern ifs_control_point cc_direction;
extern int grad_state;
extern rule_t rule;
extern int current_cmap[256 * 3];

void message(char *s);
void fill_board(fill_t *p);
void pix2heat(image8_t *fb, board_t *dst);
void change_rules2(int old_sym, int new_sym, board_t *dst);
void pix2heat2(image8_t *fb, board_t *dst,
	       int x0, int y0, int x1, int y1);
void double_board(int vh, board_t *dst, int sym);
void write_fb_mpeg(image8_t *fb);
void write_fb_ppm(image8_t *fb);
int run_hist(image8_t *fb);
void cool(image8_t *fb);
void warm(image8_t *fb);
void image8_set_cmap(int i, int n, int *cmap);
double adjust_speed(double);
void image8_flush();
void blur(board_t *from, board_t *to);
void image2grad(board_t *from, board_t *to, int s, int n, int m);
void spiral(board_t *to, double angle, int n, int m);
void hex_tile(board_t *to, double angle, int *pattern, int pat_len, int n, int m);
void do_mouse(int x, int y);
void pen(board_t *dboard, int mask, int x, int y, int bsize);
void bomb_exit();
void init();
void init_random();
#if win_bomb
void win_init();
int win_getkey();
void read_registry();
void write_registry();
extern int rule_mask;
extern int mouse_x, mouse_y, mouse_down;


int win_exit();
#endif

#if mac_bomb
/* if d_pix_rep is 0 then write directly on the screen */
#define d_pix_rep 0
#define use_sioux 0
#endif

#if xws_bomb && vga_bomb
extern int running_x;
#endif


#if xws_bomb
#define HAVE_XSHM_EXTENSION
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#define d_pix_rep 1
extern char *image_buf;
extern Display *disp;
extern Window win;
extern Visual *visual;
extern int fb_bpp;
extern int depth;
extern int *xws_lut;
extern GC gc;
extern XImage *xws_image;
extern Colormap xws_cmap;
int visual_depth (Screen *screen, Visual *visual);
#endif

#if ogl_bomb
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#define d_pix_rep 3
extern Display *disp;
extern Window win;
extern Colormap xws_cmap;
#endif
/*begin caolan*/
/*pix_rep changed to d_pix_rep and pix_rep is now a variable*/
extern int pix_rep;
/*end caolan*/

#ifdef __cplusplus
}
#endif
