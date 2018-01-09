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

static char *image_seq_c_id = "@(#) $Id: image_seq.c,v 1.1.1.1 2002/12/08 20:49:49 spotspot Exp $";

#include "defs.h"

#define maxnext 6

typedef struct {
   int start; /* index into seq_images */
   int fuse;
   int nnext;
   int next[maxnext];
} seq_t;

#define maxseq 100
seq_t seq_tab[maxseq];
int lastseq;

int fname2fnum(char *fn);

/* table of image indices, sequences terminated by eseq */
#define eseq (-1)
int seq_images[1000];

extern int image_dir_len;

int
seq_next_image(int *seq_p)
{
   int seq = seq_p[0];
   int im = seq_p[1];
   seq_t *c = &seq_tab[seq];
   int ans;

   im++;
   switch (seq) {
    case 0: ans = R; break;
    case 1: ans = im; break;
    default:
      if (eseq == seq_images[im]) {	 
	 if (c->nnext)
	    seq = c->next[R%c->nnext];
	 else
	    seq = R%(lastseq+1);
	 im = seq_tab[seq].start;
      }
      ans = seq_images[im];
      break;
   }

   seq_p[0] = seq;
   seq_p[1] = im;

   if (1) {
      extern int auto_mode;
      static int mixin = 0;
      mixin++;
      if (auto_mode && !(mixin%3))
	 return R;
   }
   return ans;
}

void
seq_start(int *seq)
{
   seq[0] = iclamp(seq[0], (lastseq+1));
   seq[1] = seq_tab[seq[0]].start;
}

void
init_seq()
{
   char buf[1000];
   FILE *in;
   int i, last;
   char bf[100];

   sprintf(buf, "%sseq-data", DATA_DIR);
   in = fopen(buf, "r");

   /* fill in default sequence */
   for (i = 0; i < maxseq; i++) {
      seq_tab[i].start = 0;
      seq_tab[i].fuse = 1;
      seq_tab[i].nnext = 1;
      seq_tab[i].next[0] = i;
   }

   last = 0;
   lastseq = 2;

   if (NULL == in)
      return;

   /* read sequences from file */

   while (1 == fscanf(in, " ( %s9", bf)) {
      if (!strcmp(bf, "comment")) {
	 while (')' != fgetc(in));
      } else if (!strcmp(bf, "seq")) {
	 int n;
	 int nnext;
	 fscanf(in, " %d *", &n);
	 if (n > maxseq) {
	    fprintf(stderr, "truncating %d to maxseq = %d\n", n, maxseq);
	    n = maxseq;
	 }
	 if (n > lastseq)
	    lastseq = n;
	 seq_tab[n].start = last;
	 seq_tab[n].fuse = 1;

	 nnext = 0;
	 while (1) {
	    fscanf(in, " %s", bf);
	    if (!strcmp(bf, "*"))
	       break;
	    seq_tab[n].next[nnext++] = atoi(bf);
	 }
	 seq_tab[n].nnext = nnext;

	 while (1) {
	    fscanf(in, " %s", bf);
	    if (!strcmp(bf, "*"))
	       break;
	    seq_images[last++] = fname2fnum(bf);
	 }
	 seq_images[last++] = eseq;
	 fscanf(in, " )");
      }
   }
}

#include "image.h"
#include "image_db.h"


int fname2fnum(char *fn)
{
   int i;
   for (i = 0; i < image_dir_len; i++)
#if use_suck_dir || win_bomb
      if (!strcmp(fn, image_names[i]))
#else
      if (!strcmp(fn, image_dir[i]->d_name))
#endif
	 return i;
   /* fprintf(stderr, "bad image name: %s\n", fn); */
   return 0;
}
