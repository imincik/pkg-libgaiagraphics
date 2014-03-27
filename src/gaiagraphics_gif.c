/* 
/ gaiagraphics_gif.c
/
/ GIF auxiliary helpers
/
/ version 1.0, 2010 July 20
/
/ Author: Sandro Furieri a.furieri@lqt.it
/
/ Copyright (C) 2010  Alessandro Furieri
/
/    This program is free software: you can redistribute it and/or modify
/    it under the terms of the GNU Lesser General Public License as published by
/    the Free Software Foundation, either version 3 of the License, or
/    (at your option) any later version.
/
/    This program is distributed in the hope that it will be useful,
/    but WITHOUT ANY WARRANTY; without even the implied warranty of
/    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/    GNU Lesser General Public License for more details.
/
/    You should have received a copy of the GNU Lesser General Public License
/    along with this program.  If not, see <http://www.gnu.org/licenses/>.
/
*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

/* 
/
/ DISCLAIMER:
/ all the following code merely is an 'ad hoc' adaption
/ deriving from the original GD lib code
/ which in turn is based on he following:
/
** Code drawn from ppmtogif.c, from the pbmplus package
**
** Based on GIFENCOD by David Rowley <mgardi@watdscu.waterloo.edu>. A
** Lempel-Zim compression based on "compress".
**
** Modified by Marcel Wijkstra <wijkstra@fwi.uva.nl>
**
** Copyright (C) 1989 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
**
** The Graphics Interchange Format(c) is the Copyright property of
** CompuServe Incorporated.  GIF(sm) is a Service Mark property of
** CompuServe Incorporated.
/
*/

typedef int code_int;

#ifdef SIGNED_COMPARE_SLOW
typedef unsigned long int count_int;
typedef unsigned short int count_short;
#else /*SIGNED_COMPARE_SLOW */
typedef long int count_int;
#endif /*SIGNED_COMPARE_SLOW */

#define GIFBITS    12

#define maxbits GIFBITS

#define maxmaxcode ((code_int)1 << GIFBITS)

#define HSIZE  5003
#define hsize HSIZE

#ifdef COMPATIBLE
#define MAXCODE(n_bits)        ((code_int) 1 << (n_bits) - 1)
#else /*COMPATIBLE*/
#define MAXCODE(n_bits)        (((code_int) 1 << (n_bits)) - 1)
#endif /*COMPATIBLE*/
#define HashTabOf(i)       ctx->htab[i]
#define CodeTabOf(i)    ctx->codetab[i]
    typedef struct
{
    int Width, Height;
    int curx, cury;
    long CountDown;
    int Pass;
    int Interlace;
    int n_bits;
    code_int maxcode;
    count_int htab[HSIZE];
    unsigned short codetab[HSIZE];
    code_int free_ent;
    int clear_flg;
    int offset;
    long int in_count;
    long int out_count;
    int g_init_bits;
    xgdIOCtx *g_outfile;
    int ClearCode;
    int EOFCode;
    unsigned long cur_accum;
    int cur_bits;
    int a_count;
    char accum[256];
} GifCtx;

#define        MAXCOLORMAPSIZE         256

#define        TRUE    1
#define        FALSE   0

#define CM_RED         0
#define CM_GREEN       1
#define CM_BLUE                2

#define        MAX_LWZ_BITS            12

#define INTERLACE              0x40
#define LOCALCOLORMAP  0x80

#define BitSet(byte, bit)      (((byte) & (bit)) == (bit))
#define        ReadOK(file,buffer,len) (xgdGetBuf(buffer, len, file) > 0)
#define LM_to_uint(a,b)                        (((b)<<8)|(a))

#define STACK_SIZE ((1<<(MAX_LWZ_BITS))*2)

typedef struct
{
    unsigned char buf[280];
    int curbit, lastbit, done, last_byte;
} CODE_STATIC_DATA;

typedef struct
{
    int fresh;
    int code_size, set_code_size;
    int max_code, max_code_size;
    int firstcode, oldcode;
    int clear_code, end_code;
    int table[2][(1 << MAX_LWZ_BITS)];
    int stack[STACK_SIZE], *sp;
    CODE_STATIC_DATA scd;
} LZW_STATIC_DATA;

static int
GetDataBlock_ (xgdIOCtx * fd, unsigned char *buf, int *ZeroDataBlockP)
{
    unsigned char count;
    if (!ReadOK (fd, &count, 1))
      {
	  return -1;
      }
    *ZeroDataBlockP = count == 0;
    if ((count != 0) && (!ReadOK (fd, buf, count)))
      {
	  return -1;
      }
    return count;
}

static int
GetDataBlock (xgdIOCtx * fd, unsigned char *buf, int *ZeroDataBlockP)
{
    int rv;
    rv = GetDataBlock_ (fd, buf, ZeroDataBlockP);
    return (rv);
}

static int
ReadColorMap (xgdIOCtx * fd, int number, unsigned char (*buffer)[256])
{
    int i;
    unsigned char rgb[3];
    for (i = 0; i < number; ++i)
      {
	  if (!ReadOK (fd, rgb, sizeof (rgb)))
	    {
		return TRUE;
	    }
	  buffer[CM_RED][i] = rgb[0];
	  buffer[CM_GREEN][i] = rgb[1];
	  buffer[CM_BLUE][i] = rgb[2];
      }
    return FALSE;
}

static int
GetCode_ (xgdIOCtx * fd, CODE_STATIC_DATA * scd, int code_size, int flag,
	  int *ZeroDataBlockP)
{
    int i, j, ret;
    unsigned char count;
    if (flag)
      {
	  scd->curbit = 0;
	  scd->lastbit = 0;
	  scd->last_byte = 0;
	  scd->done = FALSE;
	  return 0;
      }
    if ((scd->curbit + code_size) >= scd->lastbit)
      {
	  if (scd->done)
	    {
		if (scd->curbit >= scd->lastbit)
		  {
		      /* Oh well */
		  }
		return -1;
	    }
	  scd->buf[0] = scd->buf[scd->last_byte - 2];
	  scd->buf[1] = scd->buf[scd->last_byte - 1];
	  if ((count = GetDataBlock (fd, &scd->buf[2], ZeroDataBlockP)) <= 0)
	      scd->done = TRUE;
	  scd->last_byte = 2 + count;
	  scd->curbit = (scd->curbit - scd->lastbit) + 16;
	  scd->lastbit = (2 + count) * 8;
      }
    ret = 0;
    for (i = scd->curbit, j = 0; j < code_size; ++i, ++j)
	ret |= ((scd->buf[i / 8] & (1 << (i % 8))) != 0) << j;
    scd->curbit += code_size;
    return ret;
}

static int
GetCode (xgdIOCtx * fd, CODE_STATIC_DATA * scd, int code_size, int flag,
	 int *ZeroDataBlockP)
{
    int rv;
    rv = GetCode_ (fd, scd, code_size, flag, ZeroDataBlockP);
    return (rv);
}

static int
LWZReadByte_ (xgdIOCtx * fd, LZW_STATIC_DATA * sd, char flag,
	      int input_code_size, int *ZeroDataBlockP)
{
    int code, incode, i;
    if (flag)
      {
	  sd->set_code_size = input_code_size;
	  sd->code_size = sd->set_code_size + 1;
	  sd->clear_code = 1 << sd->set_code_size;
	  sd->end_code = sd->clear_code + 1;
	  sd->max_code_size = 2 * sd->clear_code;
	  sd->max_code = sd->clear_code + 2;
	  GetCode (fd, &sd->scd, 0, TRUE, ZeroDataBlockP);
	  sd->fresh = TRUE;
	  for (i = 0; i < sd->clear_code; ++i)
	    {
		sd->table[0][i] = 0;
		sd->table[1][i] = i;
	    }
	  for (; i < (1 << MAX_LWZ_BITS); ++i)
	      sd->table[0][i] = sd->table[1][0] = 0;
	  sd->sp = sd->stack;
	  return 0;
      }
    else if (sd->fresh)
      {
	  sd->fresh = FALSE;
	  do
	    {
		sd->firstcode = sd->oldcode =
		    GetCode (fd, &sd->scd, sd->code_size, FALSE,
			     ZeroDataBlockP);
	    }
	  while (sd->firstcode == sd->clear_code);
	  return sd->firstcode;
      }
    if (sd->sp > sd->stack)
	return *--sd->sp;
    while ((code =
	    GetCode (fd, &sd->scd, sd->code_size, FALSE, ZeroDataBlockP)) >= 0)
      {
	  if (code == sd->clear_code)
	    {
		for (i = 0; i < sd->clear_code; ++i)
		  {
		      sd->table[0][i] = 0;
		      sd->table[1][i] = i;
		  }
		for (; i < (1 << MAX_LWZ_BITS); ++i)
		    sd->table[0][i] = sd->table[1][i] = 0;
		sd->code_size = sd->set_code_size + 1;
		sd->max_code_size = 2 * sd->clear_code;
		sd->max_code = sd->clear_code + 2;
		sd->sp = sd->stack;
		sd->firstcode = sd->oldcode =
		    GetCode (fd, &sd->scd, sd->code_size, FALSE,
			     ZeroDataBlockP);
		return sd->firstcode;
	    }
	  else if (code == sd->end_code)
	    {
		int count;
		unsigned char buf[260];
		if (*ZeroDataBlockP)
		    return -2;
		while ((count = GetDataBlock (fd, buf, ZeroDataBlockP)) > 0)
		    ;
		if (count != 0)
		    return -2;
	    }
	  incode = code;
	  if (sd->sp == (sd->stack + STACK_SIZE))
	    {
		return -1;
	    }
	  if (code >= sd->max_code)
	    {
		*sd->sp++ = sd->firstcode;
		code = sd->oldcode;
	    }
	  while (code >= sd->clear_code)
	    {
		if (sd->sp == (sd->stack + STACK_SIZE))
		  {
		      return -1;
		  }
		*sd->sp++ = sd->table[1][code];
		if (code == sd->table[0][code])
		  {
		      /* Oh well */
		  }
		code = sd->table[0][code];
	    }
	  *sd->sp++ = sd->firstcode = sd->table[1][code];
	  if ((code = sd->max_code) < (1 << MAX_LWZ_BITS))
	    {
		sd->table[0][code] = sd->oldcode;
		sd->table[1][code] = sd->firstcode;
		++sd->max_code;
		if ((sd->max_code >= sd->max_code_size) &&
		    (sd->max_code_size < (1 << MAX_LWZ_BITS)))
		  {
		      sd->max_code_size *= 2;
		      ++sd->code_size;
		  }
	    }
	  sd->oldcode = incode;
	  if (sd->sp > sd->stack)
	      return *--sd->sp;
      }
    return code;
}

static int
LWZReadByte (xgdIOCtx * fd, LZW_STATIC_DATA * sd, char flag,
	     int input_code_size, int *ZeroDataBlockP)
{
    int rv;
    rv = LWZReadByte_ (fd, sd, flag, input_code_size, ZeroDataBlockP);
    return (rv);
}

static void
ReadImage (gGraphImagePtr img, xgdIOCtx * fd,
	   unsigned char (*cmap)[256], int interlace, int *ZeroDataBlockP)
{
    unsigned char c;
    int v;
    int xpos = 0, ypos = 0, pass = 0;
    int i;
    int red[256];
    int green[256];
    int blue[256];
    unsigned char *p_out;
    int len = img->width;
    int height = img->height;
    LZW_STATIC_DATA sd;
    if (!ReadOK (fd, &c, 1))
      {
	  return;
      }
    if (c > MAX_LWZ_BITS)
      {
	  return;
      }
    for (i = 0; (i < 256); i++)
      {
	  red[i] = cmap[CM_RED][i];
	  green[i] = cmap[CM_GREEN][i];
	  blue[i] = cmap[CM_BLUE][i];
      }
    if (LWZReadByte (fd, &sd, TRUE, c, ZeroDataBlockP) < 0)
      {
	  return;
      }
    while ((v = LWZReadByte (fd, &sd, FALSE, c, ZeroDataBlockP)) >= 0)
      {
	  if (v >= 256)
	    {
		v = 0;
	    }
	  p_out =
	      img->pixels + (ypos * img->scanline_width) +
	      (xpos * img->pixel_size);
	  if (img->pixel_format == GG_PIXEL_PALETTE)
	    {
		/* the output image is expected to be PALETTE-based */
		*p_out++ = v;
		if ((v + 1) > img->max_palette)
		    img->max_palette = v + 1;
		img->palette_red[v] = red[v];
		img->palette_green[v] = green[v];
		img->palette_blue[v] = blue[v];
	    }
	  ++xpos;
	  if (xpos == len)
	    {
		xpos = 0;
		if (interlace)
		  {
		      switch (pass)
			{
			case 0:
			case 1:
			    ypos += 8;
			    break;
			case 2:
			    ypos += 4;
			    break;
			case 3:
			    ypos += 2;
			    break;
			}
		      if (ypos >= height)
			{
			    ++pass;
			    switch (pass)
			      {
			      case 1:
				  ypos = 4;
				  break;
			      case 2:
				  ypos = 2;
				  break;
			      case 3:
				  ypos = 1;
				  break;
			      default:
				  goto fini;
			      }
			}
		  }
		else
		  {
		      ++ypos;
		  }
	    }
	  if (ypos >= height)
	      break;
      }
  fini:
    if (LWZReadByte (fd, &sd, FALSE, c, ZeroDataBlockP) >= 0)
      {
	  /* Ignore extra */
      }
}

static int
DoExtension (xgdIOCtx * fd, int label, int *Transparent, int *ZeroDataBlockP)
{
    unsigned char buf[256];
    switch (label)
      {
      case 0xf9:
	  memset (buf, 0, 4);
	  (void) GetDataBlock (fd, (unsigned char *) buf, ZeroDataBlockP);
	  if ((buf[0] & 0x1) != 0)
	      *Transparent = buf[3];
	  while (GetDataBlock (fd, (unsigned char *) buf, ZeroDataBlockP) > 0);
	  return FALSE;
      default:
	  break;
      }
    while (GetDataBlock (fd, (unsigned char *) buf, ZeroDataBlockP) > 0)
	;
    return FALSE;
}

static void
BumpPixel (GifCtx * ctx)
{
    ++(ctx->curx);
    if (ctx->curx == ctx->Width)
      {
	  ctx->curx = 0;
	  if (!ctx->Interlace)
	      ++(ctx->cury);
	  else
	    {
		switch (ctx->Pass)
		  {
		  case 0:
		      ctx->cury += 8;
		      if (ctx->cury >= ctx->Height)
			{
			    ++(ctx->Pass);
			    ctx->cury = 4;
			}
		      break;
		  case 1:
		      ctx->cury += 8;
		      if (ctx->cury >= ctx->Height)
			{
			    ++(ctx->Pass);
			    ctx->cury = 2;
			}
		      break;
		  case 2:
		      ctx->cury += 4;
		      if (ctx->cury >= ctx->Height)
			{
			    ++(ctx->Pass);
			    ctx->cury = 1;
			}
		      break;

		  case 3:
		      ctx->cury += 2;
		      break;
		  }
	    }
      }
}

static int
GIFNextPixel (gGraphImagePtr img, GifCtx * ctx)
{
    int pixel = 0;
    unsigned char *p_in;
    if (ctx->CountDown == 0)
	return EOF;
    --(ctx->CountDown);

    p_in =
	img->pixels + (ctx->cury * img->scanline_width) +
	(ctx->curx * img->pixel_size);
    if (img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_PALETTE)
      {
	  /* the input image is expected to be GRAYSCALE or PALETTE anyway */
	  pixel = *p_in;
      }
    BumpPixel (ctx);
    return pixel;
}

static void
xgdPutC (const unsigned char c, xgdIOCtx * ctx)
{
    (ctx->putC) (ctx, c);
}

static void
char_init (GifCtx * ctx)
{
    ctx->a_count = 0;
}

static void
cl_hash (register count_int chsize, GifCtx * ctx)
{
    register count_int *htab_p = ctx->htab + chsize;
    register long i;
    register long m1 = -1;
    i = chsize - 16;
    do
      {
	  *(htab_p - 16) = m1;
	  *(htab_p - 15) = m1;
	  *(htab_p - 14) = m1;
	  *(htab_p - 13) = m1;
	  *(htab_p - 12) = m1;
	  *(htab_p - 11) = m1;
	  *(htab_p - 10) = m1;
	  *(htab_p - 9) = m1;
	  *(htab_p - 8) = m1;
	  *(htab_p - 7) = m1;
	  *(htab_p - 6) = m1;
	  *(htab_p - 5) = m1;
	  *(htab_p - 4) = m1;
	  *(htab_p - 3) = m1;
	  *(htab_p - 2) = m1;
	  *(htab_p - 1) = m1;
	  htab_p -= 16;
      }
    while ((i -= 16) >= 0);
    for (i += 16; i > 0; --i)
	*--htab_p = m1;
}

static void
flush_char (GifCtx * ctx)
{
    if (ctx->a_count > 0)
      {
	  xgdPutC (ctx->a_count, ctx->g_outfile);
	  xgdPutBuf (ctx->accum, ctx->a_count, ctx->g_outfile);
	  ctx->a_count = 0;
      }
}

static void
char_out (int c, GifCtx * ctx)
{
    ctx->accum[ctx->a_count++] = c;
    if (ctx->a_count >= 254)
	flush_char (ctx);
}

static unsigned long masks[] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
    0x001F, 0x003F, 0x007F, 0x00FF,
    0x01FF, 0x03FF, 0x07FF, 0x0FFF,
    0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};

static void
output (code_int code, GifCtx * ctx)
{
    ctx->cur_accum &= masks[ctx->cur_bits];
    if (ctx->cur_bits > 0)
	ctx->cur_accum |= ((long) code << ctx->cur_bits);
    else
	ctx->cur_accum = code;
    ctx->cur_bits += ctx->n_bits;
    while (ctx->cur_bits >= 8)
      {
	  char_out ((unsigned int) (ctx->cur_accum & 0xff), ctx);
	  ctx->cur_accum >>= 8;
	  ctx->cur_bits -= 8;
      }
    if (ctx->free_ent > ctx->maxcode || ctx->clear_flg)
      {

	  if (ctx->clear_flg)
	    {

		ctx->maxcode = MAXCODE (ctx->n_bits = ctx->g_init_bits);
		ctx->clear_flg = 0;

	    }
	  else
	    {

		++(ctx->n_bits);
		if (ctx->n_bits == maxbits)
		    ctx->maxcode = maxmaxcode;
		else
		    ctx->maxcode = MAXCODE (ctx->n_bits);
	    }
      }
    if (code == ctx->EOFCode)
      {
	  while (ctx->cur_bits > 0)
	    {
		char_out ((unsigned int) (ctx->cur_accum & 0xff), ctx);
		ctx->cur_accum >>= 8;
		ctx->cur_bits -= 8;
	    }
	  flush_char (ctx);
      }
}

static void
cl_block (GifCtx * ctx)
{
    cl_hash ((count_int) hsize, ctx);
    ctx->free_ent = ctx->ClearCode + 2;
    ctx->clear_flg = 1;
    output ((code_int) ctx->ClearCode, ctx);
}

static int
gifPutWord (int w, xgdIOCtx * out)
{
    xgdPutC (w & 0xFF, out);
    xgdPutC ((w >> 8) & 0xFF, out);
    return 0;
}

static void
GIFcompress (int init_bits, xgdIOCtxPtr outfile, gGraphImagePtr img,
	     GifCtx * ctx)
{
/*
 * compress stdin to stdout
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */
    register long fcode;
    register code_int i /* = 0 */ ;
    register int c;
    register code_int ent;
    register code_int disp;
    register code_int hsize_reg;
    register int hshift;
    ctx->g_init_bits = init_bits;
    ctx->g_outfile = outfile;
    ctx->offset = 0;
    ctx->out_count = 0;
    ctx->clear_flg = 0;
    ctx->in_count = 1;
    ctx->maxcode = MAXCODE (ctx->n_bits = ctx->g_init_bits);
    ctx->ClearCode = (1 << (init_bits - 1));
    ctx->EOFCode = ctx->ClearCode + 1;
    ctx->free_ent = ctx->ClearCode + 2;
    char_init (ctx);
    ent = GIFNextPixel (img, ctx);
    hshift = 0;
    for (fcode = (long) hsize; fcode < 65536L; fcode *= 2L)
	++hshift;
    hshift = 8 - hshift;
    hsize_reg = hsize;
    cl_hash ((count_int) hsize_reg, ctx);
    output ((code_int) ctx->ClearCode, ctx);
#ifdef SIGNED_COMPARE_SLOW
    while ((c = GIFNextPixel (img)) != (unsigned) EOF)
      {
#else /*SIGNED_COMPARE_SLOW */
    while ((c = GIFNextPixel (img, ctx)) != EOF)
      {				/* } */
#endif /*SIGNED_COMPARE_SLOW */
	  ++(ctx->in_count);
	  fcode = (long) (((long) c << maxbits) + ent);
	  i = (((code_int) c << hshift) ^ ent);
	  if (HashTabOf (i) == fcode)
	    {
		ent = CodeTabOf (i);
		continue;
	    }
	  else if ((long) HashTabOf (i) < 0)
	      goto nomatch;
	  disp = hsize_reg - i;
	  if (i == 0)
	      disp = 1;
	probe:
	  if ((i -= disp) < 0)
	      i += hsize_reg;
	  if (HashTabOf (i) == fcode)
	    {
		ent = CodeTabOf (i);
		continue;
	    }
	  if ((long) HashTabOf (i) > 0)
	      goto probe;
	nomatch:
	  output ((code_int) ent, ctx);
	  ++(ctx->out_count);
	  ent = c;
#ifdef SIGNED_COMPARE_SLOW
	  if ((unsigned) ctx->free_ent < (unsigned) maxmaxcode)
	    {
#else /*SIGNED_COMPARE_SLOW */
	  if (ctx->free_ent < maxmaxcode)
	    {
#endif /*SIGNED_COMPARE_SLOW */
		CodeTabOf (i) = ctx->free_ent++;
		HashTabOf (i) = fcode;
	    }
	  else
	      cl_block (ctx);
      }
    output ((code_int) ent, ctx);
    ++(ctx->out_count);
    output ((code_int) ctx->EOFCode, ctx);
}

static int
GIFEncode (xgdIOCtxPtr fp, int GInterlace,
	   int Background, int Transparent, int BitsPerPixel, int *Red,
	   int *Green, int *Blue, gGraphImagePtr img)
{
    int B;
    int RWidth, RHeight;
    int LeftOfs, TopOfs;
    int Resolution;
    int ColorMapSize;
    int InitCodeSize;
    int i;
    int GWidth = img->width;
    int GHeight = img->height;
    GifCtx ctx;
    ctx.Interlace = GInterlace;
    ctx.in_count = 1;
    memset (&ctx, 0, sizeof (ctx));
    ColorMapSize = 1 << BitsPerPixel;
    RWidth = ctx.Width = GWidth;
    RHeight = ctx.Height = GHeight;
    LeftOfs = TopOfs = 0;
    Resolution = BitsPerPixel;
    ctx.CountDown = (long) ctx.Width * (long) ctx.Height;
    ctx.Pass = 0;
    if (BitsPerPixel <= 1)
	InitCodeSize = 2;
    else
	InitCodeSize = BitsPerPixel;
    ctx.curx = ctx.cury = 0;
    xgdPutBuf (Transparent < 0 ? "GIF87a" : "GIF89a", 6, fp);
    gifPutWord (RWidth, fp);
    gifPutWord (RHeight, fp);
    B = 0x80;
    B |= (Resolution - 1) << 5;
    B |= (BitsPerPixel - 1);
    xgdPutC (B, fp);
    xgdPutC (Background, fp);
    xgdPutC (0, fp);
    for (i = 0; i < ColorMapSize; ++i)
      {
	  xgdPutC (Red[i], fp);
	  xgdPutC (Green[i], fp);
	  xgdPutC (Blue[i], fp);
      }
    if (Transparent >= 0)
      {
	  xgdPutC ('!', fp);
	  xgdPutC (0xf9, fp);
	  xgdPutC (4, fp);
	  xgdPutC (1, fp);
	  xgdPutC (0, fp);
	  xgdPutC (0, fp);
	  xgdPutC ((unsigned char) Transparent, fp);
	  xgdPutC (0, fp);
      }
    xgdPutC (',', fp);
    gifPutWord (LeftOfs, fp);
    gifPutWord (TopOfs, fp);
    gifPutWord (ctx.Width, fp);
    gifPutWord (ctx.Height, fp);
    if (ctx.Interlace)
	xgdPutC (0x40, fp);
    else
	xgdPutC (0x00, fp);
    xgdPutC (InitCodeSize, fp);
    GIFcompress (InitCodeSize + 1, fp, img, &ctx);
    xgdPutC (0, fp);
    xgdPutC (';', fp);
    return GGRAPH_OK;
}

static int
colorstobpp (int colors)
{
    int bpp = 0;

    if (colors <= 2)
	bpp = 1;
    else if (colors <= 4)
	bpp = 2;
    else if (colors <= 8)
	bpp = 3;
    else if (colors <= 16)
	bpp = 4;
    else if (colors <= 32)
	bpp = 5;
    else if (colors <= 64)
	bpp = 6;
    else if (colors <= 128)
	bpp = 7;
    else if (colors <= 256)
	bpp = 8;
    return bpp;
}

static gGraphImageInfosPtr
xgdImageInfosFromGifCtx (xgdIOCtxPtr fd, int *errcode)
{
    int BitPixel;
    unsigned char buf[16];
    int imw, imh, screen_width, screen_height;
    int bitPixel;
    gGraphImageInfosPtr infos = NULL;
    if (!ReadOK (fd, buf, 6))
      {
	  *errcode = GGRAPH_GIF_CODEC_ERROR;
	  return NULL;
      }
    if (strncmp ((char *) buf, "GIF", 3) != 0)
      {
	  *errcode = GGRAPH_GIF_CODEC_ERROR;
	  return NULL;
      }
    if (memcmp ((char *) buf + 3, "87a", 3) == 0
	|| memcmp ((char *) buf + 3, "89a", 3) == 0)
	;
    else
      {
	  *errcode = GGRAPH_GIF_CODEC_ERROR;
	  return NULL;
      }
    if (!ReadOK (fd, buf, 7))
      {
	  *errcode = GGRAPH_GIF_CODEC_ERROR;
	  return NULL;
      }
    BitPixel = 2 << (buf[4] & 0x07);
    screen_width = imw = LM_to_uint (buf[0], buf[1]);
    screen_height = imh = LM_to_uint (buf[2], buf[3]);
    bitPixel = colorstobpp (BitPixel);
    if (!
	(infos =
	 gg_image_infos_create (GG_PIXEL_PALETTE, screen_width, screen_height,
				bitPixel, 1, GGRAPH_SAMPLE_UINT, NULL, NULL)))
      {
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  return NULL;
      }
    infos->compression = GGRAPH_TIFF_COMPRESSION_LZW;
    if (!infos)
      {
	  *errcode = GGRAPH_GIF_CODEC_ERROR;
	  return NULL;
      }
    return infos;
}

static gGraphImagePtr
xgdImageCreateFromGifCtx (xgdIOCtxPtr fd, int *errcode)
{
    int BitPixel;
    int Transparent = (-1);
    unsigned char buf[16];
    unsigned char c;
    unsigned char ColorMap[3][MAXCOLORMAPSIZE];
    unsigned char localColorMap[3][MAXCOLORMAPSIZE];
    int imw, imh, screen_width, screen_height;
    int useGlobalColormap;
    int bitPixel;
    int ZeroDataBlock = FALSE;
    int haveGlobalColormap;
    gGraphImagePtr img = NULL;
    if (!ReadOK (fd, buf, 6))
      {
	  *errcode = GGRAPH_GIF_CODEC_ERROR;
	  return NULL;
      }
    if (strncmp ((char *) buf, "GIF", 3) != 0)
      {
	  *errcode = GGRAPH_GIF_CODEC_ERROR;
	  return NULL;
      }
    if (memcmp ((char *) buf + 3, "87a", 3) == 0
	|| memcmp ((char *) buf + 3, "89a", 3) == 0)
	;
    else
      {
	  *errcode = GGRAPH_GIF_CODEC_ERROR;
	  return NULL;
      }
    if (!ReadOK (fd, buf, 7))
      {
	  *errcode = GGRAPH_GIF_CODEC_ERROR;
	  return NULL;
      }
    BitPixel = 2 << (buf[4] & 0x07);
    screen_width = imw = LM_to_uint (buf[0], buf[1]);
    screen_height = imh = LM_to_uint (buf[2], buf[3]);
    haveGlobalColormap = BitSet (buf[4], LOCALCOLORMAP);	/* Global Colormap */
    if (haveGlobalColormap)
      {
	  if (ReadColorMap (fd, BitPixel, ColorMap))
	    {
		*errcode = GGRAPH_GIF_CODEC_ERROR;
		return NULL;
	    }
      }
    for (;;)
      {
	  int top, left;
	  int width, height;
	  if (!ReadOK (fd, &c, 1))
	    {
		*errcode = GGRAPH_GIF_CODEC_ERROR;
		return NULL;
	    }
	  if (c == ';')
	    {
		goto terminated;
	    }

	  if (c == '!')
	    {
		if (!ReadOK (fd, &c, 1))
		  {
		      *errcode = GGRAPH_GIF_CODEC_ERROR;
		      return NULL;
		  }
		DoExtension (fd, c, &Transparent, &ZeroDataBlock);
		continue;
	    }

	  if (c != ',')
	    {
		continue;
	    }
	  if (!ReadOK (fd, buf, 9))
	    {
		*errcode = GGRAPH_GIF_CODEC_ERROR;
		return NULL;
	    }
	  useGlobalColormap = !BitSet (buf[8], LOCALCOLORMAP);
	  bitPixel = 1 << ((buf[8] & 0x07) + 1);
	  left = LM_to_uint (buf[0], buf[1]);
	  top = LM_to_uint (buf[2], buf[3]);
	  width = LM_to_uint (buf[4], buf[5]);
	  height = LM_to_uint (buf[6], buf[7]);
	  if (left + width > screen_width || top + height > screen_height)
	    {
		*errcode = GGRAPH_GIF_CODEC_ERROR;
		return NULL;
	    }
	  if (!
	      (img =
	       gg_image_create (GG_PIXEL_PALETTE, width, height, bitPixel, 1,
				GGRAPH_SAMPLE_UINT, NULL, NULL)))
	    {
		*errcode = GGRAPH_INSUFFICIENT_MEMORY;
		return NULL;
	    }
	  if (!useGlobalColormap)
	    {
		if (ReadColorMap (fd, bitPixel, localColorMap))
		  {
		      gg_image_destroy (img);
		      *errcode = GGRAPH_GIF_CODEC_ERROR;
		      return NULL;
		  }
		ReadImage (img, fd, localColorMap,
			   BitSet (buf[8], INTERLACE), &ZeroDataBlock);
	    }
	  else
	    {
		if (!haveGlobalColormap)
		  {
		      gg_image_destroy (img);
		      *errcode = GGRAPH_GIF_CODEC_ERROR;
		      return NULL;
		  }
		ReadImage (img, fd,
			   ColorMap,
			   BitSet (buf[8], INTERLACE), &ZeroDataBlock);
	    }
      }
  terminated:
    if (!img)
      {
	  *errcode = GGRAPH_GIF_CODEC_ERROR;
	  return NULL;
      }
    return img;
}

static int
xgdImageGifCtx (gGraphImagePtr img, xgdIOCtxPtr out, int is_transparent)
{
    int BitsPerPixel;
    int Red[256];
    int Green[256];
    int Blue[256];
    int i, colors;
    int transparent_idx = -1;
    if (img->pixel_format == GG_PIXEL_GRAYSCALE)
      {
	  /* generating a GRAYSCALE palette */
	  colors = 256;
	  for (i = 0; i < 256; i++)
	    {
		Red[i] = i;
		Green[i] = i;
		Blue[i] = i;
	    }
      }
    else
      {
	  /* copying the PALETTE from image */
	  colors = 0;
	  for (i = 0; i < img->max_palette; ++i)
	    {
		Red[i] = img->palette_red[i];
		Green[i] = img->palette_green[i];
		Blue[i] = img->palette_blue[i];
		colors++;
	    }
      }
    BitsPerPixel = colorstobpp (colors);
    if (is_transparent)
      {
	  /* identifying the Transparent Color Index */
	  for (i = 0; i < colors; ++i)
	    {
		if (Red[i] == img->transparent_red
		    && Green[i] == img->transparent_green
		    && Blue[i] == img->transparent_blue)
		    transparent_idx = i;
	    }
      }
    return GIFEncode (out, 0, 0, transparent_idx, BitsPerPixel, Red, Green,
		      Blue, img);
}

GGRAPH_PRIVATE int
gg_image_to_gif (const gGraphImagePtr img, void **mem_buf, int *mem_buf_size,
		 FILE * file, int dest_type, int is_transparent)
{
/* compressing an image as GIF */
    int ret;
    void *rv;
    int size;
    xgdIOCtx *out;

/* checkings args for validity */
    if (dest_type == GG_TARGET_IS_FILE)
      {
	  if (!file)
	      return GGRAPH_ERROR;
      }
    else
      {
	  if (!mem_buf || !mem_buf_size)
	      return GGRAPH_ERROR;
	  *mem_buf = NULL;
	  *mem_buf_size = 0;
      }

    if (dest_type == GG_TARGET_IS_FILE)
	out = xgdNewDynamicCtx (0, file, dest_type);
    else
	out = xgdNewDynamicCtx (2048, NULL, dest_type);
    ret = xgdImageGifCtx (img, out, is_transparent);
    if (dest_type == GG_TARGET_IS_FILE)
      {
	  out->xgd_free (out);
	  return ret;
      }

    if (ret == GGRAPH_OK)
	rv = xgdDPExtractData (out, &size);
    out->xgd_free (out);
    *mem_buf = rv;
    *mem_buf_size = size;
    return ret;
}

GGRAPH_PRIVATE int
gg_image_from_gif (int size, const void *data, int source_type,
		   gGraphImagePtr * image_handle)
{
/* uncompressing a GIF */
    int errcode = GGRAPH_OK;
    gGraphImagePtr img;
    xgdIOCtx *in =
	xgdNewDynamicCtxEx (size, data, XGD_CTX_DONT_FREE, source_type);
    img = xgdImageCreateFromGifCtx (in, &errcode);
    in->xgd_free (in);
    *image_handle = img;
    return errcode;
}

GGRAPH_PRIVATE int
gg_image_infos_from_gif (int size, const void *data, int source_type,
			 gGraphImageInfosPtr * infos_handle)
{
/* image infos from GIF */
    int errcode = GGRAPH_OK;
    gGraphImageInfosPtr infos;
    xgdIOCtx *in =
	xgdNewDynamicCtxEx (size, data, XGD_CTX_DONT_FREE, source_type);
    infos = xgdImageInfosFromGifCtx (in, &errcode);
    in->xgd_free (in);
    *infos_handle = infos;
    return errcode;
}
