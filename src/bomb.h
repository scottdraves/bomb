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

static char *bomb_h_id = "@(#) $Id: bomb.h,v 1.1.1.1 2002/12/08 20:49:40 spotspot Exp $";

#ifdef __cplusplus
extern "C" {
#endif


#define rule_rug            0
#define rule_rug2           1
#define rule_static         2
#define rule_rotorug        3
#define rule_acidlife1      4
#define rule_acidlife2      5
#define rule_rug_anneal     6
#define rule_rug_anneal2    7
#define rule_rug_rug        8
#define rule_rug_brain      9
#define rule_shade         10
#define rule_wave          11
#define rule_rug_image     12
#define rule_slip          13
#define rule_fuse          14
#define rule_rug_multi     15
#define rule_rd            16
#define rule_rd2           17
#define rule_quad          18
#define rule_rug3          19
#define nrules             20

#define cmap_mono   0
#define cmap_mono4  1
#define cmap_loop   2
#define cmap_path   3
#define cmap_heat   4
#define cmap_plain  5
#define cmap_split  6
#define cmap_noise  7
#define cmap_black  8
#define cmap_ramp   9

#define fill_hramp  0
#define fill_vramp  1
#define fill_grid   2
#define fill_noise  3
#define fill_vnoise 4    /* vert sym */
#define fill_sparse 5    /* each bit 1/(2^4) */
#define nfills      6

void bomb_work();
void bomb_init();

void bomb_clear(int);    /* fill_xxx */
void bomb_pulse();
void bomb_pulse_driver();

void bomb_set_rule(int);    /* rule_xxx */
void bomb_set_mask(int);
void bomb_set_speed(int);
void bomb_set_pen_size(int);
void bomb_set_drift(int);
void bomb_set_drift_speed(int);
void bomb_set_color(int);
void bomb_set_flame(int);
void bomb_set_brain(int);
void bomb_set_brain_shift(int);
void bomb_set_color_type(int);  /* cmap_xxx */
void bomb_set_randomized_underflow(int);
void bomb_set_cycle_background(int);
void bomb_set_remap_colors(int);


extern int display_fps;

#ifdef __cplusplus
}
#endif



