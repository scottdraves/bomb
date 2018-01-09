/*
    fractal flame generation package
    Copyright (C) 1992  Scott Draves <spot@cs.cmu.edu>

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


#ifndef cmap_included
#define cmap_included

static char *cmap_h_id = "@(#) $Id: cmap.h,v 1.1.1.1 2002/12/08 20:49:45 spotspot Exp $";

void init_cmaps();
int next_contrasting_color(int c, int i, int d);

#define maxcmaps 200
extern int ncmaps;
extern int the_cmaps[maxcmaps][256][3];

#endif
