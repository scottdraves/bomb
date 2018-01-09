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

static char *bomb_c_id = "@(#) $Id: bomb.c,v 1.1.1.1 2002/12/08 20:49:40 spotspot Exp $";

#include "defs.h"
#include "image.h"
#include "image_db.h"
#include "bomb.h"
#include "match.h"
#include "sound.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int randomstart=0;

#if !win_bomb

/* begin caolan */
#ifdef HAVE_XSHM_EXTENSION
#include "xshm.h"
#endif /* HAVE_XSHM_EXTENSION */

#include "vroot.h"

int dest_x;		/*offset from left of offscreen image to onscreen image*/
int dest_y;		/*offset from top of offscreen image to onscreen image*/
int use_root=0; /*display in root flag*/
int pix_rep=d_pix_rep;	/*glue to original pix_rep mechanism*/
int multiply=-1; /*magnification factor*/
int do_tile=0; /*whether to tile the 320x200 image into the window*/
int notiles=0; /*no of tiles*/
int fullscreen=0; /*if the tiles cover the screen, then we can use a background pixmap to smooth expose events*/
int nobackpixmap=0; /*flag if to use a backing pixmap*/
int use_shm=1; /*use shared memory*/
char *progname="bomb"; /*keep the xshm.c file happy*/
int nosound=0; /* on linux, disable sound input, do not even take sound device */

int windowid;
int use_windowid;

#ifdef HAVE_XSHM_EXTENSION
  XShmSegmentInfo shm_info;
#endif

/* end caolan */
#endif

#if win_bomb
int bomb_exit_flag;
#endif


board_t board[2];
board_t board2[2];
board_t board3[2];
int dbuf=0;
int remap[max_heat]; /* > largest possible mask */
int p1 = 0;

int auto_mode = 0;
int auto_rule = 0;
int delayed_fill = 0;

char status_string[status_string_length];

char *hidden_ai;

   int key;
   int frame = 0;
   double timer = 500;
   int overdrive_speed = 20;
   int dtime = 1;
   int flipper, flipper2;
   int param = 0;
   int curs = 0;
   int *gparam;
   int old_rule;
   int pulse = 0;
   int pulse_driver = 0;
   int periodic_write = 0;
   int cmap_changed = 0;
   int image_rate = 0; 	/* should be in rule struct */
   int image_dst = 0;
   int rule_lock = 0;
   int pop_back = 0, back_to;
   int permuting = 0;
   double delay = 0.0;
   int kbd_mode = 0;
   int stabilize_fps;
   int display_fps = 0;
   int rule_mask = -1;
   int last_clear = 0;
   int fastflip = 0;
int scramble = 0;

int keyboard_enabled = 1;

double fps;

/* for colormap changes */
static u_char target_cmap[256][3];
int current_cmap[256 * 3];
#define SET_TARGET_PALETTE(x, r, g, b) \
	target_cmap[x][0] = r; \
	target_cmap[x][1] = g; \
	target_cmap[x][2] = b


cmap_t cmap;
fill_t fill2;
rule_t rule;

int nrecords = 0;
FILE *recording = NULL;

image8_t fb;

int masks[max_heat];


#if use_quicktime
quicktime_t *qt_file;
#endif

#if mac_bomb && !use_sioux
#include <console.h>
short InstallConsole(short fd){return 0;}
void RemoveConsole(void){}
long WriteCharsToConsole(char *buffer, long n){return 0;}
long ReadCharsFromConsole(char *buffer, long n){return 0;}
#endif


#if xws_bomb && vga_bomb
int running_x;
#endif

#if (xws_bomb|ogl_bomb)
Display *disp;
Visual *visual;
int fb_bpp;
Window win;
Colormap xws_cmap;
int depth;
int *xws_lut;
#if xws_bomb
GC gc;
XImage *xws_image=NULL;
char *image_buf;
#endif
#endif

#if mac_bomb
#include <AppleEvents.h>
#include <Types.h>
#include <Resources.h>
#include <QuickDraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Desk.h>
#include <ToolUtils.h>
#include <Memory.h>
#include <SegLoad.h>
#include <Files.h>
#include <OSUtils.h>
#include <OSEvents.h>
#include <DiskInit.h>
#include <Packages.h>
#include <Traps.h>
#include <QDOffscreen.h>
#include <Palettes.h>
#include <LowMem.h>
#endif

void
ramp(int c, int i0, int i1, int n, int k)
{
   int x, r, g, b;
   for (x = 0; x < n; x++) {
      double alpha = x / (double) n;
      r = (int) (the_cmaps[c][i0][0] * (1 - alpha) +
	   the_cmaps[c][i1][0] * alpha);
      g = (int) (the_cmaps[c][i0][1] * (1 - alpha) +
	   the_cmaps[c][i1][1] * alpha);
      b = (int) (the_cmaps[c][i0][2] * (1 - alpha) +
	   the_cmaps[c][i1][2] * alpha);
      SET_TARGET_PALETTE(k + x, r, g, b);
   }
}

int
round_up8(int n)
{
   if (n < 0)
      n = (n - 7) >> 3;
   else
      n = (n + 7) >> 3;
   return n;
}

int
round_up5(int n)
{
   if (n < 0)
      n = (n - 31) >> 5;
   else
      n = (n + 31) >> 5;
   return n;
}

void
step_cmap()
{
   int i;
   for (i = 0; i < 256; i++) {
      current_cmap[3*i]   += round_up8(target_cmap[i][0] - current_cmap[3*i]);
      current_cmap[3*i+1] += round_up8(target_cmap[i][1] - current_cmap[3*i+1]);
      current_cmap[3*i+2] += round_up8(target_cmap[i][2] - current_cmap[3*i+2]);
   }
#define difff(x,y) ((x)!=(y))
   for (i = 0; i < 256; i++) {
      if (difff(current_cmap[3*i+0], target_cmap[i][0]) ||
	  difff(current_cmap[3*i+1], target_cmap[i][1]) ||
	  difff(current_cmap[3*i+2], target_cmap[i][2])) {
	 image8_set_cmap(0, 256, current_cmap);
	 return;
      }
   }
}

void brighten_cmap() {
   int i;
   for (i = 0; i < 256; i++) {
      int r = (255 + 4*current_cmap[3*i+0])/5;
      int g = (255 + 4*current_cmap[3*i+1])/5;
      int b = (255 + 4*current_cmap[3*i+2])/5;
      SET_TARGET_PALETTE(i, r, g, b);
   }
}

void pulse_cmap_rotate() {
   int i;
   for (i = 1; i < 220; i++) {
      current_cmap[3*i+0] = current_cmap[(3*i+0+90)%(3*256)];
      current_cmap[3*i+1] = current_cmap[(3*i+0+91)%(3*256)];
      current_cmap[3*i+2] = current_cmap[(3*i+0+92)%(3*256)];
   }
}

void pulse_cmap_black() {
   int i;
   for (i = 100; i < 150; i++) {
      current_cmap[3*i+0] = current_cmap[3*i+0] * (150 - i) / 50;
      current_cmap[3*i+1] = current_cmap[3*i+1] * (150 - i) / 50;
      current_cmap[3*i+2] = current_cmap[3*i+2] * (150 - i) / 50;
   }
   for (i = 150; i < 250; i++) {
      current_cmap[3*i+0] = 0;
      current_cmap[3*i+1] = 0;
      current_cmap[3*i+2] = 0;
   }
}

void pulse_cmap_white() {
   int i;
   for (i = 1; i < 10; i++) {
      current_cmap[3*i+0] = ((current_cmap[3*i+0] * (10 - i) / 10) +
			     (255 * i / 10));
      current_cmap[3*i+1] = ((current_cmap[3*i+1] * (10 - i) / 10) +
			     (255 * i / 10));
      current_cmap[3*i+2] = ((current_cmap[3*i+2] * (10 - i) / 10) +
			     (255 * i / 10));
   }
   for (i = 10; i < 20; i++) {
      current_cmap[3*i+0] = 255;
      current_cmap[3*i+1] = 255;
      current_cmap[3*i+2] = 255;
   }
   for (i = 20; i < 30; i++) {
      current_cmap[3*i+0] = ((current_cmap[3*i+0] * (i - 20) / 10) +
			     (255 * (30 - i) / 10));
      current_cmap[3*i+1] = ((current_cmap[3*i+1] * (i - 20) / 10) +
			     (255 * (30 - i) / 10));
      current_cmap[3*i+2] = ((current_cmap[3*i+2] * (i - 20) / 10) +
			     (255 * (30 - i) / 10));
   }
}

void
fade2cmap(cmap_t *p)
{
   int r, g, b, x;
   int c, i, i0, i1, i2;

   switch (p->cmap) {
    case cmap_mono:
      for(x=0;x<256;x++) {
	 SET_TARGET_PALETTE(x,x,x,x);
      }
      break;
    case cmap_mono4:
      i0 = p->index;
      i1 = p->index2;
      for(x=0;x<256;x++) {
	 if (x == i0) {
	    r = (i1&3)<<4;
	    g = (i1&12)<<2;
	    b = (i1&48);
	 } else
	    r = g = b = (x&63)<<2;
	 SET_TARGET_PALETTE(x,r,g,b);
      }
      break;
    case cmap_loop:
      c = p->index;
      i0 = R%256;
      i1 = next_contrasting_color(c, i0, 60000);
      i2 = next_contrasting_color(c, i1, 60000);
      ramp(c, i0, i1, 64, 0);
      ramp(c, i1, i2, 64, 64);
      ramp(c, i2, i1, 64, 128);
      ramp(c, i1, i0, 64, 192);
      break;
    case cmap_path:
      if (1) {
	 int n = 10;
	 int len = 30000;
	 int each = 256/(n+1);
	 c = p->index;
	 i2 = i0 = R%256;
	 for (i = 0; i < n; i++) {
	    i1 = next_contrasting_color(c, i0, len);
	    ramp(c, i0, i1, each, i*each);
	    i0 = i1;
	 }
	 ramp(c, i1, i2, 256 - n*each, n*each);
      }
      break;
    case cmap_heat:
      /* black -> midred -> red -> yellow -> white
	 -> = 64 */
      for (x = 0; x < 128; x++) {
	 r = g = 255;
	 b = x << 1;
	 SET_TARGET_PALETTE(128 + x, r, g, b);
      }
      for (x = 0; x < 64 ;x++) {
	 r = 255;
	 b = 0;
	 g = x<<2;
	 SET_TARGET_PALETTE(64 + x, r, g, b);
      }
      for(x = 0; x < 64; x++) {
	 r = x<<2;
	 g = 0;
	 b = 0;
	 SET_TARGET_PALETTE(x, r, g, b);
      }
      break;
    case cmap_plain:
      i = p->index;
      for (x = 0; x < 256; x++) {
	 r = the_cmaps[i][x][0];
	 g = the_cmaps[i][x][1];
	 b = the_cmaps[i][x][2];
	 SET_TARGET_PALETTE(x,r,g,b);
      }
      break;
    case cmap_split:
      i = p->index;
      for(x=0;x<128;x++) {
	 r = the_cmaps[i][x][0];
	 g = the_cmaps[i][x][1];
	 b = the_cmaps[i][x][2];
	 SET_TARGET_PALETTE(x,r,g,b);
      }
      i = p->index2;
      for(x=128;x<256;x++) {
	 r = the_cmaps[i][x][0];
	 g = the_cmaps[i][x][1];
	 b = the_cmaps[i][x][2];
	 SET_TARGET_PALETTE(x,r,g,b);
      }
      break;
    case cmap_noise:
      for (x = 0; x < 256; x++) {
	 SET_TARGET_PALETTE(x,R,R,R);
      }
      break;
    case cmap_black:
      for (x = 0; x < 256; x++) {
	 SET_TARGET_PALETTE(x,0,0,0);
      }
      break;
   case cmap_ramp:
     c = p->index;
     i0 = R%256;
     i1 = next_contrasting_color(c, i0, 80000);
     ramp(c, i0, i1, 256, 0);
     break;
   }
}

void set_remap(int mask, int rm) {
	int x;
    for (x = 0; x < mask; x++) {
	     int r;
	     r = rm
			 ? (int)(255 * x / (double)mask)
			 : x;
		 r = r & 255;
	     if (0 == r) r = 1;
	     if (255 == r) r = 254;
		 remap[x] = r;
    }
}


/* spook */
int
count_bits16(int i)
{
   int b, r = 0;
   for (b = 1; b < 0x10000; b = b << 1)
      if (b & i)
	 r++;
   return r;
}

/* find last set, like ffs */
int
fls(int i)
{
   int r = 1;
   int b = 1;
   while (b < i) {
      r++;
      b = b << 1;
   }
   return r;
}

void
init_masks()
{
   int i, c;
   c = 0;
   i = 1;
   while (c < max_heat) {
      if (8 < count_bits16(i)) {
	 masks[c++] = i;
      }
      i += 2;
   }
}

int
compare_integers(const void *k, const void *a)
{
   return *(int *)a - *(int *)k;
}

/* generally should just keep the index */
int
next_mask(int mask, int d)
{
   int s = (d > 0) ? 2 : -2;
   d = abs(d);
   d = (d > 0) ? d : -d;
   while (d) {
      if (8 < count_bits16(mask))
	 d--;
      mask += s;
   }
   return mask;
#if 0
   r = bsearch(&mask, masks, max_heat, sizeof(int), compare_integers);
   if (NULL == r)
      return masks[0];
   i = (r - masks + d) % max_heat;
   return masks[i];
#endif
}

int load_title(char *fn)
{
   int i;
   Image im;
#if !use_sioux
   /* set cmaps to grayscale */
   for (i = 0; i < 256; i++) {
      int *v = &current_cmap[3*i];
      v[0] = v[1] = v[2] = i;
   }
   image8_set_cmap(0, 256, current_cmap);

   image_init(&im);
   if (TCL_ERROR == image_read(&im, fn))
      return 0;
   image8_blit(&im, &fb);
   image8_flush();
#endif
   return 1;
}

#if mac_bomb

int GetBit( const void *data, int offset )
{
   int d = ((unsigned char *)data)[offset >> 3];
   return (d & (1 << (offset & 7))) != 0;
}

#endif


int
bomb_getkey() {
#if win_bomb
	extern int win_getkey();
	return win_getkey();
#endif
#if vga_bomb && xws_bomb
   if (!running_x)
#endif
#if vga_bomb
   return vga_getkey();
#endif
#if (xws_bomb|ogl_bomb)
   {
     XEvent ev;
	 /*keep an eye an the key presses of all interesting windows*/
     if (XCheckMaskEvent(disp, KeyPressMask, &ev)) {
       KeySym s;
       s = XKeycodeToKeysym(disp, ev.xkey.keycode, 0);
       /* doesn't work for shifted digits */
       if (s > 255) return 0;
       if (ShiftMask&ev.xkey.state) {
	 int r = ((int)s) - ('a' - 'A');
	 if (r < 32) r += 48;
	 return r;
       }
       if (ControlMask&ev.xkey.state) {
	 return ((int)s) - 'a' + 1;
       }
       return (int) s;
     } else
       return 0;
   }
#endif
#if mac_bomb
#if use_sioux
#  define GetMacEvent GetNextEvent
#else
#  define GetMacEvent GetOSEvent
#endif
   static EventRecord gEvent;
   if (GetMacEvent( keyDownMask, &gEvent)) {
      int key = gEvent.message & charCodeMask;
      if ('q' == key && (gEvent.modifiers & cmdKey))
         return 3;
      else
         return(key);
   } else
      return(0);
#endif
}

void bomb_exit() {
  if (recording)
    fclose(recording);

  exit_sound();
#if mac_bomb
  if (1) {
    GrafPtr wPort;
    FlushEvents(everyEvent - osMask - diskMask, 0);
  }
  ExitToShell();
#endif
#if vga_bomb
#if xws_bomb
  if (!running_x)
#endif
  vga_setmode(TEXT);
#endif
#if win_bomb
  bomb_exit_flag = 1;
#else
  printf("\nbye\n");
  exit(0);
#endif
}

#if ogl_bomb || vga_bomb || xws_bomb
void init_random() {
   int t = time(NULL);
   srandom(argd("seed", t));
}
#endif

#if mac_bomb
void
init_random()
{
   srand(argd("seed", TickCount()));
}

void
init_mac()
{

#if mac_bomb
   if (1) {
      Ptr base_address;
      SysEnvRec This_mac;
      Rect *rr;
      int cx, cy;
      CWindowPtr myWindow;

      printf("init bomb\n");
#if !use_sioux
      InitGraf(&qd.thePort);
      InitFonts();
      FlushEvents(everyEvent - osMask - diskMask, 0);
      InitWindows();
      InitCursor();
      MaxApplZone();
      MoreMasters();
      HideCursor();
#endif
      SysEnvirons( 1, &This_mac );
      if ( This_mac.hasColorQD ) {
         GDHandle the_monitor;
         the_monitor = GetMainDevice();         /* Get monitor with the menuBar on it. */
         if ((**(**the_monitor).gdPMap).pixelSize != 8)
            SetDepth(the_monitor, 8, 0, 0);	/* Make sure we have 8-bit pixels */
         base_address = (**(**the_monitor).gdPMap).baseAddr;
         fb.stride = ((**(**the_monitor).gdPMap).rowBytes & 0x3FFF);
         rr = &(**(**the_monitor).gdPMap).bounds;
         SetGDevice(the_monitor);
       //  world_x = (**the_monitor).gdRect.left;
       //  world_y = (**the_monitor).gdRect.top;
      } else {
	 ExitToShell();   /* Not much point if we don't have color quickdraw. */
   }
   
#if !use_sioux
   if (1) {
      int i, j;
     /* open window covering entire screen so things will be redrawn when the app quits */
      myWindow = (CWindowPtr) NewCWindow(0, rr, "\pBomb", 1, 0, (WindowPtr) -1, 0, 0);
      SetPort((WindowPtr)myWindow);
      for (i = rr->top; i < rr->bottom; i++)
         for (j = rr->left; j < rr->right; j++)
            *(((u_char*)base_address) + j + (i * fb.stride)) = 0;
   }
#endif

#if d_pix_rep
/*if usersettable
else
*/
pix_rep = d_pix_rep;
#ifdef copy_bits
      if (1) {
      extern PixMapPtr  offscreenPixMapP, windowPixMapP;
      extern Rect       windowRect, offscreenRect;
      PixMapHandle	offscreenPixMapH, windowPixMapH;
      GWorldPtr         offscreenWorldP;
      GWorldPtr         saveWorld;
      GDHandle      	saveDevice;
   
      SetRect(&windowRect, 0, 0, pix_rep*320, pix_rep*200);
      SetRect(&offscreenRect, 0, 0, 320, 200);
      OffsetRect(&windowRect, (rr->right - rr->left - (pix_rep*320)) / 2 & (~3), (rr->bottom - rr->top - (pix_rep*200)) / 2);
   // Get the Graphics World and Device associated with the window
      GetGWorld(&saveWorld, &saveDevice);
   // Set a pointer to the windows PixMap
      windowPixMapH = GetGWorldPixMap(saveWorld);
      HLockHi((Handle)windowPixMapH);
      LockPixels(windowPixMapH);
      windowPixMapP = *windowPixMapH;

   // Set Up Offscreen GWorld
      if (NewGWorld(&offscreenWorldP, 0, &offscreenRect, 0, saveDevice, noNewDevice))
         DebugStr("\pCan't get GWorld.");
   // Set a pointer to the Offscreen World's PixMap
      offscreenPixMapH = GetGWorldPixMap(offscreenWorldP);
      HLockHi((Handle)offscreenPixMapH);
      LockPixels(offscreenPixMapH);
      offscreenPixMapP = *offscreenPixMapH;
   // Set up fb.
      fb.p = (Byte*) (*offscreenPixMapH)->baseAddr;
      fb.stride = (*offscreenPixMapH)->rowBytes & 0x3fff;
   }
#else
   if (1) {
      extern unsigned char *real_screen_base;
      extern int real_screen_stride;
      cy = (rr->bottom - rr->top - (pix_rep*200)) / 2;
      cx = (rr->right - rr->left - (pix_rep*320)) / 2;
      real_screen_base = ((u_char *) base_address) + (cy * fb.stride) + cx;
      real_screen_stride = fb.stride;
      fb.p = malloc(320 * 200);
      fb.stride = 320;
   }
#endif
#else
   cy = (rr->bottom - rr->top - 200) / 2;
   cx = (rr->right - rr->left - 320) / 2;
   fb.p = ((u_char *) base_address) + (cy * fb.stride) + cx;
#endif
   fb.width = 320;
   fb.height = 200;
   }
#endif
}
#endif

char *svga_buffer_base;

#if vga_bomb
void
init_vga()
{
   vga_modeinfo *modeinfo;
   vga_init();
   vga_setmode(G320x200x256);
   // modeinfo = vga_getmodeinfo(G320x200x256);
   svga_buffer_base = vga_getgraphmem();
   fb.p = malloc(320 * 200); // vga_getgraphmem();
   fb.stride = 320;
   fb.width = 320;
   fb.height = 200;

}
#endif

#if ogl_bomb || xws_bomb

int
screen_number (Screen *screen)
{
  Display *dpy = DisplayOfScreen (screen);
  int i;
  for (i = 0; i < ScreenCount (dpy); i++)
    if (ScreenOfDisplay (dpy, i) == screen)
      return i;
  abort ();
}


int
visual_class (Screen *screen, Visual *visual)
{
  Display *dpy = DisplayOfScreen (screen);
  XVisualInfo vi_in, *vi_out;
  int out_count, c;
  vi_in.screen = screen_number (screen);
  vi_in.visualid = XVisualIDFromVisual (visual);
  vi_out = XGetVisualInfo (dpy, VisualScreenMask|VisualIDMask,
			   &vi_in, &out_count);
  if (! vi_out) abort ();
  c = vi_out [0].class;
  XFree ((char *) vi_out);
  return c;
}


int
visual_depth (Screen *screen, Visual *visual)
{
  Display *dpy = DisplayOfScreen (screen);
  XVisualInfo vi_in, *vi_out;
  int out_count, d;
  vi_in.screen = screen_number (screen);
  vi_in.visualid = XVisualIDFromVisual (visual);
  vi_out = XGetVisualInfo (dpy, VisualScreenMask|VisualIDMask,
			   &vi_in, &out_count);
  if (! vi_out) abort ();
  d = vi_out [0].depth;
  XFree ((char *) vi_out);
  return d;
}


init_x()
{
  Screen *scrn;
  XSetWindowAttributes attr;
  XWindowAttributes xgwa;
  XGCValues argh;
  int fb_sz;
  unsigned long valuemask;
/*caolan begin*/
  int tempwidth,tempheight;
  int stat;
  Window parent, *children, *child2, winret;
  unsigned int nchildren,i;

/*caolan end*/


  disp = XOpenDisplay(NULL);
  if (NULL == disp) {
    fprintf(stderr, "couldn't open display.\n");
    exit(1);
  }
  scrn = DefaultScreenOfDisplay(disp);
#if ogl_bomb
  if (1) {
    int attrs[] = {None};
    vl = glXChooseVisual(disp, 0, attrs);
    if (NULL == vl) {
      fprintf(stderr, "couldn't find GLX visual.\n");
      exit(1);
    }
  }
#endif
  visual = DefaultVisualOfScreen (scrn);
  attr.event_mask = KeyPressMask;
  valuemask = CWEventMask;
  if (PseudoColor == visual_class(scrn, visual)) {
    xws_cmap = XCreateColormap(disp, RootWindowOfScreen(scrn),
			       visual, AllocAll);
    valuemask |= CWColormap;
    attr.colormap = xws_cmap;
  }


/*	begin caolan */
if ((multiply == 0) && (!do_tile))
	{
	  /* commented out code makes the pixel replication "round up"
	     resulting in truncated input.  i prefer to round down and have
	     a black border -spot */

	tempwidth = (WidthOfScreen(scrn))/320;
	/* if ((tempwidth * 320) <  (WidthOfScreen(scrn))) tempwidth++; */
	tempheight = (HeightOfScreen(scrn))/200;
	/* if ((tempheight * 200) <  (HeightOfScreen(scrn))) tempheight++; */

	/* if (tempwidth > tempheight) pix_rep = tempwidth;
	else pix_rep = tempheight; */

	if (tempwidth > tempheight) pix_rep = tempheight;
	else pix_rep = tempwidth;
	}
else if (multiply > 0)
	pix_rep = multiply;


if ((do_tile) && (notiles == 0))
	{
	tempwidth = (WidthOfScreen(scrn))/(320*pix_rep);
	if ((tempwidth * (320*pix_rep)) <  (WidthOfScreen(scrn)))
		tempwidth++;
	tempheight = (HeightOfScreen(scrn))/(200*pix_rep);
	if ((tempheight * (200*pix_rep)) <  (HeightOfScreen(scrn)))
		tempheight++;

	if (tempwidth > tempheight)
		notiles = tempwidth;
	else 
		notiles = tempheight;
	}
else if (!do_tile)
	notiles=1;
  
/* use root window or not, use default magnification or user defined value */
/* determine offset into offscreen representation to show at 0,0 */
 if (use_windowid) {
   win = windowid;
   dest_x = 0;
   dest_y = 0;
 } else
if (!use_root)
	{
	win = XCreateWindow(disp,
				RootWindowOfScreen(scrn),
				(WidthOfScreen(scrn)-pix_rep*320)/2,
				(HeightOfScreen(scrn)-pix_rep*200)/2,
				pix_rep*320, pix_rep*200, 5, CopyFromParent,
				InputOutput, visual, valuemask, &attr);
	XMapWindow(disp, win);
	/*get keystrokes from this main window */
	XSelectInput(disp, win, KeyPressMask);

	dest_x=0;
	dest_y=0;
  	}
else
	{
	  win = RootWindowOfScreen(scrn);

	/*the default is to find a pix_rep that will fully cover the screen*/

if (!do_tile)
	{
	dest_x = (WidthOfScreen(scrn)-pix_rep*320)/2;
	dest_y = (HeightOfScreen(scrn)-pix_rep*200)/2;
	}
else
	{
	dest_x = (WidthOfScreen(scrn)-pix_rep*notiles*320)/2;
	dest_y = (HeightOfScreen(scrn)-pix_rep*notiles*200)/2;
	}

	if (((pix_rep*notiles*320) >= (WidthOfScreen(scrn))) &&
		((pix_rep*notiles*200) >= (HeightOfScreen(scrn))) )
		fullscreen=1;

	/*get keystrokes from this main window */
	XSelectInput(disp, win, KeyPressMask);
	/*but this is different for RootWindows, lets try and see if we can get
	the real input that the user might try and input into the RootWindow*/
	stat = XQueryTree(disp, win, &winret, &parent, &children, &nchildren);
	for(i=0; i < nchildren; i++)
		XSelectInput(disp, children[i], KeyPressMask);

	/*XSetWindowBackground(disp,win,BlackPixelOfScreen(scrn));*/

    attr.backing_store=0;
    valuemask = CWBackingStore;
	XChangeWindowAttributes(disp,win,valuemask,&attr);
  	}

	XGetWindowAttributes(disp,win,&xgwa);
	/* printf("bit_gravity is %d\nwin_gravity is %d\nclass is %d\n,backing_store is %d\nbacking_planes is %d\n,backing_pixel is %d\nsave_under is %d\n",xgwa.bit_gravity,xgwa.win_gravity,xgwa.class,xgwa.backing_store,xgwa.backing_planes,xgwa.backing_pixel,xgwa.save_under); */
	
/* end caolan */

  depth = visual_depth(DefaultScreenOfDisplay(disp), visual);

#if xws_bomb
  argh.function = GXcopy;
  gc = XCreateGC(disp, win, GCFunction, &argh);
  fb_bpp = ((8 == depth) ? 1 : (16 == depth) ? 2 : 4);
#else
  fb_bpp = 1;
#endif
  fb_sz = 320 * 200 * fb_bpp;
  fb.p = malloc(fb_sz);
  fb.stride = 320;
  fb.width = 320;
  fb.height = 200;

#if xws_bomb
  {
    if (1 == pix_rep && fb_bpp == 1)
      image_buf = (char *) fb.p;
    else
      image_buf = malloc(320 * 200 * pix_rep * pix_rep * fb_bpp);
#ifdef HAVE_XSHM_EXTENSION
	xws_image = create_xshm_image(disp, visual, depth, ZPixmap, 0, &shm_info, 
		pix_rep*320, pix_rep*200);
	if (xws_image)
		{
		free(image_buf);
		image_buf = xws_image->data;
		}
	else
		{
		use_shm=0;
		fprintf(stderr,"XSHM extension not working, using slower method\n");
		}
#endif /* HAVE_XSHM_EXTENSION */

	 if (xws_image == NULL) 
		xws_image = XCreateImage(disp, visual, depth, ZPixmap, 0, image_buf, 
			pix_rep*320, pix_rep*200, 8, 0);
  }
#endif
#if ogl_bomb
  if (1) {
    GLXContext cx;
    cx = glXCreateContext(disp, vl, 0, GL_TRUE);
    glXMakeCurrent(disp, win, cx);
    glRasterPos2f(-1.0, -1.0);
    glPixelZoom(pix_rep, pix_rep);
  }
#endif
  delay = 3.5;
}
#endif

void
init_rule_mask(char *s)
{
  int n, i, mask, adding, first;

  rule_mask = -1;

  if (s) {

    n = strlen(s);
    i = 0;
    first = 1;
    while (i < n) {
      if (s[i] == '-') {
	adding = 0;
	i++;
      } else if (s[i] == '+') {
	adding = 1;
	i++;
      } else
	adding = 1;

      if (!*s) goto fail;

      if (!strncmp(s+i, "acidlife", 8)) {
	mask = (1 << rule_acidlife2)|(1 << rule_acidlife1);
	i += 8;
      } else if (!strncmp(s+i, "anneal", 6)) {
	mask = (1 << rule_rug_anneal) | (1 << rule_rug_anneal2);
	i += 6;
      } else if (!strncmp(s+i, "rugrug", 6)) {
	mask = (1 << rule_rug_rug);
	i += 6;
      } else if (!strncmp(s+i, "static", 6)) {
	mask =  (1 << rule_static);
	i += 6;
      } else if (!strncmp(s+i, "flame", 5)) {
	mask =  (1 << rule_rotorug);
	i += 5;
      } else if (!strncmp(s+i, "brain", 5)) {
	mask =  (1 << rule_rug_brain);
	i += 5;
      } else if (!strncmp(s+i, "image", 5)) {
	mask =  (1 << rule_rug_image);
	i += 5;
      } else if (!strncmp(s+i, "shade", 5)) {
	mask =  (1 << rule_shade);
	i += 5;
      } else if (!strncmp(s+i, "quad", 4)) {
	mask =  (1 << rule_quad);
	i += 4;
      } else if (!strncmp(s+i, "fuse", 4)) {
	mask =  (1 << rule_fuse);
	i += 4;
      } else if (!strncmp(s+i, "wave", 4)) {
	mask =  (1 << rule_wave);
	i += 4;
      } else if (!strncmp(s+i, "fuse", 4)) {
	mask = 3 | (1 << rule_fuse);
	i += 4;
      } else if (!strncmp(s+i, "slip", 4)) {
	mask = (1 << rule_slip);
	i += 4;
      } else if (!strncmp(s+i, "rug", 3)) {
	mask = 3 | (1 << rule_rug3);
	i += 3;
      } else if (!strncmp(s+i, "rd", 2)) {
	mask =  (1 << rule_rd2) | (1 << rule_rd);
	i += 2;
      } else
	goto fail;

      // printf("%d %x\n", adding, mask);

      if (adding) {
	if (first) {
	  rule_mask = mask;
	  first = 0;
	} else
	  rule_mask |= mask;
      } else
	rule_mask &= ~mask;
    }
  }
  // printf("mask=%x\n", rule_mask);
  return;

fail:
  printf("malformed rule mask: %s\n", s);
  rule_mask = -1;
}

void
bomb_init()
{
   int i;

#if !win_bomb
   printf("bomb v%s\n", VERSION);
#endif

   init_random();

#if vga_bomb && xws_bomb
   if (getenv("DISPLAY")) {
     running_x = 1;
     init_x();
   } else {
     running_x = 0;
     init_vga();
   }
#else
#if vga_bomb
   init_vga();
#endif
#if ogl_bomb|xws_bomb
   init_x();
#endif
#endif

   stabilize_fps = 1; // (int) getenv("fps");
   fps = 10.0; // just a guess

   gparam = &rule.speed;

   init_sound();

   for (i = 0; i < status_string_length; i++)
	   status_string[i] = ' ';

#ifndef win_saver
   if (!use_guile) {
      char s[1000];
      int i = 0;
      sprintf(s, "%stitle.gif", DATA_DIR);
      load_title(s);
   }
#endif
    init_masks();
    init_rotor();
    init_images();
    init_seq();
    init_cmaps();

#if use_mpeg
    mpeg_init();
#endif

   distrib(distrib_new, &rule, &cmap, &fill2);
   rule.drift_time = 0.0;
   drive_with_image(current_image);

#ifndef win_saver
   /* start out with basically the same rule */
   rule.rule = rule_rug;
   rule.speed = -1;
   rule.speed_base = -1;
   rule.mask = 255;
   rule.remap = 0;
   rule.randomized_underflow = 1;
   rule.speed_beat_size = sound_present ? 7 : 0;
   cmap.cmap = cmap_plain;
   cmap.index = R%ncmaps;

   if (randomstart | (!((1 << rule.rule) & rule_mask))) {
     distrib(distrib_new, &rule, 0, &fill2);
     fill_board(&fill2);
   } else
     pix2heat(&fb, &board[dbuf]);

   fade2cmap(&cmap);
#else
   fade2cmap(&cmap);
   image8_set_cmap(0, 256, target_cmap);
   fill_board(&fill2);
#endif


   set_remap(rule.mask, rule.remap);
   delayed_fill = 0;

   /* this forces the next several rule changes */
   auto_rule = 4;

   // while (0 == bomb_getkey());

   begin_timer();

}

int
rule_symmetry(int rule)
{
   int r;
   switch (rule) {
    case rule_rd: r = sym_tile4_stack2; break;
    case rule_wave:
    case rule_rug_brain:
    case rule_rug_anneal: 
    case rule_rug_anneal2: r = sym_tile4; break;
    case rule_acidlife2: r = sym_mirror4; break;
    case rule_acidlife1: r = sym_mirror2; break;
    case rule_fuse:
    case rule_slip: r = sym_frame_buffer; break;
    case rule_static: /* false, but useful */
    default:
      r = sym_one; break;
   }
   return r;      
}

void
change_rules(int old, int new, image8_t *fb)
{
   int old_sym = rule_symmetry(old);
   int new_sym = rule_symmetry(new);

   if (new == rule_rug_image && old != new) {
      drive_with_image(current_image);
   }
   if (old_sym == new_sym)
      return;
   if (sym_frame_buffer == old_sym) {
      pix2heat(fb, &board[dbuf]);
      old_sym = sym_one;
   }
   change_rules2(old_sym, new_sym, &board[dbuf]);
   change_rules2(old_sym, new_sym, &board2[dbuf]);
   /* some of the rules don't copy board2
      rug_rug is always going to fail sometimes until
      dbuf/frame&1 is resolved XXX */
   change_rules2(old_sym, new_sym, &board2[1-dbuf]);
}

static last_rec_event = 0;

void
record1(char *s, int *n, int nv)
{
  if (*n == nv) return;

  if (recording &&
      frame != last_rec_event) {
    fprintf(recording, "(frame %d)\n", frame - last_rec_event);
    last_rec_event = frame;
  }
  *n = nv;
  if (recording) {
    fprintf(recording, "(%s %d)\n", s, nv);
  }
}

void
distrib(int dist, rule_t *rulep, cmap_t *cmap, fill_t *fill)
{
   int x, b, i, nbits, nv;
   if (rulep) {
      static int distrib[] =
      {rule_rug, rule_rd, rule_rug_brain, rule_rotorug, rule_rug_image};
      if (!(distrib_continuous & dist)) {
	 init_rotate();
	 init_shade();
	 init_wave();
	 if (!(distrib_rule_lock & dist)) {
	   do {
	     if (auto_rule) {
	       nv = distrib[auto_rule%alen(distrib)];
	       auto_rule--;
	     } else {
	       nv = (R%2) ? (R%nrules) : distrib[R%alen(distrib)];
	     }
	   } while (0 == (rule_mask & (1 << nv)));
	   record1("rule", &rulep->rule, nv);
	 }
	 record1("cycle-bkg", &rulep->cycle_bkg, (0 == R%10));
	 record1("brain", &rulep->brain, R);
	 if (high_growth_game(rulep)) {
	    delayed_fill = 1;
	 }
	 if (rule_rug_brain == rulep->rule)
	    record1("brain-shift", &rulep->brain_shift, (R%4) ? 0 : ((R%3) ? (R%5) : (R%25)));
	 else
	    record1("brain-shift", &rulep->brain_shift, R);
	 random_control_point(&rulep->flame_cp);
	 for (i = 0; i < flame_nspan; i++)
	   random_control_point(rulep->flame_span+i);
	 if (auto_rule) {
	    int i, j;
	    static int best[] = {0, 1, 2, 5};
	    int v = best[R%alen(best)];
	    for (i = 0; i < NXFORMS; i++) {
	       for (j = 0; j < NVARS; j++)
		  rulep->flame_cp.xform[i].var[j] = 0.0;
	       rulep->flame_cp.xform[i].var[v] = 1.0;
	    }	    
	 }
	 pick_liss_coefs();

	 if (auto_rule) {
	    record1("drift", &rulep->drift, 4);
	 } else if (rule_fuse == rulep->rule)
	    record1("drift", &rulep->drift, (R%5) ? (R%2) : ((R%2)+2));
	 else
	    record1("drift", &rulep->drift, R);
	 rulep->image_window_size = 2 + R%5;
	 rulep->speed_beat_speed = (R%3) ? (5 + R%10) : (30 + R%30);
	 if ((sound_present && (R%3)) || 0 == (R%3))
		rulep->speed_beat_size = (R%2) ? (4 + R%12) : (10 + R%22);
	 else
		rulep->speed_beat_size = 0;
	 rulep->seq[0] = R;
	 seq_start(rulep->seq);
	 p1 = R;
	 current_image = R;
      }
      rulep->fastflip_rate = ((R&1) ? 0 :
			      ((R&1) ? (R%5) :
			       (5+(R%20))));
      if (1) {
         int rbase = 4 + R%8;
         for (i = 0; i < MAXRHYTHM; i++)
	    rulep->rhythm[i] = (R%3 + 1)*rbase;
      }
      rulep->bsize = (R%4) ? (20 + R%20 + R%20) : (60 + R%80);
      rulep->randomized_underflow = R&1;
      rulep->search_time = 2 + R%5;
      if (R%2) {
	 rulep->hot = 200;
	 rulep->cool = 5 + R%20 + R%20;
      } else {
	 rulep->hot = 1 + 5 * (R%3);
	 rulep->cool = 5 + R%20 + R%10 + rulep->hot;
      }
      if ((distrib_original & dist) ||
	  rule_acidlife1 == rulep->rule ||
	  rule_acidlife2 == rulep->rule ||
	  rule_rotorug == rulep->rule) {
	 nbits = R%7 + 8;
	 rulep->mask = (1<<nbits) - 1;
	 for (b = R%4; b; b--)
	    rulep->mask &= ~(2<<(R%nbits));
      } else {
	 nbits = 8 + (R%3 + 1) * (R%2 + 1);
	 rulep->mask = (1<<nbits) - 1;
	 for (b = R%3 + 1; b; b--)
	    rulep->mask &= ~(2<<(R%nbits));
      }

      switch (rulep->rule) {
       case rule_acidlife1:
	 x = -(1 + (rulep->mask/100) + R%(rulep->mask/30));
	 break;
       case rule_acidlife2:
	 x = -(R%20);
	 break;
       case rule_rotorug:
	 x = -(9 + R%(rulep->mask/30));
	 break;
       case rule_wave:
	 x = -(10 + R%10);
	 break;
       default:
	 if (distrib_original & dist)
	    x = -(9 + R%(rulep->mask/30));
	 else
	    x = -1;
	 break;
      }
      rulep->speed_base = x;
      rulep->speed = x;

      if (R%4)
	 rulep->driver_slowdown = 1 + R%8 + R%7;
      else
	 rulep->driver_slowdown = 13 + R%8;


      rulep->drift_speed = 8 + R%5;
	  rulep->remap = R%4;

	  set_remap(rulep->mask, rulep->remap);
   
   }
   if (cmap) {
     int nc;
      static distrib0[] = {cmap_plain, cmap_plain, cmap_plain,
			   cmap_loop, cmap_path, cmap_path};
      static distrib[] = {cmap_mono, cmap_mono, cmap_mono4, cmap_heat};
      static distrib1[] = {cmap_mono, cmap_ramp, cmap_plain};
      if (rule.rule == rule_fuse) {
	nc = distrib1[R%alen(distrib1)];
      } else if (rule.rule == rule_acidlife1)
	 nc = cmap_split;
      else if ((rule.rule == rule_rd) && (R&1))
	 nc = cmap_path;
      else if ((rule.rule == rule_rd2) ||
	       (rule.rule == rule_quad))
	nc = cmap_plain;
      else if (R%7)
	 nc = distrib0[R%alen(distrib0)];
      else if (R%100)
	 nc = distrib[R%alen(distrib)];
      else
	 nc = cmap_noise;

      record1("color-type", &cmap->cmap, nc);
      record1("color" , &cmap->index,  R%ncmaps);

      cmap->index2 = R%ncmaps;
   }
   if (fill) {
      static distrib[] = {fill_vnoise, fill_noise};
	  if (rule.rule == rule_rd ||
		  rule.rule == rule_rd2 ||
		  R%2)
		fill->fill = distrib[R%alen(distrib)];
	  else
        fill->fill = R%nfills;
   }
}

double adjust_speed(double dt) {
	return dt * 10.0 / fps;
}

void print_state(rule_t *r) {
   printf("rule=%d\n", r->rule);
   printf("speed=%d\n", r->speed);
   printf("mask=%d\n", r->mask);
   printf("randomized_underflow=%d\n", r->randomized_underflow);
   printf("cycle_bkg=%d\n", r->cycle_bkg);
   printf("remap=%d\n", r->remap);
   printf("floor=%d\n", r->floor);
   printf("driver_slowdown=%d\n", r->driver_slowdown);
   printf("brain=%d (%d)\n", r->brain, iclamp(r->brain, 44));
   printf("brain_shift=%d (%d)\n", r->brain_shift, iclamp(r->brain_shift, 26));
   printf("hot=%d\n", r->hot);
   printf("cool=%d\n", r->cool);
   printf("speed_base=%d\n", r->speed_base);
   printf("speed_beast_speed=%d\n", r->speed_beat_speed);
   printf("speed_beat_size=%d\n", r->speed_beat_size);
   printf("drift=%d (%d)\n", r->drift, iclamp(r->drift, 9));
   printf("drift_speed=%d\n", r->drift_speed);
   printf("bsize=%d\n", r->bsize);
   
   printf("p1=%d\n", p1);
   printf("cmap=%d\n", cmap.index);
}
   


flip_image(int rule_lock)
{
   /* Z = zxC-b */ /* at one point, that meant something */
   int im = seq_next_image(rule.seq);
   file_to_image(im, current_image);
   distrib(distrib_new | distrib_continuous | rule_lock, &rule, 0, &fill2);
   fill_board(&fill2);
}

int grad_state;
#if 0

int grad_step(int steps) {
   int res;
   /* fprintf(stderr, "gs%d ", grad_state); */
   if (0 == grad_state)
      return 1;
   else if (grad_none == grad_state) {
      grad_state = steps;
   } else if (1 == grad_state ||
	      9 == ((steps - grad_state)%10)) {
      image2grad(&board2[dbuf], &board3[0], 1);
   } else {
      blur(&board2[dbuf], &board2[1-dbuf]);
   }

   grad_state--;
   res = ((steps - grad_state) > 10)
      || (0 == grad_state);
   fprintf(stderr, " %d ", res);
   return res;
}
#endif


cmap_permute_bits()
{
   int i;
   for (i = 0; i < 256; i++) {
      int j = (i >> 1) | ((i&1) << 7);
      int r = current_cmap[3*j+0];
      int g = current_cmap[3*j+1];
      int b = current_cmap[3*j+2];
      SET_TARGET_PALETTE(i, r, g, b);
   }
}

#define Rz(d,n) if (n <= 0) return; else d = R%(n)
void
image_to_heat(Image *src_img, image8_t *fb, board_t *board)
{
   int bsize = rule.bsize;
   int dest_x, dest_y;
   Image src_rect;
   image8_t dest = *fb;
   int move_heat = 1;

   switch (rule_symmetry(rule.rule)) {
    case sym_tile4:
    case sym_mirror4:
      Rz(dest_x, (fb->width>>1) - bsize);
      Rz(dest_y, (fb->height>>1) - bsize);
      break;
    case sym_mirror2:
      Rz(dest_x, (fb->width>>1) - bsize);
      Rz(dest_y, fb->height - bsize);
      break;
    case sym_frame_buffer:
      move_heat = 0;
    default:
      Rz(dest_x, fb->width - bsize);
      Rz(dest_y, fb->height - bsize);
      break;
   }
   image_random_tile(&src_rect, src_img, bsize);
   dest.p = fb->p + fb->stride * dest_y + dest_x;
   dest.width = src_rect.width;
   dest.height = src_rect.height;
   /* could be any operation here, not just copy */
   if (1) {
      int i, j;
      for (i = 0; i < src_rect.height; i++)
	 for (j = 0; j < src_rect.width; j++)
	    dest.p[dest.stride * i + j] =
	       255 - src_rect.pixels[src_rect.stride * i + j].r;
   }

   if (move_heat)
      pix2heat2(fb, board,
		dest_x, dest_y,
		dest_x + src_rect.width, dest_y + src_rect.height);
}


Image *pick_image(rule_t *p)
{
   Image *r;
   int n = iclamp(R, p->image_window_size);

   switch (p->rule) {
    case rule_fuse:
    case rule_static:
    case rule_slip:
      r = &global_images[n];
      break;
    default:
      r = &global_images_small[n];
      break;
   }
   return r;
}
      

void
changed_gparam(int *gparam)
{
   if (&cmap.index == gparam) {
      cmap.index = iclamp(cmap.index, ncmaps);
      fade2cmap(&cmap);
   } else if (&rule.cycle_bkg == gparam ||
	      &rule.remap == gparam ||
	      &rule.randomized_underflow == gparam)
      *gparam = (*gparam)&1;
   else if (&rule.image_window_size == gparam) {
      if (*gparam > N_RAM_IMAGES)
	 *gparam = N_RAM_IMAGES;
      else if (*gparam < 1)
	 *gparam = 1;
   } else if (&rule.seq[0] == gparam) {
      seq_start(rule.seq);
   }
   
   if (&rule.remap == gparam ||
	          &rule.mask == gparam) {
	   set_remap(rule.mask, rule.remap);
   }

   /* printf("changed gparam %#x to %d\n", gparam, *gparam); */
}


void do_mouse(int x, int y) {
  switch (rule.rule) {
     case rule_rotorug:
        rule.bsize = iclamp(x, 320);
        if (0) {
           extern double spinner_time;
           spinner_time = x + y*10;
           rule.drift_time = x/32.0 + y;
        }
        break;
     case rule_rug:
        rule.speed = iclamp(x,320)/8-20;
        break;
     case rule_rug_brain:
        pen(&board2[dbuf], 1, iclamp(x, 160), iclamp(y, 100), 10);
        break;
     case rule_rd:
        pen(&board[dbuf], 0, iclamp(x,160), iclamp(y, 100), 5);
        break;
     default:
        pen(&board[dbuf], rule.mask, iclamp(x,300), iclamp(y,180), 12);
        break;
   }
}



#if mac_bomb
static long start_time;
void begin_timer() {
  start_time = TickCount();
}
double end_timer() {
  return (TickCount() - start_time) / 60.0; /* just a guess */
}
#elif win_bomb
#else
#include <sys/time.h>

static struct timeval start_time;

void
begin_timer()
{
   struct timezone tzp;
   gettimeofday(&start_time, &tzp);
}

double
end_timer()
{
   static struct timeval end_time;
   struct timezone tzp;
   double t;
   gettimeofday(&end_time, &tzp);
   return
      ((end_time.tv_sec - start_time.tv_sec) +
       (end_time.tv_usec - start_time.tv_usec) * 1e-6);
}
#endif

void step_rule_rug(int frame, rule_t *p, image8_t *fb);
void step_rule_rug2(int frame, rule_t *p, image8_t *fb);
void step_rule_rug3(int frame, rule_t *p, image8_t *fb);
void step_rule_static(int frame, rule_t *p, image8_t *fb);
void step_rule_rotorug(int frame, rule_t *p, image8_t *fb);
void step_rule_acidlife1(int frame, rule_t *p, image8_t *fb);
void step_rule_acidlife2(int frame, rule_t *p, image8_t *fb);
void step_rule_rug_anneal(int frame, rule_t *p, image8_t *fb);
void step_rule_rug_anneal2(int frame, rule_t *p, image8_t *fb);
void step_rule_rug_rug(int frame, rule_t *p, image8_t *fb);
void step_rule_rug_brain(int frame, rule_t *p, image8_t *fb);
void step_rule_shade(int frame, rule_t *p, image8_t *fb);
void step_rule_wave(int frame, rule_t *p, image8_t *fb);
void step_rule_quad(int frame, rule_t *p, image8_t *fb);
void step_rule_rug_image(int frame, rule_t *p, image8_t *fb);
void step_rule_slip(int frame, rule_t *p, image8_t *fb);
void step_rule_fuse(int frame, rule_t *p, image8_t *fb);
void step_rule_rug_multi(int frame, rule_t *p, image8_t *fb);
void step_rule_rd(int frame, rule_t *p, image8_t *fb);
void step_rule_rd2(int frame, rule_t *p, image8_t *fb);

int self_play_enabled = !use_guile;

void self_play() {

  if (!self_play_enabled)
    return;

      if (0 >= timer) {
	 auto_mode = 1;
	 if (pop_back) {
	    rule.rule = back_to;
	    pop_back = 0;
	    timer = 100;
	 } else if (permuting) {
	    permuting--;
	    cmap_permute_bits();
	    timer = 150;
	 } else if (flipper && (rule_rug_brain == rule.rule) && (R%10)) {
	    switch (flipper) {
	     case 1:
	       rule.brain++;
	       break;
	     case 2:
	       distrib(distrib_new|distrib_continuous|rule_lock, &rule, 0, 0);
	       break;
	     case 3:
	       rule.brain_shift =
		  (R%8) ? (rule.brain_shift + ((R&1) ? -1 : 1)) : 0;
	       break;
	    }
	    timer = 50;
	 } else if (flipper && (rule_rug == rule.rule) && (R%5)) {
	    if (2 == flipper) {
	       rule.rule = rule_rug_image;
	       pop_back = 1;
	       back_to = rule_rug;
	       timer = (R%2) ? 4 : 30;
	       current_image = iclamp(current_image + 1,
				      rule.image_window_size);
	       drive_with_image(current_image);	       
	    }
	 } else if (flipper && (rule_rd == rule.rule) && (R%5)) {
	    timer = 100;
	    switch (flipper) {
	     case 1:
	       if (1) {
		  int im = seq_next_image(rule.seq);
		  file_to_image(im, current_image);
		  drive_with_image(current_image);
	       }
	       break;
	     case 2:
	       rule.brain = R;
	       break;
	     case 3:
	       rule.brain_shift = R;
	       break;
	    }
	 } else if (flipper && (rule_fuse == rule.rule) && (R%5)) {
	   rule.drift = (R%5) ? (R%2) : ((R%2)+2);
	   timer = 100;
	 } else if (flipper && (rule_rug_image == rule.rule) && (R%5)) {
	    static here = 0;
	    /* create an echo */
	    if (here = 1-here) {
	       current_image = iclamp(current_image + 1,
				      rule.image_window_size);
	       drive_with_image(current_image);	       
	    }
	    timer = 100;
	    switch (flipper) {
	     case 0:
	       break;
	     case 1:
	       if (1) {
		  int im = seq_next_image(rule.seq);
		  file_to_image(im, current_image);
		  drive_with_image(current_image);
	       }
	       break;
	     case 2:
	       flip_image(rule_lock);
	       break;
	     case 3:
	       rule.rule = rule_rug;
	       pop_back = 1;
	       back_to = rule_rug_image;
	       timer = (R%2) ? 4 : 30;
	       break;
	    }
	 } else if (flipper && (rule_rotorug == rule.rule) && (R%5)) {
	    timer = 100;
	    switch (flipper) {
	     case 1:
	       p1 = R;
	       rule.flame_cp = flames[iclamp(p1, nflames)];
	       break;
	     case 2:
	       random_control_point(&rule.flame_cp);
	       break;
	    }
	 } else if (flipper && (rule_rug_rug == rule.rule) && (R%5)) {
	    distrib(distrib_new|distrib_continuous|rule_lock, &rule, 0, 0);
	    timer = 200;
	 } else if (R%2 && (frame - cmap_changed) > 200) {
	    /* ugh */
	    if (rule.rule == rule_acidlife1) {
	       cmap.cmap = cmap_split;
	       fade2cmap(&cmap);
	    } else if (rule.rule == rule_quad && R%3) {
	      cmap_permute_bits();
	    } else if (rule.rule == rule_fuse && R%4) {
	       cmap.cmap = cmap_mono;
	       fade2cmap(&cmap);
	    } else {
	       if (frame > 10000 && !(R%20)) {
		  cmap_permute_bits();
		  permuting = 7;
	       } else {
		  distrib(distrib_new, 0, &cmap, 0);
		  fade2cmap(&cmap);
	       }
	    }
	    timer = 300;
	    cmap_changed = frame;
	 } else if (rule.rule == rule_quad && R%4) {
	   rule.brain++;
	   timer = 200;
	 } else if (R%4) {
	    int t = rule_lock;
	    /* this is the same condition as used for overdrive */
	    if (3 == flipper)
	       t |= distrib_rule_lock;
	    distrib(distrib_new | t, &rule, 0, 0);
	    timer = 300;
	    /* ugh */
	    if (rule.rule == rule_acidlife1) {
	       cmap.cmap = cmap_split;
	       fade2cmap(&cmap);
	    }
	 } else {
		if (rule.rule == rule_rd ||
			rule.rule == rule_rd2) {
			timer = 100;
		} else {
			distrib(distrib_new, 0, 0, &fill2);
			delayed_fill = 1;
			timer = 300;
		}
	 }
	 if (rule.rule == rule_quad) {
	   timer = timer / 4;
	   }
	 flipper = (frame >> 7) & 0x3;
	 if (!(R%20))
	    overdrive_speed = 5 + R%10 + R%20;
	 if (3 == flipper && !auto_rule)
	    timer = overdrive_speed;
      }

      if (auto_mode && !(R%500)) image_dst = !image_dst;
      if (auto_mode && !(frame%1000)) {
	 rule.seq[0] = R;
	 seq_start(rule.seq);
      }

      /* i really don't know how much i like this stuff */
      if (1) {
	 int tween;
	 static int tab1[] = {3000, 200, 100, 50};
	 static int tab2[] = {10000, 200, 100, 50};
	 if (auto_mode)
	    switch (rule.rule) {
	     case rule_slip:
	     case rule_fuse:
	       if (!(R%tab1[image_rate]))
		  image_rate = R&3;
	       break;
	     default:
	       if (frame < 10000)
		  image_rate = 0;
	       else if (!(R%tab2[image_rate]))
		  image_rate = R&3;
	       break;
	    }
	 if (image_rate) {
	    tween = 1 << ((3-image_rate) * 3);
	    if (!(frame%tween)) {
	       if ((3 == flipper2) && (frame > 20000)) {
		  if (image_dst)
		     image_to_heat(pick_image(&rule), &fb, &board[dbuf]);
		  else
		     image_to_heat(pick_image(&rule), &fb, &board2[dbuf]);
	       } else {
		  double_board(image_dst, &board[dbuf], rule_symmetry(rule.rule));
		  if (flipper2&1)
		     image_dst = 1-image_dst;
	       }
	    }
	 }
      }

}

void
display_manual()
{
  char buf[1000];
  sprintf(buf, "/usr/bin/less %smanual.txt", DATA_DIR);
#if xws_bomb && vga_bomb
  if (running_x)
#endif
#if xws_bomb
  {
    system(buf);
  }
#endif
#if xws_bomb && vga_bomb
  else
#endif
#if vga_bomb
  {
    vga_setmode(TEXT);
    system(buf);
    vga_setmode(G320x200x256);
    image8_set_cmap(0, 256, current_cmap);
  }
#endif
}

void param_tweeking(int key) {
  /* parameter tweeking */
  switch (key) {
    /* boy this is really dumb */
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    param = 10 * param + (key - '0');
    break;
  case '\n':
    *gparam = param;
    param = 0;
    changed_gparam(gparam);
    break;
  case '.':
  incr:
  (*gparam)++;
  changed_gparam(gparam);
  break;
  case ',':
    (*gparam)--;
    changed_gparam(gparam);
    break;

#define dogparm(field) \
    if (gparam == &(field)) goto incr; gparam = &(field); break;

  case 'q': dogparm(cmap.index);
  case 'w': dogparm(rule.speed_beat_size);
  case 'e': dogparm(rule.speed_beat_speed);
  case 'r': dogparm(rule.speed_base);
  case 't': dogparm(rule.image_window_size);
  case 'y': dogparm(rule.seq[0]);
  case 'u': dogparm(rule.remap);
    /* should skip patterns without enough bits */
  case 'i': dogparm(rule.mask);
  case 'o': dogparm(p1); break;
  case 'p': dogparm(rule.bsize);
  case '[':
  case ']': dogparm(rule.driver_slowdown);
  case '\\':dogparm(rule.search_time);
  case '{': dogparm(rule.hot);
  case '}': dogparm(rule.cool);
  case '|': dogparm(rule.brain);
  case 'P': dogparm(rule.brain_shift);
  case 'O': dogparm(rule.cycle_bkg);
  case 'I': dogparm(rule.randomized_underflow);
  case 5:   dogparm(rule.fastflip_rate);


  case 'a':
    if (rule_rug == rule.rule)
      rule.rule = rule_rug2;
    else if (rule_rug2 == rule.rule)
      rule.rule = rule_rug3;
    else
      rule.rule = rule_rug;
    break;
  case 's':
    rule.rule = rule_rug_image;
    break;
  case 'd':
    if (rule.rule == rule_rd)
      rule.rule = rule_rd2;
    else
      rule.rule =  rule_rd;
    break;
  case 'f':
    if (rule.rule == rule_rotorug)
      rule.drift++;
    else
      rule.rule = rule_rotorug;
    break;
  case 'g':
    if (rule.rule == rule_acidlife2)
      rule.rule = rule_acidlife1;
    else
      rule.rule = rule_acidlife2;
    break;
  case 'h':
    rule.rule = rule_rug_multi;
    break;
  case 'j':
    if (rule.rule == rule_rug_anneal)
      rule.rule = rule_rug_anneal2;
    else
      rule.rule = rule_rug_anneal;
    break;
  case 'k':
    rule.rule = rule_slip;
    break;
  case 'l':
    rule.rule = rule_rug_rug;
    break;
  case ';':
    rule.rule = rule_rug_brain;
    break;
  case '\'':
    if (rule.rule == rule_shade)
      rule.rule = rule_wave;
    else
      rule.rule = rule_shade;
    break;
  case 'A':
    rule.rule = rule_static;
    break;

  case 'F':
    if (rule.rule == rule_fuse)
      rule.drift++;
    else
      rule.rule = rule_fuse;
    break;
  case 'G':
    rule.rule = rule_quad;
    break;
  case 'L':
    rule_lock = rule_lock ? 0 : distrib_rule_lock;
    break;

  case 'z':
    distrib(distrib_new | distrib_continuous | rule_lock, &rule, 0, 0);
    break;
  case 'x':
    if (rule_fuse == rule.rule)
      scramble += 4;
    else {
      distrib(distrib_new, 0, 0, &fill2);
      delayed_fill = 1;
    }
    break;
  case 'c':
    distrib(distrib_new, 0, &cmap, 0);
    fade2cmap(&cmap);
    break;
  case 'v':
    switch (rule.rule) {
    case rule_rug:
    case rule_rug2:
      bomb_pulse();
      break;
    case rule_rug_image:
      current_image = iclamp(current_image + 1,
			     rule.image_window_size);
      drive_with_image(current_image);
      break;
    case rule_rug_rug:
      bomb_pulse_driver();
      break;
    case rule_rotorug:
      if (1) {
	int i;
	random_control_point(&rule.flame_cp);
	for (i = 0; i < flame_nspan; i++)
	  random_control_point(rule.flame_span+i);
	pick_liss_coefs();
	init_rotate();
	for (i = 0; i < MAXRHYTHM; i++)
	  rule.rhythm[i] = (R%3 + 1)*8;
      }
      break;
    default:
      rule.brain++;
      break;
    case rule_fuse:
      rotate_images();
      file_to_image(seq_next_image(rule.seq), 0);
      break;
    }
    break;
  case 'b':
    switch (rule.rule) {
    case rule_rug_brain:
      rule.brain_shift = rule.brain_shift ? 0 : (R%7);
      break;
    case rule_rug_multi:
      rule.brain_shift++;
      break;
    default:
    case rule_rd:
      /* maybe this should use seq_next_image, or at least
	 we want the capability (maybe use m) */
      file_to_image(p1++, current_image);
      drive_with_image(current_image);
      break;
    }
    break;
  case 'n':
    rule.speed_beat_size = rule.speed_beat_size ? 0 : (R%22);
    rule.speed_beat_speed = (R%3) ? (5 + R%10) : (30 + R%30);
    break;
  case 'm':
    if (rule_rug_image == rule.rule) {
      int im = seq_next_image(rule.seq);
      file_to_image(im, current_image);
      drive_with_image(current_image);
    } else
      rule.brain_shift++;
    break;
	     
  case '/':
    drive_with_image(current_image);
    break;
  case '?':
    display_manual();
    break;
  case '-':
    *gparam = - (*gparam);
    changed_gparam(gparam);
    break;
  case '=':
    write_fb_ppm(&fb);
    break;
  case 22: /* ^V */
#if use_mpeg
    if (periodic_write) {
      mpeg_end();
      periodic_write = 0;
    } else {
      char buf[20];
      sprintf(buf, "dribble/%03d.mpg", frame);
      mpeg_begin(320, 200, buf);
      periodic_write = 1;
    }
#elif use_quicktime
    if (periodic_write) {
      quicktime_close(qt_file);
      periodic_write = 0;
    } else {
      char buf[20];
      sprintf(buf, "%05d.mov", frame);
      qt_file = quicktime_open(buf, 0, 1);
      if (NULL == qt_file) {
	  perror(buf);
      } else {
	  if (use_jpeg) {
	      quicktime_set_video(qt_file, 1, 320, 200, 30, QUICKTIME_JPEG);
	      quicktime_set_jpeg(qt_file, 95, 0);
	  } else
	      quicktime_set_video(qt_file, 1, 320, 200, 30, QUICKTIME_RAW);
	  periodic_write = 1;
      }
    }
#else
    periodic_write = 10 - periodic_write;
#endif
    break;
  case 'Z':
    flip_image(rule_lock);
    break;
  case 'X':
    /* X = zcm */
    distrib(distrib_new | distrib_continuous | rule_lock, &rule, &cmap, 0);
    init_rotate();
    if (rule.rule == rule_acidlife1)
      cmap.cmap = cmap_split;
    fade2cmap(&cmap);
    random_control_point(&rule.flame_cp);
    pick_liss_coefs();
    break;
  case 'Q':
    timer = 10;
    break;
  case 'W':
    pulse_cmap_white();
    break;
  case 'E':
    pulse_cmap_black();
    break;
  case 'R':
    pulse_cmap_rotate();
    break;
  case 'T':
    brighten_cmap();
    break;
  case 'D':
    cmap_permute_bits();
    break;
  case 'V':
    rule.brain = 0;
    rule.brain_shift = 0;
    rule.drift--;
    break;
  case 'C':
    invert_board();
    break;
  case 'N':
    if (1) {
      extern smooth_checkers;
      smooth_checkers = 1;
    }
    break;
  case 'S':
    switch (cmap.cmap) {
    case cmap_plain: cmap.cmap = cmap_loop; break;
    case cmap_loop: cmap.cmap = cmap_path; break;
    case cmap_path: cmap.cmap = cmap_plain; break;
    }
    fade2cmap(&cmap);
    break;
  case 'Y':
    cool(&fb);
    break;
  case 'U':
    warm(&fb);
    break;
  case 'H':
    double_board(1, &board[dbuf], rule_symmetry(rule.rule));
    break;
  case 'J':
    double_board(0, &board[dbuf], rule_symmetry(rule.rule));
    break;
  case 2:		/* ^B */
    rotate_images();
    file_to_image(R, current_image);
    drive_with_image(current_image);
    break;
  case 15:	/* ^O */
    rotate_images();
    file_to_image(R, 0);
    break;
  case 26:	/* ^Z */
    random_image_set();
    break;
  case 14:	/* ^N */
    if (recording) {
      fclose(recording);
      recording = NULL;
    } else {
      char buf[30];
      sprintf(buf, "clip-%02d.scm", nrecords++);
      recording = fopen(buf, "w");
      if (!recording)
	perror("fopen");
    }
    break;
  case 'M':
	  if (rule.rule == rule_rotorug) {
         rule.flame_cp = flames[iclamp(p1, nflames)];
	  } else {
		 rule.brain_shift = 0;
	  }
    break;
  case 9:		/* ^I */
    image_to_heat(pick_image(&rule), &fb, &board[dbuf]);
    image_dst = 1;
    break;
  case 21:	/* ^U */
    image_to_heat(pick_image(&rule), &fb, &board2[dbuf]);
    image_dst = 0;
    break;
  case 25:	/* ^Y */
    image_rate = (image_rate+1)&0x3;
    break;

    /*
      for rbd's ballet music
      case 'P':
      rule.speed = -1;
      rule.speed_base = -1;
      rule.mask = 767;
      rule.driver_slowdown = 9;
      cmap.cmap = cmap_loop;
      fade2cmap(&cmap);
      break;
      case 'O':
      rule.speed = -1;
      rule.speed_base = -1;
      rule.mask = 1023;
      rule.driver_slowdown = 60;
      cmap.cmap = cmap_plain;
      cmap.index = 58;
      fade2cmap(&cmap);
      break;
      case 'K':
      cmap.cmap = cmap_mono;
      fade2cmap(&cmap);
      break;
      case 'J':
      cmap.cmap = cmap_heat;
      fade2cmap(&cmap);
      break;
      */
  case 'B':
    cmap.cmap = cmap_black;
    fade2cmap(&cmap);
    break;
  case 18:	/* ^R */
    dtime++;
    if (dtime == 8)
      dtime = 1;
    break;
	    
  default:
    printf("unknown key = %d\n", key);
    break;
  }
}

void mood_organ(int key) {
  switch (key) {
  case 'a':
    rule.rule = rule_rug;
    rule.speed = -1;
    rule.speed_base = -1;
    rule.mask = 255;
    rule.remap = 0;
    rule.randomized_underflow = 1;
    rule.speed_beat_size = 0;
    rule.hot = 1;
    rule.cool = 100;
    cmap.cmap = cmap_plain;
    fade2cmap(&cmap);
    break;
  case 'b':
    rule.rule = rule_rug_image;
    rule.speed = -1;
    rule.mask = 255;
    rule.remap = 0;
    rule.randomized_underflow = 1;
    rule.speed_base = -1;
    rule.speed_beat_size = 10;
    rule.speed_beat_speed = 50;
    rule.hot = 1;
    rule.cool = 100;
    cmap.cmap = cmap_plain;
    fade2cmap(&cmap);
    break;
  case 'c':
    rule.rule = rule_rug;
    rule.speed = -1;
    rule.speed_base = -1;
    rule.randomized_underflow = 0;
    rule.mask = 759;
    rule.remap = 0;
    rule.hot = 1;
    rule.cool = 20;
    rule.speed_beat_size = 0;
    break;
  case 'd':
    rule.rule = rule_rug2;
    rule.speed = -1;
    rule.randomized_underflow = 1;
    rule.mask = 1023;
    rule.remap = 0;
    rule.hot = 6;
    rule.cool = 32;
    rule.speed_base = -1;
    rule.speed_beat_size = 0;
    fill2.fill = fill_vnoise;
    fill_board(&fill2);
    break;
  case 'e':
    rule.rule = rule_rug;
    rule.speed = -1;
    rule.speed_base = -1;
    rule.mask = 1791;
    rule.randomized_underflow = 1;
    rule.hot = 6;
    rule.remap = 0;
    rule.cool = 2;
    rule.speed_beat_size = 0;
    fill2.fill = fill_noise;
    fill_board(&fill2);
    break;
  case 'f':
    rule.rule = rule_rug_image;
    rule.speed = -1;
    rule.mask = 255;
    rule.remap = 0;
    rule.randomized_underflow = 1;
    rule.speed_base = -1;
    rule.speed_beat_size = 0;
    file_to_image(p1++, current_image);
    drive_with_image(current_image);
    fill2.fill = fill_noise;
    fill_board(&fill2);
    break;
  case 'g':
    rule.rule = rule_rd;
    rule.brain = 0;
    rule.brain_shift = 0;
    fill2.fill = fill_noise;
    fill_board(&fill2);
    cmap.cmap = cmap_loop;
    fade2cmap(&cmap);
    break;
  case 'h':
    rule.rule = rule_rd;
    rule.brain = 1;
    rule.brain_shift = 3;
    fill2.fill = fill_noise;
    fill_board(&fill2);
    cmap.cmap = cmap_loop;
    fade2cmap(&cmap);
    break;
  case 'i':
    rule.rule = rule_rd;
    rule.brain = 1;
    rule.brain_shift = 5;
    fill2.fill = fill_noise;
    fill_board(&fill2);
    file_to_image(p1++, current_image);
    drive_with_image(current_image);
    cmap.cmap = cmap_loop;
    fade2cmap(&cmap);
    break;
  case 'j':
    rule.rule = rule_rotorug;
    rule.speed = -18;
    rule.speed_base = -18;
    rule.speed_beat_size = 0;
    rule.mask = 367;
    rule.remap = 1;
    rule.drift = 0;
    rule.drift_speed = 13;
    rule.bsize = 47;
    cmap.cmap = cmap_plain;
    fade2cmap(&cmap);
    break;
  case 'k':
    rule.rule = rule_rotorug;
    rule.mask = 255;
    rule.speed = -1;
    rule.speed_base = -1;
    rule.speed_beat_size = 0;
    rule.drift = 4;
    rule.drift_speed = 10;
    rule.bsize = 56;
    rule.remap = 0;
    cmap.cmap = cmap_plain;
    fade2cmap(&cmap);
    break;
  case 'l':
    rule.rule = rule_rotorug;
    rule.mask = 509;
    rule.speed_beat_size = 0;
    rule.speed = -11;
    rule.speed_base = -11;
    rule.drift = 8;
    rule.drift_speed = 8;
    rule.bsize = 89;
    rule.remap = 0;
    cmap.cmap = cmap_plain;
    fade2cmap(&cmap);
    break;
  case 'm':
    rule.rule = rule_acidlife2;
    rule.mask = 1919;
    rule.speed_beat_size = 9;
    rule.speed_beat_speed = 40;
    rule.speed_base = -2;
    rule.speed = -2;
    rule.brain = 0;
    break;
  case 'n':
    rule.rule = rule_acidlife1;
    rule.mask = 511;
    rule.brain = 0;
    rule.speed_base = -19;
    rule.speed = -19;
    cmap.cmap = cmap_split;
    fade2cmap(&cmap);
    fill2.fill = fill_noise;
    fill_board(&fill2);
    break;
  default:
    printf("undefined mood = %d\n", key);
    break;
  }
}


void
bomb_work()
{

  sprintf(status_string+20, "%d %d %d", rule.speed, rule.mask, cmap.index);

  {
    double e = end_timer();
    double goal_fps;
    if (e > 0) {
      fps = 1.0/e;


      sprintf(status_string, "%04.1lf ", fps);
      goal_fps = argd("fps", 80);
      if (fps > goal_fps) {
	delay = (1/goal_fps - e);

#ifdef linux
	if (stabilize_fps) {
	  usleep((int)(delay * 1e6));
	  fps = goal_fps;
	}
#endif
      }
    }
    begin_timer();
  }


	  
   if (pulse) {
	 pulse = 0;
	 rule.floor = 0;
   } else {
	   int n = rule.hot + rule.cool;
	   if (0 == n) rule.floor = 0;
	   else rule.floor = (frame % n) > rule.hot;
   }

      if (periodic_write &&
	  0 == (frame % periodic_write)) {
#if use_mpeg
	 write_fb_mpeg(&fb);
#elif use_quicktime
	 write_fb_quicktime(&fb);
#else
	 write_fb_ppm(&fb);
#endif
      }

      if (rule.rule == rule_fuse
	  && rule.fastflip_rate
	  && --fastflip<=0) {
	fastflip = rule.fastflip_rate;
	rotate_images();
	file_to_image(seq_next_image(rule.seq), 0);
      }

      {
	int boring = run_hist(&fb);
	if (rule.rule == rule_fuse && (1 == iclamp(rule.drift, fuse_ndrifts))) {
	  extern double avg_pix;
	  if (avg_pix > 180.0) cool(&fb);
	} else if ( auto_mode && boring &&
		    frame - last_clear > 400) {

	  fill2.fill = fill_noise;
	  delayed_fill = 1;
	}
      }
  
      step_cmap();
	  

#if win_bomb
	  if (1) {
		  extern int mouse_down, mouse_x, mouse_y;
		  if (mouse_down) {
			  do_mouse(mouse_x, mouse_y);
		  }
	  }
#endif
#if mac_bomb
      if (1) {
        Point p;
        EventRecord gEvent;
        if (GetOSEvent( mDownMask, &gEvent) || StillDown()) {
           GetMouse(&p);
           do_mouse(p.h, p.v);
         }
         GetOSEvent( mUpMask, &gEvent);
      }
#endif

      switch (rule.rule) {
       case rule_rug:
	 step_rule_rug(frame, &rule, &fb);
	 break;
       case rule_rug2:
	 step_rule_rug2(frame, &rule, &fb);
	 break;
       case rule_rug3:
	 step_rule_rug3(frame, &rule, &fb);
	 break;
       case rule_static:
	 step_rule_static(frame, &rule, &fb);
	 break;
       case rule_rotorug:
	 step_rule_rotorug(frame, &rule, &fb);
	 break;
       case rule_acidlife1:
	 step_rule_acidlife1(frame, &rule, &fb);
	 break;
       case rule_acidlife2:
	 step_rule_acidlife2(frame, &rule, &fb);
	 break;
       case rule_rug_anneal:
	 step_rule_rug_anneal(frame, &rule, &fb);
	 break;
       case rule_rug_anneal2:
	 step_rule_rug_anneal2(frame, &rule, &fb);
	 break;
       case rule_rug_rug:
	 step_rule_rug_rug(frame, &rule, &fb);
	 break;
       case rule_rug_brain:
	 step_rule_rug_brain(frame, &rule, &fb);
	 break;
       case rule_shade:
	 step_rule_shade(frame, &rule, &fb);
	 break;
       case rule_wave:
	 step_rule_wave(frame, &rule, &fb);
	 break;
       case rule_rug_image:
	 step_rule_rug_image(frame, &rule, &fb);
	 break;
       case rule_slip:
	 step_rule_slip(frame, &rule, &fb);
	 break;
       case rule_fuse:
	 step_rule_fuse(frame, &rule, &fb);
	 break;
       case rule_rug_multi:
	 step_rule_rug_multi(frame, &rule, &fb);
	 break;
       case rule_rd:
	 step_rule_rd(frame, &rule, &fb);
	 break;
       case rule_rd2:
	 step_rule_rd2(frame, &rule, &fb);
	 break;
       case rule_quad:
	 step_rule_quad(frame, &rule, &fb);
	 /*step_rule_quad(frame, &rule, &fb);*/
	 break;
       default:
	 printf("bad rule: %d\n", rule.rule);
	 step_rule_rug(frame, &rule, &fb);
	 break;
      }

      frame++;
      timer = timer - adjust_speed(dtime);
      old_rule = rule.rule;

		 if (pulse_driver) {
			 pulse_driver = 0;
			 rule.driver_slowdown = 0;
		 }

      if (((key = bomb_getkey()) > 0) && keyboard_enabled) {
	 timer = 100000;
	 auto_mode = 0;
	 image_rate = 0;
	 /* kbd_mode independent controls */
	 switch (key) {
	  case ' ':
	    auto_rule = 0;
	    distrib(distrib_new | rule_lock, &rule, &cmap, &fill2);
	    fade2cmap(&cmap);
	    delayed_fill = 1;
	    timer = 2000;
	    break;
	  case '`':
	    print_state(&rule);
	    break;
	  case 6: /* ^F */
		display_fps = !display_fps;
		break;

	  case '1':
	    kbd_mode = 0;
	    break;
	  case '2':
	    kbd_mode = 1;
	    break;
	  case '3':
	    kbd_mode = 2;
	    break;
	  case '4':
	    kbd_mode = 3;
	    break;
		  
	  case 27:
	  case 3:
	    bomb_exit();
	    break;
	  default:
	    switch (kbd_mode) {
	     case 0:
		   param_tweeking(key);
	       break;
	     case 1:
		   mood_organ(key);
	       break;
	     case 2:
	       /* image organ */
	       file_to_image(key, current_image);
	       drive_with_image(current_image);
	       break;
	     case 3:
	       /* color organ */
	       cmap.index = (key>>1)%ncmaps;
	       cmap.cmap = (key&1) ? cmap_plain : cmap_loop;
	       fade2cmap(&cmap);
	       break;
	    }	    
	 }
      }
#ifndef wbomb
	  self_play();
#endif


      flipper = (frame >> 9) & 0x3;
      flipper2 = (frame >> 10) & 0x3;

      change_rules(old_rule, rule.rule, &fb);

      if (delayed_fill) {
	      fill_board(&fill2);
	      delayed_fill = 0;
	 	  last_clear = frame;
      }
#if !use_sioux
     image8_flush();
#endif

      if (1) {
         int s;         
  
         if (rule.rule == rule_rotorug) {
           if (sound_present) {
             s = get_beat(1);
             rule.drift_time = frame + 1000 + 2*s;
             rule.drift_speed = s;
		   } else 
			 rule.drift_time += rule.drift_speed;
         } else {
             s = get_beat(0);
			 // rule.drift_speed = s;
             if (sound_present) {
               rule.speed = rule.speed_base - (rule.speed_beat_size * s / 3);
           } else {
               rule.speed = (int)(rule.speed_base + 0.5 * rule.speed_beat_size *
	  	       sin(2.0 * M_PI * frame / rule.speed_beat_speed));
	  }
	}

#if mac_bomb
        if (0) {
            Rect r;
            r.top = 0;
            r.bottom = 100;
            r.left = 0;
            r.right = 800;
            if (frame%800 == 0)
               EraseRect(&r);
            PenNormal();
            MoveTo(frame%800, 50+s);
            LineTo(frame%800, 50+0);
         }
#endif

	
      }
}

void
bomb_clear(int c) {
	fill2.fill = c;
    delayed_fill = 1;
}

void
bomb_pulse() {
	rule.floor = 0;
	rule.cool = 999999;
	pulse = 1;
}

void
bomb_pulse_driver() {
	rule.driver_slowdown = -1;
	pulse_driver = 1;
}
void bomb_set_flame(int r) {
	if (-1 == r) {
	   random_control_point(&rule.flame_cp);
	   pick_liss_coefs();
	} else {
	   rule.flame_cp = flames[iclamp(r, nflames)];
	}
}
void bomb_set_rule(int r) { rule.rule = r; }
void bomb_set_speed(int r) {
	rule.speed = r;
	rule.speed_base = r;
    rule.speed_beat_size = 0;
}
void bomb_set_mask(int r) { rule.mask = r; }
void bomb_set_pen_size(int r) { rule.bsize = r; }
void bomb_set_drift(int r) { rule.drift = r; }
void bomb_set_brain(int r) { rule.brain = r; }
void bomb_set_brain_shift(int r) { rule.brain_shift = r; }
void bomb_set_drift_speed(int r) { rule.drift_speed = r; }
void bomb_set_randomized_underflow(int r) { rule.randomized_underflow = r; }
void bomb_set_color(int r) { cmap.index = r; fade2cmap(&cmap); }
void bomb_set_color_type(int r) { cmap.cmap = r; fade2cmap(&cmap); }
void bomb_set_cycle_background(int r) {rule.cycle_bkg = r;}
void bomb_set_remap_colors(int r) {rule.remap = r;}

#if !win_bomb

#if use_guile

#define gh_void (gh_int2scm(0))

SCM
c_bomb_work(SCM s_n) {
  int n = gh_scm2int(s_n);
  int i;

  for (i = 0; i < n; i++) {
    gh_defer_ints();
    bomb_work();
    gh_allow_ints();
  }

 return gh_void;
}

SCM
c_fill_board(SCM s_fill) {
  int n = gh_scm2int(s_fill);
  fill2.fill = n;
  gh_defer_ints();
  fill_board(&fill2);
  gh_allow_ints();
  return gh_void;
}

SCM
c_set_color(SCM s_color) {
  int c;
  gh_defer_ints();
  c = gh_scm2int(s_color);
  bomb_set_color(iclamp(c, ncmaps));
  gh_allow_ints();
  return gh_void;
}

SCM
c_set_color_type(SCM s_color) {
  gh_defer_ints();
  bomb_set_color_type(gh_scm2int(s_color));
  gh_allow_ints();
  return gh_void;
}

SCM
c_pulse() {
  bomb_pulse();
  return gh_void;
}

SCM
c_pulse_driver() {
  bomb_pulse_driver();
  return gh_void;
}

SCM
c_get_flame_cp() {
  char buf[4000];
  gh_defer_ints();
  buf[0] = 0;
  sprint_control_point(buf, &rule.flame_cp, 0);
  gh_allow_ints();
  return gh_str2scm(buf, strlen(buf));
}

SCM
c_set_flame_cp(SCM s_s) {
  char *s = gh_scm2newstr(s_s, NULL);
  gh_defer_ints();
  parse_control_point(&s, &rule.flame_cp);
  gh_allow_ints();
  return gh_void;
}

SCM
c_random_flame_dir() {
  pick_liss_coefs();
  return gh_void;
}

SCM
c_random_flame_cp() {
  random_control_point(&rule.flame_cp);
  return gh_void;
}

SCM
c_get_flame_dir() {
  char buf[4000];
  gh_defer_ints();
  buf[0] = 0;
  sprint_control_point(buf, &cc_direction, 0);
  gh_allow_ints();
  return gh_str2scm(buf, strlen(buf));
}

SCM
c_set_flame_dir(SCM s_s) {
  char *s = gh_scm2newstr(s_s, NULL);
  gh_defer_ints();
  parse_control_point(&s, &cc_direction);
  normalize_liss_coefs();
  gh_allow_ints();
  return gh_void;
}

SCM
c_srandom(SCM s_s) {
  int seed = gh_scm2int(s_s);
  srandom(seed);
  return gh_void;
}

SCM
c_set_drift_time(SCM s_var) {
  int n = gh_scm2double(s_var);
  rule.drift_time = n;
  return gh_void;
}

SCM
c_get_drift_time() {
  return gh_double2scm(rule.drift_time);
}

SCM
c_file_to_image(SCM s_im, SCM s_n) {
  int im = gh_scm2int(s_im);
  int n = gh_scm2int(s_n);
  file_to_image(im, n);
  return gh_void;
}

SCM
c_drive_with_image(s_n) {
  int n = gh_scm2int(s_n);
  drive_with_image(n);
  return gh_void;
}

/* fails why? */
#define make_var(loc_name, name) \
SCM \
c_var_ ## name(SCM s_var) { \
  if (NULL == s_var) \
    return gh_int2scm(loc_name); \
  loc_name = gh_scm2int(s_var); \
  return gh_void; \
}

#define make_get_set(loc_name, name) \
\
SCM \
c_set_ ## name(SCM s_var) {\
  int n = gh_scm2int(s_var);\
  loc_name = n;\
  return gh_void;\
}\
  \
SCM \
c_get_ ## name() {\
  return gh_int2scm(rule.rule);\
}



make_get_set(rule.rule, rule)
make_get_set(rule.speed, speed)
make_get_set(rule.mask, mask)
make_get_set(rule.randomized_underflow, randomized_underflow)
make_get_set(rule.cycle_bkg, cycle_bkg)
make_get_set(rule.remap, remap)
make_get_set(rule.floor, floor)
make_get_set(rule.driver_slowdown, driver_slowdown)
make_get_set(rule.brain, brain)
make_get_set(rule.brain_shift, brain_shift)
make_get_set(rule.drift, drift)
make_get_set(rule.drift_speed, drift_speed)
make_get_set(rule.bsize, bsize)
make_get_set(rule.hot, hot)
make_get_set(rule.cool, cool)

void
main_prog(int argc, char *argv[])
{

  bomb_init();
      
  gh_new_procedure1_0("bomb-do-frame", c_bomb_work);
  gh_new_procedure1_0("bomb-fill", c_fill_board);
  gh_new_procedure0_0("bomb-pulse", c_pulse);
  gh_new_procedure0_0("bomb-pulse-driver", c_pulse_driver);

  gh_new_procedure1_0("bomb-set-color", c_set_color);
  gh_new_procedure1_0("bomb-set-color-type", c_set_color_type);

#if 0
  gh_new_procedure0_1("bomb-rule", c_var_rule);
#endif

  gh_new_procedure1_0("bomb-set-rule", c_set_rule);
  gh_new_procedure0_0("bomb-get-rule", c_get_rule);
  gh_new_procedure1_0("bomb-set-speed", c_set_speed);
  gh_new_procedure0_0("bomb-get-speed", c_get_speed);
  gh_new_procedure1_0("bomb-set-mask", c_set_mask);
  gh_new_procedure0_0("bomb-get-mask", c_get_mask);
  gh_new_procedure1_0("bomb-set-randomized-underflow", c_set_randomized_underflow);
  gh_new_procedure0_0("bomb-get-randomized-underflow", c_get_randomized_underflow);
  gh_new_procedure1_0("bomb-set-remap", c_set_remap);
  gh_new_procedure0_0("bomb-get-remap", c_get_remap);
  gh_new_procedure1_0("bomb-set-floor", c_set_floor);
  gh_new_procedure0_0("bomb-get-floor", c_get_floor);
  gh_new_procedure1_0("bomb-set-driver-slowdown", c_set_driver_slowdown);
  gh_new_procedure0_0("bomb-get-driver-slowdown", c_get_driver_slowdown);
  gh_new_procedure1_0("bomb-set-brain", c_set_brain);
  gh_new_procedure0_0("bomb-get-brain", c_get_brain);
  gh_new_procedure1_0("bomb-set-brain-shift", c_set_brain_shift);
  gh_new_procedure0_0("bomb-get-brain-shift", c_get_brain_shift);
  gh_new_procedure1_0("bomb-set-drift", c_set_drift);
  gh_new_procedure0_0("bomb-get-drift", c_get_drift);
  gh_new_procedure1_0("bomb-set-drift-speed", c_set_drift_speed);
  gh_new_procedure0_0("bomb-get-drift-speed", c_get_drift_speed);
  gh_new_procedure1_0("bomb-set-pen-size", c_set_bsize);
  gh_new_procedure0_0("bomb-get-pen-size", c_get_bsize);
  gh_new_procedure1_0("bomb-set-hot", c_set_hot);
  gh_new_procedure0_0("bomb-get-hot", c_get_hot);
  gh_new_procedure1_0("bomb-set-cool", c_set_cool);
  gh_new_procedure0_0("bomb-get-cool", c_get_cool);
  gh_new_procedure1_0("bomb-set-cycle-bkg", c_set_cycle_bkg);
  gh_new_procedure0_0("bomb-get-cycle-bkg", c_get_cycle_bkg);
  gh_new_procedure1_0("bomb-set-flame-cp", c_set_flame_cp);
  gh_new_procedure0_0("bomb-get-flame-cp", c_get_flame_cp);
  gh_new_procedure1_0("bomb-set-flame-dir", c_set_flame_dir);
  gh_new_procedure0_0("bomb-get-flame-dir", c_get_flame_dir);
  gh_new_procedure1_0("bomb-set-drift-time", c_set_drift_time);
  gh_new_procedure0_0("bomb-get-drift-time", c_get_drift_time);

  gh_new_procedure0_0("bomb-random-flame-cp", c_random_flame_cp);
  gh_new_procedure0_0("bomb-random-flame-dir", c_random_flame_dir);
  gh_new_procedure1_0("bomb-srandom", c_srandom);

  gh_new_procedure1_0("bomb-drive-with-image", c_drive_with_image);
  gh_new_procedure2_0("bomb-file-to-image", c_file_to_image);

  gh_eval_str("(load \"bomb.scm\")");

  if (1 == argc) {
    while (1)
      bomb_work();
  } else {
    int i;
    char s[1000];
    for (i = 1; i < argc; i++) {
      sprintf(s, "(load \"%s\")", argv[i]);
      gh_eval_str(s);
    }
  }
}

void
main(int argc, char *argv[])
{
  gh_enter(argc, argv, main_prog);
}

#else
/* only look for the command line options in the X version of bomb */
/* allow display in root window, and take an optional magnification factor,
and allow tiling and take a tiling factor, and allow disabling of background
pixmap for faster running, but crappy expose event handling*/
#if !mac_bomb && !win_bomb
int main(int argc,char *argv[])
{
char *endptr;

 argc--, argv++;
 while (argc > 0) 
   {
     if (strcmp(*argv, "-root") == 0)
       {
	 if ((multiply == -1) && (!do_tile))
	   multiply=0;
	 use_root = 1;
       }
     else if (strcmp(*argv, "-nobackpixmap") == 0)
       nobackpixmap=1;
     else if (strcmp(*argv, "-nosound") == 0)
       nosound=1;
     else if (strcmp(*argv, "-randomstart") == 0)
       randomstart=1;
     else if (strcmp(*argv, "-noshm") == 0)
       use_shm=0;
#if use_quicktime
     else if (strcmp(*argv, "-jpeg") == 0)
       use_jpeg=1;
#endif
     else if (strcmp(*argv, "-window-id") == 0) {
       argc--, argv++;
       if (argc > 0) {
	 use_windowid = 1;
	 sscanf(argv[0], "%x", &windowid);
       } else
	 break;
     } else if (strcmp(*argv, "-tile") == 0)
       {
	 do_tile=1;
	 if (multiply == 0)
	   multiply=-1;
	 use_root = 1;

	 argc--, argv++;
	 if (argc > 0)
	   {
	     notiles = strtol(*argv,&endptr,10);
	     if ((**argv == '\0') || (*endptr  != '\0'))
	       argc++, argv--;
	   }
	 else
	   break;
		
       }
     else if ( (strcmp(*argv, "-magnify") == 0)  
	       || (strcmp(*argv, "-multiply") == 0) )
       {
	 argc--, argv++;
	 if (argc > 0)
	   {
	     multiply = strtol(*argv,&endptr,10);
	     if ((**argv == '\0') || (*endptr  != '\0'))
	       printf("multiply has to be followed by a number\n");
	   }
	 else
	   {
	     printf("multiply has to be followed by a number\n");
	     break;
	   }
       }
     else if ( !strcmp(*argv, "-quiet")) {
       /* redirect stderr and stdout to dev null */
       int d = open("/dev/null", O_WRONLY);
       if (-1 == d) {perror("open"); exit(1);}
       if (-1 == dup2(d, 1)) {perror("dup2"); exit(1);}
       if (-1 == dup2(d, 2)) {perror("dup2"); exit(1);}
     }
     else if ( !strcmp(*argv, "-rule")) {
       argc--, argv++;
       if (argc > 0) {
	 init_rule_mask(*argv);
	 argc--, argv++;
       } 
       else
	 printf("rule requires an argument\n");
     }
     else {
       fprintf(stderr,"unknown option %s\n",*argv);
       fprintf(stderr, "usage: "
	       "\n"
	       "  -root   causes the image to be displayed in the root window,\n"
	       "          automatically a single image is magnified to fit the screen\n"
	       "          and a backing pixmap is used.\n"
	       "\n"
	       "  -magnify x    causes the 320x200 image to be scaled by a factor of x.\n"
	       "\n"
	       "  -tile [x]     causes the image to be tiled on the root window, can be\n"
	       "                used in combination with magnify, without an argument\n"
	       "                enough tiles are created to cover the root, with an\n"
	       "                option an x by x grid of tiles centered on the center\n"
	       "                of the screen will be created, if the screen is filled\n"
	       "                automatically a backing pixmap is used.\n"
	       "\n"
	       "  -nobackpixmap    by default when the bomb images fill the screen a root\n"
	       "                   background image is created that layers the\n"
	       "                   background to allow seamless movement of windows\n"
	       "                   across a working bomb without blank areas appearing\n"
	       "                   and reappearing, but this can be slow especially\n"
	       "                   with a single large image.  so if this is too slow,\n"
	       "                   or if you dont want it use this feature use this\n"
	       "                   option.\n"
	       "\n"
	       "  -noshm     the MIT SHM is used by default if compiled in, use this if\n"
	       "             for some reason you dont want to use it.\n"
	       "\n"
	       "  -nosound   disable sound input, do not even open the /dev/dsp device.\n"
	       "\n"
	       "  -randomstart   do not melt the splash page - start with random mode.\n"
	       "\n"
	       "  -quiet     no output to stdout or stderr.  this is essential for use\n"
	       "             with xscreensaver!\n"
	       "\n"
	       "  -rule xxx  Run with a particular rule, depending on xxx.  available\n"
	       "             rules are acidlife, anneal, rugrug, static, flame, brain,\n"
	       "             image, shade, quad, fuse, wave, fuse, slip, rug, and rd.\n"
	       "             You may add them together, or subtract them, in which\n"
	       "             case, they become rules to avoid.  For example:\n"
	       "\n"
	       "    bomb -rule flame                    # flame mode only\n"
	       "    bomb -rule flame+image              # flame and image modes\n"
	       "    bomb -rule -static                  # any but static\n"
	       "    bomb -rule -static-fuse             # any but static or fuse \n");
     }
     argc--, argv++;
   }

   bomb_init();
   
   while (1) {
     bomb_work();
   }
   return 0;
}
#else
void
main()
{
   bomb_init();
   
   while (1) {
     bomb_work();
   }
}
#endif
#endif
void
message(char *s) {}
#endif
