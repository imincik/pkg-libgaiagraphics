/* 
/ gaiagraphics_io.c
/
/ IO helper methods
/
/ version 1.0, 2010 July 20
/
/ Author: Sandro Furieri a.furieri@lqt.it
/
/ Copyright (C) 2009  Alessandro Furieri
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
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <tiffio.h>

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

#define TRUE 1
#define FALSE 0

GGRAPH_DECLARE void
gGraphSmartPrintf (double value, char *buf)
{
/* well formatting a decimal number */
    int i;
    sprintf (buf, "%1.18f", value);
    for (i = strlen (buf) - 1; i >= 0; i--)
      {
	  if (buf[i] == '0')
	      buf[i] = '\0';
	  else
	      break;
      }
    i = strlen (buf) - 1;
    if (buf[i] == '.')
	buf[i] = '\0';
}

GGRAPH_PRIVATE int
gg_endian_arch ()
{
/* checking if target CPU is a little-endian one */
    union cvt
    {
	unsigned char byte[4];
	int int_value;
    } convert;
    convert.int_value = 1;
    if (convert.byte[0] == 0)
	return 0;
    return 1;
}

GGRAPH_PRIVATE short
gg_import_int16 (const unsigned char *p, int little_endian,
		 int little_endian_arch)
{
/* fetches a signed 16bit int from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[2];
	short short_value;
    } convert;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 1);
		convert.byte[1] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 1);
		convert.byte[1] = *(p + 0);
	    }
      }
    return convert.short_value;
}

GGRAPH_PRIVATE unsigned short
gg_import_uint16 (const unsigned char *p, int little_endian,
		  int little_endian_arch)
{
/* fetches an unsigned 16bit int from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[2];
	unsigned short short_value;
    } convert;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 1);
		convert.byte[1] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 1);
		convert.byte[1] = *(p + 0);
	    }
      }
    return convert.short_value;
}

GGRAPH_PRIVATE int
gg_import_int32 (const unsigned char *p, int little_endian,
		 int little_endian_arch)
{
/* fetches a signed 32bit int from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[4];
	int int_value;
    } convert;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
      }
    return convert.int_value;
}

GGRAPH_PRIVATE unsigned int
gg_import_uint32 (const unsigned char *p, int little_endian,
		  int little_endian_arch)
{
/* fetches an unsigned 32bit int from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[4];
	unsigned int int_value;
    } convert;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
      }
    return convert.int_value;
}

GGRAPH_PRIVATE float
gg_import_float (const unsigned char *p, int little_endian,
		 int little_endian_arch)
{
/* fetches a 32bit float from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[4];
	float flt_value;
    } convert;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
      }
    return convert.flt_value;
}

GGRAPH_PRIVATE double
gg_import_double (const unsigned char *p, int little_endian,
		  int little_endian_arch)
{
/* fetches a 64bit double from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[8];
	double double_value;
    } convert;
    if (little_endian_arch)
      {
/* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 7);
		convert.byte[1] = *(p + 6);
		convert.byte[2] = *(p + 5);
		convert.byte[3] = *(p + 4);
		convert.byte[4] = *(p + 3);
		convert.byte[5] = *(p + 2);
		convert.byte[6] = *(p + 1);
		convert.byte[7] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
		convert.byte[4] = *(p + 4);
		convert.byte[5] = *(p + 5);
		convert.byte[6] = *(p + 6);
		convert.byte[7] = *(p + 7);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
		convert.byte[4] = *(p + 4);
		convert.byte[5] = *(p + 5);
		convert.byte[6] = *(p + 6);
		convert.byte[7] = *(p + 7);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 7);
		convert.byte[1] = *(p + 6);
		convert.byte[2] = *(p + 5);
		convert.byte[3] = *(p + 4);
		convert.byte[4] = *(p + 3);
		convert.byte[5] = *(p + 2);
		convert.byte[6] = *(p + 1);
		convert.byte[7] = *(p + 0);
	    }
      }
    return convert.double_value;
}

GGRAPH_PRIVATE void
gg_export_int16 (short value, unsigned char *p, int little_endian,
		 int little_endian_arch)
{
/* stores a signed 16bit int into a BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[2];
	short short_value;
    } convert;
    convert.short_value = value;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 1) = convert.byte[0];
		*(p + 0) = convert.byte[1];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 1) = convert.byte[0];
		*(p + 0) = convert.byte[1];
	    }
      }
}

GGRAPH_PRIVATE void
gg_export_uint16 (unsigned short value, unsigned char *p, int little_endian,
		  int little_endian_arch)
{
/* stores an unsigned 16bit int into a BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[2];
	unsigned short ushort_value;
    } convert;
    convert.ushort_value = value;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 1) = convert.byte[0];
		*(p + 0) = convert.byte[1];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 1) = convert.byte[0];
		*(p + 0) = convert.byte[1];
	    }
      }
}

GGRAPH_PRIVATE void
gg_export_int32 (int value, unsigned char *p, int little_endian,
		 int little_endian_arch)
{
/* stores a signed 32bit int into a BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[4];
	int int_value;
    } convert;
    convert.int_value = value;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 3) = convert.byte[0];
		*(p + 2) = convert.byte[1];
		*(p + 1) = convert.byte[2];
		*(p + 0) = convert.byte[3];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 3) = convert.byte[0];
		*(p + 2) = convert.byte[1];
		*(p + 1) = convert.byte[2];
		*(p + 0) = convert.byte[3];
	    }
      }
}

GGRAPH_PRIVATE void
gg_export_uint32 (unsigned int value, unsigned char *p, int little_endian,
		  int little_endian_arch)
{
/* stores an unsigned 32bit int into a BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[4];
	unsigned int uint_value;
    } convert;
    convert.uint_value = value;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 3) = convert.byte[0];
		*(p + 2) = convert.byte[1];
		*(p + 1) = convert.byte[2];
		*(p + 0) = convert.byte[3];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 3) = convert.byte[0];
		*(p + 2) = convert.byte[1];
		*(p + 1) = convert.byte[2];
		*(p + 0) = convert.byte[3];
	    }
      }
}

GGRAPH_PRIVATE void
gg_export_float (float value, unsigned char *p, int little_endian,
		 int little_endian_arch)
{
/* stores a Float into a BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[4];
	float flt_value;
    } convert;
    convert.flt_value = value;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 3) = convert.byte[0];
		*(p + 2) = convert.byte[1];
		*(p + 1) = convert.byte[2];
		*(p + 0) = convert.byte[3];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 3) = convert.byte[0];
		*(p + 2) = convert.byte[1];
		*(p + 1) = convert.byte[2];
		*(p + 0) = convert.byte[3];
	    }
      }
}

GGRAPH_PRIVATE void
gg_export_double (double value, unsigned char *p, int little_endian,
		  int little_endian_arch)
{
/* stores a Double into a BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[8];
	double dbl_value;
    } convert;
    convert.dbl_value = value;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 7) = convert.byte[0];
		*(p + 6) = convert.byte[1];
		*(p + 5) = convert.byte[2];
		*(p + 4) = convert.byte[3];
		*(p + 3) = convert.byte[4];
		*(p + 2) = convert.byte[5];
		*(p + 1) = convert.byte[6];
		*(p + 0) = convert.byte[7];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
		*(p + 4) = convert.byte[4];
		*(p + 5) = convert.byte[5];
		*(p + 6) = convert.byte[6];
		*(p + 7) = convert.byte[7];
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
		*(p + 4) = convert.byte[4];
		*(p + 5) = convert.byte[5];
		*(p + 6) = convert.byte[6];
		*(p + 7) = convert.byte[7];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 7) = convert.byte[0];
		*(p + 6) = convert.byte[1];
		*(p + 5) = convert.byte[2];
		*(p + 4) = convert.byte[3];
		*(p + 3) = convert.byte[4];
		*(p + 2) = convert.byte[5];
		*(p + 1) = convert.byte[6];
		*(p + 0) = convert.byte[7];
	    }
      }
}

/* 
/
/ DISCLAIMER:
/ all the following code merely is an 'ad hoc' adaption
/ deriving from the original GD lib code
/
*/

static int
xgdReallocMemory (dynamicPtr * dp, int required)
{
    void *newPtr;
    if ((newPtr = realloc (dp->data, required)))
      {
	  dp->realSize = required;
	  dp->data = newPtr;
	  return TRUE;
      }
    newPtr = malloc (required);
    if (!newPtr)
      {
	  dp->dataGood = FALSE;
	  return FALSE;
      }
    memcpy (newPtr, dp->data, dp->logicalSize);
    free (dp->data);
    dp->data = newPtr;
    dp->realSize = required;
    return TRUE;
}

static void
xgdFreeMemoryCtx (struct xgdIOCtx *ctx)
{
    dynamicPtr *dp;
    dpIOCtx *dctx;
    dctx = (dpIOCtx *) ctx;
    dp = dctx->dp;
    free (ctx);
    if ((dp->data != NULL) && (dp->freeOK))
      {
	  free (dp->data);
	  dp->data = NULL;
      }
    dp->realSize = 0;
    dp->logicalSize = 0;
    free (dp);
}

static void
xgdFreeFileCtx (struct xgdIOCtx *ctx)
{
    dynamicPtr *dp;
    dpIOCtx *dctx;
    dctx = (dpIOCtx *) ctx;
    dp = dctx->dp;
    free (ctx);
    free (dp);
}

static int
allocMemory (dynamicPtr * dp, int initialSize, const void *data)
{
    if (data == NULL)
      {
	  dp->logicalSize = 0;
	  dp->dataGood = FALSE;
	  dp->data = malloc (initialSize);
      }
    else
      {
	  dp->logicalSize = initialSize;
	  dp->dataGood = TRUE;
	  dp->data = (void *) data;
      }
    if (dp->data != NULL)
      {
	  dp->realSize = initialSize;
	  dp->dataGood = TRUE;
	  dp->pos = 0;
	  return TRUE;
      }
    else
      {
	  dp->realSize = 0;
	  return FALSE;
      }
}

static int
appendMemory (dynamicPtr * dp, const void *src, int size)
{
    int bytesNeeded;
    char *tmp;
    if (!dp->dataGood)
	return FALSE;
    bytesNeeded = dp->pos + size;
    if (bytesNeeded > dp->realSize)
      {
	  if (!dp->freeOK)
	      return FALSE;
	  if (overflow2 (dp->realSize, 2))
	      return FALSE;
	  if (!xgdReallocMemory (dp, bytesNeeded * 2))
	    {
		dp->dataGood = FALSE;
		return FALSE;
	    }
      }
    tmp = (char *) dp->data;
    memcpy ((void *) (tmp + (dp->pos)), src, size);
    dp->pos += size;
    if (dp->pos > dp->logicalSize)
	dp->logicalSize = dp->pos;
    return TRUE;
}

static int
appendFile (dynamicPtr * dp, const void *src, int size)
{
    size_t wr = fwrite (src, 1, size, dp->file);
    return (int) wr;
}

static int
memoryPutbuf (struct xgdIOCtx *ctx, const void *buf, int size)
{
    dpIOCtx *dctx;
    dctx = (dpIOCtx *) ctx;
    appendMemory (dctx->dp, buf, size);
    if (dctx->dp->dataGood)
	return size;
    else
	return -1;
}

static int
filePutbuf (struct xgdIOCtx *ctx, const void *buf, int size)
{
    dpIOCtx *dctx;
    dctx = (dpIOCtx *) ctx;
    if (appendFile (dctx->dp, buf, size) == size)
	return size;
    else
	return -1;
}

static void
memoryPutchar (struct xgdIOCtx *ctx, int a)
{
    unsigned char b;
    dpIOCtxPtr dctx;
    b = a;
    dctx = (dpIOCtxPtr) ctx;
    appendMemory (dctx->dp, &b, 1);
}

static void
filePutchar (struct xgdIOCtx *ctx, int a)
{
    unsigned char b;
    dpIOCtxPtr dctx;
    b = a;
    dctx = (dpIOCtxPtr) ctx;
    appendFile (dctx->dp, &b, 1);
}

static int
memoryGetbuf (xgdIOCtxPtr ctx, void *buf, int len)
{
    int rlen, remain;
    dpIOCtxPtr dctx;
    dynamicPtr *dp;
    dctx = (dpIOCtxPtr) ctx;
    dp = dctx->dp;
    remain = dp->logicalSize - dp->pos;
    if (remain >= len)
	rlen = len;
    else
      {
	  if (remain == 0)
	      return 0;
	  rlen = remain;
      }
    memcpy (buf, (void *) ((char *) dp->data + dp->pos), rlen);
    dp->pos += rlen;
    return rlen;
}

static int
fileGetbuf (xgdIOCtxPtr ctx, void *buf, int len)
{
    dynamicPtr *dp;
    dpIOCtxPtr dctx;
    dctx = (dpIOCtxPtr) ctx;
    dp = dctx->dp;
    return fread (buf, 1, len, dp->file);
}

static int
memoryGetchar (xgdIOCtxPtr ctx)
{
    unsigned char b;
    int rv;
    rv = memoryGetbuf (ctx, &b, 1);
    if (rv != 1)
	return EOF;
    else
	return b;
}

static int
fileGetchar (xgdIOCtxPtr ctx)
{
    unsigned char b;
    int rv;
    rv = fileGetbuf (ctx, &b, 1);
    if (rv != 1)
	return EOF;
    else
	return b;
}

static long
memoryTell (struct xgdIOCtx *ctx)
{
    dpIOCtx *dctx;
    dctx = (dpIOCtx *) ctx;
    return (dctx->dp->pos);
}

static long
fileTell (struct xgdIOCtx *ctx)
{
    dynamicPtr *dp;
    dpIOCtx *dctx;
    dctx = (dpIOCtx *) ctx;
    dp = dctx->dp;
    return ftell (dp->file);
}

static int
memorySeek (struct xgdIOCtx *ctx, const int pos)
{
    int bytesNeeded;
    dynamicPtr *dp;
    dpIOCtx *dctx;
    dctx = (dpIOCtx *) ctx;
    dp = dctx->dp;
    if (!dp->dataGood)
	return FALSE;
    bytesNeeded = pos;
    if (bytesNeeded > dp->realSize)
      {
	  if (!dp->freeOK)
	      return FALSE;
	  if (overflow2 (dp->realSize, 2))
	      return FALSE;
	  if (!xgdReallocMemory (dp, dp->realSize * 2))
	    {
		dp->dataGood = FALSE;
		return FALSE;
	    }
      }
    if (pos > dp->logicalSize)
	dp->logicalSize = pos;
    dp->pos = pos;
    return TRUE;
}

static int
fileSeek (struct xgdIOCtx *ctx, const int pos)
{
    dynamicPtr *dp;
    dpIOCtx *dctx;
    dctx = (dpIOCtx *) ctx;
    dp = dctx->dp;
    if (fseek (dp->file, pos, SEEK_SET) < 0)
	return FALSE;
    else
	return TRUE;
}

static dynamicPtr *
newMemory (int initialSize, const void *data, int freeOKFlag)
{
    dynamicPtr *dp;
    dp = malloc (sizeof (dynamicPtr));
    if (dp == NULL)
	return NULL;
    if (!allocMemory (dp, initialSize, data))
	return NULL;
    dp->pos = 0;
    if (freeOKFlag == XGD_CTX_FREE)
	dp->freeOK = 1;
    else
	dp->freeOK = 0;
    return dp;
}

static dynamicPtr *
newFile (const void *data)
{
    dynamicPtr *dp;
    dp = malloc (sizeof (dynamicPtr));
    if (dp == NULL)
	return NULL;
    dp->file = (FILE *) data;
    return dp;
}

static int
trimMemory (dynamicPtr * dp)
{
    if (!dp->freeOK)
	return TRUE;
    return xgdReallocMemory (dp, dp->logicalSize);
}

GGRAPH_PRIVATE int
overflow2 (int a, int b)
{
    if (a < 0 || b < 0)
      {
	  fprintf (stderr,
		   "warning: one parameter to a memory allocation multiplication is negative, failing operation gracefully\n");
	  return 1;
      }
    if (b == 0)
	return 0;
    if (a > INT_MAX / b)
      {
	  fprintf (stderr,
		   "warning: product of memory allocation multiplication would exceed INT_MAX, failing operation gracefully\n");
	  return 1;
      }
    return 0;
}

GGRAPH_PRIVATE void *
xgdDPExtractData (struct xgdIOCtx *ctx, int *size)
{
    dynamicPtr *dp;
    dpIOCtx *dctx;
    void *data;
    dctx = (dpIOCtx *) ctx;
    dp = dctx->dp;
    if (dp->dataGood)
      {
	  trimMemory (dp);
	  *size = dp->logicalSize;
	  data = dp->data;
      }
    else
      {
	  *size = 0;
	  data = NULL;
	  if ((dp->data != NULL) && (dp->freeOK))
	      free (dp->data);
      }
    dp->data = NULL;
    dp->realSize = 0;
    dp->logicalSize = 0;
    return data;
}

GGRAPH_PRIVATE xgdIOCtx *
xgdNewDynamicCtx (int initialSize, const void *data, int mem_or_file)
{
    return xgdNewDynamicCtxEx (initialSize, data, XGD_CTX_FREE, mem_or_file);
}

GGRAPH_PRIVATE xgdIOCtx *
xgdNewDynamicCtxEx (int initialSize, const void *data, int freeOKFlag,
		    int mem_or_file)
{
    dpIOCtx *ctx;
    dynamicPtr *dp;
    ctx = malloc (sizeof (dpIOCtx));
    if (ctx == NULL)
      {
	  return NULL;
      }
    if (mem_or_file == GG_TARGET_IS_FILE)
      {
	  dp = newFile (data);
	  if (!dp)
	    {
		free (ctx);
		return NULL;
	    };
	  ctx->dp = dp;
	  ctx->ctx.getC = fileGetchar;
	  ctx->ctx.putC = filePutchar;
	  ctx->ctx.getBuf = fileGetbuf;
	  ctx->ctx.putBuf = filePutbuf;
	  ctx->ctx.seek = fileSeek;
	  ctx->ctx.tell = fileTell;
	  ctx->ctx.xgd_free = xgdFreeFileCtx;
      }
    else
      {
	  dp = newMemory (initialSize, data, freeOKFlag);
	  if (!dp)
	    {
		free (ctx);
		return NULL;
	    };
	  ctx->dp = dp;
	  ctx->ctx.getC = memoryGetchar;
	  ctx->ctx.putC = memoryPutchar;
	  ctx->ctx.getBuf = memoryGetbuf;
	  ctx->ctx.putBuf = memoryPutbuf;
	  ctx->ctx.seek = memorySeek;
	  ctx->ctx.tell = memoryTell;
	  ctx->ctx.xgd_free = xgdFreeMemoryCtx;
      }
    return (xgdIOCtx *) ctx;
}

GGRAPH_PRIVATE int
xgdPutBuf (const void *buf, int size, xgdIOCtx * ctx)
{
    return (ctx->putBuf) (ctx, buf, size);
}

GGRAPH_PRIVATE int
xgdGetBuf (void *buf, int size, xgdIOCtx * ctx)
{
    return (ctx->getBuf) (ctx, buf, size);
}
