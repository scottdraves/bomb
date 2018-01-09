bomb - organic visual music
Scott Draves (bombreadme@draves.org)
http://draves.org/bomb
see the file COPYING for the license covering this software.

$Id: README.txt,v 1.1.1.1 2002/12/08 20:49:37 spotspot Exp $

This is release 1.30 for Windows using the DirectDraw and DirectSound
interfaces.  It requires DirectX8, currently downloadable from:

  http://www.microsoft.com/directx/homeuser/downloads/default.asp

Bomb runs on many other platforms, see the web page for downloads.

The file manual.txt documents how to run and use Bomb.  When Bomb is
running, use the '?' key or F1 to bring up the manual on-screen.

___


You are encouraged to exhibit the output of this software publicly.

I also encourage you to let me know how you use it, what you like
about it, and how it can be improved.

___

!! If you are compiling the source, you must set DATA_DIR in defs.h !!

The source code is available from the web page under 
the GPL - the GNU GENERAL PUBLIC LICENSE.

chunks of code and various rules have been taken from or contributed
by various sources (i'm always looking for more stuff, so email if you
have something that you think fits).  sources include:

jepler@herbie.unl.edu  originally derived from vga_eyecandy
Tom Lawrence           idea for rotor
Nick Thompson          wisps, waves, fade2cmap, many suck/*.gif
Eric Nyberg            gifs
Jim Frost              xli header file (copyright below)
Kirk L. Johnson        gif code (copyright below)
Torben Mogensen        idea for symmetric life
Andrew Willmott        lots of help for all sorts of mac things
rus@apple.com          mac sound code originally from cthugha
Peter Broadwell        showed me coupled quadratic maps
Jamie Zawinski         bits of X code (copyright below)
David Zicarelli        bug fix and max version
Martin C. Martin       bug found in tiff reading, ported to BeOS
Caolan McNamara        added root window display
Sebastian Kuzminsky    bug found when flame data missing
Andreas Stolcke        vroot.h (copyright below)
Jeremy Braun           ESD support

thanks to nix for many interesting discussions and don
hopkins@interval.com for inspiration.  thanks to toffoli and margolus
for their excellent book _cellular automata machines_ (isbn
0-262-20060-0, or see http://www.im.lcs.mit.edu/cam8).  thanks to the
graphics lab at CMU SCS for the use of their fine hardware, and the
Transmeta Corporation for their indulgence.

-----------------------

release 1.30, as of 6/8/2003
support any combination of (32 and 24 bits) vs (320x200 and 640x480).

release 1.29, as of 12/8/2002
put into cvs, made DATA_DIR relative for development, added rcs Ids to
all the files.

release 1.28, as of 11/4/2001
added filter to sound level that removes DC component of the signal.
If graphics card does not support 320x200, then failover into 640x480
with lower performance.

release 1.27, as of Fri Oct 19 2001
ported to win2k.

release 1.26, as of Mon Apr 17 2000
automatically failover from ESD to nosound.  add quicktime output
(on control-v).

release 1.25, as of ?
cleaned warnings out of code.  add -nosound option.  added
-randomstart option.  make errors in /dev/dsp initialization be
nonfatal.  support ESD (enlightenment sound daemon, from Jeremy Braun
jtbraun@mit.edu).  support Gnome screensaver control center.

release 1.24, as of Thu Jul  8 1999
made work on redhat 6 (put -lvga before -lX11, convert all images to
gif).  some cleanup.  made RPM files.

release 1.23, as of Wed Mar 24 1999
fix pixmap leak, add histograms (display ifdefed out), change splash
page, add usage message.

release 1.22, as of Wed Nov 18 1998
fix from caolan to work with virtual root windows (aka xscreensaver).
add -quiet option for same purpose.  moved rule_mask control from
envar to command-line argument.

release 1.21, as of Tue Nov 17 1998
merged root window code from caolan, added rule_mask control, fixed
bug reported by sebastian.  doesn't work in root window in 8 bits.
removed guile version from standard dist.  added vroot.h, but still
doesn't work with xscreensaver.

release 1.20, as of Mon Sep 28 1998
fixed bug found by Martin C. Martin in tiff reading code.  rebuilt
gbomb.  update doc.

release 1.19, as of Sep 24, 1998
use a buffer with svgalib, since framebuffer ram is on the PCI bus and
is very slow (this fixes slip mode 'k').  use a better algorithm to
keep a constant frame rate.  handle X key events for shifted digits
and other symbols (ie the question mark).  help file works in
xws_bomb.  stabilize_fps now defaults to true at 80fps.  moved version
into makefile, redid dist and install targets.  the default is now to
find it's files in /usr/lib/bomb instead of ".".

release 1.18, as of Mar 8, 1998
fixed X code, works in 8, 16, and 32 bpp.  integrated svga and X
versions into same executable.  added new variant of the rug rule (on
'a') which makes circular impressions.  fixed mod by zero bug in
image_to_heat (found by David Zicarelli).

release 1.17, as of Jan 15, 1998
smoothed use of rhythm in dancing flames.  changed license.  added
guile.  sorted contents of suck-dir.  began to add 16/32 bpp X, but
not finished.  xbomb from prev release included instead.

release 1.16, as of July 22, 1997
added fastflip (on ^E) and auto-cooling to fuse mode.  added
splatter-blitting to fuse mode.  moved right mirror of quad mode over
by one pixel to eliminate double middle pixel.  removed climbing
drift-mode from fuse mode.  fixed OBO in match.c.  made cmap_plain
more likely.  adjusted self play for fuse mode.  '?' displays
manual.txt in vga/linux version.  included X windows version.

release 1.15, as of Thu Jun 26 1997
added quad mode.  in rotor, made quickly rotating flame change slowly
in addition to rotating quickly.  in rotor, made pen size depend on
sound power.  in linux version, made cheaper small random number
routine.  added brain=0 to 'V'.  added creeping edges to spiral slip
mode.

release 1.14, as of Apr 25 1997
autoplay doesn't fill the screen when in rd mode.  in initial
parameters, enable audio input by default (if present).  also, made
use of audio more likely if present (~3/4 instead of 1/3).  added
schwa and yugung to suck; removed centipedes.  adjusted rd2 to only
throw away low 7 bits instead of 8.  John E. Pearson reports that the
equations for the original RD are from his paper "Complex Patterns in
a Simple System" Science, 261,189, 9 July 1993.  added sound to linux
version. added santafe, acoma, and chilkat to suck.  changed rd2 to
use 15 bits, so now it basically works, but ther is still a problem.

release 1.13, as of Dec 6, 1996
added MPEG output with code from the MPEG Software Simulation Group
(http://www.mpeg.org/MSSG/) (SGI version only).
preliminary release for win32 API.  second win32 release 10/5.
third win32 release 10/21---included are bomb.exe and the screen-saver
bomb.scr.  fourth win32 release 11/21---direct draw version.  still
no sound input.  removed screen saver.  added new mode:  RD2.
release 12/1 sound input mostly working.  added image-driven masking
to static.  12/5 sound input apparently working.  added about five
more eskimo masks and two islamic patterns.  adjusts speed of
drifting and controls according to frame rate.  only released on win32
and mac.

release 1.12, as of Sep 18, 1996
added several new suck/*, especially masks.  added sound input to SGI
and Mac (mac code from cthugha), hooked up to diffusion-speed and
flame-drift.  it can't yet tell if the device is present but no music
is playing, so it doesn't always use the sound input on the
diffusion-speed, since it is also good without it.  use 'n' to toggle
this.  added triangular tilings for RD textures.  blocks on title page
waiting for keystroke.  implemented mac version of fname2fnum to fix
'Z'. (released on mac only)


release 1.11, as of May  6 1996
location of data files now a variable in makefile as requested by
joey@kite.preferred.com.  'm' in image rule like 'b' but follows image
sequence.  in Makefile, added tiff.h tiffio.h, and dependencies to
DIST, made linux version link statically.  autorule fixed (was
starting at 3 not 4), fiddled with auto_mode basically to make rule_rd
work better.  changed how rhythm is used by the flame/rotor.  mac
version in cludes `big bomb' which runs at 640x400.

release 1.10, as of Mar 25 1996
added cmap_path, added rule_rd to preferred list.  added rotating
spiral to rd and slip.  added new image2grad, used in rd and slip..
fill_board in slip works.  adjusted hist to ignore edges of screen, so
it detects empty slip board from expansion.  fixed board doubling.
added pixel replication on the mac, but it's much too slow, and the
cmaps are broken.  added v simple mouse input on mac.

release 1.9, as of Mar 10 1996
handles tiff images via libtiff, converted suck/* and title* to tiff.
it only accepts tiff images in IBM-PC order (i think this is a fault of
TIFFReadRGBAImage).  libtiff is the work of Sam Leffler	(sam@engr.sgi.com)
and can be found at ftp://ftp.sgi.com/graphics/tiff.  various
new diffusion patterns for RD textures, including prototype gradient
mode.  properly refresh screen on exit on mac.

release 1.8, as of Feb 7 1996
parameterized RD to various diffusion patterns, including driving by
image.  RD dithers between kernels to allow any angle (eigenvectors)
(try `dP6 RET |1 RET Sx'), but 1) even/odd zones due to rotated kernel
2) eigenvalues sometimes wrong because we don't mix in circular
component. fixed waves (wasn't initializing because before it wasn't
static so it was sharing values with shade).  added color, shape, and
mood organs.  defined a-n on the mood organ.  (on the mac: fixed colors
(had exchanged blue and green channels).  fixed keyboard handling (use
GetOSEvent), added handling of command-q.  moved suck files into
subdirectory.  hides cursor. R is 32 bits not just 16.)

release 1.7, as of Jan 30 1996
added reaction diffusion textures, as per
http://www.ccsf.caltech.edu/ismap/image.html.  lots more to do with
these.  moved RD to 'd', various updates to manual.txt.

release 1.6, as of Jan 25 1996
merged in version from laptop (exchanged rotor shapes `rectangular'
and `vertical'.  added 'H' (horizontal) and 'J' (vertical) double heat
buffer (though i think they're broken), added to auto_mode.)  mac
version flushes events before it exits.  if not in emacs, then use CR
to keep ticks on one line.  added delay with feedback control to keep
frames per second near 15.  moved cmap pulse code into procedures.

release 1.5, as of Jan 15 1996
improved Mac port: it basically works, though the keyboard handling is
a mess.  it's based on polling so a quick key press-release is missed.
i'm not sure how to get events without handing over control to other
threads.  the mac version can only load ppm files, and for some reason
i can't load files in a subdirectory.  various bug fixes.  added a
bunch of new colormaps from paintings.  added image2cmap.c, which
computes the colormaps.

release 1.4, as of Jan 10 1996
flipped OpenGL version right-side-up, boosted pixel replication to
3x3.  note that some implementations of OpenGL perform terribly on
pixel replication (eg a GU1-Extreme); in this situation try setting
pix_rep to 1.  added flush to title screen display, added mac port,
replaced missing title.gif file.

release 1.3, as of Dec 22 1995
added ports to X windows and OpenGL (i was hoping it would be faster,
but instead it's slower.  still upside down anyway).  fixed bug in
slip (it was writing off the edge of the fb).  until my laptop
recovers a bunch of other changes are lost on its disk.  in the
meantime i reimplemented 'S' which toggles smooth/plain cmaps, cuz i
like it so much.

release 1.2, as of 
added manual.txt.  sometimes screen activity stops but still accepts
key input, has the rule gone bad???.  reduced chance of using
image_to_heat() in slip automode by 10x.  added symmetric life from
torben, added fredkin rule and special filling for it (controlled by
brain parameter 'v'). these still need some work.  adjusted edge
conditions for all the acidlife rules.  doesn't overwrite files in
dribble directory (write_fb_ppm).  displays any titleN.gif files (N =
0,1,2,...) on startup with keypress to go to next.  added pop_back to
automode.  added 'D' permute colormap by bits, and incorporated into
automode.  added angular slugs (shade.c). added ticking and fps
(frames per second) printout.

release 1.1, as of Aug 14 1995
adjusted how overdrive and rule_lock work in automode.  ' ' disables
auto_mode.  added 'B' fade to black.  added quick startup option.
adjusted bsize for the rotor (reduced the high end).  fixed clearing
buffers with driving image.

release 1.0, as of Jul 14 1995
auto_rule makes sure it starts out well.  on transition to
rule_rug_image, clear the buffers with the driving image, too often we
have noise, which looks bad.

release 0.20, as of Jul 12 1995
fixed bad scanline bug in multi_rug.  redid many of the kbd controls
so that shift and control are not needed for common controls, more
context sensitive keys in general.  zxcvbn cause events, asdfghjkl;'
switch modes, qwertyuiop[]| pick a parameter to vary, ,.1234567890 and
return vary the parameter.  if you repeated hit a parameter key, it
varies that parameter.  added lissajous and oscilloscope-oid to rotor,
added pens to rotor (draws bsize rects instead of just dots), added
image sequences, rearranged all sorts of image controls, wrote some
sequences.  wrote manual.txt, explaining the ui.  fixed bug in initial
parameters (wasn't setting speed_base).

release 0.19, as of Jun 18 1995
added more images, cropped and tuned existing images.  tracks variance
of pixels in fb, if it's too low, then randomized the fb.  improved
next_contrasting_color, seems ok now.  in auto-mode, reduced chance of
filling.  uses a checkerboard if no images are found in the suck dir.
in rotorug, drift-mode is more likely to be chosen.  added mode-lock
on 'L'.  added rug_multi mode.  more images in the suck dir.  added
title page (need a better one though).  fixed %0 bug in
image_random_tile().  added optional underflow to shade mode.  removed
night-blue-sky and mauve-splat cmaps (too often result in
near-blackness).  split off fast.c from bomb.c, and disabled
optimization of bomb.c.  added blitting image tiles into the heat on
'^I' and into the game board on '^U', '^Y' cycles auto-blit rate.
added speed beat (rate on '}', size on '{').  reads cmaps at runtime.
wrote image2cmap.  fixed uninitialized driver plane bug.

release 0.18, as of May 25 1995
quick-flip auto-mode, redid interpretation of driving function in
brain mode to include asymmetric diffusion kernels, added fuse mode
(still needs work), fixed bug in writing ppm files.  when leaving
modes that operate directly on the fb (eg slip, fuse), pixel values
are interpreted as heat.  bsize made variable.

release 0.17, as of Apr 25 1995
removed contents of dribble directory. added more auto-mode features.
added rhythmic flames.


release 0.16, as of Apr 21 1995
added 'C' (invert driver) '^B' (load random image) 'Z' (combined zx
and ^B), all basically for rug_image mode.  included a bunch of
images. added 'X' = zcm for flame/rotor.  enabled `bent' flame
variation (?).  reads gif files (code taken from xli, see copyright
notices below).  '=' writes current screen into dribble
directory. brain chooses random shift bits, and includes PHASE_ZAP
code to prevent strobing (somewhat effective).  'P' modifies these
shift bits, should build a different sub_brain_table for different
shift bits.  added 'E', 'W', 'R' colormap pulsers.  added 'T' colormap
brightener. added 'Q' which enters auto-mode immediately (instead of
waiting for the time delay), made auto-mode fancier.  added some
variations to slip. '^Q' speeds up auto-mode.  added many gif files to
suck directory.

release 0.15, as of Feb 16 1995
put drift_mode into distrib now that flame is fixed.  removed
mode_life since it was never implemented and was causing a bug.  added
terrible hacks so that random mode can get display images.  smoothed
over transitions.  loading of files less hacked, holds 5 of them in
memory for fast access, 'B' rotates through them.  code really getting
cruddy.  moved rule_rug_image and support into image_rule.c.  included
a ppm file in the release.

release 0.14, as of Feb 15 1995
'R' pulse_gparam (not quite right, i think), suck.ppm replaced by
suck/$(p1).ppm (p1 still under 'o').  new drift modes for flames:
rotation, and none (ie the flame stays still).  bug fix from
bcsaller@mailbox.syr.edu (call vga_init()).  flames finally fixed by
resetting point if it strays outside 1e10.  reads library of flames in
at startup, 'M' sets to p1.

release 0.13, as of Feb 10 1995
'z' no longer resets flame coefs, 'm' does instead.  smooth colormap
transition (though the whole cmap is reset every frame producing
an icky video artifact on my display) from nix.  image mode on 'S'
(reads from suck.ppm on '/') and wispy waves on 'A' from nix.  both of
these are still quite rough.

release 0.12, as of Feb  7 1995
mostly fixed flames.  nice!

release 0.11, as of Feb  7 1995
used most of the brain parameters (p key), handles changing symmetry
more smoothly, brain remaps heat, new compiler flags, flame option in
rotor code (v key) (though still somewhat broken), pulsing (b
key). generalized brain some more (see XXY), though still not
used. twiddled constants, batched flame movement, added drift controls
(\[, - keys).

release 0.10, as of Jan 23 1995
memory maps frame buffer (still usually ignores width/height though),
renamed the whole thing `bomb'.  redid the makefile.

release 0.9, as of Jan 19 1995
kbd commands redone, new rules brain (lots of unused parameters here)
and shade, broke rules into separate files

release 0.8, as of Jan 13 1995
more kbd commands (including simple scripting), bugs fixed

release 0.7, as of Jan 10 1995
keyboard input, life/rug variants are mirrored

release 0.6, as of Jan  3 1995

-----------------------

the following is for gif.h and gif.c

/****
  Copyright 1989, 1990 Kirk L. Johnson

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that copyright notice and this
  permission notice appear in supporting documentation. The
  author makes no representations about the suitability of this
  software for any purpose. It is provided "as is" without express
  or implied warranty.

  THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS,
  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT
  OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
****/

the following is for xli-image.h

/*
 * Copyright 1989, 1993 Jim Frost
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  The author makes no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */


the following is for the code taken from xscreensaver:

/* xscreensaver, Copyright (c) 1993, 1994, 1995, 1996, 1997
 *  by Jamie Zawinski <jwz@netscape.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */


/*****************************************************************************/
/**                   Copyright 1991 by Andreas Stolcke                     **/
/**               Copyright 1990 by Solbourne Computer Inc.                 **/
/**                          Longmont, Colorado                             **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    name of Solbourne not be used in advertising                         **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    ANDREAS STOLCKE AND SOLBOURNE COMPUTER INC. DISCLAIMS ALL WARRANTIES **/
/**    WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF    **/
/**    MERCHANTABILITY  AND  FITNESS,  IN  NO  EVENT SHALL ANDREAS STOLCKE  **/
/**    OR SOLBOURNE BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL    **/
/**    DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA   **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/
