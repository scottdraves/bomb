/* zio.c:
 *
 * this properly opens and reads from an image file, compressed or otherwise.
 *
 * jim frost 10.03.89
 *
 * this was hacked on 09.12.90 to cache reads and to use stdin.
 *
 * Copyright 1989, 1990 Jim Frost.  See included file "copyright.h" for
 * complete copyright information.
 */

static char *zio_c_id = "@(#) $Id: zio.c,v 1.1.1.1 2002/12/08 20:49:54 spotspot Exp $";

#include "xli-image.h"
#include "defs.h"

#if defined(VMS) || mac_bomb || win_bomb
#define NO_PIPES /* VMS doesn't have pipes */
#endif

#ifndef NO_PIPES
extern struct filter *Filters;
#endif

#define MAX_ZFILES 32

static ZFILE ZFileTable[MAX_ZFILES];

#if mac_bomb
char *strdup(char *s)
{
  char *r = malloc(strlen(s) + 1);
  strcpy(r, s);
  return r;
}
#endif

/* read some info through the read cache
 */

static int doRead(zf, buf, len)
     ZFILE *zf;
     byte *buf;
     int len;
{ int bread, readlen;

  /* loop through the read
   */

  for (bread= 0; bread < len; bread += readlen, zf->bufptr += readlen) {

    /* read new or move to next data block if necessary
     */

    if (!zf->dataptr || (zf->bufptr == zf->dataptr->len)) {
      if (zf->dataptr && (zf->dataptr->len == 0)) /* EOF */
	return(bread);
      if (zf->dataptr && zf->dataptr->next) {
	zf->dataptr= zf->dataptr->next;
      }
      else {
	if (!zf->dataptr) {

	  /* if the reader told us to stop caching, just do the read.
	   */
	  if (zf->nocache)
	    return(fread(zf->dataptr->buf, 1, BUFSIZ, zf->stream));
	  zf->data= zf->dataptr= (struct cache *)lmalloc(sizeof(struct cache));
	}
	else {
	  zf->dataptr->next= (struct cache *)lmalloc(sizeof(struct cache));
	  zf->dataptr= zf->dataptr->next;
	}
	zf->dataptr->next= NULL;
	zf->dataptr->len= fread(zf->dataptr->buf, 1, BUFSIZ, zf->stream);
	if (zf->dataptr->len < 0) {
	  perror("fread");
	  exit(1);
	}
      }
      zf->bufptr= 0;
    }

    /* calculate length we can get out of read buffer
     */

    readlen= (len - bread > zf->dataptr->len - zf->bufptr ?
	      zf->dataptr->len - zf->bufptr : len - bread);
    if (!readlen) /* we're at EOF */
      return(bread);
    bcopy(zf->dataptr->buf + zf->bufptr, buf + bread, readlen);
  }
  return(bread);
}

/* reset a read cache
 */

void zreset(filename)
     char *filename;
{ int a;
  struct cache *old;

  /* if NULL filename, reset the entire table
   */

  if (!filename) {
    for (a= 0; a < MAX_ZFILES; a++)
      if (ZFileTable[a].filename)
	zreset(ZFileTable[a].filename);
    return;
  }

  for (a= 0; a < MAX_ZFILES; a++)
    if (ZFileTable[a].filename && !strcmp(filename, ZFileTable[a].filename))
      break;

  if (a == MAX_ZFILES) /* no go joe */
    return;

  if (ZFileTable[a].dataptr != ZFileTable[a].data)
    fprintf(stderr, "zreset: warning: ZFILE for %s was not closed properly\n",
	   ZFileTable[a].filename);
  while (ZFileTable[a].data) {
    old= ZFileTable[a].data;
    ZFileTable[a].data= ZFileTable[a].data->next;
    free(old);
  }
  lfree((byte *)ZFileTable[a].filename);
  ZFileTable[a].filename= NULL;
  ZFileTable[a].dataptr= NULL;
  ZFileTable[a].bufptr= 0;
  ZFileTable[a].nocache= 0;

  switch(ZFileTable[a].type) {
  case ZSTANDARD:
    fclose(ZFileTable[a].stream);
    break;
#ifndef NO_PIPES
  case ZPIPE:
    pclose(ZFileTable[a].stream);
    break;
#endif /* NO_PIPES */
  case ZSTDIN:
    break;
  default:
    fprintf(stderr, "zreset: bad ZFILE structure\n");
    exit(1);
  }
}

ZFILE *zopen(name)
     char *name;
{ int    a;
  ZFILE *zf;

  debug(("zopen(\"%s\") called\n", name));

  /* look for filename in open file table
   */

#if 0
  for (a= 0; a < MAX_ZFILES; a++)
    if (ZFileTable[a].filename && !strcmp(name, ZFileTable[a].filename)) {

      /* if we try to reopen a file whose caching was disabled, warn the user
       * and try to recover.  we cannot recover if it was stdin.
       */

      if (ZFileTable[a].nocache) {
	if (ZFileTable[a].type == ZSTDIN) {
	  fprintf(stderr, "zopen: caching was disabled by previous caller; can't reopen stdin\n");
	  return(NULL);
	}
	fprintf(stderr, "zopen: warning: caching was disabled by previous caller\n");
	zreset(ZFileTable[a].filename);
	break;
      }
      if (ZFileTable[a].dataptr != ZFileTable[a].data)
	fprintf(stderr, "zopen: warning: file doubly opened\n");
      ZFileTable[a].dataptr= ZFileTable[a].data;
      ZFileTable[a].bufptr= 0;
      return(ZFileTable + a);
    }
#endif

  /* find open ZFileTable entry
   */

  for (a= 0; (a < MAX_ZFILES) && ZFileTable[a].filename; a++)
    /* EMPTY */
    ;

  if (a == MAX_ZFILES) {
    fprintf(stderr, "zopen: no more files available\n");
    exit(1);
  }
  zf= ZFileTable + a;

  zf->filename= dupString(name);
  zf->dataptr= NULL;
  zf->bufptr= 0;
  zf->nocache= 0;

  /* file filename is `stdin' then use stdin
   */

  if (!strcmp(name, "stdin")) {
    zf->type= ZSTDIN;
    zf->stream= stdin;
    return(zf);
  }

#if 0
  /* check suffix against the table of filters and invoke the filter
   * if necessary.  if your system doesn't support pipes you can
   * define NO_PIPES and this will not happen.
   */

  for (filter= Filters; filter; filter= filter->next) {
    if ((strlen(name) > strlen(filter->extension)) &&
	!strcmp(filter->extension,
		name + (strlen(name) - strlen(filter->extension)))) {
      debug(("Filtering image through '%s'\n", filter->filter));
      zf->type= ZPIPE;
      sprintf(buf, "%s %s", filter->filter, name);
      if (! (zf->stream= popen(buf, "r"))) {
	lfree((byte *)zf->filename);
	zf->filename= NULL;
	return(NULL);
      }
      return(zf);
    }
  }
#endif /* !NO_PIPES */

  /* default to normal stream
   */

  zf->type= ZSTANDARD;
  if (!
#if 1
      (zf->stream= fopen(name, "rb"))
#else
      (zf->stream= fopen(name, "r"))
#endif
      ) {
    lfree((byte *)zf->filename);
    zf->filename= NULL;
    return(NULL);
  }
  return(zf);
}

int zread(zf, buf, len)
     ZFILE        *zf;
     byte         *buf;
     unsigned int  len;
{
  return(doRead(zf, buf, len));
}

int zgetc(zf)
     ZFILE *zf;
{ unsigned char c;

  if (doRead(zf, &c, 1) > 0)
    return(c);
  else
    return(EOF);
}

char *zgets(buf, size, zf)
     byte         *buf;
     unsigned int  size;
     ZFILE        *zf;
{ int p= 0;

  while (doRead(zf, buf + p, 1) > 0) {
    if (p == size)
      return((char *)buf);
    if (*(buf + p) == '\n') {
      *(buf + p + 1)= '\0';
      return((char *)buf);
    }
    p++;
  }
  return(NULL);
}

/* this turns off caching when an image has been identified and we will not
 * need to re-open it
 */

void znocache(zf)
     ZFILE *zf;
{
  debug(("znocache(\"%s\") called\n", zf->filename));
  zf->nocache= 1;
}

/* reset cache pointers in a ZFILE.  nothing is actually reset until a
 * zreset() is called with the filename.
 */

void zclose(zf)
     ZFILE *zf;
{
  debug(("zclose(\"%s\") called\n", zf->filename));
  zf->dataptr= zf->data;
  zf->bufptr= 0;
}
