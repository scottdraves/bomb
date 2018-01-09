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

static char *sound_h_id = "@(#) $Id: sound.h,v 1.1.1.1 2002/12/08 20:49:54 spotspot Exp $";


#ifdef __cplusplus
extern "C" {
#endif

extern int sound_present;

extern void init_sound();
extern void exit_sound();

extern int get_sound();
extern int get_beat(int type);

extern unsigned char *bomb_sound_buf;
extern int sound_buf_last;
extern int sound_buf_end;

#ifdef __cplusplus
}
#endif
