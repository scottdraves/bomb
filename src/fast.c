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

static char *fast_c_id = "@(#) $Id: fast.c,v 1.1.1.1 2002/12/08 20:49:48 spotspot Exp $";

#include "defs.h"
#include "image.h"
#include "image_db.h"
#include "bomb.h"

#if xws_bomb
/* begin caolan, see bomb.c for details */
void PutImage(Drawable win,int src_x, int src_y,int dest_x,int dest_y,unsigned int width, unsigned int height);

#ifdef HAVE_XSHM_EXTENSION
# include "xshm.h"
#endif /* HAVE_XSHM_EXTENSION */

extern int dest_x;
extern int dest_y;
extern int notiles;
extern int use_root;
extern int do_tile;
extern int fullscreen;
extern int nobackpixmap;
extern int use_shm;
/* end caolan */
#endif

#if 0
int
random_byte()
{
    static int stage = 0;
    static unsigned int savebits;
    int r;

    if (0 == stage) {
       savebits = R;
       stage = 3;
    }
    r = savebits & 0xff;
    savebits = savebits >> 8;
    stage--;
    return r;
}
#else
int
random_byte()
{
  static int qr = 99;
  static int rc = 0;
  qr = (qr ^ (qr >> 4) ^ (qr << 12)) + 1;
  if (1000 == rc++) {
    rc = 0;
    qr ^= R;
  }
  return qr & 255;
}
#endif

#if 1
int quantize(double d) {
  int i = (int)floor(d);
  double f = d - i;
  if (R8b < f*0xff)
    i++;
  return i;
}
#else
int quantize(double d) {
  static int qr = 99;
  static int rc = 0;
  int i = floor(d);
  double f = d - i;
  if ((qr&255) < (f*255))
    i++;
  qr = (qr ^ (qr >> 4) ^ (qr << 12)) + 1;
  if (1000 == rc++) {
    rc = 0;
    qr ^= R;
  }
  return i;
}
#endif

void
change_rules2(int old_sym, int new_sym, board_t *dst)
{
   if (sym_tile4 == old_sym &&
       sym_one == new_sym) {
      int x, y;
      for (y = 0; y <= SMALL_YSIZE; y++)
	 for (x = 0; x <= SMALL_XSIZE; x++) {
	    int t = (*dst)[x][y];
	    (*dst)[SMALL_XSIZE+x][y] = t;
	    (*dst)[x][SMALL_YSIZE+y] = t;
	    (*dst)[SMALL_XSIZE+x][SMALL_YSIZE+y] = t;
	 }
   } else if (sym_mirror4 == old_sym &&
	      sym_one == new_sym) {
      int x, y;
      for (y = 0; y <= SMALL_YSIZE; y++)
	 for (x = 0; x <= SMALL_XSIZE; x++) {
	    int t = (*dst)[x][y];
	    (*dst)[XSIZE-x][y] = t;
	    (*dst)[x][YSIZE-y] = t;
	    (*dst)[XSIZE-x][YSIZE-y] = t;
	 }
   } else if (sym_one == old_sym &&
	      sym_tile4_stack2 == new_sym) {
      int x, y;
      int m = rule.mask;
      for (y = 0; y <= SMALL_YSIZE; y++)
	 for (x = 0; x <= SMALL_XSIZE; x++) {
	    int t = (*dst)[x][y];
	    (*dst)[x][y] = ((0xffff * t / m) << 16)
	       | (0xffff * (m - t/2) / m);
	 }
   } else if (sym_tile4_stack2 == old_sym &&
	      sym_one == new_sym) {
      int x, y;
      int m = rule.mask;
      for (y = 0; y <= SMALL_YSIZE; y++)
	 for (x = 0; x <= SMALL_XSIZE; x++) {
	    int t = (*dst)[x][y];
	    t = (t&0xffff) * m / 0xffff;
	    (*dst)[x][y] = t;
	    (*dst)[SMALL_XSIZE+x][y] = t;
	    (*dst)[x][SMALL_YSIZE+y] = t;
	    (*dst)[SMALL_XSIZE+x][SMALL_YSIZE+y] = t;
	 }
   }
}

void
pix2heat2(image8_t *fb, board_t *dst,
	  int x0, int y0, int x1, int y1)
{
   int x, y;
   int s = fb->stride;
   u_char *p = fb->p;
   for (y = y0; y < y1; y++)
      for (x = x0; x < x1; x++) {
	 int pix = p[x + y * s];
	 (*dst)[x+1][y+1] = (pix * rule.mask) >> 8;
      }
}

void
pix2heat(image8_t *fb, board_t *dst)
{
   pix2heat2(fb, dst, 0, 0, XSIZE, YSIZE);
}

#if use_quicktime
static u_char ppmbuf[YSIZE][XSIZE][3];
int use_jpeg;
void
write_fb_quicktime(image8_t *fb)
{
   int x, y, j;
   unsigned char *row_pointers[YSIZE];

   for (y = 0; y < YSIZE; y++) {
      u_char *fbp = fb->p + fb->stride * y;
      for (x = 0; x < XSIZE; x++) {
	 int ci = ((int) fbp[x]) * 3;
	 ppmbuf[y][x][0] = current_cmap[ci];
	 ppmbuf[y][x][1] = current_cmap[ci+1];
	 ppmbuf[y][x][2] = current_cmap[ci+2];
      }
   }

   for (j = 0; j < YSIZE; j++) {
       row_pointers[j] = &ppmbuf[j][0][0];
   }

   quicktime_encode_video(qt_file, row_pointers, 0);
}
#endif


#if use_mpeg
static u_char ppmbuf[YSIZE][XSIZE][3];
void
write_fb_mpeg(image8_t *fb)
{
   int x, y;

   for (y = 0; y < YSIZE; y++) {
      u_char *fbp = fb->p + fb->stride * y;
      for (x = 0; x < XSIZE; x++) {
	 int ci = ((int) fbp[x]) * 3;
	 ppmbuf[y][x][0] = current_cmap[ci];
	 ppmbuf[y][x][1] = current_cmap[ci+1];
	 ppmbuf[y][x][2] = current_cmap[ci+2];
      }
   }

   mpeg_frame((char *) ppmbuf);
}
#endif

void
write_fb_ppm(image8_t *fb)
{
   static int frame_num = 0;
   static u_char ppmbuf[YSIZE][XSIZE][3];
   int x, y;

   for (y = 0; y < YSIZE; y++) {
      u_char *fbp = fb->p + fb->stride * y;
      for (x = 0; x < XSIZE; x++) {
	 int ci = ((int) fbp[x]) * 3;
	 ppmbuf[y][x][0] = current_cmap[ci];
	 ppmbuf[y][x][1] = current_cmap[ci+1];
	 ppmbuf[y][x][2] = current_cmap[ci+2];
      }
   }

   if (1) {
      char fname[40];
      FILE *o;
    try_again:
      frame_num++;
      sprintf(fname, "dribble/%03d.ppm", frame_num);
      if ((o = fopen(fname, "r"))) {
	 fclose(o);
	 goto try_again;
      }
      if (NULL != (o = fopen(fname, "w"))) {
	 /* put parms in comment */
	 fprintf(o, "P6\n%d %d\n255\n", XSIZE, YSIZE);
	 fwrite(ppmbuf, 3, XSIZE * YSIZE, o);
	 fclose(o);
      } else
	 fprintf(stderr, "can't open %s\n", fname);
   }
}

double avg_pix;

int histo[256];
int histo_max;
int histo_max_slow = 0;
double histo_smooth_max = 0.0;
double histo_smooth[256];

/* returns true if the screen is boring, as determined by statistical variance */
int
run_hist(image8_t *fb)
{
#define nsamp 3000
#define nframes 30
   static int samples = 0;
   static int hist[nsamp];
   static int sum = 0;
   int i;
   int sum2 = 0;

   for (i = 0; i < (nsamp/nframes); i++) {
      int x = (5 + R%(fb->width-10));
      int y = (5 + R%(fb->height-10));
      u_char p = fb->p[fb->stride * y + x];
      hist[samples++] = p;
      sum2 += p;
   }
   sum += sum2;

   avg_pix = (3*avg_pix + sum2/(double)(nsamp/nframes))/4.0;

   if (samples >= nsamp) {
      double avg = sum / (double) nsamp;
      double var = 0;
      for (i = 0; i < nsamp; i++) {
	 double t = avg - hist[i];
	 var += t * t;
      }
      var = var / samples;
      samples = 0;
      sum = 0;

#if 0
      printf("avg = %g var = %g\n", avg_pix, var);
#endif
      if (var < 10.0) {
	     return 1;
      }
   }
   {
     int t;
     double maxl = 0.0;
     histo_max = 0;
     for (i = 0; i < 256; i++) {
       int n = histo[i];
       histo[i] = (n+n+n)>>2;
       /* histo[i] = 0; */
     }
     for (i = 0; i < 1000; i++) {
       int x = (5 + R%(fb->width-10));
       int y = (5 + R%(fb->height-10));
       u_char p = fb->p[fb->stride * y + x];
       if ((t = histo[p]++) > histo_max)
	 histo_max = t;
     }
     histo_max_slow = (7 * histo_max_slow + histo_max) / 8;
     maxl = log(1.0 + histo_max_slow/5.0);
     for (i = 0; i < 256; i++) {
       histo_smooth[i] = log(1.0 + histo[i]/5.0) / maxl;
     }
   }
   return 0;
}

int iclamp(int p, int n)
{
   if (p < 0)
      p = -p;
   return p%n;
}

void
cool(image8_t *fb)
{
   int i, j;
   for (j = 0; j < fb->height; j++) {
      u_char *p = &fb->p[j * fb->stride];
      for (i = 0; i < fb->width; i++)
	 p[i] = (unsigned char)(p[i] * 0.9);
   }
}

void
warm(image8_t *fb)
{
   int i, j;
   for (j = 0; j < fb->height; j++) {
      u_char *p = &fb->p[j * fb->stride];
      for (i = 0; i < fb->width; i++) {
	 int t = (int)(p[i] * 1.11111);
	 p[i] = (t > 255) ? 255 : t;
      }
   }
}



void
double_board(int vh, board_t *dst, int sym)
{
   int i, j;

   if (vh) {
      switch (sym) {
       case sym_tile4:
       case sym_mirror4:
       case sym_mirror2:
	 for (j = 0; j < YSIZE/4; j++) {
	    for (i = 1; i <= XSIZE/2; i++) {
	       (*dst)[i][j] = ((*dst)[i][2*j] + (*dst)[i][2*j+1]) / 2;
	    }
	 }
	 for (j = YSIZE/2-1; j >= YSIZE/4; j--) {
	    for (i = 1; i <= XSIZE/2; i++) {
	       (*dst)[i][j] = (*dst)[i][j-YSIZE/4];
	    }
	 }
	 break;
       case sym_one:
	 for (j = 0;  j< YSIZE/2; j++) {
	    for (i = 1; i <= XSIZE; i++) {
	       (*dst)[i][j] = ((*dst)[i][2*j] + (*dst)[i][2*j+1]) / 2;
	    }
	 }
	 for (j = YSIZE-1; j >= YSIZE/2; j--) {
	    for (i = 1; i <= XSIZE; i++) {
	       (*dst)[i][j] = (*dst)[i][j-YSIZE/2];
	    }
	 }
	 break;
       case sym_frame_buffer:
	 break;
      }
   } else {
      switch (sym) {
       case sym_tile4:
       case sym_mirror4:
       case sym_mirror2:
	 for (j = 1; j <= YSIZE/2; j++) {
	    for (i = 0; i < XSIZE/4; i++) {
	       (*dst)[i][j] = ((*dst)[2*i][j] + (*dst)[2*i+1][j]) / 2;
	    }
	    for (i = XSIZE/4+1; i <= XSIZE/2; i++) {
	       (*dst)[i][j] = (*dst)[i-XSIZE/4][j];
	    }
	 }
	 break;
       case sym_frame_buffer:
	 break;
       default:
	 for (j = 1; j <= YSIZE; j++) {
	    for (i = 0; i < XSIZE/2; i++) {
	       (*dst)[i][j] = ((*dst)[2*i][j] + (*dst)[2*i+1][j]) / 2;
	    }
	    for (i = XSIZE/2+1; i <= XSIZE; i++) {
	       (*dst)[i][j] = (*dst)[i-XSIZE/2][j];
	    }
	 }
	 break;
      }
   }
}

void
fill_board(fill_t *p)
{
   int r,x,y,f;
   board_t *dst;

   grad_state = 0;

   dst = &board[dbuf];

   /* there are bugs in the boundary conditions of this code */

   f = p->fill;
   if (rule.rule == rule_rd)
      f = fill_noise;
   if (rule.rule == rule_quad &&
       (f == fill_hramp ||
	f == fill_vramp))
     f = fill_sparse;

   switch (f) {
    case fill_hramp:
      for (y = 1; y <= YSIZE; y++)
	 for (x = 1; x <= XSIZE; x++)
	    (*dst)[x][y] = (x * 255) / XSIZE;
      break;
    case fill_vramp:
      for (y = 1; y <= YSIZE; y++)
	 for (x = 1; x <= XSIZE; x++)
	    (*dst)[x][y] = (y * 255) / YSIZE;
      break;
    case fill_grid:
      for (y = 1; y <= YSIZE; y++)
	 for (x = 1; x <= XSIZE; x++)
	    (*dst)[x][y] =
	       ((x * 16) / XSIZE) |
		  (((y * 16) / YSIZE) << 4);
      break;
    case fill_noise:
      for (y = 1; y <= YSIZE; y++)
	 for (x = 1; x <= XSIZE; x++)
	    (*dst)[x][y] = R;
      break;
    case fill_vnoise:
      for (y = 1; y <= YSIZE; y++)
	 for (x = 1; x <= XSIZE/2; x++)
	    (*dst)[x][y] = (*dst)[XSIZE-x][y] = R;
      break;
    case fill_sparse:
      for (y = 0; y <= YSIZE; y++)
	 for (x = 0; x <= XSIZE; x++)
	    (*dst)[x][y] = 0;
      for (r = 0; r < 10; r++)
	 (*dst)[1+R%XSIZE][1+R%YSIZE] = R;
      break;
   }

   /* XXX */
   if (rule_rug_image == rule.rule) {
      drive_with_image(current_image);
   } else if (rule_rd == rule.rule) {
      drive_with_image(current_image);
   } else if (rule_rug_rug == rule.rule) {
	    for (x = 1; x <= XSIZE; x++)
			for (y = 1; y <= YSIZE; y++) {
				board2[0][x][y] = R;
				board2[1][x][y] = R;
			}
   } else if (rule_slip == rule.rule) {
      extern image8_t fb;
      extern void step_rule_rug(int frame, rule_t *p, image8_t *fb);
      step_rule_rug(0, &rule, &fb);
   } else {
      if (high_growth_game(&rule)) {
		int n = 1+(R%4)*(R%4);
		for (x = 0; x <= XSIZE+1; x++)
			for (y = 0; y <= YSIZE+1; y++)
				board2[dbuf][x][y] = 0;
		for (x = 0; x < n; x++)
			board2[dbuf][1+R%(XSIZE/2)][1+R%(YSIZE/2)] = R;
      } else {
		for (x = 1; x <= XSIZE; x++)
			for (y = 1; y <= YSIZE; y++)
				board2[dbuf][x][y] = R;
      }
   }
}


#ifndef use_ddraw

/* these routines are the beginning of the abstract
   eight bit frame buffer interface, so they should take an image8, one of
   which happens to be associated with the real screen, or some such */
#if win_bomb
#include <windows.h>
	extern HWND hwnd;
#endif

#if xws_bomb|ogl_bomb
int index_to_pixels[256];

void
get_cmap_shifts(int mask, int *left, int *right) {
  int l, r;
  l = r = 0;
  if (0 == mask) {
    printf("bad channel mask: %x\n", mask);
    *left = *right = 0;
    return;
  }
  while (!(mask & 1)) {
    mask >>= 1;
    l++;
  }
  while (mask & 1) {
    mask >>= 1;
    r++;
  }
  *left = l;
  *right = 8 - r;
}

#endif

void
image8_set_cmap(int first, int n, int *cmap) {
#if win_bomb
// xxx for now we are doing our own 8->32 bit expansion
#if 0
  int i;
  LOGPALETTE *lp = malloc(sizeof(LOGPALETTE) +
			  n * sizeof(PALETTEENTRY));
  HDC  hdc;
  HPALETTE ph;

  return;
#ifdef wbomb
  return; //XXXX
#endif

	      lp->palVersion = 0x300;
  lp->palNumEntries = n;
  for (i = 0; i < n; i++) {
    PALETTEENTRY *pe = &lp->palPalEntry[i];
    pe->peRed = cmap[i*3];
    pe->peGreen = cmap[i*3+1];
    pe->peBlue = cmap[i*3+2];
    //		pe->peFlags = 0;
    pe->peFlags = PC_NOCOLLAPSE;
  }

  hdc = GetDC (hwnd) ;

  // this is bad
	
       ph = CreatePalette(lp);
  if (NULL == ph) {
    message ("CreatePalette failed") ;
    return FALSE ;	
  }
  if (NULL == SelectPalette(hdc, ph, FALSE)) {
    message("SelectPalette failed") ;
    return FALSE ;
  }
  DeleteObject(ph);
  if (GDI_ERROR == RealizePalette(hdc)) {
    message ( "RealizePalette failed") ;
    return FALSE ;
  }
  ReleaseDC (hwnd, hdc) ;
  free(lp);
#endif
#endif
#if vga_bomb && xws_bomb
  if (!running_x)
#endif
#if vga_bomb
  {
    int m[256 * 3];
    int i;
    for (i = 0; i < 256 * 3; i++)
      m[i] = cmap[i]>>2;
    vga_setpalvec(first, n, m);
  } 
#endif
#if vga_bomb && xws_bomb
else
#endif
#if (xws_bomb|ogl_bomb)
  {
    int i;
    XColor c[256];
    if (visual->class == PseudoColor) {
      for (i = 0; i < n; i++) {
	c[i].pixel = i+first;
	c[i].flags = DoRed|DoGreen|DoBlue;
	c[i].red   = cmap[(i+first)*3+0] << 8;
	c[i].green = cmap[(i+first)*3+1] << 8;
	c[i].blue  = cmap[(i+first)*3+2] << 8;
      } 
      XStoreColors(disp, xws_cmap, c, n);
    } else {
      int rl, rr, gl, gr, bl, br;
      get_cmap_shifts(visual->red_mask, &rl, &rr);
      get_cmap_shifts(visual->green_mask, &gl, &gr);
      get_cmap_shifts(visual->blue_mask, &bl, &br);
      for (i = 0; i < 256; i++) {
	int r = (cmap[i*3] >> rr) << rl;
	int g = (cmap[i*3+1] >> gr) << gl;
	int b = (cmap[i*3+2] >> br) << bl;
	index_to_pixels[i] = r | g | b;
      }
    }
  }
#endif
#if mac_bomb
  int i;
  ColorSpec ctable[256];
#if use_sioux
  return;
#endif
  for (i = 0; i < 256; i++)
    {
      ctable[i].rgb.red   = cmap[3 * i] << 8;
      ctable[i].rgb.green = cmap[3 * i + 1] << 8;
      ctable[i].rgb.blue  = cmap[3 * i + 2] << 8;
    }

  SetEntries(first, n - 1, ctable);
#endif
}

#if mac_bomb && d_pix_rep
#ifdef copy_bits
#include <Quickdraw.h>
#include <QDOffscreen.h>
PixMapPtr offscreenPixMapP, windowPixMapP;
Rect windowRect, offscreenRect;
#else
unsigned char *real_screen_base;
int real_screen_stride;
#endif
#endif
/* int world_x, world_y; */

void image8_flush() {
#if win_bomb
  // xxx not yet
#if 0
  HBITMAP bm;
  int i, j;
  HDC hdc;
  extern image8_t fb;
  hdc = GetDC(hwnd);
  bm = CreateBitmap(320, 200, 1, 8, fb.p);
  DrawBitmap(hdc, bm);
  DeleteObject(bm);
  DrawStatus(hdc, display_fps, status_string, status_string_length);
  ReleaseDC (hwnd, hdc) ;
#endif
#endif


#if mac_bomb && d_pix_rep
#ifdef copy_bits
  /* AJW */
  if ((*offscreenPixMapP->pmTable)->ctSeed != (*windowPixMapP->pmTable)->ctSeed)
    {
      DebugStr("\pCopybits converting colours.");
      (*offscreenPixMapP->pmTable)->ctSeed = (*windowPixMapP->pmTable)->ctSeed;
    }
  CopyBits((BitMapPtr)offscreenPixMapP, (BitMapPtr)windowPixMapP, &offscreenRect, &windowRect, srcCopy, nil);

#else
  int i, j;
  int s = real_screen_stride;
  int s4 = real_screen_stride/4;
  for (i = 0; i < 200; i++) {
#if 1
    unsigned long *p = (unsigned long *) (real_screen_base + 2 * i * s);
    unsigned long *q = (unsigned long *) (fb.p + i * fb.stride);
    for (j = 0; j < 320; j += 4) {
      unsigned long a;
      unsigned long b = *q++;
      unsigned long c;
      unsigned long d;
        
      a = b & 0xFF;
      d = (a << 8) | (a);
      a = b & 0xFF00;
      d |= (a << 16) | (a << 8);
      a = b & 0xFF0000;
      c = (a >> 8) | (a >> 16);
      a = b & 0xFF000000;
      c |= (a) | (a >> 8);
        
      p[0] = c;
      p[1] = d;
      /* lose next two lines for more speed */
      p[s4] = c;
      p[s4+1] = d;
      p+=2;
    }

#else
    unsigned char *p = (real_screen_base + 2 * i * s);
    unsigned char *q = (fb.p + i * fb.stride);
    for (j = 0; j < 320; j++) {
      unsigned char b = *q++;
        
      p[0] = b;
      p[1] = b;
      p[s] = b;
      p[s+1] = b;
      p+=2;
    }
#endif

  }
#endif
#endif

#if mac_bomb && 0
  if (1) {
    static int frames = -1;
    static int startTicks;
    unsigned char str[16];
    int framesSec;

    frames++;
    if ((frames & 15) == 0) {
      framesSec = (frames * 6000) / (TickCount() - startTicks);
      NumToString(framesSec, str);
      TextMode(srcCopy);
      MoveTo(30, 30);
      DrawString(str);
      startTicks = TickCount();
      frames = 0;
    }
  }
#endif

#if xws_bomb && vga_bomb
  int l,m,xoffset,yoffset,temp;

  if (running_x)
#endif
#if xws_bomb
/*caolan begin*/
  int k;
/*caolan end*/
  {
/*begin caolan*/
/* allow offscreen representation to be displayed at an offset to the window
useful for root window display*/
/*extend the number of options and a modification or two to allow X display 
in 8,16 and 24 bits at a multiple of 320x200*/

  Pixmap test = XCreatePixmap(disp, win, 320*pix_rep, 200*pix_rep, visual_depth(DefaultScreenOfDisplay(disp), visual));

for (m=0;m<notiles;m++)
for (l=0;l<notiles;l++)
	{
	k=0;
    if (1 == pix_rep && 1 == fb_bpp)
      PutImage(win,0, 0, dest_x+(l*320*pix_rep), dest_y+(m*200*pix_rep),320,200);
    else if (1 == pix_rep && 2 == fb_bpp) {
      int i, j, k = fb.stride;
      for (i = 0; i < 200; i++)
	for (j = 0; j < 320; j++) {
	  ((short *) image_buf) [i * 320 + j] =
#if 0
	    (((i/200.0) < (1.0-histo_smooth[j])) || (j > 255)) ?
	    index_to_pixels[fb.p[i * k + j]] : 0;
#elif 0
	    index_to_pixels[(((i/200.0) > (histo[j]/(double)histo_max_slow)) || (j > 255)) ? fb.p[i * k + j] : -1];
#else
	    index_to_pixels[fb.p[i * k + j]];
#endif
	}
      PutImage(win,0, 0, dest_x+(l*320*pix_rep), dest_y+(m*200*pix_rep),320,200);
	  
	  
    } else if (1 == pix_rep && 4 == fb_bpp) {
      int i, j, k = fb.stride;
      for (i = 0; i < 200; i++)
	for (j = 0; j < 320; j++)
	  ((int *) image_buf) [i * 320 + j] =
	    index_to_pixels[fb.p[i * k + j]];
      PutImage(win,0, 0, dest_x+(l*320*pix_rep), dest_y+(m*200*pix_rep),320,200);
    } else if (2 == fb_bpp) {
      extern image8_t fb;
      int i, j, ii, jj;

      /* copy fb.p to image_buf, replicating pixels and
	 expanding to number of bytes per pixel */
      for (i = 0; i < 200; i++)
	for (j = 0; j < 320; j++)
	  for (ii = 0; ii < pix_rep; ii++)
	    for (jj = 0; jj < pix_rep; jj++)
	      ((short *)image_buf)[((i*pix_rep + ii) * (320*pix_rep)) +
		       (j*pix_rep + jj)] = index_to_pixels[fb.p[i * fb.stride + j]];

      PutImage(win,0, 0, dest_x+(320*pix_rep*l), dest_y+(m*200*pix_rep),
		pix_rep*320, pix_rep*200);
		
    } else if (4 == fb_bpp) {
      extern image8_t fb;
      int i, j, ii, jj;

      /* copy fb.p to image_buf, replicating pixels and
	 expanding to number of bytes per pixel */
      for (i = 0; i < 200; i++)
	for (j = 0; j < 320; j++)
	  for (ii = 0; ii < pix_rep; ii++)
	    for (jj = 0; jj < pix_rep; jj++)
	      ((int *)image_buf)[((i*pix_rep + ii) * (320*pix_rep)) +
		       (j*pix_rep + jj)] =
		index_to_pixels[fb.p[i * fb.stride + j]];
      PutImage(win,0, 0, dest_x+(320*pix_rep*l), dest_y+(m*200*pix_rep),
		pix_rep*320, pix_rep*200);

		
   } else {
     extern image8_t fb;
     int i, j, ii, jj;

     /* copy fb.p to image_buf, replicating pixels and
	 expanding to number of bytes per pixel */
     for (i = 0; i < 200; i++)
	for (j = 0; j < 320; j++)
	  for (ii = 0; ii < pix_rep; ii++)
	    for (jj = 0; jj < pix_rep; jj++)
	      image_buf[((i*pix_rep + ii) * (320*pix_rep)) +
		       (j*pix_rep + jj)] = fb.p[i * fb.stride + j];
     PutImage(win,0, 0, dest_x+(320*pix_rep*l), dest_y+(m*200*pix_rep),
  		pix_rep*320, pix_rep*200);
    }
	}
	/*
	the purpose of all of this is to create a background pixmap that matches the tiled foreground
	so that if the user moves a window when the image is running as a tiled background that the
	blank areas under windows will be tiled with the current image, so that nasty incorrect areas
	(like pure black) dont show up to annoy them, the reason i dont just use the background and
	dont bother with the foreground is that youd have to use XClearWindow on the root window and
	in may cases i see that it makes the cursor flash so hard that you feel a fit coming on, though
	this might just be a pecularity of one of the machines i use myself
	*/
if ((use_root) && (fullscreen) && (!nobackpixmap) )
	{
	xoffset = dest_x/(320*pix_rep);
	temp = xoffset/320*pix_rep;
	xoffset = xoffset-temp*320*pix_rep;
	xoffset = -dest_x + xoffset*320*pix_rep;

	yoffset = dest_y/(200*pix_rep);
	temp = yoffset/200*pix_rep;
	yoffset = yoffset-temp*200*pix_rep;
	yoffset = -dest_y + yoffset*200*pix_rep;

	PutImage(test, xoffset, yoffset, 0, 0,320*pix_rep-xoffset,200*pix_rep-yoffset);
	PutImage(test, 0, yoffset, 320*pix_rep-xoffset, 0,xoffset,200*pix_rep-yoffset);
	PutImage(test, xoffset, 0, 0, 200*pix_rep-yoffset,320*pix_rep-xoffset,yoffset);
	PutImage(test, 0, 0, 320*pix_rep-xoffset, 200*pix_rep-yoffset,xoffset,yoffset);
	XSetWindowBackgroundPixmap(disp,win,test);
	}
/* end caolan */
	XFreePixmap(disp,test);

  }
  else
    {
      int i;
      extern char *svga_buffer_base;
      // double *dst = (double *) vga_getgraphmem();
      double *dst = (double *) svga_buffer_base;
      double *src = (double *) fb.p;
      int n = 320 * 200 / 8;
      for (i = 0; i < n; i++)
	*dst++ = *src++;
    }

#endif
#if ogl_bomb
  if (1) {
    extern image8_t fb;
    int buf[200][80];
    int i, j;
    for (i = 0; i < 200; i++) {
      int *p = (int *)(fb.p + fb.stride * (199 - i));
      for (j = 0; j < 80; j++)
	buf[i][j] = p[j];
    }
    glDrawPixels(320, 200, GL_COLOR_INDEX, GL_BYTE, buf);
  }
#endif
}

#endif

void hex_tile(board_t *to, double angle, int *pattern, int pat_len, int n, int m) {
   int x, y;
   int dx, dy;
   int y0, y1;
   double th;
   y0 = (n * 200) / m;
   y1 = ((n+1) * 200) / m;

   for  (y = y0; y < y1; y++) {
      for (x = 0; x <= 320; x++) {
	 double yy = (6.0 * y) / 200.0;
	 double xx = (6.0 * x) / 320.0 + yy * .5773502;
	 int xi = (int) xx;
	 int yi = (int) yy;
	 double xr = xx - xi;
	 double yr = yy - yi;
	 int pi = 2*xi + 2*yi + (xr > yr);
	 switch (pattern[pi%pat_len]) {
	 default:
	  case 0:
	    th = 0.0;
	  doth:
	    th += angle;
	    dx = (int)(128 * (1.0 + cos(th)));
	    dy = (int)(128 * (1.0 + sin(th)));
	    if (dx < 0)   dx = 0;
	    if (dx > 255) dx = 255;
	    if (dy < 0)   dy = 0;
	    if (dy > 255) dy = 255;
	    break;
	  case 1:
	    th = M_PI / 3;
	    goto doth;
	  case 2:
	    th = 2 * M_PI / 3;
	    goto doth;
	  case 3:
	    dx = dy = 128;
	    break;
	 }
	 (*to)[x][y] = (dx << 8) | dy;
      }
   }
}

void spiral(board_t *to, double angle, int n, int m) {
   int x, y;
   int dx, dy;
   int y0, y1;
   y0 = 1 + (n * 200) / m;
   y1 = 1 + ((n+1) * 200) / m;

   for  (y = y0; y < y1; y++) {
      for (x = 1; x < 320; x++) {
	 double th = angle + atan2(y - 100, x - 160);
	 dx = (int)(128 * (1.0 + cos(th)));
	 dy = (int)(128 * (1.0 + sin(th)));
	 if (dx < 0)   dx = 0;
	 if (dx > 255) dx = 255;
	 if (dy < 0)   dy = 0;
	 if (dy > 255) dy = 255;
	 (*to)[x][y] = (dx << 8) | dy;
      }
   }
}

#define filter_gap 3

void image2grad(board_t *from, board_t *to, int s, int n, int m) {
   int x, y;
   int y0, y1;
   int d;
   y0 = 1 + (n * 200) / m;
   y1 = 1 + ((n+1) * 200) / m;

   s = s / filter_gap;
   d = s * s;
   s =  s * filter_gap;

   for  (y = y0; y < y1; y++) {
      for (x = 1; x < 320; x++) {
	 int i, j, dx, dy;
	 int p00, p01, p10, p11;
	 p00 = p01 = p10 = p11 = 0;
	 if (x > s && x < (320-s) &&
	     y > s && y < (200-s)) {
	    for (i = 0; i < s; i+=filter_gap)
	       for (j = 0; j < s; j+=filter_gap) {
		  p00 += (*from)[x+i][y+j];
		  p01 += (*from)[x+i][y-j];
		  p10 += (*from)[x-i][y+j];
		  p11 += (*from)[x-i][y-j];
	       }
	 } else {
	    for (i = 0; i < s; i+=filter_gap)
	       for (j = 0; j < s; j+=filter_gap) {
		  p00 += (*from)[(x+i)%320][(y+j)%200];
		  p01 += (*from)[(x+i)%320][(y-j+200)%200];
		  p10 += (*from)[(x-i+320)%320][(y+j)%200];
		  p11 += (*from)[(x-i+320)%320][(y-j+200)%200];
	       }
	 }
	 dx = (p00 - p10) + (p01 - p11);
	 dy = (p00 - p01) + (p10 - p11);
	 dx = (dx / d + 128);
	 dy = (dy / d + 128);

	 if (dx < 0)   dx = 0;
	 if (dx > 255) dx = 255;
	 if (dy < 0)   dy = 0;
	 if (dy > 255) dy = 255;
	 (*to)[x][y] = (dx << 8) | dy;
      }
   }   
}


#if 0
void image2grad(board_t *from, board_t *to, int s) {
   int x, y, t;
   int mx = 320 - s;
   int my = 200 - s;
   for  (y = 1; y <= my; y++) {
      for (x = 1; x <= mx; x++) {
	 int p00 = (*from)[x  ][y  ];
	 int p01 = (*from)[x  ][y+s];
	 int p10 = (*from)[x+s][y  ];
	 int p11 = (*from)[x+s][y+s];
	 int dx = (p00 - p10) + (p01 - p11);
	 int dy = (p00 - p01) + (p10 - p11);

	 dx = (dx + 512) / 4;
	 dy = (dy + 512) / 4;

	 if (dx < 0)   dx = 0;
	 if (dx > 255) dx = 255;
	 if (dy < 0)   dy = 0;
	 if (dy > 255) dy = 255;

	 (*to)[x][y] = (dx << 8) | dy;
      }
   }
}
#endif

void blur(board_t *from, board_t *to) {
   int x, y;

   fprintf(stderr, "blur ");

   for  (y = 1; y <= 200; y++) {
      for (x = 1; x <= 320; x++) {
	 (*to)[x][y] = (((*from)[x  ][y-1])+
			((*from)[x-1][y  ])+
			((*from)[x  ][y  ])+
			((*from)[x+1][y  ])+
			((*from)[x  ][y+1])) / 5;
      }
   }
}


void
pen(board_t *dboard, int mask, int x, int y, int bsize)
{
   int i, j;
   if (x >= 1 && y >= 1 && x <= XSIZE-bsize && y <= YSIZE-bsize) {
      switch (bsize) {
       case 1:
	 (*dboard)[x][y] = mask;
	 break;
       case 2:
	 for (i = 0; i < 2; i++)
	    for (j = 0; j < 2; j++)
	       (*dboard)[x+i][y+j] = mask;
	 break;
       case 3:
	 for (i = 0; i < 3; i++)
	    for (j = 0; j < 3; j++)
	       (*dboard)[x+i][y+j] = mask;
	 break;
       case 4:
	 for (i = 0; i < 4; i++)
	    for (j = 0; j < 4; j++)
	       (*dboard)[x+i][y+j] = mask;
	 break;
       default:
	 for (i = 0; i < bsize; i++)
	    for (j = 0; j < bsize; j++)
	       (*dboard)[x+i][y+j] = mask;
	 break;
      }
   }
}

#if xws_bomb
void PutImage(Drawable win,int src_x, int src_y,int dest_x,int dest_y,unsigned int width, unsigned int height)
	{
#if 0	
#ifdef HAVE_XSHM_EXTENSION
	if (use_shm)
    	XShmPutImage(disp, win, gc, xws_image, src_x, src_y,dest_x, dest_y, width, height,False); 
	else
#endif /* HAVE_XSHM_EXTENSION */
#endif
    	XPutImage(disp, win, gc, xws_image, src_x, src_y, dest_x, dest_y,width,height);
	}
#endif
