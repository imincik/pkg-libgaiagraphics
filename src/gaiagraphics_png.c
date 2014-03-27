/* 
/ gaiagraphics_png.c
/
/ PNG auxiliary helpers
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

#include <png.h>

#define PNG_TRUE 1
#define PNG_FALSE 0

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

struct png_codec_data
{
/* a struct used by PNG codec */
    int is_writer;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer;
    int palette_allocated;
    int num_palette;
    png_colorp palette;
    int bit_depth;
    int color_type;
    int interlace_type;
    int quantization_factor;
    xgdIOCtx *io_ctx;
};

/* 
/
/ DISCLAIMER:
/ the following code is vaguely derived from the original GD lib 
/ code, which was originally released under a BSD-like license
/
*/

#ifndef PNG_SETJMP_NOT_SUPPORTED
typedef struct _jmpbuf_wrapper
{
    jmp_buf jmpbuf;
}
jmpbuf_wrapper;

static jmpbuf_wrapper xgdPngJmpbufStruct;

static void
xgdPngErrorHandler (png_structp png_ptr, png_const_charp msg)
{
    jmpbuf_wrapper *jmpbuf_ptr;
    fprintf (stderr, "png-wrapper:  fatal libpng error: %s\n", msg);
    fflush (stderr);
    jmpbuf_ptr = png_get_error_ptr (png_ptr);
    if (jmpbuf_ptr == NULL)
      {
	  fprintf (stderr,
		   "png-wrapper:  EXTREMELY fatal error: jmpbuf unrecoverable; terminating.\n");
	  fflush (stderr);
	  exit (99);
      }
    longjmp (jmpbuf_ptr->jmpbuf, 1);
}
#endif

static void
xgdPngReadData (png_structp png_ptr, png_bytep data, png_size_t length)
{
    int check;
    check = xgdGetBuf (data, length, (xgdIOCtx *) png_get_io_ptr (png_ptr));
    if (check != (int) length)
      {
	  png_error (png_ptr, "Read Error: truncated data");
      }
}

static void
xgdPngWriteData (png_structp png_ptr, png_bytep data, png_size_t length)
{
    xgdPutBuf (data, length, (xgdIOCtx *) png_get_io_ptr (png_ptr));
}

static void
xgdPngFlushData (png_structp png_ptr)
{
    if (png_ptr)
	return;			/* does absolutely nothing - required in order to suppress warnings */
}

static gGraphImageInfosPtr
xgdImageInfosFromPngCtx (xgdIOCtx * infile, int *errcode)
{
/* retrieving general infos from a PNG image */
    png_byte sig[8];
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type;
    int num_palette;
    png_colorp palette;
    int i;
    gGraphImageInfosPtr infos = NULL;
    memset (sig, 0, sizeof (sig));
    if (xgdGetBuf (sig, 8, infile) < 8)
      {
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
    if (png_sig_cmp (sig, 0, 8))
      {
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr =
	png_create_read_struct (PNG_LIBPNG_VER_STRING, &xgdPngJmpbufStruct,
				xgdPngErrorHandler, NULL);
#else
    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
      {
	  fprintf (stderr,
		   "png-wrapper error: cannot allocate libpng main struct\n");
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  fprintf (stderr,
		   "png-wrapper error: cannot allocate libpng info struct\n");
	  png_destroy_read_struct (&png_ptr, NULL, NULL);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  fprintf (stderr,
		   "png-wrapper error: setjmp returns error condition 1\n");
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
#endif
    png_set_sig_bytes (png_ptr, 8);
    png_set_read_fn (png_ptr, (void *) infile, xgdPngReadData);
    png_read_info (png_ptr, info_ptr);
    png_get_IHDR (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		  &interlace_type, NULL, NULL);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
      {
	  infos =
	      gg_image_infos_create (GG_PIXEL_PALETTE, (int) width,
				     (int) height, bit_depth, 1,
				     GGRAPH_SAMPLE_UINT, NULL, NULL);
	  png_get_PLTE (png_ptr, info_ptr, &palette, &num_palette);
	  infos->max_palette = num_palette;
	  for (i = 0; i < num_palette; i++)
	    {
		infos->palette_red[i] = palette[i].red;
		infos->palette_green[i] = palette[i].green;
		infos->palette_blue[i] = palette[i].blue;
		fprintf (stderr, "plt %d/%d %02x%02x%02x\n", i, num_palette,
			 infos->palette_red[i], infos->palette_green[i],
			 infos->palette_blue[i]);
		fflush (stderr);
	    }
      }
    else if (color_type == PNG_COLOR_TYPE_GRAY
	     || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	infos =
	    gg_image_infos_create (GG_PIXEL_GRAYSCALE, (int) width,
				   (int) height, bit_depth, 1,
				   GGRAPH_SAMPLE_UINT, NULL, NULL);
    else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	infos =
	    gg_image_infos_create (GG_PIXEL_RGBA, (int) width, (int) height,
				   bit_depth, 4, GGRAPH_SAMPLE_UINT, NULL,
				   NULL);
    else
	infos =
	    gg_image_infos_create (GG_PIXEL_RGB, (int) width, (int) height,
				   bit_depth, 3, GGRAPH_SAMPLE_UINT, NULL,
				   NULL);
    if (infos == NULL)
      {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  return NULL;
      }
    infos->compression = GGRAPH_TIFF_COMPRESSION_DEFLATE;
    if (png_get_interlace_type (png_ptr, info_ptr) == PNG_INTERLACE_ADAM7)
      {
	  infos->scale_1_2 = 1;
	  infos->scale_1_4 = 1;
	  infos->scale_1_8 = 1;
      }
    return infos;
}

static gGraphImagePtr
xgdImageCreateFromPngCtx (xgdIOCtx * infile, int *errcode, int scale)
{
/* decompressing a PNG image */
    png_byte sig[8];
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height, rowbytes, w, h;
    int img_width, img_height;
    int bit_depth, color_type, interlace_type;
    int num_palette;
    png_colorp palette;
    int is_adam7;
    png_bytep row_pointer = NULL;
    gGraphImagePtr img = NULL;
    int i, j;
    volatile int palette_allocated = PNG_FALSE;
    memset (sig, 0, sizeof (sig));
    if (xgdGetBuf (sig, 8, infile) < 8)
      {
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
    if (png_sig_cmp (sig, 0, 8))
      {
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr =
	png_create_read_struct (PNG_LIBPNG_VER_STRING, &xgdPngJmpbufStruct,
				xgdPngErrorHandler, NULL);
#else
    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
      {
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  return NULL;
      }
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  png_destroy_read_struct (&png_ptr, NULL, NULL);
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  return NULL;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  fprintf (stderr,
		   "png-wrapper error: setjmp returns error condition 1\n");
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
#endif
    png_set_sig_bytes (png_ptr, 8);
    png_set_read_fn (png_ptr, (void *) infile, xgdPngReadData);
    png_read_info (png_ptr, info_ptr);
    png_get_IHDR (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		  &interlace_type, NULL, NULL);
    if (png_get_interlace_type (png_ptr, info_ptr) == PNG_INTERLACE_ADAM7)
      {
	  /* Adam7 interlaced [supporting 1:1, 1:2, 1:4 and 1:8 reduced image] */
	  is_adam7 = 1;
	  if (scale == 8)
	    {
		/* requesting 1:8 scaling */
		img_width = width / 8;
		if ((img_width * 8) < (int) width)
		    img_width++;
		img_height = height / 8;
		if ((img_height * 8) < (int) height)
		    img_height++;
	    }
	  else if (scale == 4)
	    {
		/* requesting 1:4 scaling */
		img_width = width / 4;
		if ((img_width * 4) < (int) width)
		    img_width++;
		img_height = height / 4;
		if ((img_height * 4) < (int) height)
		    img_height++;
	    }
	  else if (scale == 2)
	    {
		/* requesting 1:2 scaling */
		img_width = width / 2;
		if ((img_width * 2) < (int) width)
		    img_width++;
		img_height = height / 2;
		if ((img_height * 2) < (int) height)
		    img_height++;
	    }
	  else
	    {
		/* no scaling, full dimension */
		img_width = width;
		img_height = height;
	    }
      }
    else
      {
	  /* normal, not Adam7 interlaced */
	  is_adam7 = 0;
	  img_width = width;
	  img_height = height;
      }

    if (color_type == PNG_COLOR_TYPE_PALETTE)
	img =
	    gg_image_create (GG_PIXEL_PALETTE, img_width, img_height, bit_depth,
			     1, GGRAPH_SAMPLE_UINT, NULL, NULL);
    else if (color_type == PNG_COLOR_TYPE_GRAY
	     || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	img =
	    gg_image_create (GG_PIXEL_GRAYSCALE, img_width, img_height,
			     bit_depth, 1, GGRAPH_SAMPLE_UINT, NULL, NULL);
    else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	img =
	    gg_image_create (GG_PIXEL_RGBA, img_width, img_height, bit_depth, 4,
			     GGRAPH_SAMPLE_UINT, NULL, NULL);
    else
	img =
	    gg_image_create (GG_PIXEL_RGB, img_width, img_height, bit_depth, 3,
			     GGRAPH_SAMPLE_UINT, NULL, NULL);
    if (img == NULL)
      {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  return NULL;
      }

    if (bit_depth == 16)
      {
	  png_set_strip_16 (png_ptr);
      }
    else if (bit_depth < 8)
      {
	  png_set_packing (png_ptr);
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  fprintf (stderr,
		   "png-wrapper error: setjmp returns error condition 2");
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  free (row_pointer);
	  if (img)
	      gg_image_destroy (img);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
#endif
    switch (color_type)
      {
      case PNG_COLOR_TYPE_PALETTE:
	  png_get_PLTE (png_ptr, info_ptr, &palette, &num_palette);
	  img->max_palette = num_palette;
	  for (i = 0; i < num_palette; i++)
	    {
		img->palette_red[i] = palette[i].red;
		img->palette_green[i] = palette[i].green;
		img->palette_blue[i] = palette[i].blue;
	    }
	  break;
      case PNG_COLOR_TYPE_GRAY:
      case PNG_COLOR_TYPE_GRAY_ALPHA:
	  if ((palette = malloc (256 * sizeof (png_color))) == NULL)
	    {
		png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
		*errcode = GGRAPH_INSUFFICIENT_MEMORY;
		return NULL;
	    }
	  palette_allocated = PNG_TRUE;
	  if (bit_depth < 8)
	    {
		num_palette = 1 << bit_depth;
		for (i = 0; i < 256; ++i)
		  {
		      j = (255 * i) / (num_palette - 1);
		      palette[i].red = palette[i].green = palette[i].blue = j;
		  }
	    }
	  else
	    {
		num_palette = 256;
		for (i = 0; i < 256; ++i)
		  {
		      palette[i].red = palette[i].green = palette[i].blue = i;
		  }
	    }
	  break;
      }
    png_read_update_info (png_ptr, info_ptr);
    rowbytes = png_get_rowbytes (png_ptr, info_ptr);
    if (overflow2 (rowbytes, height))
      {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
    row_pointer = malloc (rowbytes);
    if (!row_pointer)
      {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  if (img)
	      gg_image_destroy (img);
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  return NULL;
      }
    if (overflow2 (height, sizeof (png_bytep)))
      {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  free (row_pointer);
	  if (img)
	      gg_image_destroy (img);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
    if (is_adam7)
      {
	  /* reading an ADAM7 interlaced image */
	  int row_no;
	  int pass;
	  int y_base, y_step;
	  png_uint_32 h_limit, w_limit;
	  for (pass = 0; pass < 7; pass++)
	    {
		/* reading an interlaced image requires 7 passes */
		if (scale == 8 && pass > 0)
		  {
		      /* 1:8 reduced image already exctracted */
		      break;
		  }
		if (scale == 4 && pass > 2)
		  {
		      /* 1:4 reduced image already extracted */
		      break;
		  }
		if (scale == 2 && pass > 4)
		  {
		      /* 1:2 reduced image already extracted */
		      break;
		  }

		switch (pass)
		  {
		  case 0:
		      y_base = 0;
		      y_step = 8;
		      break;
		  case 1:
		      y_base = 0;
		      y_step = 8;
		      break;
		  case 2:
		      y_base = 4;
		      y_step = 8;
		      break;
		  case 3:
		      y_base = 0;
		      y_step = 4;
		      break;
		  case 4:
		      y_base = 2;
		      y_step = 4;
		      break;
		  case 5:
		      y_base = 0;
		      y_step = 2;
		      break;
		  case 6:
		      y_base = 1;
		      y_step = 2;
		      break;
		  };
		h_limit = (height - y_base) / y_step;
		if ((h_limit * y_step) < (height - y_base))
		    h_limit++;
		row_no = 0;
		for (h = 0; h < h_limit; h++)
		  {
		      int skip;
		      int boffset = 0;
		      int x_base;
		      int x_step;
		      unsigned char *p_out;
		      png_read_row (png_ptr, row_pointer, NULL);

		      switch (pass)
			{
			case 0:
			    x_base = 0;
			    x_step = 8;
			    break;
			case 1:
			    x_base = 4;
			    x_step = 8;
			    break;
			case 2:
			    x_base = 0;
			    x_step = 4;
			    break;
			case 3:
			    x_base = 4;
			    x_step = 4;
			    break;
			case 4:
			    x_base = 0;
			    x_step = 2;
			    break;
			case 5:
			    x_base = 1;
			    x_step = 2;
			    break;
			case 6:
			    x_base = 0;
			    x_step = 1;
			    break;
			};
		      w_limit = (width - x_base) / x_step;
		      if ((w_limit * x_step) < (width - x_base))
			  w_limit++;
		      switch (pass)
			{
			case 0:
			    p_out =
				img->pixels + (row_no * img->scanline_width);
			    switch (scale)
			      {
			      case 8:
				  skip = 0;
				  row_no += 1;
				  break;
			      case 4:
				  skip = 1;
				  row_no += 2;
				  break;
			      case 2:
				  skip = 3;
				  row_no += 4;
				  break;
			      case 1:
				  skip = 7;
				  row_no += 8;
				  break;
			      };
			    break;
			case 1:
			    switch (scale)
			      {
			      case 4:
				  p_out =
				      img->pixels +
				      (row_no * img->scanline_width) +
				      img->pixel_size;
				  skip = 1;
				  row_no += 2;
				  break;
			      case 2:
				  p_out =
				      img->pixels +
				      (row_no * img->scanline_width) +
				      (img->pixel_size * 2);
				  skip = 3;
				  row_no += 4;
				  break;
			      case 1:
				  p_out =
				      img->pixels +
				      (row_no * img->scanline_width) +
				      (img->pixel_size * 4);
				  skip = 7;
				  row_no += 8;
				  break;
			      };
			    break;
			case 2:
			    switch (scale)
			      {
			      case 4:
				  p_out =
				      img->pixels +
				      ((row_no + 1) * img->scanline_width);
				  skip = 0;
				  row_no += 2;
				  break;
			      case 2:
				  p_out =
				      img->pixels +
				      ((row_no + 2) * img->scanline_width);
				  skip = 1;
				  row_no += 4;
				  break;
			      case 1:
				  p_out =
				      img->pixels +
				      ((row_no + 4) * img->scanline_width);
				  skip = 3;
				  row_no += 8;
				  break;
			      };
			    break;
			case 3:
			    switch (scale)
			      {
			      case 2:
				  p_out =
				      img->pixels +
				      (row_no * img->scanline_width) +
				      img->pixel_size;
				  skip = 1;
				  row_no += 2;
				  break;
			      case 1:
				  p_out =
				      img->pixels +
				      (row_no * img->scanline_width) +
				      (img->pixel_size * 2);
				  skip = 3;
				  row_no += 4;
				  break;
			      };
			    break;
			case 4:
			    switch (scale)
			      {
			      case 2:
				  p_out =
				      img->pixels +
				      ((row_no + 1) * img->scanline_width);
				  skip = 0;
				  row_no += 2;
				  break;
			      case 1:
				  p_out =
				      img->pixels +
				      ((row_no + 2) * img->scanline_width);
				  skip = 1;
				  row_no += 4;
				  break;
			      };
			    break;
			case 5:
			    p_out =
				img->pixels + (row_no * img->scanline_width) +
				(img->pixel_size);
			    skip = 1;
			    row_no += 2;
			    break;
			case 6:
			    p_out =
				img->pixels +
				((row_no + 1) * img->scanline_width);
			    skip = 0;
			    row_no += 2;
			    break;
			};

		      switch (color_type)
			{
			case PNG_COLOR_TYPE_RGB:
			    for (w = 0; w < w_limit; w++)
			      {
				  register png_byte r = row_pointer[boffset++];
				  register png_byte g = row_pointer[boffset++];
				  register png_byte b = row_pointer[boffset++];
				  *p_out++ = r;
				  *p_out++ = g;
				  *p_out++ = b;
				  p_out += skip * 3;
			      }
			    break;
			case PNG_COLOR_TYPE_RGB_ALPHA:
			    for (w = 0; w < w_limit; w++)
			      {
				  register png_byte r = row_pointer[boffset++];
				  register png_byte g = row_pointer[boffset++];
				  register png_byte b = row_pointer[boffset++];
				  register png_byte alpha =
				      row_pointer[boffset++];
				  *p_out++ = r;
				  *p_out++ = g;
				  *p_out++ = b;
				  *p_out++ = alpha;
				  p_out += skip * 4;
			      }
			    break;
			case PNG_COLOR_TYPE_GRAY:
			case PNG_COLOR_TYPE_GRAY_ALPHA:
			    for (w = 0; w < w_limit; w++)
			      {
				  register png_byte idx = row_pointer[w];
				  *p_out++ = idx;
				  p_out += skip;
			      }
			    break;
			default:
			    for (w = 0; w < w_limit; w++)
			      {
				  register png_byte idx = row_pointer[w];
				  *p_out++ = idx;
				  p_out += skip;
			      }
			};
		  }
	    }
      }
    else
      {
	  /* reading a non-interlaced image */
	  for (h = 0; h < height; h++)
	    {
		int boffset = 0;
		unsigned char *p_out = img->pixels + (h * img->scanline_width);
		png_read_row (png_ptr, row_pointer, NULL);
		switch (color_type)
		  {
		  case PNG_COLOR_TYPE_RGB:
		      for (w = 0; w < width; w++)
			{
			    register png_byte r = row_pointer[boffset++];
			    register png_byte g = row_pointer[boffset++];
			    register png_byte b = row_pointer[boffset++];
			    *p_out++ = r;
			    *p_out++ = g;
			    *p_out++ = b;
			}
		      break;
		  case PNG_COLOR_TYPE_RGB_ALPHA:
		      for (w = 0; w < width; w++)
			{
			    register png_byte r = row_pointer[boffset++];
			    register png_byte g = row_pointer[boffset++];
			    register png_byte b = row_pointer[boffset++];
			    register png_byte alpha = row_pointer[boffset++];
			    *p_out++ = r;
			    *p_out++ = g;
			    *p_out++ = b;
			    *p_out++ = alpha;
			}
		      break;
		  case PNG_COLOR_TYPE_GRAY:
		  case PNG_COLOR_TYPE_GRAY_ALPHA:
		      for (w = 0; w < width; ++w)
			{
			    register png_byte idx = row_pointer[w];
			    *p_out++ = idx;
			}
		      break;
		  default:
		      for (w = 0; w < width; ++w)
			{
			    register png_byte idx = row_pointer[w];
			    *p_out++ = idx;
			}
		  }
	    }
      }
    png_destroy_read_struct (&png_ptr, &info_ptr, NULL);

    if (palette_allocated == PNG_TRUE)
	free (palette);
    free (row_pointer);
    return img;
}

GGRAPH_PRIVATE void
gg_png_codec_destroy (void *p)
{
/* destroyng the PNG codec data */
    struct png_codec_data *codec = (struct png_codec_data *) p;
    if (!codec)
	return;
    if (codec->is_writer)
      {
	  png_write_end (codec->png_ptr, codec->info_ptr);
	  png_destroy_write_struct (&(codec->png_ptr), &(codec->info_ptr));
      }
    else
      {
	  png_read_end (codec->png_ptr, codec->info_ptr);
	  png_destroy_read_struct (&(codec->png_ptr), &(codec->info_ptr), NULL);
      }
    if (codec->palette_allocated == PNG_TRUE)
	free (codec->palette);
    free (codec->row_pointer);
    codec->io_ctx->xgd_free (codec->io_ctx);
    free (codec);
}

static gGraphStripImagePtr
xgdStripImageCreateFromPngCtx (xgdIOCtx * infile, int *errcode, FILE * file)
{
/* preparing to decompress a PNG image [by strips] */
    png_byte sig[8];
    png_structp png_ptr;
    png_infop info_ptr;
    png_uint_32 width, height, rowbytes;
    int bit_depth, color_type, interlace_type;
    int num_palette;
    png_colorp palette;
    png_bytep row_pointer = NULL;
    gGraphStripImagePtr img = NULL;
    struct png_codec_data *png_codec;
    int i, j;
    volatile int palette_allocated = PNG_FALSE;
    memset (sig, 0, sizeof (sig));
    if (xgdGetBuf (sig, 8, infile) < 8)
      {
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
    if (png_sig_cmp (sig, 0, 8))
      {
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr =
	png_create_read_struct (PNG_LIBPNG_VER_STRING, &xgdPngJmpbufStruct,
				xgdPngErrorHandler, NULL);
#else
    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
      {
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  return NULL;
      }
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  png_destroy_read_struct (&png_ptr, NULL, NULL);
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  return NULL;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  fprintf (stderr,
		   "png-wrapper error: setjmp returns error condition 1\n");
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
#endif
    png_set_sig_bytes (png_ptr, 8);
    png_set_read_fn (png_ptr, (void *) infile, xgdPngReadData);
    png_read_info (png_ptr, info_ptr);
    png_get_IHDR (png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		  &interlace_type, NULL, NULL);

    if (interlace_type == PNG_INTERLACE_ADAM7)
      {
	  fprintf (stderr,
		   "png-wrapper error: ADAM7 interlace cannot be accessed by strips\n");
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }

    if (color_type == PNG_COLOR_TYPE_PALETTE)
      {
	  img =
	      gg_strip_image_create (file, GGRAPH_IMAGE_PNG, GG_PIXEL_PALETTE,
				     width, height, bit_depth, 1,
				     GGRAPH_SAMPLE_UINT, NULL, NULL);
      }
    else if (color_type == PNG_COLOR_TYPE_GRAY
	     || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      {
	  img =
	      gg_strip_image_create (file, GGRAPH_IMAGE_PNG, GG_PIXEL_GRAYSCALE,
				     width, height, bit_depth, 1,
				     GGRAPH_SAMPLE_UINT, NULL, NULL);
      }
    else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
      {
	  img =
	      gg_strip_image_create (file, GGRAPH_IMAGE_PNG, GG_PIXEL_RGBA,
				     width, height, bit_depth, 4,
				     GGRAPH_SAMPLE_UINT, NULL, NULL);
      }
    else
      {
	  img =
	      gg_strip_image_create (file, GGRAPH_IMAGE_PNG, GG_PIXEL_RGB,
				     width, height, bit_depth, 3,
				     GGRAPH_SAMPLE_UINT, NULL, NULL);
      }
    if (img == NULL)
      {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  return NULL;
      }

    if (bit_depth == 16)
      {
	  png_set_strip_16 (png_ptr);
      }
    else if (bit_depth < 8)
      {
	  png_set_packing (png_ptr);
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  fprintf (stderr,
		   "png-wrapper error: setjmp returns error condition 2");
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  free (row_pointer);
	  if (img)
	      gg_strip_image_destroy (img);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
#endif
    switch (color_type)
      {
      case PNG_COLOR_TYPE_PALETTE:
	  png_get_PLTE (png_ptr, info_ptr, &palette, &num_palette);
	  img->max_palette = num_palette;
	  for (i = 0; i < num_palette; i++)
	    {
		img->palette_red[i] = palette[i].red;
		img->palette_green[i] = palette[i].green;
		img->palette_blue[i] = palette[i].blue;
	    }
	  break;
      case PNG_COLOR_TYPE_GRAY:
      case PNG_COLOR_TYPE_GRAY_ALPHA:
	  if ((palette = malloc (256 * sizeof (png_color))) == NULL)
	    {
		png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
		*errcode = GGRAPH_INSUFFICIENT_MEMORY;
		return NULL;
	    }
	  palette_allocated = PNG_TRUE;
	  if (bit_depth < 8)
	    {
		num_palette = 1 << bit_depth;
		for (i = 0; i < 256; ++i)
		  {
		      j = (255 * i) / (num_palette - 1);
		      palette[i].red = palette[i].green = palette[i].blue = j;
		  }
	    }
	  else
	    {
		num_palette = 256;
		for (i = 0; i < 256; ++i)
		  {
		      palette[i].red = palette[i].green = palette[i].blue = i;
		  }
	    }
	  break;
      }
    png_read_update_info (png_ptr, info_ptr);
    rowbytes = png_get_rowbytes (png_ptr, info_ptr);
    if (overflow2 (rowbytes, height))
      {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
    row_pointer = malloc (rowbytes);
    if (!row_pointer)
      {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  if (img)
	      gg_strip_image_destroy (img);
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  return NULL;
      }
    if (overflow2 (height, sizeof (png_bytep)))
      {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  free (row_pointer);
	  if (img)
	      gg_strip_image_destroy (img);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }

/* setting up the PNG codec struct */
    png_codec = malloc (sizeof (struct png_codec_data));
    if (!png_codec)
      {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  free (row_pointer);
	  gg_strip_image_destroy (img);
	  *errcode = GGRAPH_PNG_CODEC_ERROR;
	  return NULL;
      }
    png_codec->is_writer = 0;
    png_codec->row_pointer = row_pointer;
    png_codec->png_ptr = png_ptr;
    png_codec->info_ptr = info_ptr;
    png_codec->palette_allocated = palette_allocated;
    png_codec->num_palette = num_palette;
    png_codec->palette = palette;
    png_codec->bit_depth = bit_depth;
    png_codec->color_type = color_type;
    png_codec->interlace_type = interlace_type;
    png_codec->io_ctx = infile;
    img->codec_data = png_codec;

    return img;
}

static int
xgdStripImageReadFromPngCtx (gGraphStripImagePtr img)
{
/* decompressing a PNG image [by strip] */
    png_uint_32 w, h;
    struct png_codec_data *png_codec =
	(struct png_codec_data *) (img->codec_data);
    png_structp png_ptr = png_codec->png_ptr;
    int color_type = png_codec->color_type;
    png_bytep row_pointer = png_codec->row_pointer;
    png_uint_32 width = img->width;
    png_uint_32 height = img->rows_per_block;

    if (img->next_row >= img->height)
      {
	  /* EOF condition */
	  fprintf (stderr, "png-wrapper error: attempting to read beyond EOF");
	  return GGRAPH_PNG_CODEC_ERROR;
      }
    if ((img->next_row + img->rows_per_block) >= img->height)
	height = img->height - img->next_row;
    img->current_available_rows = height;

#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  fprintf (stderr,
		   "png-wrapper error: setjmp returns error condition 2");
	  return GGRAPH_PNG_CODEC_ERROR;
      }
#endif

/* reading a non-interlaced image */
    for (h = 0; h < height; h++)
      {
	  int boffset = 0;
	  unsigned char *p_out = img->pixels + (h * img->scanline_width);
	  png_read_row (png_ptr, row_pointer, NULL);
	  switch (color_type)
	    {
	    case PNG_COLOR_TYPE_RGB:
		for (w = 0; w < width; w++)
		  {
		      register png_byte r = row_pointer[boffset++];
		      register png_byte g = row_pointer[boffset++];
		      register png_byte b = row_pointer[boffset++];
		      *p_out++ = r;
		      *p_out++ = g;
		      *p_out++ = b;
		  }
		break;
	    case PNG_COLOR_TYPE_RGB_ALPHA:
		for (w = 0; w < width; w++)
		  {
		      register png_byte r = row_pointer[boffset++];
		      register png_byte g = row_pointer[boffset++];
		      register png_byte b = row_pointer[boffset++];
		      register png_byte alpha = row_pointer[boffset++];
		      *p_out++ = r;
		      *p_out++ = g;
		      *p_out++ = b;
		      *p_out++ = alpha;
		  }
		break;
	    case PNG_COLOR_TYPE_GRAY:
	    case PNG_COLOR_TYPE_GRAY_ALPHA:
		for (w = 0; w < width; ++w)
		  {
		      register png_byte idx = row_pointer[w];
		      *p_out++ = idx;
		  }
		break;
	    default:
		for (w = 0; w < width; ++w)
		  {
		      register png_byte idx = row_pointer[w];
		      *p_out++ = idx;
		  }
	    }
      }
    img->next_row += height;
    return GGRAPH_OK;
}

static int
xgdImagePngCtxPalette (gGraphImagePtr img, xgdIOCtx * outfile,
		       int compression_level, int interlaced)
{
/* compressing a PNG image [palette] */
    int i, j, bit_depth = 0, interlace_type;
    int width = img->width;
    int height = img->height;
    png_color palette[256];
    int colors = img->max_palette;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer = NULL;
    int pass;
    int num_passes;
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
				       &xgdPngJmpbufStruct, xgdPngErrorHandler,
				       NULL);
#else
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
	return GGRAPH_INSUFFICIENT_MEMORY;
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  if (row_pointer)
	      free (row_pointer);
	  png_destroy_write_struct (&png_ptr, &info_ptr);
	  return GGRAPH_PNG_CODEC_ERROR;
      }
#endif
    png_set_write_fn (png_ptr, (void *) outfile, xgdPngWriteData,
		      xgdPngFlushData);
    png_set_compression_level (png_ptr, compression_level);
    if (colors <= 2)
	bit_depth = 1;
    else if (colors <= 4)
	bit_depth = 2;
    else if (colors <= 16)
	bit_depth = 4;
    else
	bit_depth = 8;
    if (interlaced)
	interlace_type = PNG_INTERLACE_ADAM7;
    else
	interlace_type = PNG_INTERLACE_NONE;
    png_set_IHDR (png_ptr, info_ptr, width, height, bit_depth,
		  PNG_COLOR_TYPE_PALETTE, interlace_type,
		  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    for (i = 0; i < colors; i++)
      {
	  palette[i].red = img->palette_red[i];
	  palette[i].green = img->palette_green[i];
	  palette[i].blue = img->palette_blue[i];
      }
    png_set_PLTE (png_ptr, info_ptr, palette, colors);
    png_write_info (png_ptr, info_ptr);
    png_set_packing (png_ptr);
    if (overflow2 (sizeof (png_bytep), height))
	return GGRAPH_PNG_CODEC_ERROR;
    row_pointer = malloc (width);
    if (!row_pointer)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    num_passes = png_set_interlace_handling (png_ptr);
    for (pass = 0; pass < num_passes; pass++)
      {
	  /* Adam7 interlacing may require seven distinct images */
	  for (j = 0; j < height; ++j)
	    {
		png_bytep p_out = row_pointer;
		unsigned char *p_in = img->pixels + (j * img->scanline_width);
		for (i = 0; i < width; ++i)
		    *p_out++ = *p_in++;
		png_write_row (png_ptr, row_pointer);
	    }
      }
    png_write_end (png_ptr, info_ptr);
    free (row_pointer);
    png_destroy_write_struct (&png_ptr, &info_ptr);
    return GGRAPH_OK;
}

static int
xgdStripImagePngCtxPalette (gGraphStripImagePtr img, xgdIOCtx * outfile,
			    int compression_level)
{
/* preparing to compress a PNG image [palette] (by strip) */
    int i, bit_depth = 0, interlace_type;
    int width = img->width;
    int height = img->height;
    png_color palette[256];
    int colors = img->max_palette;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer = NULL;
    struct png_codec_data *png_codec;
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
				       &xgdPngJmpbufStruct, xgdPngErrorHandler,
				       NULL);
#else
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
	return GGRAPH_INSUFFICIENT_MEMORY;
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  if (row_pointer)
	      free (row_pointer);
	  png_destroy_write_struct (&png_ptr, &info_ptr);
	  return GGRAPH_PNG_CODEC_ERROR;
      }
#endif
    png_set_write_fn (png_ptr, (void *) outfile, xgdPngWriteData,
		      xgdPngFlushData);
    png_set_compression_level (png_ptr, compression_level);
    if (colors <= 2)
	bit_depth = 1;
    else if (colors <= 4)
	bit_depth = 2;
    else if (colors <= 16)
	bit_depth = 4;
    else
	bit_depth = 8;
    interlace_type = PNG_INTERLACE_NONE;
    png_set_IHDR (png_ptr, info_ptr, width, height, bit_depth,
		  PNG_COLOR_TYPE_PALETTE, interlace_type,
		  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    for (i = 0; i < colors; i++)
      {
	  palette[i].red = img->palette_red[i];
	  palette[i].green = img->palette_green[i];
	  palette[i].blue = img->palette_blue[i];
      }
    png_set_PLTE (png_ptr, info_ptr, palette, colors);
    png_write_info (png_ptr, info_ptr);
    png_set_packing (png_ptr);
    if (overflow2 (sizeof (png_bytep), height))
	return GGRAPH_PNG_CODEC_ERROR;
    row_pointer = malloc (width * 3);
    if (!row_pointer)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }

/* setting up the PNG codec struct */
    png_codec = malloc (sizeof (struct png_codec_data));
    if (!png_codec)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  free (row_pointer);
	  gg_strip_image_destroy (img);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    png_codec->is_writer = 1;
    png_codec->row_pointer = row_pointer;
    png_codec->png_ptr = png_ptr;
    png_codec->info_ptr = info_ptr;
    png_codec->palette_allocated = PNG_FALSE;
    png_codec->palette = NULL;
    png_codec->bit_depth = bit_depth;
    png_codec->color_type = PNG_COLOR_TYPE_GRAY;
    png_codec->interlace_type = PNG_INTERLACE_NONE;
    png_codec->quantization_factor = 0;
    png_codec->io_ctx = outfile;
    img->codec_data = png_codec;

    return GGRAPH_OK;
}

static int
xgdImagePngCtxGrayscale (gGraphImagePtr img, xgdIOCtx * outfile,
			 int compression_level, int quantization_factor,
			 int interlaced)
{
/* compressing a PNG image [grayscale] */
    int i, j, bit_depth = 0, interlace_type;
    int width = img->width;
    int height = img->height;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer = NULL;
    int pass;
    int num_passes;
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
				       &xgdPngJmpbufStruct, xgdPngErrorHandler,
				       NULL);
#else
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
	return GGRAPH_INSUFFICIENT_MEMORY;
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
	return GGRAPH_PNG_CODEC_ERROR;
#endif
    png_set_write_fn (png_ptr, (void *) outfile, xgdPngWriteData,
		      xgdPngFlushData);
    png_set_compression_level (png_ptr, compression_level);
    bit_depth = 8;
    if (interlaced)
	interlace_type = PNG_INTERLACE_ADAM7;
    else
	interlace_type = PNG_INTERLACE_NONE;
    png_set_IHDR (png_ptr, info_ptr, width, height, bit_depth,
		  PNG_COLOR_TYPE_GRAY, interlace_type,
		  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info (png_ptr, info_ptr);
    png_set_packing (png_ptr);
    if (overflow2 (sizeof (png_bytep), height))
	return GGRAPH_PNG_CODEC_ERROR;
    row_pointer = malloc (width);
    if (!row_pointer)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    num_passes = png_set_interlace_handling (png_ptr);
    for (pass = 0; pass < num_passes; pass++)
      {
	  /* Adam7 interlacing may require seven distinct images */
	  for (j = 0; j < height; ++j)
	    {
		png_bytep p_out = row_pointer;
		unsigned char *p_in = img->pixels + (j * img->scanline_width);
		for (i = 0; i < width; ++i)
		  {
		      int gray = *p_in++;
		      if (quantization_factor <= 0)
			  ;
		      else if (quantization_factor == 1)
			  gray |= 0x01;
		      else if (quantization_factor == 2)
			  gray |= 0x03;
		      else if (quantization_factor == 3)
			  gray |= 0x07;
		      else
			  gray |= 0x0f;
		      *p_out++ = gray;
		  }
		png_write_row (png_ptr, row_pointer);
	    }
      }
    png_write_end (png_ptr, info_ptr);
    free (row_pointer);
    png_destroy_write_struct (&png_ptr, &info_ptr);
    return GGRAPH_OK;
}

static int
xgdStripImagePngCtxGrayscale (gGraphStripImagePtr img, xgdIOCtx * outfile,
			      int compression_level, int quantization_factor)
{
/* preparing to compress a PNG image [grayscale] (by strip) */
    int bit_depth = 0, interlace_type;
    int width = img->width;
    int height = img->height;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer = NULL;
    struct png_codec_data *png_codec;
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
				       &xgdPngJmpbufStruct, xgdPngErrorHandler,
				       NULL);
#else
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
	return GGRAPH_INSUFFICIENT_MEMORY;
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  if (row_pointer)
	      free (row_pointer);
	  png_destroy_write_struct (&png_ptr, &info_ptr);
	  return GGRAPH_PNG_CODEC_ERROR;
      }
#endif
    png_set_write_fn (png_ptr, (void *) outfile, xgdPngWriteData,
		      xgdPngFlushData);
    png_set_compression_level (png_ptr, compression_level);
    bit_depth = 8;
    interlace_type = PNG_INTERLACE_NONE;
    png_set_IHDR (png_ptr, info_ptr, width, height, bit_depth,
		  PNG_COLOR_TYPE_GRAY, interlace_type,
		  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info (png_ptr, info_ptr);
    png_set_packing (png_ptr);
    if (overflow2 (sizeof (png_bytep), height))
	return GGRAPH_PNG_CODEC_ERROR;
    row_pointer = malloc (width * 3);
    if (!row_pointer)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }

/* setting up the PNG codec struct */
    png_codec = malloc (sizeof (struct png_codec_data));
    if (!png_codec)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  free (row_pointer);
	  gg_strip_image_destroy (img);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    png_codec->is_writer = 1;
    png_codec->row_pointer = row_pointer;
    png_codec->png_ptr = png_ptr;
    png_codec->info_ptr = info_ptr;
    png_codec->palette_allocated = PNG_FALSE;
    png_codec->palette = NULL;
    png_codec->bit_depth = bit_depth;
    png_codec->color_type = PNG_COLOR_TYPE_GRAY;
    png_codec->interlace_type = PNG_INTERLACE_NONE;
    png_codec->quantization_factor = quantization_factor;
    png_codec->io_ctx = outfile;
    img->codec_data = png_codec;

    return GGRAPH_OK;
}

static int
xgdImagePngCtxRgb (gGraphImagePtr img, xgdIOCtx * outfile,
		   int compression_level, int quantization_factor,
		   int interlaced)
{
/* compressing a PNG image [RGB] */
    int i, j, bit_depth = 0, interlace_type;
    int width = img->width;
    int height = img->height;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer = NULL;
    int pass;
    int num_passes;
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
				       &xgdPngJmpbufStruct, xgdPngErrorHandler,
				       NULL);
#else
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
	return GGRAPH_INSUFFICIENT_MEMORY;
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  if (row_pointer)
	      free (row_pointer);
	  png_destroy_write_struct (&png_ptr, &info_ptr);
	  return GGRAPH_PNG_CODEC_ERROR;
      }
#endif
    png_set_write_fn (png_ptr, (void *) outfile, xgdPngWriteData,
		      xgdPngFlushData);
    png_set_compression_level (png_ptr, compression_level);
    bit_depth = 8;
    if (interlaced)
	interlace_type = PNG_INTERLACE_ADAM7;
    else
	interlace_type = PNG_INTERLACE_NONE;
    png_set_IHDR (png_ptr, info_ptr, width, height, bit_depth,
		  PNG_COLOR_TYPE_RGB, interlace_type,
		  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info (png_ptr, info_ptr);
    png_set_packing (png_ptr);
    if (overflow2 (sizeof (png_bytep), height))
	return GGRAPH_PNG_CODEC_ERROR;
    row_pointer = malloc (width * 3);
    if (!row_pointer)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    num_passes = png_set_interlace_handling (png_ptr);
    for (pass = 0; pass < num_passes; pass++)
      {
	  /* Adam7 interlacing may require seven distinct images */
	  for (j = 0; j < height; j++)
	    {
		png_bytep p_out = row_pointer;
		unsigned char *p_in = img->pixels + (j * img->scanline_width);
		for (i = 0; i < width; ++i)
		  {
		      unsigned char r;
		      unsigned char g;
		      unsigned char b;
		      if (img->pixel_format == GG_PIXEL_RGB)
			{
			    r = *p_in++;
			    g = *p_in++;
			    b = *p_in++;
			}
		      else if (img->pixel_format == GG_PIXEL_BGR)
			{
			    b = *p_in++;
			    g = *p_in++;
			    r = *p_in++;
			}
		      if (quantization_factor <= 0)
			  ;
		      else if (quantization_factor == 1)
			{
			    r |= 0x01;
			    g |= 0x01;
			    b |= 0x01;
			}
		      else if (quantization_factor == 2)
			{
			    r |= 0x03;
			    g |= 0x03;
			    b |= 0x03;
			}
		      else if (quantization_factor == 3)
			{
			    r |= 0x07;
			    g |= 0x07;
			    b |= 0x07;
			}
		      else
			{
			    r |= 0x0f;
			    g |= 0x0f;
			    b |= 0x0f;
			}
		      *p_out++ = r;
		      *p_out++ = g;
		      *p_out++ = b;
		  }
		png_write_row (png_ptr, row_pointer);
	    }
      }
    png_write_end (png_ptr, info_ptr);
    free (row_pointer);
    png_destroy_write_struct (&png_ptr, &info_ptr);
    return GGRAPH_OK;
}

static int
xgdStripImagePngCtxRgb (gGraphStripImagePtr img, xgdIOCtx * outfile,
			int compression_level, int quantization_factor)
{
/* preparing to compress a PNG image [RGB] (by strip) */
    int bit_depth = 0, interlace_type;
    int width = img->width;
    int height = img->height;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer = NULL;
    struct png_codec_data *png_codec;
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
				       &xgdPngJmpbufStruct, xgdPngErrorHandler,
				       NULL);
#else
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
	return GGRAPH_INSUFFICIENT_MEMORY;
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  if (row_pointer)
	      free (row_pointer);
	  png_destroy_write_struct (&png_ptr, &info_ptr);
	  return GGRAPH_PNG_CODEC_ERROR;
      }
#endif
    png_set_write_fn (png_ptr, (void *) outfile, xgdPngWriteData,
		      xgdPngFlushData);
    png_set_compression_level (png_ptr, compression_level);
    bit_depth = 8;
    interlace_type = PNG_INTERLACE_NONE;
    png_set_IHDR (png_ptr, info_ptr, width, height, bit_depth,
		  PNG_COLOR_TYPE_RGB, interlace_type,
		  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info (png_ptr, info_ptr);
    png_set_packing (png_ptr);
    if (overflow2 (sizeof (png_bytep), height))
	return GGRAPH_PNG_CODEC_ERROR;
    row_pointer = malloc (width * 3);
    if (!row_pointer)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }

/* setting up the PNG codec struct */
    png_codec = malloc (sizeof (struct png_codec_data));
    if (!png_codec)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  free (row_pointer);
	  gg_strip_image_destroy (img);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    png_codec->is_writer = 1;
    png_codec->row_pointer = row_pointer;
    png_codec->png_ptr = png_ptr;
    png_codec->info_ptr = info_ptr;
    png_codec->palette_allocated = PNG_FALSE;
    png_codec->palette = NULL;
    png_codec->bit_depth = bit_depth;
    png_codec->color_type = PNG_COLOR_TYPE_RGB;
    png_codec->interlace_type = PNG_INTERLACE_NONE;
    png_codec->quantization_factor = quantization_factor;
    png_codec->io_ctx = outfile;
    img->codec_data = png_codec;

    return GGRAPH_OK;
}

static int
xgdImagePngCtxRgbAlpha (gGraphImagePtr img, xgdIOCtx * outfile,
			int compression_level, int quantization_factor,
			int interlaced)
{
/* compressing a PNG image [RGBA] */
    int i, j, bit_depth = 0, interlace_type;
    int width = img->width;
    int height = img->height;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer = NULL;
    int pass;
    int num_passes;
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
				       &xgdPngJmpbufStruct, xgdPngErrorHandler,
				       NULL);
#else
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
	return GGRAPH_INSUFFICIENT_MEMORY;
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  if (row_pointer)
	      free (row_pointer);
	  png_destroy_write_struct (&png_ptr, &info_ptr);
	  return GGRAPH_PNG_CODEC_ERROR;
      }
#endif
    png_set_write_fn (png_ptr, (void *) outfile, xgdPngWriteData,
		      xgdPngFlushData);
    png_set_compression_level (png_ptr, compression_level);
    bit_depth = 8;
    if (interlaced)
	interlace_type = PNG_INTERLACE_ADAM7;
    else
	interlace_type = PNG_INTERLACE_NONE;
    png_set_IHDR (png_ptr, info_ptr, width, height, bit_depth,
		  PNG_COLOR_TYPE_RGB_ALPHA, interlace_type,
		  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info (png_ptr, info_ptr);
    png_set_packing (png_ptr);
    if (overflow2 (sizeof (png_bytep), height))
	return GGRAPH_PNG_CODEC_ERROR;
    row_pointer = malloc (width * 4);
    if (!row_pointer)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    num_passes = png_set_interlace_handling (png_ptr);
    for (pass = 0; pass < num_passes; pass++)
      {
	  /* Adam7 interlacing may require seven distinct images */
	  for (j = 0; j < height; ++j)
	    {
		png_bytep p_out = row_pointer;
		unsigned char *p_in = img->pixels + (j * img->scanline_width);
		for (i = 0; i < width; ++i)
		  {
		      unsigned char r;
		      unsigned char g;
		      unsigned char b;
		      unsigned char alpha;
		      if (img->pixel_format == GG_PIXEL_RGBA)
			{
			    r = *p_in++;
			    g = *p_in++;
			    b = *p_in++;
			    alpha = *p_in++;
			}
		      else if (img->pixel_format == GG_PIXEL_ARGB)
			{
			    alpha = *p_in++;
			    r = *p_in++;
			    g = *p_in++;
			    b = *p_in++;
			}
		      else if (img->pixel_format == GG_PIXEL_BGRA)
			{
			    b = *p_in++;
			    g = *p_in++;
			    r = *p_in++;
			    alpha = *p_in++;
			}
		      else if (img->pixel_format == GG_PIXEL_RGB)
			{
			    r = *p_in++;
			    g = *p_in++;
			    b = *p_in++;
			    if (r == img->transparent_red
				&& g == img->transparent_green
				&& b == img->transparent_blue)
				alpha = 0;
			    else
				alpha = 255;
			}
		      else if (img->pixel_format == GG_PIXEL_BGR)
			{
			    b = *p_in++;
			    g = *p_in++;
			    r = *p_in++;
			    if (r == img->transparent_red
				&& g == img->transparent_green
				&& b == img->transparent_blue)
				alpha = 0;
			    else
				alpha = 255;
			}
		      else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
			{
			    r = *p_in++;
			    g = r;
			    b = r;
			    if (r == img->transparent_red
				&& g == img->transparent_green
				&& b == img->transparent_blue)
				alpha = 0;
			    else
				alpha = 255;
			}
		      else if (img->pixel_format == GG_PIXEL_PALETTE)
			{
			    int idx = *p_in++;
			    r = img->palette_red[idx];
			    g = img->palette_green[idx];
			    b = img->palette_blue[idx];
			    if (r == img->transparent_red
				&& g == img->transparent_green
				&& b == img->transparent_blue)
				alpha = 0;
			    else
				alpha = 255;
			}
		      if (quantization_factor <= 0)
			  ;
		      else if (quantization_factor == 1)
			{
			    r |= 0x01;
			    g |= 0x01;
			    b |= 0x01;
			}
		      else if (quantization_factor == 2)
			{
			    r |= 0x03;
			    g |= 0x03;
			    b |= 0x03;
			}
		      else if (quantization_factor == 3)
			{
			    r |= 0x07;
			    g |= 0x07;
			    b |= 0x07;
			}
		      else
			{
			    r |= 0x0f;
			    g |= 0x0f;
			    b |= 0x0f;
			}
		      *p_out++ = r;
		      *p_out++ = g;
		      *p_out++ = b;
		      *p_out++ = alpha;
		  }
		png_write_row (png_ptr, row_pointer);
	    }
      }
    png_write_end (png_ptr, info_ptr);
    free (row_pointer);
    png_destroy_write_struct (&png_ptr, &info_ptr);
    return GGRAPH_OK;
}

static int
xgdStripImagePngCtxRgbAlpha (gGraphStripImagePtr img, xgdIOCtx * outfile,
			     int compression_level, int quantization_factor)
{
/* preparing to compress a PNG image [RGBA] (by strip) */
    int bit_depth = 0, interlace_type;
    int width = img->width;
    int height = img->height;
    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep row_pointer = NULL;
    struct png_codec_data *png_codec;
#ifndef PNG_SETJMP_NOT_SUPPORTED
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
				       &xgdPngJmpbufStruct, xgdPngErrorHandler,
				       NULL);
#else
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
#endif
    if (png_ptr == NULL)
	return GGRAPH_INSUFFICIENT_MEMORY;
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
      {
	  if (row_pointer)
	      free (row_pointer);
	  png_destroy_write_struct (&png_ptr, &info_ptr);
	  return GGRAPH_PNG_CODEC_ERROR;
      }
#endif
    png_set_write_fn (png_ptr, (void *) outfile, xgdPngWriteData,
		      xgdPngFlushData);
    png_set_compression_level (png_ptr, compression_level);
    bit_depth = 8;
    interlace_type = PNG_INTERLACE_NONE;
    png_set_IHDR (png_ptr, info_ptr, width, height, bit_depth,
		  PNG_COLOR_TYPE_RGB_ALPHA, interlace_type,
		  PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info (png_ptr, info_ptr);
    png_set_packing (png_ptr);
    if (overflow2 (sizeof (png_bytep), height))
	return GGRAPH_PNG_CODEC_ERROR;
    row_pointer = malloc (width * 4);
    if (!row_pointer)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }

/* setting up the PNG codec struct */
    png_codec = malloc (sizeof (struct png_codec_data));
    if (!png_codec)
      {
	  png_destroy_write_struct (&png_ptr, (png_infopp) NULL);
	  free (row_pointer);
	  gg_strip_image_destroy (img);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    png_codec->is_writer = 1;
    png_codec->row_pointer = row_pointer;
    png_codec->png_ptr = png_ptr;
    png_codec->info_ptr = info_ptr;
    png_codec->palette_allocated = PNG_FALSE;
    png_codec->palette = NULL;
    png_codec->bit_depth = bit_depth;
    png_codec->color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    png_codec->interlace_type = PNG_INTERLACE_NONE;
    png_codec->quantization_factor = quantization_factor;
    png_codec->io_ctx = outfile;
    img->codec_data = png_codec;

    return GGRAPH_OK;
}

static int
xgdStripImagePngCtx (gGraphStripImagePtr img)
{
/* compressing a PNG image [by strip] */
    int i, j;
    int width = img->width;
    int height = img->height;
    struct png_codec_data *png_codec =
	(struct png_codec_data *) (img->codec_data);
    png_structp png_ptr = png_codec->png_ptr;
    png_bytep row_pointer = png_codec->row_pointer;
    int quantization_factor = png_codec->quantization_factor;

    if (img->next_row >= img->height)
      {
	  /* EOF condition */
	  fprintf (stderr, "png-wrapper error: attempting to write beyond EOF");
	  return GGRAPH_PNG_CODEC_ERROR;
      }
    height = img->current_available_rows;

#ifndef PNG_SETJMP_NOT_SUPPORTED
    if (setjmp (xgdPngJmpbufStruct.jmpbuf))
	return GGRAPH_PNG_CODEC_ERROR;
#endif
    for (j = 0; j < height; ++j)
      {
	  png_bytep p_out = row_pointer;
	  unsigned char *p_in = img->pixels + (j * img->scanline_width);
	  for (i = 0; i < width; ++i)
	    {
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char alpha;
		if (img->pixel_format == GG_PIXEL_PALETTE
		    || img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      /* simply copying pixels */
		      *p_out++ = *p_in++;
		      continue;
		  }

		/* expected to be RGB or RGB-Alpha */
		if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      r = *p_in++;
		      g = *p_in++;
		      b = *p_in++;
		      alpha = *p_in++;
		  }
		else
		  {
		      r = *p_in++;
		      g = *p_in++;
		      b = *p_in++;
		  }

		/* applying color quantization (if required) */
		if (quantization_factor <= 0)
		    ;
		else if (quantization_factor == 1)
		  {
		      r |= 0x01;
		      g |= 0x01;
		      b |= 0x01;
		  }
		else if (quantization_factor == 2)
		  {
		      r |= 0x03;
		      g |= 0x03;
		      b |= 0x03;
		  }
		else if (quantization_factor == 3)
		  {
		      r |= 0x07;
		      g |= 0x07;
		      b |= 0x07;
		  }
		else
		  {
		      r |= 0x0f;
		      g |= 0x0f;
		      b |= 0x0f;
		  }

		/* outputting the pixel */
		if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      *p_out++ = r;
		      *p_out++ = g;
		      *p_out++ = b;
		      *p_out++ = alpha;
		  }
		else
		  {
		      *p_out++ = r;
		      *p_out++ = g;
		      *p_out++ = b;
		  }
	    }
	  png_write_row (png_ptr, row_pointer);
      }
    img->next_row += height;
    return GGRAPH_OK;
}

static int
image_to_png_palette (const gGraphImagePtr img, void **mem_buf,
		      int *mem_buf_size, FILE * file, int dest_type,
		      int compression_level, int interlaced)
{
/* compressing an image as PNG PALETTE */
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
    if (compression_level < 0 || compression_level > 9)
	compression_level = 4;
    ret = xgdImagePngCtxPalette (img, out, compression_level, interlaced);
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

static int
image_prepare_to_png_palette_by_strip (const gGraphStripImagePtr img,
				       FILE * file, int compression_level)
{
/* preparing to compress an image as PNG PALETTE [by strip] */
    xgdIOCtx *out;

/* checkings args for validity */
    if (!file)
	return GGRAPH_ERROR;

    out = xgdNewDynamicCtx (0, file, GG_TARGET_IS_FILE);
    if (compression_level < 0 || compression_level > 9)
	compression_level = 4;
    return xgdStripImagePngCtxPalette (img, out, compression_level);
}

static int
image_to_png_grayscale (const gGraphImagePtr img, void **mem_buf,
			int *mem_buf_size, FILE * file, int dest_type,
			int compression_level, int quantization_factor,
			int interlaced)
{
/* compressing an image as PNG GRAYSCALE */
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
    if (compression_level < 0 || compression_level > 9)
	compression_level = 4;
    ret =
	xgdImagePngCtxGrayscale (img, out, compression_level,
				 quantization_factor, interlaced);
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

static int
image_prepare_to_png_grayscale_by_strip (const gGraphStripImagePtr img,
					 FILE * file, int compression_level,
					 int quantization_factor)
{
/* preparing to compress an image as PNG GRAYSCALE [by strip] */
    xgdIOCtx *out;

/* checkings args for validity */
    if (!file)
	return GGRAPH_ERROR;

    out = xgdNewDynamicCtx (0, file, GG_TARGET_IS_FILE);
    if (compression_level < 0 || compression_level > 9)
	compression_level = 4;
    return xgdStripImagePngCtxGrayscale (img, out, compression_level,
					 quantization_factor);
}

static int
image_to_png_rgb (const gGraphImagePtr img, void **mem_buf, int *mem_buf_size,
		  FILE * file, int dest_type, int compression_level,
		  int quantization_factor, int interlaced)
{
/* compressing an image as PNG RGB */
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
    if (compression_level < 0 || compression_level > 9)
	compression_level = 4;
    ret =
	xgdImagePngCtxRgb (img, out, compression_level, quantization_factor,
			   interlaced);
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

static int
image_prepare_to_png_rgb_by_strip (const gGraphStripImagePtr img, FILE * file,
				   int compression_level,
				   int quantization_factor)
{
/* preparing to compress an image as PNG RGB [by strip] */
    xgdIOCtx *out;

/* checkings args for validity */
    if (!file)
	return GGRAPH_ERROR;

    out = xgdNewDynamicCtx (0, file, GG_TARGET_IS_FILE);
    if (compression_level < 0 || compression_level > 9)
	compression_level = 4;
    return xgdStripImagePngCtxRgb (img, out, compression_level,
				   quantization_factor);
}

static int
image_to_png_rgba (const gGraphImagePtr img, void **mem_buf, int *mem_buf_size,
		   FILE * file, int dest_type, int compression_level,
		   int quantization_factor, int interlaced)
{
/* compressing an image as PNG RGBA */
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
    if (compression_level < 0 || compression_level > 9)
	compression_level = 4;
    ret =
	xgdImagePngCtxRgbAlpha (img, out, compression_level,
				quantization_factor, interlaced);
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

static int
image_prepare_to_png_rgba_by_strip (const gGraphStripImagePtr img, FILE * file,
				    int compression_level,
				    int quantization_factor)
{
/* preparing to compress an image as PNG RGBA [by strip] */
    xgdIOCtx *out;

/* checkings args for validity */
    if (!file)
	return GGRAPH_ERROR;

    out = xgdNewDynamicCtx (0, file, GG_TARGET_IS_FILE);
    if (compression_level < 0 || compression_level > 9)
	compression_level = 4;
    return xgdStripImagePngCtxRgbAlpha (img, out, compression_level,
					quantization_factor);
}

GGRAPH_PRIVATE int
gg_image_to_png (const gGraphImagePtr img, void **mem_buf, int *mem_buf_size,
		 FILE * file, int dest_type, int compression_level,
		 int quantization_factor, int interlaced, int is_transparent)
{
/* dispatching PNG compression */
    if (img->pixel_format == GG_PIXEL_RGBA || img->pixel_format == GG_PIXEL_ARGB
	|| img->pixel_format == GG_PIXEL_BGRA || is_transparent)
	return image_to_png_rgba (img, mem_buf, mem_buf_size, file, dest_type,
				  compression_level, quantization_factor,
				  interlaced);
    if (img->pixel_format == GG_PIXEL_PALETTE)
	return image_to_png_palette (img, mem_buf, mem_buf_size, file,
				     dest_type, compression_level, interlaced);
    if (img->pixel_format == GG_PIXEL_GRAYSCALE)
	return image_to_png_grayscale (img, mem_buf, mem_buf_size, file,
				       dest_type, compression_level,
				       quantization_factor, interlaced);
    return image_to_png_rgb (img, mem_buf, mem_buf_size, file, dest_type,
			     compression_level, quantization_factor,
			     interlaced);
}

GGRAPH_PRIVATE int
gg_image_prepare_to_png_by_strip (const gGraphStripImagePtr img,
				  FILE * file, int compression_level,
				  int quantization_factor)
{
/* dispatching PNG compression [by strip] */
    if (img->pixel_format == GG_PIXEL_PALETTE)
	return image_prepare_to_png_palette_by_strip (img, file,
						      compression_level);
    if (img->pixel_format == GG_PIXEL_GRAYSCALE)
	return image_prepare_to_png_grayscale_by_strip (img, file,
							compression_level,
							quantization_factor);
    if (img->pixel_format == GG_PIXEL_RGBA || img->pixel_format == GG_PIXEL_ARGB
	|| img->pixel_format == GG_PIXEL_BGRA)
	return image_prepare_to_png_rgba_by_strip (img, file, compression_level,
						   quantization_factor);
    return image_prepare_to_png_rgb_by_strip (img, file, compression_level,
					      quantization_factor);
}

GGRAPH_PRIVATE int
gg_image_write_to_png_by_strip (const gGraphStripImagePtr img, int *progress)
{
/* scanline(s) PNG compression [by strip] */
    int ret = xgdStripImagePngCtx (img);
    if (ret == GGRAPH_OK && progress != NULL)
	*progress =
	    (int) (((double) (img->next_row + 1) * 100.0) /
		   (double) (img->height));
    return ret;
}

GGRAPH_PRIVATE int
gg_image_from_png (int size, const void *data, int source_type,
		   gGraphImagePtr * image_handle, int scale)
{
/* uncompressing a PNG */
    int errcode = GGRAPH_OK;
    gGraphImagePtr img;
    xgdIOCtx *in =
	xgdNewDynamicCtxEx (size, data, XGD_CTX_DONT_FREE, source_type);
    img = xgdImageCreateFromPngCtx (in, &errcode, scale);
    in->xgd_free (in);
    *image_handle = img;
    return errcode;
}

GGRAPH_PRIVATE int
gg_image_infos_from_png (int size, const void *data, int source_type,
			 gGraphImageInfosPtr * infos_handle)
{
/* image infos from PNG */
    int errcode = GGRAPH_OK;
    gGraphImageInfosPtr infos;
    xgdIOCtx *in =
	xgdNewDynamicCtxEx (size, data, XGD_CTX_DONT_FREE, source_type);
    infos = xgdImageInfosFromPngCtx (in, &errcode);
    in->xgd_free (in);
    *infos_handle = infos;
    return errcode;
}

GGRAPH_PRIVATE int
gg_image_strip_prepare_from_png (FILE * file,
				 gGraphStripImagePtr * image_handle)
{
/* preparing to uncompress a PNG [by strips] */
    int errcode = GGRAPH_OK;
    gGraphStripImagePtr img;
    xgdIOCtx *in =
	xgdNewDynamicCtxEx (0, file, XGD_CTX_DONT_FREE, GG_TARGET_IS_FILE);
    img = xgdStripImageCreateFromPngCtx (in, &errcode, file);
    *image_handle = img;
    return errcode;
}

GGRAPH_PRIVATE int
gg_image_strip_read_from_png (gGraphStripImagePtr img, int *progress)
{
/* uncompressing a PNG [by strips] */
    int ret = xgdStripImageReadFromPngCtx (img);
    if (ret == GGRAPH_OK && progress != NULL)
	*progress =
	    (int) (((double) (img->next_row + 1) * 100.0) /
		   (double) (img->height));
    return ret;
}
