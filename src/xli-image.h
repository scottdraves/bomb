/* image.h: (renamed xli-image.h by spot)
 *
 * portable image type declarations
 *
 * jim frost 10.02.89
 *
 * Copyright 1989 Jim Frost.  See included file README for complete
 * copyright information.
 */

static char *xli_image_h_id = "@(#) $Id: xli-image.h,v 1.1.1.1 2002/12/08 20:49:54 spotspot Exp $";


#include <stdio.h>

/* ANSI-C stuff
 */
#if defined(__STDC__)

#if !defined(_ArgProto)
#define _ArgProto(ARGS) ARGS
#endif

#include <stdlib.h>

#else /* !__STDC__ */

#if !defined(const) /* "const" is an ANSI thing */
#define const
#endif
#if !defined(_ArgProto)
#define _ArgProto(ARGS) ()
#endif

#endif /* !__STDC__ */

/* handle strings stuff that varies between BSD and ANSI/SYSV
 */
#if defined(IS_BSD) && !defined(__STDC__)
#include <strings.h>
#if !defined(strchr) && !defined(index)
#define strchr index
#endif
#if !defined(strrchr) && !defined(rindex)
#define strrchr rindex
#endif
#if !defined(memcpy) && !defined(bcopy)
#define memcpy(D,S,L) bcopy((char *)(S),(char *)(D),(L))
#endif
#if !defined(memset) && !defined(bzero)
/* #define memset(D,V,L) bzero(D,L) */
#endif
#else /* !IS_BSD || __STDC__ */
#include <string.h>
#if !defined(index) && !defined(strchr)
#define index strchr
#endif
#if !defined(rindex) && !defined(strrchr)
#define rindex strrchr
#endif
#if !defined(bcopy) && !defined(memcpy)
#define bcopy(S,D,L) memcpy((void *)(D),(void *)(S),(L))
#endif
#if !defined(bzero) && !defined(memset)
#define bzero(D,L) memset((void *)(D),0,(L))
#endif
#endif /* !IS_BSD || __STDC__ */

#ifdef VMS
#define R_OK 4
#define NO_UNCOMPRESS
#endif


typedef unsigned short Intensity; /* what X thinks an RGB intensity is */
typedef unsigned char  byte;      /* byte type */

/* filter/extension pair for user-defined filters
 */
struct filter {
  char          *extension; /* extension to match */
  char          *filter;    /* filter to invoke */
  struct filter *next;
};

struct cache {
  int           len;
  char          buf[BUFSIZ];
  struct cache *next;
};

typedef struct {
  unsigned int  type;     /* ZIO file type */
  unsigned int  nocache;  /* true if caching has been disabled */
  FILE         *stream;   /* file input stream */
  char         *filename; /* filename */
  struct cache *data;     /* data cache */
  struct cache *dataptr;  /* ptr to current cache block */
  int           bufptr;   /* ptr within current cache block */
} ZFILE;

#define ZSTANDARD 0 /* standard file */
#define ZPIPE     1 /* file is a pipe (ie uncompress) */
#define ZSTDIN    2 /* file is stdin */

typedef struct rgbmap {
  unsigned int  size;       /* size of RGB map */
  unsigned int  used;       /* number of colors used in RGB map */
  unsigned int  compressed; /* image uses colormap fully */
  Intensity    *red;        /* color values in X style */
  Intensity    *green;
  Intensity    *blue;
} RGBMap;

/* image structure
 */

typedef struct {
  char         *title;  /* name of image */
  unsigned int  type;   /* type of image */
  RGBMap        rgb;    /* RGB map of image if IRGB type */
  unsigned int  width;  /* width of image in pixels */
  unsigned int  height; /* height of image in pixels */
  unsigned int  depth;  /* depth of image in bits if IRGB type */
  unsigned int  pixlen; /* length of pixel if IRGB type */
  float		gamma;	/* gamma of display the image is adjusted for */
  byte         *data;   /* data rounded to full byte for each row */
} xli_Image;

#define IBAD    0 /* invalid image (used when freeing) */
#define IBITMAP 1 /* image is a bitmap */
#define IRGB    2 /* image is RGB */
#define ITRUE   3 /* image is true color */

#define BITMAPP(IMAGE) ((IMAGE)->type == IBITMAP)
#define RGBP(IMAGE)    ((IMAGE)->type == IRGB)
#define TRUEP(IMAGE)   ((IMAGE)->type == ITRUE)

#define TRUE_RED(PIXVAL)   (((PIXVAL) & 0xff0000) >> 16)
#define TRUE_GREEN(PIXVAL) (((PIXVAL) & 0xff00) >> 8)
#define TRUE_BLUE(PIXVAL)  ((PIXVAL) & 0xff)
#define RGB_TO_TRUE(R,G,B) \
  (((unsigned int)((R) & 0xff00) << 8) | ((unsigned int)(G) & 0xff00) | \
   ((unsigned int)(B) >> 8))

#ifdef NO_INLINE
/* only inline 1-byte transfers.  this is provided for systems whose C
 * compilers can't hash the complexity of the inlined functions.
 */

#define memToVal(PTR,LEN)    ((LEN) == 1 ? (unsigned long)(*(PTR)) : \
			      doMemToVal(PTR,LEN))
#define memToValLSB(PTR,LEN) ((LEN) == 1 ? (unsigned long)(*(PTR)) : \
			      doMemToValLSB(PTR,LEN))
#define valToMem(VAL,PTR,LEN)    ((LEN) == 1 ? \
				  (unsigned long)(*(PTR) = (byte)(VAL)) : \
				  doValToMem(VAL,PTR,LEN))
#define valToMemLSB(VAL,PTR,LEN) ((LEN) == 1 ? \
				  (unsigned long)(*(PTR) = (byte)(VAL)) : \
				  (int)doValToMemLSB(VAL,PTR,LEN))

#else /* !NO_INLINE */
/* inline these functions for speed.  these only work for {len : 1,2,3,4}.
 */

#define memToVal(PTR,LEN) \
  ((LEN) == 1 ? ((unsigned long)(*((byte *)PTR))) : \
   ((LEN) == 3 ? ((unsigned long) \
		  (*(byte *)(PTR) << 16) | \
		  (*((byte *)(PTR) + 1) << 8) | \
		  (*((byte *)(PTR) + 2))) : \
    ((LEN) == 2 ? ((unsigned long) \
		   (*(byte *)(PTR) << 8) | \
		   (*((byte *)(PTR) + 1))) : \
     ((unsigned long)((*(byte *)(PTR) << 24) | \
		      (*((byte *)(PTR) + 1) << 16) | \
		      (*((byte *)(PTR) + 2) << 8) | \
		      (*((byte *)(PTR) + 3)))))))

#define memToValLSB(PTR,LEN) \
  ((LEN) == 1 ? ((unsigned long)(*(byte *)(PTR))) : \
   ((LEN) == 3 ? ((unsigned long) \
		  (*(byte *)(PTR)) | \
		  (*((byte *)(PTR) + 1) << 8) | \
		  (*((byte *)(PTR) + 2) << 16)) : \
    ((LEN) == 2 ? ((unsigned long) \
		   (*(byte *)(PTR)) | (*((byte *)(PTR) + 1) << 8)) : \
     ((unsigned long)((*(byte *)(PTR)) | \
		      (*((byte *)(PTR) + 1) << 8) | \
		      (*((byte *)(PTR) + 2) << 16) | \
		      (*((byte *)(PTR) + 3) << 24))))))

#define valToMem(VAL,PTR,LEN) \
  ((LEN) == 1 ? (*(byte *)(PTR) = ((unsigned int)(VAL) & 0xff)) : \
   ((LEN) == 3 ? (((*(byte *)(PTR)) = ((unsigned int)(VAL) & 0xff0000) >> 16), \
		  ((*((byte *)(PTR) + 1)) = ((unsigned int)(VAL) & 0xff00) >> 8), \
		  ((*((byte *)(PTR) + 2)) = ((unsigned int)(VAL) & 0xff))) : \
    ((LEN) == 2 ? (((*(byte *)(PTR)) = ((unsigned int)(VAL) & 0xff00) >> 8), \
		   ((*((byte *)(PTR) + 1)) = ((unsigned int)(VAL) & 0xff))) : \
     (((*(byte *)(PTR)) = ((unsigned int)(VAL) & 0xff000000) >> 24), \
      ((*((byte *)(PTR) + 1)) = ((unsigned int)(VAL) & 0xff0000) >> 16), \
      ((*((byte *)(PTR) + 2)) = ((unsigned int)(VAL) & 0xff00) >> 8), \
      ((*((byte *)(PTR) + 3)) = ((unsigned int)(VAL) & 0xff))))))

#define valToMemLSB(VAL,PTR,LEN) \
  ((LEN) == 1 ? (*(byte *)(PTR) = ((unsigned int)(VAL) & 0xff)) : \
   ((LEN) == 3 ? (((*(byte *)(PTR) + 2) = ((unsigned int)(VAL) & 0xff0000) >> 16), \
		  ((*((byte *)(PTR) + 1)) = ((unsigned int)(VAL) & 0xff00) >> 8), \
		  ((*(byte *)(PTR)) = ((unsigned int)(VAL) & 0xff))) : \
    ((LEN) == 2 ? (((*((byte *)(PTR) + 1) = ((unsigned int)(VAL) & 0xff00) >> 8), \
		    ((*(byte *)(PTR)) = ((unsigned int)(VAL) & 0xff)))) : \
     (((*((byte *)(PTR) + 3)) = ((unsigned int)(VAL) & 0xff000000) >> 24), \
      ((*((byte *)(PTR) + 2)) = ((unsigned int)(VAL) & 0xff0000) >> 16), \
      ((*((byte *)(PTR) + 1)) = ((unsigned int)(VAL) & 0xff00) >> 8), \
      ((*(byte *)(PTR)) = ((unsigned int)(VAL) & 0xff))))))
#endif /* !NO_INLINE */

/* zio.c */
ZFILE *zopen _ArgProto((char *name));
int    zread _ArgProto((ZFILE *zf, byte *buf, unsigned int len));
int    zgetc _ArgProto((ZFILE *zf));
char  *zgets _ArgProto((byte *buf, unsigned int size, ZFILE *zf));
void   zclose _ArgProto((ZFILE *zf));
void   znocache _ArgProto((ZFILE *zf));
void   zreset _ArgProto((char *filename));

#define lmalloc malloc
#define lfree free
#define dupString strdup

/* this returns the (approximate) intensity of an RGB triple
 */

#define colorIntensity(R,G,B) \
  (RedIntensity[(R) >> 8] + GreenIntensity[(G) >> 8] + BlueIntensity[(B) >> 8])

extern unsigned short RedIntensity[];
extern unsigned short GreenIntensity[];
extern unsigned short BlueIntensity[];

#ifdef DEBUG
extern int _Xdebug;
#define debug(ARGS) if (_Xdebug) printf ARGS
#else /* !DEBUG */
#define debug(ARGS)
#endif /* !DEBUG */
