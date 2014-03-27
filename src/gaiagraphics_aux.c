/* 
/ gaiagraphics_aux.c
/
/ RAW image helpers
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include <tiffio.h>

/*
/ the following patch supporting GeoTiff headers
/ was kindly contributed by Brad Hards: 2011-09-02
*/
#ifdef HAVE_GEOTIFF_GEOTIFF_H
#include <geotiff/geotiff.h>
#include <geotiff/xtiffio.h>
#include <geotiff/geo_normalize.h>
#include <geotiff/geovalues.h>
#elif HAVE_LIBGEOTIFF_GEOTIFF_H
#include <libgeotiff/geotiff.h>
#include <libgeotiff/xtiffio.h>
#include <libgeotiff/geo_normalize.h>
#include <libgeotiff/geovalues.h>
#else
#include <geotiff.h>
#include <xtiffio.h>
#include <geo_normalize.h>
#include <geovalues.h>
#endif

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

GGRAPH_DECLARE const void *
gGraphCreateRgbImage (int width, int height)
{
/* creating a new RGB image */
    return gg_image_create (GG_PIXEL_RGB, width, height, 8, 3,
			    GGRAPH_SAMPLE_UINT, NULL, NULL);
}

GGRAPH_DECLARE const void *
gGraphCreateRgbImageFromBitmap (unsigned char *bitmap, int width, int height)
{
/* creating a new RGB image */
    return gg_image_create_from_bitmap (bitmap, GG_PIXEL_RGB, width, height, 8,
					3, GGRAPH_SAMPLE_UINT, NULL, NULL);
}

GGRAPH_DECLARE const void *
gGraphCreateRgbaImage (int width, int height)
{
/* creating a new RGBA image */
    return gg_image_create (GG_PIXEL_RGBA, width, height, 8, 3,
			    GGRAPH_SAMPLE_UINT, NULL, NULL);
}

GGRAPH_DECLARE const void *
gGraphCreateGrayscaleImage (int width, int height)
{
/* creating a new Grayscale image */
    return gg_image_create (GG_PIXEL_GRAYSCALE, width, height, 8, 1,
			    GGRAPH_SAMPLE_UINT, NULL, NULL);
}

GGRAPH_DECLARE const void *
gGraphCreatePaletteImage (int width, int height)
{
/* creating a new Palette image */
    return gg_image_create (GG_PIXEL_PALETTE, width, height, 8, 1,
			    GGRAPH_SAMPLE_UINT, NULL, NULL);
}

GGRAPH_DECLARE const void *
gGraphCreateMonochromeImage (int width, int height)
{
/* creating a new Monochrome image */
    gGraphImagePtr img = gg_image_create (GG_PIXEL_PALETTE, width, height, 8, 1,
					  GGRAPH_SAMPLE_UINT, NULL, NULL);
    if (!img)
	return NULL;
    img->palette_red[0] = 255;
    img->palette_green[0] = 255;
    img->palette_blue[0] = 255;
    img->palette_red[1] = 0;
    img->palette_green[1] = 0;
    img->palette_blue[1] = 0;
    img->max_palette = 2;
    return img;
}

GGRAPH_DECLARE const void *
gGraphCreateGridInt16Image (int width, int height)
{
/* creating a new GRID-INT16 image */
    return gg_image_create (GG_PIXEL_GRID, width, height, 16, 1,
			    GGRAPH_SAMPLE_INT, NULL, NULL);
}

GGRAPH_DECLARE const void *
gGraphCreateGridUInt16Image (int width, int height)
{
/* creating a new GRID-UINT16 image */
    return gg_image_create (GG_PIXEL_GRID, width, height, 16, 1,
			    GGRAPH_SAMPLE_UINT, NULL, NULL);
}

GGRAPH_DECLARE const void *
gGraphCreateGridInt32Image (int width, int height)
{
/* creating a new GRID-INT32 image */
    return gg_image_create (GG_PIXEL_GRID, width, height, 32, 1,
			    GGRAPH_SAMPLE_INT, NULL, NULL);
}

GGRAPH_DECLARE const void *
gGraphCreateGridUInt32Image (int width, int height)
{
/* creating a new GRID-UINT32 image */
    return gg_image_create (GG_PIXEL_GRID, width, height, 32, 1,
			    GGRAPH_SAMPLE_UINT, NULL, NULL);
}

GGRAPH_DECLARE const void *
gGraphCreateGridFloatImage (int width, int height)
{
/* creating a new GRID-FLOAT image */
    return gg_image_create (GG_PIXEL_GRID, width, height, 32, 1,
			    GGRAPH_SAMPLE_FLOAT, NULL, NULL);
}

GGRAPH_DECLARE const void *
gGraphCreateGridDoubleImage (int width, int height)
{
/* creating a new GRID-DOUBLE image */
    return gg_image_create (GG_PIXEL_GRID, width, height, 64, 1,
			    GGRAPH_SAMPLE_FLOAT, NULL, NULL);
}

static unsigned char
to_grayscale2 (unsigned char r, unsigned char g, unsigned char b)
{
/* computing a grayscale value */
    double dval =
	(0.3 * (double) r) + (0.59 * (double) g) + (0.11 * (double) b);
    int ival = (int) dval;
    if (ival < 0)
	ival = 0;
    if (ival > 255)
	ival = 255;
    return (unsigned char) ival;
}

GGRAPH_DECLARE int
gGraphImageBackgroundFill (const void *ptr, unsigned char red,
			   unsigned char green, unsigned char blue,
			   unsigned char alpha)
{
/* filling the image background */
    int x;
    int y;
    unsigned char *p_out;
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    for (y = 0; y < img->height; y++)
      {
	  p_out = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
		if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		      *p_out++ = alpha;
		  }
		if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      *p_out++ = alpha;
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
		if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      *p_out++ = blue;
		      *p_out++ = green;
		      *p_out++ = red;
		  }
		if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      *p_out++ = blue;
		      *p_out++ = green;
		      *p_out++ = red;
		      *p_out++ = alpha;
		  }
		if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      int gray;
		      if (red == green && green == blue)
			  gray = red;
		      else
			  gray = to_grayscale2 (red, green, blue);
		      *p_out++ = gray;
		  }
		if (img->pixel_format == GG_PIXEL_PALETTE)
		    *p_out++ = 0;
	    }
      }
    if (img->pixel_format == GG_PIXEL_PALETTE)
      {
	  /* resetting an empty palette */
	  img->max_palette = 1;
	  img->palette_red[0] = red;
	  img->palette_green[0] = green;
	  img->palette_blue[0] = blue;
      }
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageSetTransparentColor (const void *ptr, unsigned char red,
				unsigned char green, unsigned char blue)
{
/* setting the image's transparent color */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    img->transparent_red = red;
    img->transparent_green = green;
    img->transparent_blue = blue;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageGetTransparentColor (const void *ptr, unsigned char *red,
				unsigned char *green, unsigned char *blue)
{
/* querying the image's transparent color */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    *red = img->transparent_red;
    *green = img->transparent_green;
    *blue = img->transparent_blue;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphGridBackgroundFill (const void *ptr, double no_data_value)
{
/* filling the image (GRID) background */
    int x;
    int y;
    unsigned char *p_out;
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    for (y = 0; y < img->height; y++)
      {
	  p_out = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		if (img->pixel_format == GG_PIXEL_GRID)
		  {
		      switch (img->sample_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img->bits_per_sample == 16)
			      {
				  short *p = (short *) p_out;
				  *p = (short) no_data_value;
				  p_out += sizeof (short);
			      }
			    else
			      {
				  int *p = (int *) p_out;
				  *p = (int) no_data_value;
				  p_out += sizeof (int);
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_out;
				  *p = (unsigned short) no_data_value;
				  p_out += sizeof (unsigned short);
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_out;
				  *p = (unsigned int) no_data_value;
				  p_out += sizeof (unsigned int);
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img->bits_per_sample == 32)
			      {
				  float *p = (float *) p_out;
				  *p = (float) no_data_value;
				  p_out += sizeof (float);
			      }
			    else
			      {
				  double *p = (double *) p_out;
				  *p = no_data_value;
				  p_out += sizeof (double);
			      }
			    break;
			};
		  }
	    }
      }
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageSetNoDataValue (const void *ptr, double no_data_value)
{
/* setting the image's NoDataValue */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    img->no_data_value = no_data_value;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageGetNoDataValue (const void *ptr, double *no_data_value)
{
/* querying the image's NoDataValue */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    *no_data_value = img->no_data_value;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageInfosFromMemBuf (const void *mem_buf, int mem_buf_size,
			    int image_type, const void **infos_handle)
{
/* retrieving Image infos from a memory block containing an encoded image */
    gGraphImageInfosPtr infos = NULL;
    int ret;

    *infos_handle = NULL;
    switch (image_type)
      {
      case GGRAPH_IMAGE_GIF:
	  ret =
	      gg_image_infos_from_gif (mem_buf_size, mem_buf,
				       GG_TARGET_IS_MEMORY, &infos);
	  break;
      case GGRAPH_IMAGE_PNG:
	  ret =
	      gg_image_infos_from_png (mem_buf_size, mem_buf,
				       GG_TARGET_IS_MEMORY, &infos);
	  break;
      case GGRAPH_IMAGE_JPEG:
	  ret =
	      gg_image_infos_from_jpeg (mem_buf_size, mem_buf,
					GG_TARGET_IS_MEMORY, &infos);
	  break;
      case GGRAPH_IMAGE_TIFF:
      case GGRAPH_IMAGE_GEOTIFF:
	  ret = gg_image_infos_from_mem_tiff (mem_buf_size, mem_buf, &infos);
	  break;
      }
    if (ret != GGRAPH_OK)
	return ret;

    *infos_handle = infos;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageInfosFromFile (const char *path, int image_type,
			  const void **infos_handle)
{
/* retrieving Image infos from file */
    FILE *in = NULL;
    gGraphImageInfosPtr infos = NULL;
    int ret;

    *infos_handle = NULL;

/* attempting to open the image file */
    if (image_type == GGRAPH_IMAGE_TIFF || image_type == GGRAPH_IMAGE_GEOTIFF)
	;
    else
      {
	  in = fopen (path, "rb");
	  if (in == NULL)
	      return GGRAPH_FILE_OPEN_ERROR;
      }
    switch (image_type)
      {
      case GGRAPH_IMAGE_GIF:
	  ret = gg_image_infos_from_gif (0, in, GG_TARGET_IS_FILE, &infos);
	  break;
      case GGRAPH_IMAGE_PNG:
	  ret = gg_image_infos_from_png (0, in, GG_TARGET_IS_FILE, &infos);
	  break;
      case GGRAPH_IMAGE_JPEG:
	  ret = gg_image_infos_from_jpeg (0, in, GG_TARGET_IS_FILE, &infos);
	  break;
      case GGRAPH_IMAGE_TIFF:
	  ret = gg_image_infos_from_tiff (path, &infos);
	  break;
      case GGRAPH_IMAGE_GEOTIFF:
	  ret = gg_image_infos_from_geo_tiff (path, &infos);
	  break;
      };
    if (in)
	fclose (in);
    if (ret != GGRAPH_OK)
	return ret;

    *infos_handle = infos;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromMemBuf (const void *mem_buf, int mem_buf_size, int image_type,
		       const void **image_handle, int scale)
{
/* decompressing a memory block containing an encoded image */
    gGraphImagePtr img = NULL;
    int ret;

    *image_handle = NULL;
    switch (image_type)
      {
      case GGRAPH_IMAGE_GIF:
	  ret =
	      gg_image_from_gif (mem_buf_size, mem_buf, GG_TARGET_IS_MEMORY,
				 &img);
	  break;
      case GGRAPH_IMAGE_PNG:
	  ret =
	      gg_image_from_png (mem_buf_size, mem_buf, GG_TARGET_IS_MEMORY,
				 &img, scale);
	  break;
      case GGRAPH_IMAGE_JPEG:
	  ret =
	      gg_image_from_jpeg (mem_buf_size, mem_buf, GG_TARGET_IS_MEMORY,
				  &img, scale);
	  break;
      case GGRAPH_IMAGE_TIFF:
      case GGRAPH_IMAGE_GEOTIFF:
	  ret = gg_image_from_mem_tiff (mem_buf_size, mem_buf, &img);
	  break;
      }
    if (ret != GGRAPH_OK)
	return ret;

    *image_handle = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromFile (const char *path, int image_type,
		     const void **image_handle, int scale)
{
/* reading an image from file */
    FILE *in = NULL;
    gGraphImagePtr img = NULL;
    int ret;

    *image_handle = NULL;

/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;
    switch (image_type)
      {
      case GGRAPH_IMAGE_GIF:
	  ret = gg_image_from_gif (0, in, GG_TARGET_IS_FILE, &img);
	  break;
      case GGRAPH_IMAGE_PNG:
	  ret = gg_image_from_png (0, in, GG_TARGET_IS_FILE, &img, scale);
	  break;
      case GGRAPH_IMAGE_JPEG:
	  ret = gg_image_from_jpeg (0, in, GG_TARGET_IS_FILE, &img, scale);
	  break;
      };
    fclose (in);
    if (ret != GGRAPH_OK)
	return ret;

    *image_handle = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromFileByStrips (const char *path, int image_type,
			     const void **image_handle)
{
/* reading an image from file [by strips] */
    FILE *in = NULL;
    gGraphStripImagePtr img = NULL;
    int ret;

    *image_handle = NULL;

/* attempting to open the image file */
    if (image_type == GGRAPH_IMAGE_TIFF || image_type == GGRAPH_IMAGE_GEOTIFF)
	;
    else
      {
	  in = fopen (path, "rb");
	  if (in == NULL)
	      return GGRAPH_FILE_OPEN_ERROR;
      }
    switch (image_type)
      {
      case GGRAPH_IMAGE_PNG:
	  ret = gg_image_strip_prepare_from_png (in, &img);
	  break;
      case GGRAPH_IMAGE_JPEG:
	  ret = gg_image_strip_prepare_from_jpeg (in, &img);
	  break;
      case GGRAPH_IMAGE_TIFF:
	  ret = gg_image_strip_prepare_from_tiff (path, &img);
	  break;
      case GGRAPH_IMAGE_GEOTIFF:
	  ret = gg_image_strip_prepare_from_geotiff (path, &img);
	  break;
      };
    if (ret != GGRAPH_OK)
      {
	  if (in)
	      fclose (in);
	  return ret;
      }

    *image_handle = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromHgtFileByStrips (const char *path, int lat, int lon,
				const void **image_handle)
{
/* reading an HGT GRID from file [by strips] */
    FILE *in = NULL;
    gGraphStripImagePtr img = NULL;
    int ret;

    *image_handle = NULL;

/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;
    ret = gg_image_strip_prepare_from_hgt (in, lat, lon, &img);
    if (ret != GGRAPH_OK)
      {
	  if (in)
	      fclose (in);
	  return ret;
      }

    *image_handle = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromBinFileByStrips (const char *path, const char *hdr_path,
				const void **image_handle)
{
/* reading a BIN GRID from file [by strips] */
    FILE *in = NULL;
    gGraphStripImagePtr img = NULL;
    int ret;

    *image_handle = NULL;

/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;
    ret = gg_image_strip_prepare_from_bin_hdr (in, hdr_path, &img);
    if (ret != GGRAPH_OK)
      {
	  if (in)
	      fclose (in);
	  return ret;
      }

    *image_handle = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromFltFileByStrips (const char *path, const char *hdr_path,
				const void **image_handle)
{
/* reading a FLT GRID from file [by strips] */
    FILE *in = NULL;
    gGraphStripImagePtr img = NULL;
    int ret;

    *image_handle = NULL;

/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;
    ret = gg_image_strip_prepare_from_flt_hdr (in, hdr_path, &img);
    if (ret != GGRAPH_OK)
      {
	  if (in)
	      fclose (in);
	  return ret;
      }

    *image_handle = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromDemFileByStrips (const char *path, const char *hdr_path,
				const void **image_handle)
{
/* reading a DEM GRID from file [by strips] */
    FILE *in = NULL;
    gGraphStripImagePtr img = NULL;
    int ret;

    *image_handle = NULL;

/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;
    ret = gg_image_strip_prepare_from_dem_hdr (in, hdr_path, &img);
    if (ret != GGRAPH_OK)
      {
	  if (in)
	      fclose (in);
	  return ret;
      }

    *image_handle = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromAscFileByStrips (const char *path, const void **image_handle)
{
/* reading a ASCII GRID from file [by strips] */
    FILE *in = NULL;
    gGraphStripImagePtr img = NULL;
    int ret;

    *image_handle = NULL;

/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;
    ret = gg_image_strip_prepare_from_ascii_grid (in, &img);
    if (ret != GGRAPH_OK)
      {
	  if (in)
	      fclose (in);
	  return ret;
      }

    *image_handle = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphCountColors (const char *path, int image_type, int rows_per_block)
{
/* attempting to count how many different colors are into the given image file */
    FILE *in = NULL;
    gGraphStripImagePtr img = NULL;
    int num_colors = 0;
    int ret;
    int x;
    int y;
    unsigned char *count_array = NULL;
    int index;

/* attempting to open the image file */
    if (image_type == GGRAPH_IMAGE_TIFF || image_type == GGRAPH_IMAGE_GEOTIFF)
	;
    else
      {
	  in = fopen (path, "rb");
	  if (in == NULL)
	      return GGRAPH_FILE_OPEN_ERROR;
      }
    switch (image_type)
      {
      case GGRAPH_IMAGE_PNG:
	  ret = gg_image_strip_prepare_from_png (in, &img);
	  break;
      case GGRAPH_IMAGE_JPEG:
	  ret = gg_image_strip_prepare_from_jpeg (in, &img);
	  break;
      case GGRAPH_IMAGE_TIFF:
	  ret = gg_image_strip_prepare_from_tiff (path, &img);
	  break;
      case GGRAPH_IMAGE_GEOTIFF:
	  ret = gg_image_strip_prepare_from_geotiff (path, &img);
	  break;
      };
    if (ret != GGRAPH_OK)
      {
	  if (in)
	      fclose (in);
	  return 0;
      }

/* creating the input buffer */
    if (gGraphStripImageAllocPixels (img, rows_per_block) != GGRAPH_OK)
      {
	  gGraphDestroyImage (img);
	  return 0;
      }

/* creating the count array */
    count_array = malloc (256 * 256 * 256);
    if (!count_array)
      {
	  gGraphDestroyImage (img);
	  return 0;
      }
    for (index = 0; index < (256 * 256 * 256); index++)
	count_array[index] = 0;

    while (1)
      {
	  /* inspecting the image pixels */
	  if (gGraphStripImageEOF (img) == GGRAPH_OK)
	      goto done;
	  if (gGraphReadNextStrip (img, NULL) != GGRAPH_OK)
	      goto error;
	  for (y = 0; y < img->current_available_rows; y++)
	    {
		unsigned char *p_in = img->pixels + (y * img->scanline_width);
		for (x = 0; x < img->width; x++)
		  {
		      unsigned char red;
		      unsigned char green;
		      unsigned char blue;
		      if (img->pixel_format == GG_PIXEL_RGB)
			{
			    red = *p_in++;
			    green = *p_in++;
			    blue = *p_in++;
			}
		      else if (img->pixel_format == GG_PIXEL_RGBA)
			{
			    red = *p_in++;
			    green = *p_in++;
			    blue = *p_in++;
			    p_in++;	/* skipping Alpha */
			}
		      else if (img->pixel_format == GG_PIXEL_RGB)
			{
			    p_in++;	/* skipping Alpha */
			    red = *p_in++;
			    green = *p_in++;
			    blue = *p_in++;
			}
		      else if (img->pixel_format == GG_PIXEL_BGR)
			{
			    blue = *p_in++;
			    green = *p_in++;
			    red = *p_in++;
			}
		      else if (img->pixel_format == GG_PIXEL_BGRA)
			{
			    blue = *p_in++;
			    green = *p_in++;
			    red = *p_in++;
			    p_in++;	/* skipping Alpha */
			}
		      else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
			{
			    red = *p_in++;
			    green = red;
			    blue = red;
			}
		      else if (img->pixel_format == GG_PIXEL_PALETTE)
			{
			    index = *p_in++;
			    red = img->palette_red[index];
			    green = img->palette_green[index];
			    blue = img->palette_blue[index];
			}
		      index = (red * 256 * 256) + (green * 256) + blue;
		      count_array[index] = 1;
		  }
	    }
      }

  error:
    gGraphDestroyImage (img);
    if (!count_array)
	free (count_array);
    return 0;

  done:
    for (index = 0; index < (256 * 256 * 256); index++)
      {
	  if (count_array[index])
	      num_colors++;
      }
    gGraphDestroyImage (img);
    if (!count_array)
	free (count_array);
    return num_colors;
}

GGRAPH_DECLARE int
gGraphImageToJpegFile (const void *ptr, const char *path, int quality)
{
/* exporting an image into a JPEG compressed file */
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    int ret;
    FILE *out = NULL;

    if (!img)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

/* opening the output image file */
    out = fopen (path, "wb");
    if (out == NULL)
	return GGRAPH_FILE_OPEN_ERROR;
/* compressing as JPEG */
    ret = gg_image_to_jpeg (img, NULL, NULL, out, GG_TARGET_IS_FILE, quality);
    fclose (out);
    if (ret != GGRAPH_OK)
      {
	  /* some unexpected error occurred */
	  unlink (path);
	  return ret;
      }

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToJpegFileByStrips (const void **ptr, const char *path, int width,
			       int height, int color_model, int quality)
{
/* exporting an image into a JPEG compressed file [by strips] */
    gGraphStripImagePtr img;
    int ret;
    FILE *out = NULL;

    *ptr = NULL;
    if (color_model == GGRAPH_COLORSPACE_GRAYSCALE
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR)
	;
    else
	return GGRAPH_INVALID_IMAGE;

/* opening the output image file */
    out = fopen (path, "wb");
    if (out == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

/* creating a Strip Image */
    if (color_model == GGRAPH_COLORSPACE_GRAYSCALE)
      {
	  img =
	      gg_strip_image_create (out, GGRAPH_IMAGE_JPEG, GG_PIXEL_GRAYSCALE,
				     width, height, 8, 1, GGRAPH_SAMPLE_UINT,
				     NULL, NULL);
	  if (!img)
	    {
		fclose (out);
		unlink (path);
		return GGRAPH_INSUFFICIENT_MEMORY;
	    }
      }
    if (color_model == GGRAPH_COLORSPACE_TRUECOLOR)
      {
	  img =
	      gg_strip_image_create (out, GGRAPH_IMAGE_JPEG, GG_PIXEL_RGB,
				     width, height, 8, 3, GGRAPH_SAMPLE_UINT,
				     NULL, NULL);
	  if (!img)
	    {
		fclose (out);
		unlink (path);
		return GGRAPH_INSUFFICIENT_MEMORY;
	    }
      }

    ret = gg_image_prepare_to_jpeg_by_strip (img, out, quality);
    if (ret != GGRAPH_OK)
      {
	  gg_strip_image_destroy (img);
	  return ret;
      }

    *ptr = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToJpegMemBuf (const void *ptr, void **mem_buf, int *mem_buf_size,
			 int quality)
{
/* exporting an image into a JPEG compressed memory buffer */
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    void *buf = NULL;
    int size;
    int ret;

    *mem_buf = NULL;
    *mem_buf_size = 0;
    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

/* compressing as JPEG */
    ret =
	gg_image_to_jpeg (img, &buf, &size, NULL, GG_TARGET_IS_MEMORY, quality);
    if (ret != GGRAPH_OK)
	return ret;

/* exporting the memory buffer */
    *mem_buf = buf;
    *mem_buf_size = size;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToPngFile (const void *ptr, const char *path, int compression_level,
		      int quantization_factor, int interlaced)
{
/* exporting an image into a PNG compressed file */
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    int ret;
    FILE *out = NULL;

    if (!img)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

/* opening the output image file */
    out = fopen (path, "wb");
    if (out == NULL)
	return GGRAPH_FILE_OPEN_ERROR;
/* compressing as PNG */
    ret =
	gg_image_to_png (img, NULL, NULL, out, GG_TARGET_IS_FILE,
			 compression_level, quantization_factor, interlaced, 0);
    fclose (out);
    if (ret != GGRAPH_OK)
      {
	  /* some unexpected error occurred */
	  unlink (path);
	  return ret;
      }

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToPngFileByStrips (const void **ptr, const char *path, int width,
			      int height, int color_model, int bits_per_sample,
			      int num_palette, unsigned char *red,
			      unsigned char *green, unsigned char *blue,
			      int compression_level, int quantization_factor)
{
/* exporting an image into a PNG compressed file [by strips] */
    gGraphStripImagePtr img;
    int ret;
    int i;
    FILE *out = NULL;

    *ptr = NULL;
    if (color_model == GGRAPH_COLORSPACE_PALETTE
	|| color_model == GGRAPH_COLORSPACE_GRAYSCALE
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR_ALPHA)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (color_model == GGRAPH_COLORSPACE_PALETTE)
      {
	  if (num_palette < 1)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 1 || bits_per_sample == 2
	      || bits_per_sample == 4 || bits_per_sample == 8)
	      ;
	  else
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 1 && num_palette > 2)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 2 && num_palette > 4)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 4 && num_palette > 16)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 8 && num_palette > 256)
	      return GGRAPH_INVALID_IMAGE;
      }
    if (color_model == GGRAPH_COLORSPACE_GRAYSCALE)
      {
	  if (bits_per_sample == 1 || bits_per_sample == 2
	      || bits_per_sample == 4 || bits_per_sample == 8)
	      ;
	  else
	      return GGRAPH_INVALID_IMAGE;
      }

/* opening the output image file */
    out = fopen (path, "wb");
    if (out == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

/* creating a Strip Image */
    if (color_model == GGRAPH_COLORSPACE_PALETTE)
      {
	  img =
	      gg_strip_image_create (out, GGRAPH_IMAGE_PNG, GG_PIXEL_PALETTE,
				     width, height, bits_per_sample, 1,
				     GGRAPH_SAMPLE_UINT, NULL, NULL);
	  if (!img)
	    {
		fclose (out);
		unlink (path);
		return GGRAPH_INSUFFICIENT_MEMORY;
	    }
	  for (i = 0; i < num_palette; i++)
	    {
		img->palette_red[i] = red[i];
		img->palette_green[i] = green[i];
		img->palette_blue[i] = blue[i];
		img->max_palette = i + 1;
	    }
      }
    if (color_model == GGRAPH_COLORSPACE_GRAYSCALE)
      {
	  img =
	      gg_strip_image_create (out, GGRAPH_IMAGE_PNG, GG_PIXEL_GRAYSCALE,
				     width, height, bits_per_sample, 1,
				     GGRAPH_SAMPLE_UINT, NULL, NULL);
	  if (!img)
	    {
		fclose (out);
		unlink (path);
		return GGRAPH_INSUFFICIENT_MEMORY;
	    }
      }
    if (color_model == GGRAPH_COLORSPACE_TRUECOLOR)
      {
	  img =
	      gg_strip_image_create (out, GGRAPH_IMAGE_PNG, GG_PIXEL_RGB, width,
				     height, 8, 3, GGRAPH_SAMPLE_UINT, NULL,
				     NULL);
	  if (!img)
	    {
		fclose (out);
		unlink (path);
		return GGRAPH_INSUFFICIENT_MEMORY;
	    }
      }
    if (color_model == GGRAPH_COLORSPACE_TRUECOLOR_ALPHA)
      {
	  img =
	      gg_strip_image_create (out, GGRAPH_IMAGE_PNG, GG_PIXEL_RGBA,
				     width, height, 8, 4, GGRAPH_SAMPLE_UINT,
				     NULL, NULL);
	  if (!img)
	    {
		fclose (out);
		unlink (path);
		return GGRAPH_INSUFFICIENT_MEMORY;
	    }
      }

    ret =
	gg_image_prepare_to_png_by_strip (img, out, compression_level,
					  quantization_factor);
    if (ret != GGRAPH_OK)
      {
	  gg_strip_image_destroy (img);
	  return ret;
      }

    *ptr = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToTiffFileByStrips (const void **ptr, const char *path,
			       int width, int height, int color_model,
			       int tiff_layout,
			       int tile_width,
			       int tile_height,
			       int rows_per_strip,
			       int bits_per_sample, int sample_format,
			       int num_palette,
			       unsigned char *red, unsigned char *green,
			       unsigned char *blue, int compression)
{
/* exporting an image into a TIFF compressed file [by strips] */
    gGraphStripImagePtr img;
    int ret;
    int i;

    *ptr = NULL;
    if (color_model == GGRAPH_COLORSPACE_PALETTE
	|| color_model == GGRAPH_COLORSPACE_GRAYSCALE
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR_ALPHA
	|| color_model == GGRAPH_COLORSPACE_GRID)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (color_model == GGRAPH_COLORSPACE_PALETTE)
      {
	  if (num_palette < 1)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 1 || bits_per_sample == 2
	      || bits_per_sample == 4 || bits_per_sample == 8)
	      ;
	  else
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 1 && num_palette > 2)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 2 && num_palette > 4)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 4 && num_palette > 16)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 8 && num_palette > 256)
	      return GGRAPH_INVALID_IMAGE;
      }
    if (color_model == GGRAPH_COLORSPACE_GRID)
      {
	  if (sample_format == GGRAPH_SAMPLE_UINT
	      || sample_format == GGRAPH_SAMPLE_INT)
	    {
		if (bits_per_sample == 8 || bits_per_sample == 16
		    || bits_per_sample == 32)
		    ;
		else
		    return GGRAPH_INVALID_IMAGE;
	    }
	  else if (GGRAPH_SAMPLE_FLOAT)
	    {
		if (bits_per_sample == 32 || bits_per_sample == 64)
		    ;
		else
		    return GGRAPH_INVALID_IMAGE;
	    }
	  else
	      return GGRAPH_INVALID_IMAGE;
      }

/* creating a Strip Image */
    if (color_model == GGRAPH_COLORSPACE_PALETTE)
      {
	  img =
	      gg_strip_image_create (NULL, GGRAPH_IMAGE_TIFF, GG_PIXEL_PALETTE,
				     width, height, bits_per_sample, 1,
				     GGRAPH_SAMPLE_UINT, NULL, NULL);
	  if (!img)
	      return GGRAPH_INSUFFICIENT_MEMORY;
	  for (i = 0; i < num_palette; i++)
	    {
		img->palette_red[i] = red[i];
		img->palette_green[i] = green[i];
		img->palette_blue[i] = blue[i];
		img->max_palette = i + 1;
	    }
      }
    if (color_model == GGRAPH_COLORSPACE_GRAYSCALE)
      {
	  img =
	      gg_strip_image_create (NULL, GGRAPH_IMAGE_TIFF,
				     GG_PIXEL_GRAYSCALE, width, height,
				     bits_per_sample, 1, GGRAPH_SAMPLE_UINT,
				     NULL, NULL);
	  if (!img)
	      return GGRAPH_INSUFFICIENT_MEMORY;
      }
    if (color_model == GGRAPH_COLORSPACE_TRUECOLOR
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR_ALPHA)
      {
	  img =
	      gg_strip_image_create (NULL, GGRAPH_IMAGE_TIFF, GG_PIXEL_RGB,
				     width, height, 8, 3, GGRAPH_SAMPLE_UINT,
				     NULL, NULL);
	  if (!img)
	      return GGRAPH_INSUFFICIENT_MEMORY;
      }
    if (color_model == GGRAPH_COLORSPACE_GRID)
      {
	  img =
	      gg_strip_image_create (NULL, GGRAPH_IMAGE_TIFF, GG_PIXEL_GRID,
				     width, height, bits_per_sample, 1,
				     sample_format, NULL, NULL);
	  if (!img)
	      return GGRAPH_INSUFFICIENT_MEMORY;
      }

    img->tile_width = tile_width;
    img->tile_height = tile_height;
    img->rows_per_strip = rows_per_strip;

    ret =
	gg_image_prepare_to_tiff_by_strip (img, path, tiff_layout, tile_width,
					   tile_height, rows_per_strip,
					   color_model, bits_per_sample,
					   sample_format, num_palette, red,
					   green, blue, compression);
    if (ret != GGRAPH_OK)
      {
	  gg_strip_image_destroy (img);
	  return ret;
      }

    *ptr = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToGeoTiffFileByStrips (const void **ptr, const char *path,
				  int width, int height, int color_model,
				  int tiff_layout,
				  int tile_width,
				  int tile_height,
				  int rows_per_strip,
				  int bits_per_sample, int sample_format,
				  int num_palette,
				  unsigned char *red, unsigned char *green,
				  unsigned char *blue, int compression,
				  int srid, const char *srs_name,
				  const char *proj4text, double upper_left_x,
				  double upper_left_y, double pixel_x_size,
				  double pixel_y_size)
{
/* exporting an image into a GeoTIFF compressed file [by strips] */
    gGraphStripImagePtr img;
    int ret;
    int i;

    *ptr = NULL;
    if (color_model == GGRAPH_COLORSPACE_PALETTE
	|| color_model == GGRAPH_COLORSPACE_GRAYSCALE
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR_ALPHA
	|| color_model == GGRAPH_COLORSPACE_GRID)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (color_model == GGRAPH_COLORSPACE_PALETTE)
      {
	  if (num_palette < 1)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 1 || bits_per_sample == 2
	      || bits_per_sample == 4 || bits_per_sample == 8)
	      ;
	  else
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 1 && num_palette > 2)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 2 && num_palette > 4)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 4 && num_palette > 16)
	      return GGRAPH_INVALID_IMAGE;
	  if (bits_per_sample == 8 && num_palette > 256)
	      return GGRAPH_INVALID_IMAGE;
      }
    if (color_model == GGRAPH_COLORSPACE_GRID)
      {
	  if (sample_format == GGRAPH_SAMPLE_UINT
	      || sample_format == GGRAPH_SAMPLE_INT)
	    {
		if (bits_per_sample == 8 || bits_per_sample == 16
		    || bits_per_sample == 32)
		    ;
		else
		    return GGRAPH_INVALID_IMAGE;
	    }
	  else if (GGRAPH_SAMPLE_FLOAT)
	    {
		if (bits_per_sample == 32 || bits_per_sample == 64)
		    ;
		else
		    return GGRAPH_INVALID_IMAGE;
	    }
	  else
	      return GGRAPH_INVALID_IMAGE;
      }

/* creating a Strip Image */
    if (color_model == GGRAPH_COLORSPACE_PALETTE)
      {
	  img =
	      gg_strip_image_create (NULL, GGRAPH_IMAGE_GEOTIFF,
				     GG_PIXEL_PALETTE, width, height,
				     bits_per_sample, 1, GGRAPH_SAMPLE_UINT,
				     NULL, NULL);
	  if (!img)
	      return GGRAPH_INSUFFICIENT_MEMORY;
	  for (i = 0; i < num_palette; i++)
	    {
		img->palette_red[i] = red[i];
		img->palette_green[i] = green[i];
		img->palette_blue[i] = blue[i];
		img->max_palette = i + 1;
	    }
      }
    if (color_model == GGRAPH_COLORSPACE_GRAYSCALE)
      {
	  img =
	      gg_strip_image_create (NULL, GGRAPH_IMAGE_GEOTIFF,
				     GG_PIXEL_GRAYSCALE, width, height,
				     bits_per_sample, 1, GGRAPH_SAMPLE_UINT,
				     NULL, NULL);
	  if (!img)
	      return GGRAPH_INSUFFICIENT_MEMORY;
      }
    if (color_model == GGRAPH_COLORSPACE_TRUECOLOR
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR_ALPHA)
      {
	  img =
	      gg_strip_image_create (NULL, GGRAPH_IMAGE_GEOTIFF, GG_PIXEL_RGB,
				     width, height, 8, 3, GGRAPH_SAMPLE_UINT,
				     NULL, NULL);
	  if (!img)
	      return GGRAPH_INSUFFICIENT_MEMORY;
      }
    if (color_model == GGRAPH_COLORSPACE_GRID)
      {
	  img =
	      gg_strip_image_create (NULL, GGRAPH_IMAGE_GEOTIFF, GG_PIXEL_GRID,
				     width, height, bits_per_sample, 1,
				     sample_format, NULL, NULL);
	  if (!img)
	      return GGRAPH_INSUFFICIENT_MEMORY;
      }
    gGraphImageSetGeoRef (img, srid, srs_name, proj4text, upper_left_x,
			  upper_left_y, pixel_x_size, pixel_y_size);

    img->tile_width = tile_width;
    img->tile_height = tile_height;
    img->rows_per_strip = rows_per_strip;

    ret =
	gg_image_prepare_to_geotiff_by_strip (img, path, tiff_layout,
					      tile_width, tile_height,
					      rows_per_strip, color_model,
					      bits_per_sample, sample_format,
					      num_palette, red, green, blue,
					      compression);
    if (ret != GGRAPH_OK)
      {
	  gg_strip_image_destroy (img);
	  return ret;
      }

    *ptr = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToBinHdrFileByStrips (const void **ptr, const char *path, int width,
				 int height, int bits_per_sample,
				 double upper_left_x, double upper_left_y,
				 double pixel_x_size, double pixel_y_size,
				 double no_data_value)
{
/* exporting an image into a BIN+HDR GRID file [by strips] */
    gGraphStripImagePtr img = NULL;
    int ret;
    FILE *out = NULL;

    *ptr = NULL;

/* opening the output image file */
    out = fopen (path, "wb");
    if (out == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

/* creating a Strip Image */
    if (bits_per_sample == 16)
      {
	  img = gg_strip_image_create (out, GGRAPH_IMAGE_BIN_HDR, GG_PIXEL_GRID,
				       width, height, 16, 1, GGRAPH_SAMPLE_INT,
				       NULL, NULL);
      }
    if (bits_per_sample == 32)
      {
	  img = gg_strip_image_create (out, GGRAPH_IMAGE_BIN_HDR, GG_PIXEL_GRID,
				       width, height, 32, 1, GGRAPH_SAMPLE_INT,
				       NULL, NULL);
      }
    if (!img)
      {
	  fclose (out);
	  unlink (path);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }

    img->upper_left_x = upper_left_x;
    img->upper_left_y = upper_left_y;
    img->pixel_x_size = pixel_x_size;
    img->pixel_y_size = pixel_y_size;
    img->no_data_value = no_data_value;
    ret = gg_image_prepare_to_bin_hdr_by_strip (img);
    if (ret != GGRAPH_OK)
	if (ret != GGRAPH_OK)
	  {
	      gg_strip_image_destroy (img);
	      return ret;
	  }

    *ptr = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToFltHdrFileByStrips (const void **ptr, const char *path, int width,
				 int height, int bits_per_sample,
				 double upper_left_x, double upper_left_y,
				 double pixel_x_size, double pixel_y_size,
				 double no_data_value)
{
/* exporting an image into a FLT+HDR GRID file [by strips] */
    gGraphStripImagePtr img = NULL;
    int ret;
    FILE *out = NULL;

    *ptr = NULL;

/* opening the output image file */
    out = fopen (path, "wb");
    if (out == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

/* creating a Strip Image */
    if (bits_per_sample == 32)
      {
	  img = gg_strip_image_create (out, GGRAPH_IMAGE_FLT_HDR, GG_PIXEL_GRID,
				       width, height, 32, 1,
				       GGRAPH_SAMPLE_FLOAT, NULL, NULL);
      }
    if (bits_per_sample == 64)
      {
	  img = gg_strip_image_create (out, GGRAPH_IMAGE_FLT_HDR, GG_PIXEL_GRID,
				       width, height, 64, 1,
				       GGRAPH_SAMPLE_FLOAT, NULL, NULL);
      }
    if (!img)
      {
	  fclose (out);
	  unlink (path);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }

    img->upper_left_x = upper_left_x;
    img->upper_left_y = upper_left_y;
    img->pixel_x_size = pixel_x_size;
    img->pixel_y_size = pixel_y_size;
    img->no_data_value = no_data_value;
    ret = gg_image_prepare_to_flt_hdr_by_strip (img);
    if (ret != GGRAPH_OK)
      {
	  gg_strip_image_destroy (img);
	  return ret;
      }

    *ptr = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToAscFileByStrips (const void **ptr, const char *path, int width,
			      int height, int sample, int bits_per_sample,
			      double upper_left_x, double upper_left_y,
			      double pixel_x_size, double pixel_y_size,
			      double no_data_value)
{
/* exporting an image into an ASCII GRID file [by strips] */
    gGraphStripImagePtr img = NULL;
    int ret;
    FILE *out = NULL;

    *ptr = NULL;

/* opening the output image file */
    out = fopen (path, "wb");
    if (out == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

/* creating a Strip Image */
    if (sample == GGRAPH_SAMPLE_UINT)
      {
	  if (bits_per_sample == 8)
	    {
		img =
		    gg_strip_image_create (out, GGRAPH_IMAGE_ASCII_GRID,
					   GG_PIXEL_GRID, width, height, 8, 1,
					   GGRAPH_SAMPLE_UINT, NULL, NULL);
	    }
	  if (bits_per_sample == 16)
	    {
		img =
		    gg_strip_image_create (out, GGRAPH_IMAGE_ASCII_GRID,
					   GG_PIXEL_GRID, width, height, 16, 1,
					   GGRAPH_SAMPLE_UINT, NULL, NULL);
	    }
	  if (bits_per_sample == 32)
	    {
		img =
		    gg_strip_image_create (out, GGRAPH_IMAGE_ASCII_GRID,
					   GG_PIXEL_GRID, width, height, 32, 1,
					   GGRAPH_SAMPLE_UINT, NULL, NULL);
	    }
      }
    if (sample == GGRAPH_SAMPLE_INT)
      {
	  if (bits_per_sample == 8)
	    {
		img =
		    gg_strip_image_create (out, GGRAPH_IMAGE_ASCII_GRID,
					   GG_PIXEL_GRID, width, height, 8, 1,
					   GGRAPH_SAMPLE_INT, NULL, NULL);
	    }
	  if (bits_per_sample == 16)
	    {
		img =
		    gg_strip_image_create (out, GGRAPH_IMAGE_ASCII_GRID,
					   GG_PIXEL_GRID, width, height, 16, 1,
					   GGRAPH_SAMPLE_INT, NULL, NULL);
	    }
	  if (bits_per_sample == 32)
	    {
		img =
		    gg_strip_image_create (out, GGRAPH_IMAGE_ASCII_GRID,
					   GG_PIXEL_GRID, width, height, 32, 1,
					   GGRAPH_SAMPLE_INT, NULL, NULL);
	    }
      }
    if (sample == GGRAPH_SAMPLE_FLOAT)
      {
	  if (bits_per_sample == 32)
	    {
		img =
		    gg_strip_image_create (out, GGRAPH_IMAGE_ASCII_GRID,
					   GG_PIXEL_GRID, width, height, 32, 1,
					   GGRAPH_SAMPLE_FLOAT, NULL, NULL);
	    }
	  if (bits_per_sample == 64)
	    {
		img =
		    gg_strip_image_create (out, GGRAPH_IMAGE_ASCII_GRID,
					   GG_PIXEL_GRID, width, height, 64, 1,
					   GGRAPH_SAMPLE_FLOAT, NULL, NULL);
	    }
      }
    if (!img)
      {
	  fclose (out);
	  unlink (path);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }

    img->upper_left_x = upper_left_x;
    img->upper_left_y = upper_left_y;
    img->pixel_x_size = pixel_x_size;
    img->pixel_y_size = pixel_y_size;
    img->no_data_value = no_data_value;
    ret = gg_image_prepare_to_ascii_grid_by_strip (img, out);
    if (ret != GGRAPH_OK)
      {
	  gg_strip_image_destroy (img);
	  return ret;
      }

    *ptr = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToPngMemBuf (const void *ptr, void **mem_buf, int *mem_buf_size,
			int compression_level, int quantization_factor,
			int interlaced, int is_transparent)
{
/* exporting an image into a PNG compressed memory buffer */
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    void *buf = NULL;
    int size;
    int ret;

    *mem_buf = NULL;
    *mem_buf_size = 0;
    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

/* compressing as PNG */
    ret =
	gg_image_to_png (img, &buf, &size, NULL, GG_TARGET_IS_MEMORY,
			 compression_level, quantization_factor, interlaced,
			 is_transparent);
    if (ret != GGRAPH_OK)
	return ret;

/* exporting the memory buffer */
    *mem_buf = buf;
    *mem_buf_size = size;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToGifFile (const void *ptr, const char *path)
{
/* exporting an image into a GIF compressed file */
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    int ret;
    FILE *out = NULL;

    if (!img)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

/* opening the output image file */
    out = fopen (path, "wb");
    if (out == NULL)
	return GGRAPH_FILE_OPEN_ERROR;
/* compressing as GIF */
    ret = gg_image_to_gif (img, NULL, NULL, out, GG_TARGET_IS_FILE, 0);
    fclose (out);
    if (ret != GGRAPH_OK)
      {
	  /* some unexpected error occurred */
	  unlink (path);
	  return ret;
      }

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageToGifMemBuf (const void *ptr, void **mem_buf, int *mem_buf_size,
			int is_transparent)
{
/* exporting an image into a GIF compressed memory buffer */
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    void *buf = NULL;
    int size;
    int ret;

    *mem_buf = NULL;
    *mem_buf_size = 0;
    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

/* compressing as GIF */
    ret =
	gg_image_to_gif (img, &buf, &size, NULL, GG_TARGET_IS_MEMORY,
			 is_transparent);
    if (ret != GGRAPH_OK)
	return ret;

/* exporting the memory buffer */
    *mem_buf = buf;
    *mem_buf_size = size;
    return GGRAPH_OK;
}

GGRAPH_DECLARE void
gGraphDestroyImage (const void *ptr)
{
/* destroyng an image object */
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    gGraphStripImagePtr strip_img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return;
    if (img->signature == GG_IMAGE_MAGIC_SIGNATURE)
      {
	  gg_image_destroy (img);
	  return;
      }
    if (strip_img->signature == GG_STRIP_IMAGE_MAGIC_SIGNATURE)
      {
	  gg_strip_image_destroy (strip_img);
	  return;
      }
}

GGRAPH_DECLARE int
gGraphStripImageClonePalette (const void *ptr, int *color_model,
			      int *num_palette, unsigned char *red,
			      unsigned char *green, unsigned char *blue)
{
/* exporting the palette and color-space infos from a Strip Image */
    int i;
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    switch (img->pixel_format)
      {
      case GG_PIXEL_GRID:
	  *color_model = GGRAPH_COLORSPACE_GRID;
	  *num_palette = 0;
	  break;
      case GG_PIXEL_PALETTE:
	  *color_model = GGRAPH_COLORSPACE_PALETTE;
	  for (i = 0; i < img->max_palette; i++)
	    {
		red[i] = img->palette_red[i];
		green[i] = img->palette_green[i];
		blue[i] = img->palette_blue[i];
	    }
	  *num_palette = img->max_palette;
	  break;
      case GG_PIXEL_GRAYSCALE:
	  *color_model = GGRAPH_COLORSPACE_GRAYSCALE;
	  *num_palette = 0;
	  break;
      case GG_PIXEL_RGBA:
      case GG_PIXEL_ARGB:
      case GG_PIXEL_BGRA:
	  *color_model = GGRAPH_COLORSPACE_TRUECOLOR_ALPHA;
	  *num_palette = 0;
	  break;
      default:
	  *color_model = GGRAPH_COLORSPACE_TRUECOLOR;
	  *num_palette = 0;
	  break;
      };
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphStripImageAllocPixels (const void *ptr, int rows_per_block)
{
/* allocating the pixels buffer */
    unsigned char *pixels;
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

/* allocating the pixel buffer */
    pixels = malloc (img->scanline_width * rows_per_block);
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
/* freeing an already allocated buffer (if any) */
    if (img->pixels)
	free (img->pixels);
    img->pixels = pixels;
    img->rows_per_block = rows_per_block;

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphStripImageEOF (const void *ptr)
{
/* checking if there is a StripImage EOF condition */
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    if (img->next_row >= img->height)
	return GGRAPH_OK;
    return GGRAPH_ERROR;
}

static int
copy_pixels_float (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out)
{
/* copying the pixels buffer between two images [FLOAT] */
    int x;
    int y;
    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  float *p_in = (float *) (img_in->pixels);
	  float *p_out = (float *) (img_out->pixels);
	  p_in += y * img_in->width;
	  p_out += y * img_out->width;
	  for (x = 0; x < img_in->width; x++)
	      *p_out++ = *p_in++;
      }
    img_out->current_available_rows = img_in->current_available_rows;
    return GGRAPH_OK;
}

static int
copy_pixels_double (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out)
{
/* copying the pixels buffer between two images [DOUBLE] */
    int x;
    int y;
    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  double *p_in = (double *) (img_in->pixels);
	  double *p_out = (double *) (img_out->pixels);
	  p_in += y * img_in->width;
	  p_out += y * img_out->width;
	  for (x = 0; x < img_in->width; x++)
	      *p_out++ = *p_in++;
      }
    img_out->current_available_rows = img_in->current_available_rows;
    return GGRAPH_OK;
}

static int
copy_pixels_uint8 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out)
{
/* copying the pixels buffer between two images [UINT-8] */
    int x;
    int y;
    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  unsigned char *p_in = (unsigned char *) (img_in->pixels);
	  unsigned char *p_out = (unsigned char *) (img_out->pixels);
	  p_in += y * img_in->width;
	  p_out += y * img_out->width;
	  for (x = 0; x < img_in->width; x++)
	      *p_out++ = *p_in++;
      }
    img_out->current_available_rows = img_in->current_available_rows;
    return GGRAPH_OK;
}

static int
copy_pixels_int8 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out)
{
/* copying the pixels buffer between two images [INT-8] */
    int x;
    int y;
    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  char *p_in = (char *) (img_in->pixels);
	  char *p_out = (char *) (img_out->pixels);
	  p_in += y * img_in->width;
	  p_out += y * img_out->width;
	  for (x = 0; x < img_in->width; x++)
	      *p_out++ = *p_in++;
      }
    img_out->current_available_rows = img_in->current_available_rows;
    return GGRAPH_OK;
}

static int
copy_pixels_uint16 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out)
{
/* copying the pixels buffer between two images [UINT-16] */
    int x;
    int y;
    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  unsigned short *p_in = (unsigned short *) (img_in->pixels);
	  unsigned short *p_out = (unsigned short *) (img_out->pixels);
	  p_in += y * img_in->width;
	  p_out += y * img_out->width;
	  for (x = 0; x < img_in->width; x++)
	      *p_out++ = *p_in++;
      }
    img_out->current_available_rows = img_in->current_available_rows;
    return GGRAPH_OK;
}

static int
copy_pixels_int16 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out)
{
/* copying the pixels buffer between two images [INT-16] */
    int x;
    int y;
    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  short *p_in = (short *) (img_in->pixels);
	  short *p_out = (short *) (img_out->pixels);
	  p_in += y * img_in->width;
	  p_out += y * img_out->width;
	  for (x = 0; x < img_in->width; x++)
	      *p_out++ = *p_in++;
      }
    img_out->current_available_rows = img_in->current_available_rows;
    return GGRAPH_OK;
}

static int
copy_pixels_uint32 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out)
{
/* copying the pixels buffer between two images [UINT-32] */
    int x;
    int y;
    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  unsigned int *p_in = (unsigned int *) (img_in->pixels);
	  unsigned int *p_out = (unsigned int *) (img_out->pixels);
	  p_in += y * img_in->width;
	  p_out += y * img_out->width;
	  for (x = 0; x < img_in->width; x++)
	      *p_out++ = *p_in++;
      }
    img_out->current_available_rows = img_in->current_available_rows;
    return GGRAPH_OK;
}

static int
copy_pixels_int32 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out)
{
/* copying the pixels buffer between two images [INT-32] */
    int x;
    int y;
    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  int *p_in = (int *) (img_in->pixels);
	  int *p_out = (int *) (img_out->pixels);
	  p_in += y * img_in->width;
	  p_out += y * img_out->width;
	  for (x = 0; x < img_in->width; x++)
	      *p_out++ = *p_in++;
      }
    img_out->current_available_rows = img_in->current_available_rows;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphStripImageCopyPixels (const void *in_ptr, const void *out_ptr)
{
/* copying the pixels buffer between two images */
    int x;
    int y;
    gGraphStripImagePtr img_in = (gGraphStripImagePtr) in_ptr;
    gGraphStripImagePtr img_out = (gGraphStripImagePtr) out_ptr;

    if (img_in == NULL || img_out == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_out->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

/* checking if the strip buffers does actually have the same size */
    if (img_in->width == img_out->width
	&& img_in->rows_per_block == img_out->rows_per_block)
	;
    else
	return GGRAPH_ERROR;
    if (img_in->pixel_format == GG_PIXEL_GRID)
      {
	  /* handling GRID data */
	  if (img_in->sample_format == GGRAPH_SAMPLE_FLOAT)
	    {
		if (img_in->bits_per_sample == 32)
		    return copy_pixels_float (img_in, img_out);
		else

		    return copy_pixels_double (img_in, img_out);
	    }
	  else
	    {
		/* UINT or INT */
		if (img_in->bits_per_sample == 8)
		  {
		      if (img_in->sample_format == GGRAPH_SAMPLE_UINT)
			  return copy_pixels_uint8 (img_in, img_out);
		      else
			  return copy_pixels_int8 (img_in, img_out);
		  }
		else if (img_in->bits_per_sample == 16)
		  {
		      if (img_in->sample_format == GGRAPH_SAMPLE_UINT)
			  return copy_pixels_uint16 (img_in, img_out);
		      else
			  return copy_pixels_int16 (img_in, img_out);
		  }
		else
		  {
		      if (img_in->sample_format == GGRAPH_SAMPLE_UINT)
			  return copy_pixels_uint32 (img_in, img_out);
		      else
			  return copy_pixels_int32 (img_in, img_out);
		  }
	    }
      }
    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  unsigned char *p_in = img_in->pixels + (y * img_in->scanline_width);
	  unsigned char *p_out =
	      img_out->pixels + (y * img_out->scanline_width);
	  for (x = 0; x < img_in->width; x++)
	    {
		if (img_in->pixel_format == GG_PIXEL_GRAYSCALE
		    && img_out->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      *p_out++ = *p_in++;
		  }
		else if (img_in->pixel_format == GG_PIXEL_PALETTE
			 && img_out->pixel_format == GG_PIXEL_PALETTE)
		  {
		      *p_out++ = *p_in++;
		  }
		else
		  {
		      unsigned char red;
		      unsigned char green;
		      unsigned char blue;
		      unsigned char alpha;
		      if (img_in->pixel_format == GG_PIXEL_RGB)
			{
			    red = *p_in++;
			    green = *p_in++;
			    blue = *p_in++;
			    alpha = 255;
			}
		      else if (img_in->pixel_format == GG_PIXEL_RGBA)
			{
			    red = *p_in++;
			    green = *p_in++;
			    blue = *p_in++;
			    alpha = *p_in++;
			}
		      else if (img_in->pixel_format == GG_PIXEL_ARGB)
			{
			    alpha = *p_in++;
			    red = *p_in++;
			    green = *p_in++;
			    blue = *p_in++;
			}
		      else if (img_in->pixel_format == GG_PIXEL_BGR)
			{
			    blue = *p_in++;
			    green = *p_in++;
			    red = *p_in++;
			    alpha = 255;
			}
		      else if (img_in->pixel_format == GG_PIXEL_BGRA)
			{
			    blue = *p_in++;
			    green = *p_in++;
			    red = *p_in++;
			    alpha = *p_in++;
			}
		      else if (img_in->pixel_format == GG_PIXEL_GRAYSCALE)
			{
			    red = *p_in++;
			    green = red;
			    blue = red;
			    alpha = 255;
			}
		      else if (img_in->pixel_format == GG_PIXEL_PALETTE)
			{
			    int index = *p_in++;
			    red = img_in->palette_red[index];
			    green = img_in->palette_green[index];
			    blue = img_in->palette_blue[index];
			    alpha = 255;
			}
		      if (img_out->pixel_format == GG_PIXEL_RGB)
			{
			    *p_out++ = red;
			    *p_out++ = green;
			    *p_out++ = blue;
			}
		      else if (img_out->pixel_format == GG_PIXEL_RGBA)
			{
			    *p_out++ = red;
			    *p_out++ = green;
			    *p_out++ = blue;
			    *p_out++ = alpha;
			}
		      else if (img_out->pixel_format == GG_PIXEL_ARGB)
			{
			    *p_out++ = alpha;
			    *p_out++ = red;
			    *p_out++ = green;
			    *p_out++ = blue;
			}
		      else if (img_out->pixel_format == GG_PIXEL_BGR)
			{
			    *p_out++ = blue;
			    *p_out++ = green;
			    *p_out++ = red;
			}
		      else if (img_out->pixel_format == GG_PIXEL_BGRA)
			{
			    *p_out++ = blue;
			    *p_out++ = green;
			    *p_out++ = red;
			    *p_out++ = alpha;
			}
		  }
	    }
	  img_out->current_available_rows = img_in->current_available_rows;
      }
    return GGRAPH_OK;
}

static int
subset_pixels_float (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out,
		     int start_from, int row)
{
/* copying the pixels buffer between two images [SubSet, FLOAT] */
    int x;
    float *p_in = (float *) (img_in->pixels);
    float *p_out = (float *) (img_out->pixels);
    p_in += start_from;
    p_out += row * img_out->width;
    for (x = 0; x < img_out->width; x++)
	*p_out++ = *p_in++;
    img_out->current_available_rows = row + 1;
    return GGRAPH_OK;
}

static int
subset_pixels_double (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out,
		      int start_from, int row)
{
/* copying the pixels buffer between two images [SubSet, DOUBLE] */
    int x;
    double *p_in = (double *) (img_in->pixels);
    double *p_out = (double *) (img_out->pixels);
    p_in += start_from;
    p_out += row * img_out->width;
    for (x = 0; x < img_out->width; x++)
	*p_out++ = *p_in++;
    img_out->current_available_rows = row + 1;
    return GGRAPH_OK;
}

static int
subset_pixels_uint8 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out,
		     int start_from, int row)
{
/* copying the pixels buffer between two images [SubSet, UINT-8] */
    int x;
    unsigned char *p_in = (unsigned char *) (img_in->pixels);
    unsigned char *p_out = (unsigned char *) (img_out->pixels);
    p_in += start_from;
    p_out += row * img_out->width;
    for (x = 0; x < img_out->width; x++)
	*p_out++ = *p_in++;
    img_out->current_available_rows = row + 1;
    return GGRAPH_OK;
}

static int
subset_pixels_int8 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out,
		    int start_from, int row)
{
/* copying the pixels buffer between two images [SubSet, INT-8] */
    int x;
    char *p_in = (char *) (img_in->pixels);
    char *p_out = (char *) (img_out->pixels);
    p_in += start_from;
    p_out += row * img_out->width;
    for (x = 0; x < img_out->width; x++)
	*p_out++ = *p_in++;
    img_out->current_available_rows = row + 1;
    return GGRAPH_OK;
}

static int
subset_pixels_uint16 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out,
		      int start_from, int row)
{
/* copying the pixels buffer between two images [SubSet, UINT-16] */
    int x;
    unsigned short *p_in = (unsigned short *) (img_in->pixels);
    unsigned short *p_out = (unsigned short *) (img_out->pixels);
    p_in += start_from;
    p_out += row * img_out->width;
    for (x = 0; x < img_out->width; x++)
	*p_out++ = *p_in++;
    img_out->current_available_rows = row + 1;
    return GGRAPH_OK;
}

static int
subset_pixels_int16 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out,
		     int start_from, int row)
{
/* copying the pixels buffer between two images [SubSet, INT-16] */
    int x;
    short *p_in = (short *) (img_in->pixels);
    short *p_out = (short *) (img_out->pixels);
    p_in += start_from;
    p_out += row * img_out->width;
    for (x = 0; x < img_out->width; x++)
	*p_out++ = *p_in++;
    img_out->current_available_rows = row + 1;
    return GGRAPH_OK;
}

static int
subset_pixels_uint32 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out,
		      int start_from, int row)
{
/* copying the pixels buffer between two images [SubSet, UINT-32] */
    int x;
    unsigned int *p_in = (unsigned int *) (img_in->pixels);
    unsigned int *p_out = (unsigned int *) (img_out->pixels);
    p_in += start_from;
    p_out += row * img_out->width;
    for (x = 0; x < img_out->width; x++)
	*p_out++ = *p_in++;
    img_out->current_available_rows = row + 1;
    return GGRAPH_OK;
}

static int
subset_pixels_int32 (gGraphStripImagePtr img_in, gGraphStripImagePtr img_out,
		     int start_from, int row)
{
/* copying the pixels buffer between two images [SubSet, INT-32] */
    int x;
    int *p_in = (int *) (img_in->pixels);
    int *p_out = (int *) (img_out->pixels);
    p_in += start_from;
    p_out += row * img_out->width;
    for (x = 0; x < img_out->width; x++)
	*p_out++ = *p_in++;
    img_out->current_available_rows = row + 1;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphStripImageSubSetPixels (const void *in_ptr, const void *out_ptr,
			      int start_from, int row)
{
/* copying the pixels buffer between two images [SubSet] */
    int x;
    unsigned char *p_in;
    unsigned char *p_out;
    gGraphStripImagePtr img_in = (gGraphStripImagePtr) in_ptr;
    gGraphStripImagePtr img_out = (gGraphStripImagePtr) out_ptr;

    if (img_in == NULL || img_out == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_out->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_out->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    if (img_in->pixel_format == GG_PIXEL_GRID)
      {
	  /* handling GRID data */
	  if (img_in->sample_format == GGRAPH_SAMPLE_FLOAT)
	    {
		if (img_in->bits_per_sample == 32)
		    return subset_pixels_float (img_in, img_out, start_from,
						row);
		else

		    return subset_pixels_double (img_in, img_out, start_from,
						 row);
	    }
	  else
	    {
		/* UINT or INT */
		if (img_in->bits_per_sample == 8)
		  {
		      if (img_in->sample_format == GGRAPH_SAMPLE_UINT)
			  return subset_pixels_uint8 (img_in, img_out,
						      start_from, row);
		      else
			  return subset_pixels_int8 (img_in, img_out,
						     start_from, row);
		  }
		else if (img_in->bits_per_sample == 16)
		  {
		      if (img_in->sample_format == GGRAPH_SAMPLE_UINT)
			  return subset_pixels_uint16 (img_in, img_out,
						       start_from, row);
		      else
			  return subset_pixels_int16 (img_in, img_out,
						      start_from, row);
		  }
		else
		  {
		      if (img_in->sample_format == GGRAPH_SAMPLE_UINT)
			  return subset_pixels_uint32 (img_in, img_out,
						       start_from, row);
		      else
			  return subset_pixels_int32 (img_in, img_out,
						      start_from, row);
		  }
	    }
      }

    p_in = img_in->pixels + (start_from * img_in->pixel_size);
    p_out = img_out->pixels + (row * img_out->scanline_width);
    for (x = 0; x < img_out->width; x++)
      {
	  if (img_in->pixel_format == GG_PIXEL_GRAYSCALE
	      && img_out->pixel_format == GG_PIXEL_GRAYSCALE)
	    {
		*p_out++ = *p_in++;
	    }
	  else if (img_in->pixel_format == GG_PIXEL_PALETTE
		   && img_out->pixel_format == GG_PIXEL_PALETTE)
	    {
		*p_out++ = *p_in++;
	    }
	  else
	    {
		unsigned char red;
		unsigned char green;
		unsigned char blue;
		unsigned char alpha;
		if (img_in->pixel_format == GG_PIXEL_RGB)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      alpha = 255;
		  }
		else if (img_in->pixel_format == GG_PIXEL_RGBA)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      alpha = *p_in++;
		  }
		else if (img_in->pixel_format == GG_PIXEL_ARGB)
		  {
		      alpha = *p_in++;
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		  }
		else if (img_in->pixel_format == GG_PIXEL_BGR)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      alpha = 255;
		  }
		else if (img_in->pixel_format == GG_PIXEL_BGRA)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      alpha = *p_in++;
		  }
		else if (img_in->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      red = *p_in++;
		      green = red;
		      blue = red;
		      alpha = 255;
		  }
		else if (img_in->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img_in->palette_red[index];
		      green = img_in->palette_green[index];
		      blue = img_in->palette_blue[index];
		      alpha = 255;
		  }
		if (img_out->pixel_format == GG_PIXEL_RGB)
		  {
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
		else if (img_out->pixel_format == GG_PIXEL_RGBA)
		  {
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		      *p_out++ = alpha;
		  }
		else if (img_out->pixel_format == GG_PIXEL_ARGB)
		  {
		      *p_out++ = alpha;
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
		else if (img_out->pixel_format == GG_PIXEL_BGR)
		  {
		      *p_out++ = blue;
		      *p_out++ = green;
		      *p_out++ = red;
		  }
		else if (img_out->pixel_format == GG_PIXEL_BGRA)
		  {
		      *p_out++ = blue;
		      *p_out++ = green;
		      *p_out++ = red;
		      *p_out++ = alpha;
		  }
	    }
      }
    img_out->current_available_rows = row + 1;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphReadNextStrip (const void *ptr, int *progress)
{
/* reading the next strip from a strip image */
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    if (img->next_row < img->height)
      {
	  if (img->codec_id == GGRAPH_IMAGE_PNG)
	      return gg_image_strip_read_from_png (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_JPEG)
	      return gg_image_strip_read_from_jpeg (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_TIFF
	      || img->codec_id == GGRAPH_IMAGE_GEOTIFF)
	      return gg_image_strip_read_from_tiff (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_HGT)
	      return gg_image_strip_read_from_hgt (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_BIN_HDR)
	      return gg_image_strip_read_from_bin_grid (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_FLT_HDR)
	      return gg_image_strip_read_from_bin_grid (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_DEM_HDR)
	      return gg_image_strip_read_from_dem_grid (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_ASCII_GRID)
	      return gg_image_strip_read_from_ascii_grid (img, progress);
      }

    return GGRAPH_INVALID_IMAGE;
}

GGRAPH_DECLARE int
gGraphWriteNextStrip (const void *ptr, int *progress)
{
/* writing the next strip into a strip image */
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    if (img->next_row < img->height)
      {
	  if (img->codec_id == GGRAPH_IMAGE_PNG)
	      return gg_image_write_to_png_by_strip (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_JPEG)
	      return gg_image_write_to_jpeg_by_strip (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_TIFF
	      || img->codec_id == GGRAPH_IMAGE_GEOTIFF)
	      return gg_image_write_to_tiff_by_strip (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_BIN_HDR)
	      return gg_image_write_to_bin_hdr_by_strip (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_FLT_HDR)
	      return gg_image_write_to_flt_hdr_by_strip (img, progress);
	  if (img->codec_id == GGRAPH_IMAGE_ASCII_GRID)
	      return gg_image_write_to_ascii_grid_by_strip (img, progress);
      }

    return GGRAPH_INVALID_IMAGE;
}

GGRAPH_DECLARE int
gGraphStripImageGetNextRow (const void *ptr, int *next_row)
{
/* retrieving the current Next Row */
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    *next_row = -1;
    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    *next_row = img->next_row;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphStripIsFull (const void *ptr)
{
/* checks it the strip is full (ready to be written) */
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    if (img->current_available_rows == img->rows_per_block)
	return GGRAPH_OK;
    if (img->next_row + img->current_available_rows == img->height)
	return GGRAPH_OK;
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphStripImageRewind (const void *ptr)
{
/* rewinds the image (to first scanline) */
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    img->next_row = 0;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphStripImageGetCurrentRows (const void *ptr, int *rows)
{
/* return how many rows are currently into the buffer */
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    *rows = 0;
    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    *rows = img->current_available_rows;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphStripImageGetPixelRGB (const void *ptr, int col, int row,
			     unsigned char *red, unsigned char *green,
			     unsigned char *blue)
{
/* retrieving an RGB pixel by col and row */
    unsigned char *p_in;
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    *red = 0;
    *green = 0;
    *blue = 0;
    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    if (col < 0 || col >= img->width)
	return GGRAPH_ERROR;
    if (row < 0 || row >= img->current_available_rows)
	return GGRAPH_ERROR;

    p_in = img->pixels + (row * img->scanline_width) + (col * img->pixel_size);
    if (img->pixel_format == GG_PIXEL_RGB)
      {
	  *red = *p_in++;
	  *green = *p_in++;
	  *blue = *p_in++;
      }
    else if (img->pixel_format == GG_PIXEL_RGBA)
      {
	  *red = *p_in++;
	  *green = *p_in++;
	  *blue = *p_in++;
	  p_in++;		/* discarding Alpha */
      }
    else if (img->pixel_format == GG_PIXEL_ARGB)
      {
	  p_in++;		/* discarding Alpha */
	  *red = *p_in++;
	  *green = *p_in++;
	  *blue = *p_in++;
      }
    else if (img->pixel_format == GG_PIXEL_BGR)
      {
	  *blue = *p_in++;
	  *green = *p_in++;
	  *red = *p_in++;
      }
    else if (img->pixel_format == GG_PIXEL_BGRA)
      {
	  *blue = *p_in++;
	  *green = *p_in++;
	  *red = *p_in++;
	  p_in++;		/* discarding Alpha */
      }
    else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
      {
	  *red = *p_in++;
	  *green = *red;
	  *blue = *red;
      }
    else if (img->pixel_format == GG_PIXEL_PALETTE)
      {
	  int index = *p_in++;
	  *red = img->palette_red[index];
	  *green = img->palette_green[index];
	  *blue = img->palette_blue[index];
      }

    return GGRAPH_OK;
}

static unsigned char
match_palette2 (const gGraphStripImagePtr img, unsigned char r, unsigned char g,
		unsigned char b)
{
/* handling palette colors */
    int index;
    double min_dist = DBL_MAX;
    double dist;
    int min_index;
    for (index = 0; index < img->max_palette; index++)
      {
	  /* searching if already defined */
	  if (img->palette_red[index] == r && img->palette_green[index] == g
	      && img->palette_blue[index] == b)
	      return (unsigned char) index;
      }
    if (img->max_palette < 255)
      {
	  /* inserting a new palette entry */
	  unsigned char i = img->max_palette;
	  img->max_palette += 1;
	  img->palette_red[i] = r;
	  img->palette_green[i] = g;
	  img->palette_blue[i] = b;
	  return i;
      }
/* all right, the palette is already fully populated */
    for (index = 0; index < img->max_palette; index++)
      {
	  /* computing the minimal euclidean distance */
	  dist =
	      sqrt (((img->palette_red[index] - r) * (img->palette_red[index] -
						      r)) +
		    ((img->palette_green[index] -
		      g) * (img->palette_green[index] - g)) +
		    ((img->palette_blue[index] -
		      b) * (img->palette_blue[index] - b)));
	  if (dist < min_dist)
	    {
		min_dist = dist;
		min_index = index;
	    }
      }
    return (unsigned char) min_index;
}

GGRAPH_DECLARE int
gGraphStripImageSetPixelRGB (const void *ptr, int col, int row,
			     unsigned char red, unsigned char green,
			     unsigned char blue)
{
/* setting an RGB pixel by col and row */
    unsigned char *p_out;
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    if (col < 0 || col >= img->width)
	return GGRAPH_ERROR;
    if (row < 0 || row >= img->current_available_rows)
	return GGRAPH_ERROR;

    p_out = img->pixels + (row * img->scanline_width) + (col * img->pixel_size);
    if (img->pixel_format == GG_PIXEL_RGB)
      {
	  *p_out++ = red;
	  *p_out++ = green;
	  *p_out++ = blue;
      }
    else if (img->pixel_format == GG_PIXEL_RGBA)
      {
	  *p_out++ = red;
	  *p_out++ = green;
	  *p_out++ = blue;
	  *p_out++ = 255;	/* defaulting Alpha [opaque] */
      }
    else if (img->pixel_format == GG_PIXEL_ARGB)
      {

	  *p_out++ = 255;	/* defaulting Alpha [opaque] */
	  *p_out++ = red;
	  *p_out++ = green;
	  *p_out++ = blue;
      }
    else if (img->pixel_format == GG_PIXEL_BGR)
      {
	  *p_out++ = blue;
	  *p_out++ = green;
	  *p_out++ = red;
      }
    else if (img->pixel_format == GG_PIXEL_BGRA)
      {
	  *p_out++ = blue;
	  *p_out++ = green;
	  *p_out++ = red;

	  *p_out++ = 255;	/* defaulting Alpha [opaque] */
      }
    else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
      {
	  int gray;
	  if (red == green && green == blue)
	      gray = red;
	  else
	      gray = to_grayscale2 (red, green, blue);
	  *p_out++ = gray;
      }
    else if (img->pixel_format == GG_PIXEL_PALETTE)
      {
	  int index = match_palette2 (img, red, green, blue);
	  *p_out++ = index;
      }

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphStripImageSetCurrentRows (const void *ptr, int rows)
{
/* sets the number of rows currently into the buffer */
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    img->current_available_rows = rows;
    return GGRAPH_OK;
}

GGRAPH_DECLARE void
gGraphDestroyImageInfos (const void *ptr)
{
/* destroyng an image INFOS object */
    gGraphImageInfosPtr img = (gGraphImageInfosPtr) ptr;

    if (img == NULL)
	return;
    if (img->signature != GG_IMAGE_INFOS_MAGIC_SIGNATURE)
	return;
    gg_image_infos_destroy (img);
}

GGRAPH_DECLARE int
gGraphGetImageInfos (const void *ptr, int *width, int *height, int *colorspace,
		     int *max_palette, int *bits_per_sample,
		     int *samples_per_pixel, int *sample_format,
		     int *tile_width, int *tile_height, int *rows_per_strip,
		     int *compression, double *no_data_value, double *min_value,
		     double *max_value, int *scale_1_2, int *scale_1_4,
		     int *scale_1_8)
{
/* querying image INFOSt */
    gGraphImageInfosPtr infos = (gGraphImageInfosPtr) ptr;
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    gGraphStripImagePtr strip_img = (gGraphStripImagePtr) ptr;

    if (ptr == NULL)
	return GGRAPH_ERROR;
    if (infos->signature == GG_IMAGE_INFOS_MAGIC_SIGNATURE)
      {
	  /* form the INFO struct */
	  *width = infos->width;
	  *height = infos->height;
	  switch (infos->pixel_format)
	    {
	    case GG_PIXEL_RGB:
	    case GG_PIXEL_BGR:
		*colorspace = GGRAPH_COLORSPACE_TRUECOLOR;
		break;
	    case GG_PIXEL_RGBA:
	    case GG_PIXEL_ARGB:
	    case GG_PIXEL_BGRA:
		*colorspace = GGRAPH_COLORSPACE_TRUECOLOR;
		break;
	    case GG_PIXEL_GRAYSCALE:
		*colorspace = GGRAPH_COLORSPACE_GRAYSCALE;
		break;
	    case GG_PIXEL_PALETTE:
		if (infos->max_palette == 2)
		  {
		      if ((infos->palette_red[0] == 255
			   && infos->palette_green[0] == 255
			   && infos->palette_blue[0] == 255
			   && infos->palette_red[1] == 0
			   && infos->palette_green[1] == 0
			   && infos->palette_blue[1] == 0)
			  || (infos->palette_red[0] == 0
			      && infos->palette_green[0] == 0
			      && infos->palette_blue[0] == 0
			      && infos->palette_red[1] == 255
			      && infos->palette_green[1] == 255
			      && infos->palette_blue[1] == 255))
			  *colorspace = GGRAPH_COLORSPACE_MONOCHROME;
		      else
			  *colorspace = GGRAPH_COLORSPACE_PALETTE;
		  }
		else
		    *colorspace = GGRAPH_COLORSPACE_PALETTE;
		break;
	    case GG_PIXEL_GRID:
		*colorspace = GGRAPH_COLORSPACE_GRID;
		break;
	    default:
		*colorspace = GGRAPH_COLORSPACE_UNKNOWN;
		break;
	    };
	  *bits_per_sample = infos->bits_per_sample;
	  *samples_per_pixel = infos->samples_per_pixel;
	  *sample_format = infos->sample_format;
	  *tile_width = infos->tile_width;
	  *tile_height = infos->tile_height;
	  *rows_per_strip = infos->rows_per_strip;
	  *compression = infos->compression;
	  *no_data_value = infos->no_data_value;
	  *min_value = infos->min_value;
	  *max_value = infos->max_value;
	  *scale_1_2 = infos->scale_1_2;
	  *scale_1_4 = infos->scale_1_4;
	  *scale_1_8 = infos->scale_1_8;
	  return GGRAPH_OK;
      }
    else if (img->signature == GG_IMAGE_MAGIC_SIGNATURE)
      {
	  /* from the IMAGE struct */
	  *width = img->width;
	  *height = img->height;
	  switch (img->pixel_format)
	    {
	    case GG_PIXEL_RGB:
	    case GG_PIXEL_BGR:
		*colorspace = GGRAPH_COLORSPACE_TRUECOLOR;
		break;
	    case GG_PIXEL_RGBA:
	    case GG_PIXEL_ARGB:
	    case GG_PIXEL_BGRA:
		*colorspace = GGRAPH_COLORSPACE_TRUECOLOR;
		break;
	    case GG_PIXEL_GRAYSCALE:
		*colorspace = GGRAPH_COLORSPACE_GRAYSCALE;
		break;
	    case GG_PIXEL_PALETTE:
		if (img->max_palette == 2)
		  {
		      if ((img->palette_red[0] == 255
			   && img->palette_green[0] == 255
			   && img->palette_blue[0] == 255
			   && img->palette_red[1] == 0
			   && img->palette_green[1] == 0
			   && img->palette_blue[1] == 0)
			  || (img->palette_red[0] == 0
			      && img->palette_green[0] == 0
			      && img->palette_blue[0] == 0
			      && img->palette_red[1] == 255
			      && img->palette_green[1] == 255
			      && img->palette_blue[1] == 255))
			  *colorspace = GGRAPH_COLORSPACE_MONOCHROME;
		      else
			  *colorspace = GGRAPH_COLORSPACE_PALETTE;
		  }
		else
		    *colorspace = GGRAPH_COLORSPACE_PALETTE;
		break;
	    case GG_PIXEL_GRID:
		*colorspace = GGRAPH_COLORSPACE_GRID;
		break;
	    default:
		*colorspace = GGRAPH_COLORSPACE_UNKNOWN;
		break;
	    };
	  *max_palette = img->max_palette;
	  *bits_per_sample = img->bits_per_sample;
	  *samples_per_pixel = img->samples_per_pixel;
	  *sample_format = img->sample_format;
	  *tile_width = img->tile_width;
	  *tile_height = img->tile_height;
	  *rows_per_strip = img->rows_per_strip;
	  *compression = img->compression;
	  *no_data_value = img->no_data_value;
	  *min_value = img->min_value;
	  *max_value = img->max_value;
	  return GGRAPH_OK;
      }
    else if (strip_img->signature == GG_STRIP_IMAGE_MAGIC_SIGNATURE)
      {
	  /* from the IMAGE struct */
	  *width = strip_img->width;
	  *height = strip_img->height;
	  switch (strip_img->pixel_format)
	    {
	    case GG_PIXEL_RGB:
	    case GG_PIXEL_BGR:
		*colorspace = GGRAPH_COLORSPACE_TRUECOLOR;
		break;
	    case GG_PIXEL_RGBA:
	    case GG_PIXEL_ARGB:
	    case GG_PIXEL_BGRA:
		*colorspace = GGRAPH_COLORSPACE_TRUECOLOR;
		break;
	    case GG_PIXEL_GRAYSCALE:
		*colorspace = GGRAPH_COLORSPACE_GRAYSCALE;
		break;
	    case GG_PIXEL_PALETTE:
		if (strip_img->max_palette == 2)
		  {
		      if ((strip_img->palette_red[0] == 255
			   && strip_img->palette_green[0] == 255
			   && strip_img->palette_blue[0] == 255
			   && strip_img->palette_red[1] == 0
			   && strip_img->palette_green[1] == 0
			   && strip_img->palette_blue[1] == 0)
			  || (strip_img->palette_red[0] == 0
			      && strip_img->palette_green[0] == 0
			      && strip_img->palette_blue[0] == 0
			      && strip_img->palette_red[1] == 255
			      && strip_img->palette_green[1] == 255
			      && strip_img->palette_blue[1] == 255))
			  *colorspace = GGRAPH_COLORSPACE_MONOCHROME;
		      else
			  *colorspace = GGRAPH_COLORSPACE_PALETTE;
		  }
		else
		    *colorspace = GGRAPH_COLORSPACE_PALETTE;
		break;
	    case GG_PIXEL_GRID:
		*colorspace = GGRAPH_COLORSPACE_GRID;
		break;
	    default:
		*colorspace = GGRAPH_COLORSPACE_UNKNOWN;
		break;
	    };
	  *max_palette = strip_img->max_palette;
	  *bits_per_sample = strip_img->bits_per_sample;
	  *samples_per_pixel = strip_img->samples_per_pixel;
	  *sample_format = strip_img->sample_format;
	  *tile_width = strip_img->tile_width;
	  *tile_height = strip_img->tile_height;
	  *rows_per_strip = strip_img->rows_per_strip;
	  *compression = strip_img->compression;
	  *no_data_value = strip_img->no_data_value;
	  *min_value = strip_img->min_value;
	  *max_value = strip_img->max_value;
	  return GGRAPH_OK;
      }
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphGetImageDims (const void *ptr, int *width, int *height)
{
/* querying image Width / Height */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    *width = img->width;
    *height = img->height;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphGetImageSize (const void *ptr, int *size)
{
/* querying image size */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    *size = img->height * img->scanline_width;
    if (gg_is_image_monochrome_ready (img) == GGRAPH_TRUE)
	*size /= 8;

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageColorSpaceOptimize (const void *ptr)
{
/* attempting to optimize the actual color space */
    int current_color_space;
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    if (img->pixel_format == GG_PIXEL_PALETTE)
      {
	  if (gg_is_image_monochrome_ready (img) == GGRAPH_TRUE)
	      current_color_space = GGRAPH_COLORSPACE_MONOCHROME;
	  else
	      current_color_space = GGRAPH_COLORSPACE_PALETTE;
      }
    else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
	current_color_space = GGRAPH_COLORSPACE_GRAYSCALE;
    else
	current_color_space = GGRAPH_COLORSPACE_TRUECOLOR;

    if (current_color_space == GGRAPH_COLORSPACE_MONOCHROME)
      {
	  /* already optimized */
	  return GGRAPH_OK;
      }
    if (gg_is_image_monochrome (img) == GGRAPH_TRUE)
      {
	  /* attempting to optimize into MONOCHROME */
	  return gg_convert_image_to_monochrome (img);
      }
    if (gg_is_image_grayscale (img) == GGRAPH_TRUE)
      {
	  /* attempting to optimize into GRAYSCALE */
	  return gg_convert_image_to_grayscale (img);
      }
    if (current_color_space == GGRAPH_COLORSPACE_PALETTE)
      {
	  /* already optimized */
	  return GGRAPH_OK;
      }
    if (gg_is_image_palette256 (img) == GGRAPH_TRUE)
      {
	  /* attempting to optimize into PALETTE */
	  return gg_convert_image_to_palette (img);
      }
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageResampleAsGridInt16 (const void *ptr)
{
/* promoting to GRID-Int16 */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    return gg_convert_image_to_grid_int16 (img);
}

GGRAPH_DECLARE int
gGraphImageResampleAsGridUInt16 (const void *ptr)
{
/* promoting to GRID-UInt16 */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    return gg_convert_image_to_grid_uint16 (img);
}

GGRAPH_DECLARE int
gGraphImageResampleAsGridInt32 (const void *ptr)
{
/* promoting to GRID-Int32 */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    return gg_convert_image_to_grid_int32 (img);
}

GGRAPH_DECLARE int
gGraphImageResampleAsGridUInt32 (const void *ptr)
{
/* promoting to GRID-UInt32 */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    return gg_convert_image_to_grid_uint32 (img);
}

GGRAPH_DECLARE int
gGraphImageResampleAsGridFloat (const void *ptr)
{
/* promoting to GRID-Float */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    return gg_convert_image_to_grid_float (img);
}

GGRAPH_DECLARE int
gGraphImageResampleAsGridDouble (const void *ptr)
{
/* promoting to GRID-Double */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    return gg_convert_image_to_grid_double (img);
}

GGRAPH_DECLARE int
gGraphImageResampleAsRgb (const void *ptr)
{
/* promoting to RGB */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    return gg_convert_image_to_rgb (img);
}

GGRAPH_DECLARE int
gGraphImageTransparentResample (const void *ptr)
{
/* resampling the Transparent Color near-values as Transparent */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    switch (img->pixel_format)
      {
      case GG_PIXEL_RGB:
	  return gg_resample_transparent_rgb (img);
      case GG_PIXEL_RGBA:
	  return gg_resample_transparent_rgba (img);
      case GG_PIXEL_GRAYSCALE:
	  return gg_resample_transparent_grayscale (img);
      case GG_PIXEL_PALETTE:
	  return gg_resample_transparent_palette (img);
      default:
	  return GGRAPH_OK;
      };
}

GGRAPH_DECLARE int
gGraphImageResampleAsMonochrome (const void *ptr)
{
/* applying BILEVEL quantization */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    return gg_convert_image_to_monochrome (img);
}

GGRAPH_DECLARE int
gGraphImageResampleAsPalette (const void *ptr, int num_colors)
{
/* applying quantization in order to fit into PALETTE colorspace */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    return gg_image_resample_as_palette (img, num_colors);
}

GGRAPH_DECLARE int
gGraphImageResampleAsGrayscale (const void *ptr)
{
/* converting into GRAYSCALE */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    return gg_convert_image_to_grayscale (img);
}

GGRAPH_DECLARE int
gGraphImageResampleAsPhotographic (const void *ptr)
{
/* converting into GRAYSCALE or RGB colorspace */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    if (img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_RGB)
	return GGRAPH_OK;

    if (gg_is_image_grayscale (img) == GGRAPH_TRUE)
      {
	  /* attempting to convert into GRAYSCALE */
	  return gg_convert_image_to_grayscale (img);
      }

    return gg_convert_image_to_rgb (img);
}

GGRAPH_DECLARE int
gGraphImageGuessFormat (const void *mem_buf, int mem_buf_size)
{
/* attempting to guess the Image type from its Magic Number */
    const unsigned char *p = mem_buf;
    if (mem_buf_size > 6)
      {
	  if (p[0] == 'G' && p[1] == 'I' && p[2] == 'F' && p[3] == '8'
	      && (p[4] == '7' || p[4] == '9') && p[5] == 'a')
	      return GGRAPH_IMAGE_GIF;
	  if (p[0] == 0x89 && p[1] == 'P' && p[2] == 'N' && p[3] == 'G'
	      && p[4] == 0x0d && p[5] == 0x0a)
	      return GGRAPH_IMAGE_PNG;
      }
    if (mem_buf_size > 4)
      {
	  if (p[0] == 'M' && p[1] == 'M' && p[2] == 0x00 && p[3] == '*')
	      return GGRAPH_IMAGE_TIFF;
	  if (p[0] == 'I' && p[1] == 'I' && p[2] == '*' && p[3] == 0x00)
	      return GGRAPH_IMAGE_TIFF;
      }
    if (mem_buf_size > 2)
      {
	  if (p[0] == 0xff && p[1] == 0xd8)
	      return GGRAPH_IMAGE_JPEG;
      }
    return GGRAPH_IMAGE_UNKNOWN;
}

GGRAPH_DECLARE int
gGraphFileImageGuessFormat (const char *path, int *type)
{
/* attempting to guess the FileImage type from its Magic Number */
    FILE *in = NULL;
    unsigned char mem_buf[10];
    int mem_buf_size;

    *type = GGRAPH_IMAGE_UNKNOWN;
/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;
    mem_buf_size = fread (mem_buf, 1, 10, in);
    if (mem_buf_size < 10)
      {
	  fclose (in);
	  return GGRAPH_FILE_READ_ERROR;
      }
    fclose (in);
    *type = gGraphImageGuessFormat (mem_buf, 10);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageIsGeoRef (const void *ptr)
{
/* checking if an Image is georeferenced or not */
    gGraphImagePtr img = (gGraphImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_ERROR;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_ERROR;

    if (img->is_georeferenced)
	return GGRAPH_OK;
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphImageSetGeoRef (const void *ptr, int srid, const char *srs_name,
		      const char *proj4text, double upper_left_x,
		      double upper_left_y, double pixel_x_size,
		      double pixel_y_size)
{
/* setting the georeferencing infos for some Image */
    int len;
    char *SrsName = NULL;
    char *Proj4Text = NULL;
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    gGraphStripImagePtr strip_img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_IMAGE_UNKNOWN;
    if (img->signature == GG_IMAGE_MAGIC_SIGNATURE)
      {
	  /* setting up an Image struct */
	  if (srs_name)
	    {
		len = strlen (srs_name);
		if (len > 0)
		  {
		      SrsName = malloc (len + 1);
		      if (!SrsName)
			  return GGRAPH_ERROR;
		      strcpy (SrsName, srs_name);
		  }
	    }
	  if (proj4text)
	    {
		len = strlen (proj4text);
		if (len > 0)
		  {
		      Proj4Text = malloc (len + 1);
		      if (!Proj4Text)
			{
			    free (SrsName);
			    return GGRAPH_ERROR;
			}
		      strcpy (Proj4Text, proj4text);
		  }
	    }

	  if (img->srs_name)
	      free (img->srs_name);
	  if (img->proj4text)
	      free (img->proj4text);

	  img->is_georeferenced = 1;
	  img->srid = srid;
	  img->srs_name = SrsName;
	  img->proj4text = Proj4Text;
	  img->upper_left_x = upper_left_x;
	  img->upper_left_y = upper_left_y;
	  img->pixel_x_size = pixel_x_size;
	  img->pixel_y_size = pixel_y_size;
	  return GGRAPH_OK;
      }
    else if (strip_img->signature == GG_STRIP_IMAGE_MAGIC_SIGNATURE)
      {				/* setting up a Strip Image struct */
	  if (srs_name)
	    {
		len = strlen (srs_name);
		if (len > 0)
		  {
		      SrsName = malloc (len + 1);
		      if (!SrsName)
			  return GGRAPH_ERROR;
		      strcpy (SrsName, srs_name);
		  }
	    }
	  if (proj4text)
	    {
		len = strlen (proj4text);
		if (len > 0)
		  {
		      Proj4Text = malloc (len + 1);
		      if (!Proj4Text)
			{
			    free (SrsName);
			    return GGRAPH_ERROR;
			}
		      strcpy (Proj4Text, proj4text);
		  }
	    }

	  if (strip_img->srs_name)
	      free (strip_img->srs_name);
	  if (strip_img->proj4text)
	      free (strip_img->proj4text);

	  strip_img->is_georeferenced = 1;
	  strip_img->srid = srid;
	  strip_img->srs_name = SrsName;
	  strip_img->proj4text = Proj4Text;
	  strip_img->upper_left_x = upper_left_x;
	  strip_img->upper_left_y = upper_left_y;
	  strip_img->pixel_x_size = pixel_x_size;
	  strip_img->pixel_y_size = pixel_y_size;
	  return GGRAPH_OK;
      }
    return GGRAPH_IMAGE_UNKNOWN;
}

GGRAPH_DECLARE int
gGraphImageGetGeoRef (const void *ptr, int *srid, char *srs_name,
		      char *proj4text, double *upper_left_x,
		      double *upper_left_y, double *pixel_x_size,
		      double *pixel_y_size)
{
/* retrieving the georeferencing infos from some Image */
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    gGraphStripImagePtr strip_img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_IMAGE_UNKNOWN;
    if (img->signature == GG_IMAGE_MAGIC_SIGNATURE)
      {
	  /* from normal Image */
	  if (img->is_georeferenced == 0)
	      return GGRAPH_ERROR;

	  *srid = img->srid;
	  if (img->srs_name)
	      strcpy (srs_name, img->srs_name);
	  else
	      *srs_name = '\0';
	  if (img->proj4text)
	      strcpy (proj4text, img->proj4text);
	  else
	      *proj4text = '\0';
	  *upper_left_x = img->upper_left_x;
	  *upper_left_y = img->upper_left_y;
	  *pixel_x_size = img->pixel_x_size;
	  *pixel_y_size = img->pixel_y_size;
	  return GGRAPH_OK;
      }
    else if (strip_img->signature == GG_STRIP_IMAGE_MAGIC_SIGNATURE)
      {
	  /* from strip Image */
	  if (strip_img->is_georeferenced == 0)
	      return GGRAPH_ERROR;

	  *srid = strip_img->srid;
	  if (strip_img->srs_name)
	      strcpy (srs_name, strip_img->srs_name);
	  else
	      *srs_name = '\0';
	  if (strip_img->proj4text)
	      strcpy (proj4text, strip_img->proj4text);
	  else
	      *proj4text = '\0';
	  *upper_left_x = strip_img->upper_left_x;
	  *upper_left_y = strip_img->upper_left_y;
	  *pixel_x_size = strip_img->pixel_x_size;
	  *pixel_y_size = strip_img->pixel_y_size;
	  return GGRAPH_OK;
      }
    return GGRAPH_IMAGE_UNKNOWN;
}

GGRAPH_DECLARE int
gGraphImageInfosGetGeoRef (const void *ptr, int *srid, char *srs_name,
			   char *proj4text, double *upper_left_x,
			   double *upper_left_y, double *pixel_x_size,
			   double *pixel_y_size)
{
/* retrieving the georeferencing infos from some Image */
    gGraphImageInfosPtr img = (gGraphImageInfosPtr) ptr;

    if (img == NULL)
	return GGRAPH_IMAGE_UNKNOWN;
    if (img->signature != GG_IMAGE_INFOS_MAGIC_SIGNATURE)
	return GGRAPH_IMAGE_UNKNOWN;
    if (img->is_georeferenced == 0)
	return GGRAPH_ERROR;

    *srid = img->srid;
    if (img->srs_name)
	strcpy (srs_name, img->srs_name);
    else
	*srs_name = '\0';
    if (img->proj4text)
	strcpy (proj4text, img->proj4text);
    else
	*proj4text = '\0';
    *upper_left_x = img->upper_left_x;
    *upper_left_y = img->upper_left_y;
    *pixel_x_size = img->pixel_x_size;
    *pixel_y_size = img->pixel_y_size;
    return GGRAPH_OK;
}

static int
token_to_double (const char *str, double *value)
{
/* attempting to convert from string to double */
    const char *p = str;
    int error = 0;
    int digits = 0;
    int comma = 0;
    int sign = 0;
    while (*p != '\0')
      {
	  if (*p >= '0' && *p <= '9')
	      digits++;
	  else if (*p == '.')
	      comma++;
	  else if (*p == '-' || *p == '+')
	    {
		if (digits == 0)
		    sign++;
		else
		    error = 1;
	    }
	  else
	      error = 1;
	  p++;
      }
    if (digits > 0 && sign < 2 && comma < 2 && error == 0)
      {
	  *value = atof (str);
	  return 1;
      }
    return 0;
}

GGRAPH_DECLARE int
gGraphGetWorldFilePath (const char *main_path, char *world_file_path)
{
/* generating a World File path corresponding to the Image path */
    char suffix[1024];
    char string[1024];
    int i;
    int len;

    strcpy (string, main_path);
    len = strlen (string) - 1;
    for (i = len - 1; i >= 0; i--)
      {
	  if (string[i] == '.')
	    {
		strcpy (suffix, string + i);
		string[i] = '\0';
		if (strcasecmp (suffix, ".jpg") == 0)
		  {
		      strcpy (world_file_path, string);
		      strcat (world_file_path, ".jgw");
		      return GGRAPH_OK;
		  }
		if (strcasecmp (suffix, ".png") == 0)
		  {
		      strcpy (world_file_path, string);
		      strcat (world_file_path, ".pgw");
		      return GGRAPH_OK;
		  }
		if (strcasecmp (suffix, ".gif") == 0)
		  {
		      strcpy (world_file_path, string);
		      strcat (world_file_path, ".gfw");
		      return GGRAPH_OK;
		  }
		if (strcasecmp (suffix, ".tif") == 0)
		  {
		      strcpy (world_file_path, string);
		      strcat (world_file_path, ".tfw");
		      return GGRAPH_OK;
		  }
	    }
      }
    *world_file_path = '\0';
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphReadWorldFile (const char *path, double *ul_x, double *ul_y,
		     double *x_size, double *y_size)
{
/* attempting to read georeferencing infos from some World File */
    FILE *in = NULL;
    double upper_left_x = DBL_MAX;
    double upper_left_y = DBL_MAX;
    double pixel_x = DBL_MAX;
    double pixel_y = DBL_MAX;
    double value;
    int c;
    int i;
    char wf_buf[1024];
    char *p_wf;
    int wf_row;

    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

    wf_row = 0;
    p_wf = wf_buf;
    while ((c = getc (in)) != EOF)
      {
	  /* parsing the Worldfile */
	  if (c == '\r')
	      continue;
	  if (c == '\n')
	    {
		*p_wf = '\0';
		for (i = 0; i < (int) strlen (wf_buf); i++)
		  {
		      /* replacing any COMMA (badly formatted decimal separator) */
		      if (wf_buf[i] == ',')
			  wf_buf[i] = '.';
		  }
		switch (wf_row)
		  {
		  case 0:
		      if (!token_to_double (wf_buf, &value))
			  goto stop;
		      else
			  pixel_x = value;
		      break;
		  case 3:
		      if (!token_to_double (wf_buf, &value))
			  goto stop;
		      else
			  pixel_y = value * -1.0;
		      break;
		  case 4:
		      if (!token_to_double (wf_buf, &value))
			  goto stop;
		      else
			  upper_left_x = value;
		      break;
		  case 5:
		      if (!token_to_double (wf_buf, &value))
			  goto stop;
		      else
			  upper_left_y = value;
		      break;
		  };
		p_wf = wf_buf;
		wf_row++;
		continue;
	    }
	  *p_wf++ = c;
	  if (p_wf - wf_buf > 1024)
	      goto stop;
      }
    fclose (in);

    if (pixel_x == DBL_MAX || pixel_y == DBL_MAX || upper_left_x == DBL_MAX
	|| upper_left_y == DBL_MAX)
	return GGRAPH_ERROR;

    *ul_x = upper_left_x;
    *ul_y = upper_left_y;
    *x_size = pixel_x;
    *y_size = pixel_y;
    return GGRAPH_OK;

  stop:
    fclose (in);
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphImageResizeNormal (const void *orig, const void **dest, int width,
			 int height)
{
/* generating a resized image [Normal Quality] */
    gGraphImagePtr img2 = NULL;
    gGraphImagePtr img = (gGraphImagePtr) orig;

    *dest = NULL;
    if (img == NULL)
	return GGRAPH_ERROR;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_ERROR;

    img2 =
	gg_image_create (img->pixel_format, width, height, img->bits_per_sample,
			 img->samples_per_pixel, img->sample_format,
			 img->srs_name, img->proj4text);
    if (!img2)
	return GGRAPH_INSUFFICIENT_MEMORY;
    if (img->pixel_format == GG_PIXEL_GRID)
	gg_grid_resize (img2, img);
    else
	gg_image_resize (img2, img);
    gg_image_clone_georeferencing (img2, img);
    *dest = img2;

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageResizeHighQuality (const void *orig, const void **dest, int width,
			      int height)
{
/* generating a resized image [High Quality] */
    gGraphImagePtr img2 = NULL;
    gGraphImagePtr img = (gGraphImagePtr) orig;

    *dest = NULL;
    if (img == NULL)
	return GGRAPH_ERROR;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_ERROR;

    img2 =
	gg_image_create (img->pixel_format, width, height, img->bits_per_sample,
			 img->samples_per_pixel, img->sample_format,
			 img->srs_name, img->proj4text);
    if (!img2)
	return GGRAPH_INSUFFICIENT_MEMORY;
    if (img->pixel_format == GG_PIXEL_GRID)
	gg_make_grid_thumbnail (img2, img);
    else
	gg_make_thumbnail (img2, img);
    gg_image_clone_georeferencing (img2, img);
    *dest = img2;

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageResizeToResolution (const void *orig, const void **dest,
			       double pixel_x_size, double pixel_y_size,
			       int *x_width, int *x_height)
{
/* generating a resized image [High Quality] */
    int width;
    int height;
    double ww;
    double wh;
    gGraphImagePtr img2 = NULL;
    gGraphImagePtr img = (gGraphImagePtr) orig;

    *dest = NULL;
    *x_width = 0;
    *x_height = 0;
    if (img == NULL)
	return GGRAPH_ERROR;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_ERROR;

    ww = (double) (img->width) * img->pixel_x_size;
    wh = (double) (img->height) * img->pixel_y_size;
    width = (int) (ww / pixel_x_size);
    height = (int) (wh / pixel_y_size);

    img2 =
	gg_image_create (img->pixel_format, width, height, img->bits_per_sample,
			 img->samples_per_pixel, img->sample_format,
			 img->srs_name, img->proj4text);
    if (!img2)
	return GGRAPH_INSUFFICIENT_MEMORY;
    if (img->pixel_format == GG_PIXEL_GRID)
	gg_make_grid_thumbnail (img2, img);
    else
	gg_make_thumbnail (img2, img);
    gg_image_clone_georeferencing (img2, img);
    *dest = img2;
    *x_width = width;
    *x_height = height;

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageSubSet (const void *orig, const void **dest, int upper_left_x,
		   int upper_left_y, int width, int height)
{
/* generating an Image SubSet */
    gGraphImagePtr img2 = NULL;
    gGraphImagePtr img = (gGraphImagePtr) orig;

    *dest = NULL;
    if (img == NULL)
	return GGRAPH_ERROR;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_ERROR;

    img2 =
	gg_image_create (img->pixel_format, width, height, img->bits_per_sample,
			 img->samples_per_pixel, img->sample_format,
			 img->srs_name, img->proj4text);
    if (!img2)
	return GGRAPH_INSUFFICIENT_MEMORY;
    gg_image_sub_set (img2, img, upper_left_x, upper_left_y);
    *dest = img2;

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageBufferReferenceRGB (const void *ptr, unsigned char **buffer)
{
/*
/ this function create a separate copy of the internal image buffer
/ if required color-space conversion will be applied
/ and the original image will be unaffected
*/
    unsigned char *buf;
    unsigned char *p_in;
    unsigned char *p_out;
    int scanline_width;
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char gray;
    unsigned char index;
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    *buffer = NULL;
    if (img == NULL)
	return GGRAPH_ERROR;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_ERROR;

    scanline_width = img->width * 3;
    buf = malloc (scanline_width * img->height);
    if (!buf)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = buf + (y * scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* blue */
		  }
		else if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* blue */
		      p_in++;	/* discarding Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* blue */
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      p_in++;	/* discarding Alpha */
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      gray = *p_in++;
		      *p_out++ = gray;
		      *p_out++ = gray;
		      *p_out++ = gray;
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      index = *p_in++;
		      *p_out++ = img->palette_red[index];
		      *p_out++ = img->palette_green[index];
		      *p_out++ = img->palette_blue[index];
		  }
	    }
      }
    *buffer = buf;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageBufferReferenceRGBA (const void *ptr, unsigned char **buffer)
{
/*
/ this function create a separate copy of the internal image buffer
/ if required color-space conversion will be applied
/ and the original image will be unaffected
*/
    unsigned char *buf;
    unsigned char *p_in;
    unsigned char *p_out;
    int scanline_width;
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    unsigned char gray;
    unsigned char index;
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    *buffer = NULL;
    if (img == NULL)
	return GGRAPH_ERROR;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_ERROR;

    scanline_width = img->width * 4;
    buf = malloc (scanline_width * img->height);
    if (!buf)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = buf + (y * scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* blue */
		      *p_out++ = 255;	/* opaque Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* blue */
		      *p_out++ = *p_in++;	/* Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      alpha = *p_in++;
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* blue */
		      *p_out++ = alpha;
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		      *p_out++ = 255;	/* opaque Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      alpha = *p_in++;
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		      *p_out++ = alpha;
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      gray = *p_in++;
		      *p_out++ = gray;
		      *p_out++ = gray;
		      *p_out++ = gray;
		      *p_out++ = 255;	/* opaque Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      index = *p_in++;
		      *p_out++ = img->palette_red[index];
		      *p_out++ = img->palette_green[index];
		      *p_out++ = img->palette_blue[index];
		      *p_out++ = 255;	/* opaque Alpha */
		  }
	    }
      }
    *buffer = buf;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageBufferReferenceARGB (const void *ptr, unsigned char **buffer)
{
/*
/ this function create a separate copy of the internal image buffer
/ if required color-space conversion will be applied
/ and the original image will be unaffected
*/
    unsigned char *buf;
    unsigned char *p_in;
    unsigned char *p_out;
    int scanline_width;
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    unsigned char gray;
    unsigned char index;
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    *buffer = NULL;
    if (img == NULL)
	return GGRAPH_ERROR;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_ERROR;

    scanline_width = img->width * 4;
    buf = malloc (scanline_width * img->height);
    if (!buf)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = buf + (y * scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      *p_out++ = 255;	/* opaque Alpha */
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* blue */
		  }
		else if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      alpha = *p_in++;
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* blue */
		      *p_in++ = alpha;
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      *p_out++ = *p_in++;	/* Alpha */
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* blue */
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      *p_out++ = 255;	/* opaque Alpha */
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      *p_out++ = *p_in++;	/* Alpha */
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      gray = *p_in++;
		      *p_out++ = 255;	/* opaque Alpha */
		      *p_out++ = gray;
		      *p_out++ = gray;
		      *p_out++ = gray;
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      index = *p_in++;
		      *p_out++ = 255;	/* opaque Alpha */
		      *p_out++ = img->palette_red[index];
		      *p_out++ = img->palette_green[index];
		      *p_out++ = img->palette_blue[index];
		  }
	    }
      }
    *buffer = buf;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageBufferReferenceBGR (const void *ptr, unsigned char **buffer)
{
/*
/ this function create a separate copy of the internal image buffer
/ if required color-space conversion will be applied
/ and the original image will be unaffected
*/
    unsigned char *buf;
    unsigned char *p_in;
    unsigned char *p_out;
    int scanline_width;
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char gray;
    unsigned char index;
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    *buffer = NULL;
    if (img == NULL)
	return GGRAPH_ERROR;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_ERROR;

    scanline_width = img->width * 3;
    buf = malloc (scanline_width * img->height);
    if (!buf)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = buf + (y * scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      *p_out++ = blue;
		      *p_out++ = green;
		      *p_out++ = red;
		  }
		else if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      p_in++;	/* discarding Alpha */
		      *p_out++ = blue;
		      *p_out++ = green;
		      *p_out++ = red;
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      *p_out++ = blue;
		      *p_out++ = green;
		      *p_out++ = red;
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      *p_out++ = *p_in++;	/* blue */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* red */
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      *p_out++ = *p_in++;	/* blue */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* red */
		      p_in++;	/* discarding Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      gray = *p_in++;
		      *p_out++ = gray;
		      *p_out++ = gray;
		      *p_out++ = gray;
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      index = *p_in++;
		      *p_out++ = img->palette_blue[index];
		      *p_out++ = img->palette_green[index];
		      *p_out++ = img->palette_red[index];
		  }
	    }
      }
    *buffer = buf;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageBufferReferenceBGRA (const void *ptr, unsigned char **buffer)
{
/*
/ this function create a separate copy of the internal image buffer
/ if required color-space conversion will be applied
/ and the original image will be unaffected
*/
    unsigned char *buf;
    unsigned char *p_in;
    unsigned char *p_out;
    int scanline_width;
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    unsigned char gray;
    unsigned char index;
    gGraphImagePtr img = (gGraphImagePtr) ptr;
    *buffer = NULL;
    if (img == NULL)
	return GGRAPH_ERROR;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_ERROR;

    scanline_width = img->width * 4;
    buf = malloc (scanline_width * img->height);
    if (!buf)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = buf + (y * scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      *p_out++ = blue;
		      *p_out++ = green;
		      *p_out++ = red;
		      *p_out++ = 255;	/* opaque Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      *p_out++ = blue;
		      *p_out++ = green;
		      *p_out++ = red;
		      *p_out++ = *p_in++;	/* Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      alpha = *p_in++;
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      *p_out++ = blue;
		      *p_out++ = green;
		      *p_out++ = red;
		      *p_out++ = alpha;
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      *p_out++ = *p_in++;	/* blue */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = 255;	/* opaque Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      *p_out++ = *p_in++;	/* blue */
		      *p_out++ = *p_in++;	/* green */
		      *p_out++ = *p_in++;	/* red */
		      *p_out++ = *p_in++;	/* Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      gray = *p_in++;
		      *p_out++ = gray;
		      *p_out++ = gray;
		      *p_out++ = gray;
		      *p_out++ = 255;	/* opaque Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      index = *p_in++;
		      *p_out++ = img->palette_blue[index];
		      *p_out++ = img->palette_green[index];
		      *p_out++ = img->palette_red[index];
		      *p_out++ = 255;	/* opaque Alpha */
		  }
	    }
      }
    *buffer = buf;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromRawMemBuf (const void *mem_buf, int mem_buf_size,
			  const void **image_handle)
{
/* importing an image from a RAW un-compressed memory buffer */
    gGraphImagePtr img = NULL;
    int ret;

    *image_handle = NULL;
    ret = gg_image_from_raw (mem_buf_size, mem_buf, &img);
    if (ret != GGRAPH_OK)
	return ret;

    *image_handle = img;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphIsRawImage (const void *mem_buf, int mem_buf_size)
{
/* checks if this one is a RAW un-compressed memory buffer */
    unsigned char *p = (unsigned char *) mem_buf;
    short start_signature;
    short end_signature;
    int endian_arch = gg_endian_arch ();

    if (mem_buf_size < (int) (sizeof (short) * 4))
	return GGRAPH_ERROR;

/* checking the magic signature */
    start_signature = gg_import_int16 (p, 1, endian_arch);
    p = (unsigned char *) mem_buf + mem_buf_size - sizeof (short);
    end_signature = gg_import_int16 (p, 1, endian_arch);
    if (start_signature == GG_MONOCHROME_START
	&& end_signature == GG_MONOCHROME_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_1_RGB_START
	&& end_signature == GG_ADAM7_1_RGB_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_3_RGB_START
	&& end_signature == GG_ADAM7_3_RGB_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_5_RGB_START
	&& end_signature == GG_ADAM7_5_RGB_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_1_GRAYSCALE_START
	&& end_signature == GG_ADAM7_1_GRAYSCALE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_3_GRAYSCALE_START
	&& end_signature == GG_ADAM7_3_GRAYSCALE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_5_GRAYSCALE_START
	&& end_signature == GG_ADAM7_5_GRAYSCALE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_1_PALETTE_START
	&& end_signature == GG_ADAM7_1_PALETTE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_3_PALETTE_START
	&& end_signature == GG_ADAM7_3_PALETTE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_5_PALETTE_START
	&& end_signature == GG_ADAM7_5_PALETTE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_1_INT16_START
	&& end_signature == GG_ADAM7_1_INT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_3_INT16_START
	&& end_signature == GG_ADAM7_3_INT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_5_INT16_START
	&& end_signature == GG_ADAM7_5_INT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_1_UINT16_START
	&& end_signature == GG_ADAM7_1_UINT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_3_UINT16_START
	&& end_signature == GG_ADAM7_3_UINT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_5_UINT16_START
	&& end_signature == GG_ADAM7_5_UINT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_1_INT32_START
	&& end_signature == GG_ADAM7_1_INT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_3_INT32_START
	&& end_signature == GG_ADAM7_3_INT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_5_INT32_START
	&& end_signature == GG_ADAM7_5_INT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_1_UINT32_START
	&& end_signature == GG_ADAM7_1_UINT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_3_UINT32_START
	&& end_signature == GG_ADAM7_3_UINT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_5_UINT32_START
	&& end_signature == GG_ADAM7_5_UINT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_1_FLOAT_START
	&& end_signature == GG_ADAM7_1_FLOAT_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_3_FLOAT_START
	&& end_signature == GG_ADAM7_3_FLOAT_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_5_FLOAT_START
	&& end_signature == GG_ADAM7_5_FLOAT_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_1_DOUBLE_START
	&& end_signature == GG_ADAM7_1_DOUBLE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_3_DOUBLE_START
	&& end_signature == GG_ADAM7_3_DOUBLE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_5_DOUBLE_START
	&& end_signature == GG_ADAM7_5_DOUBLE_END)
	return GGRAPH_OK;

    p = (unsigned char *) mem_buf;
    start_signature = gg_import_int16 (p, 0, endian_arch);
    p = (unsigned char *) mem_buf + mem_buf_size - sizeof (short);
    end_signature = gg_import_int16 (p, 0, endian_arch);
    if (start_signature == GG_ADAM7_0_RGB_START
	&& end_signature == GG_ADAM7_0_RGB_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_2_RGB_START
	&& end_signature == GG_ADAM7_2_RGB_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_4_RGB_START
	&& end_signature == GG_ADAM7_4_RGB_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_6_RGB_START
	&& end_signature == GG_ADAM7_6_RGB_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_0_GRAYSCALE_START
	&& end_signature == GG_ADAM7_0_GRAYSCALE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_2_GRAYSCALE_START
	&& end_signature == GG_ADAM7_2_GRAYSCALE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_4_GRAYSCALE_START
	&& end_signature == GG_ADAM7_4_GRAYSCALE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_6_GRAYSCALE_START
	&& end_signature == GG_ADAM7_6_GRAYSCALE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_0_PALETTE_START
	&& end_signature == GG_ADAM7_0_PALETTE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_2_PALETTE_START
	&& end_signature == GG_ADAM7_2_PALETTE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_4_PALETTE_START
	&& end_signature == GG_ADAM7_4_PALETTE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_6_PALETTE_START
	&& end_signature == GG_ADAM7_6_PALETTE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_0_INT16_START
	&& end_signature == GG_ADAM7_0_INT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_2_INT16_START
	&& end_signature == GG_ADAM7_2_INT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_4_INT16_START
	&& end_signature == GG_ADAM7_4_INT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_6_INT16_START
	&& end_signature == GG_ADAM7_6_INT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_0_UINT16_START
	&& end_signature == GG_ADAM7_0_UINT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_2_UINT16_START
	&& end_signature == GG_ADAM7_2_UINT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_4_UINT16_START
	&& end_signature == GG_ADAM7_4_UINT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_6_UINT16_START
	&& end_signature == GG_ADAM7_6_UINT16_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_0_INT32_START
	&& end_signature == GG_ADAM7_0_INT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_2_INT32_START
	&& end_signature == GG_ADAM7_2_INT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_4_INT32_START
	&& end_signature == GG_ADAM7_4_INT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_6_INT32_START
	&& end_signature == GG_ADAM7_6_INT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_0_UINT32_START
	&& end_signature == GG_ADAM7_0_UINT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_2_UINT32_START
	&& end_signature == GG_ADAM7_2_UINT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_4_UINT32_START
	&& end_signature == GG_ADAM7_4_UINT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_6_UINT32_START
	&& end_signature == GG_ADAM7_6_UINT32_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_0_FLOAT_START
	&& end_signature == GG_ADAM7_0_FLOAT_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_2_FLOAT_START
	&& end_signature == GG_ADAM7_2_FLOAT_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_4_FLOAT_START
	&& end_signature == GG_ADAM7_4_FLOAT_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_6_FLOAT_START
	&& end_signature == GG_ADAM7_6_FLOAT_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_0_DOUBLE_START
	&& end_signature == GG_ADAM7_0_DOUBLE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_2_DOUBLE_START
	&& end_signature == GG_ADAM7_2_DOUBLE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_4_DOUBLE_START
	&& end_signature == GG_ADAM7_4_DOUBLE_END)
	return GGRAPH_OK;
    if (start_signature == GG_ADAM7_6_DOUBLE_START
	&& end_signature == GG_ADAM7_6_DOUBLE_END)
	return GGRAPH_OK;

    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphOutputPixelsToStripImage (const void *ptr_in, const void *ptr_out,
				int in_row, int out_row)
{
/* copying pixels from a memory image into a Strip Image */
    unsigned char *p_in;
    unsigned char *p_out;
    gGraphImagePtr img_in = (gGraphImagePtr) ptr_in;
    gGraphStripImagePtr img_out = (gGraphStripImagePtr) ptr_out;

    if (img_in == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_out == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_out->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->pixel_format != img_out->pixel_format)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->width != img_out->width)
	return GGRAPH_INVALID_IMAGE;

    if (in_row < 0 || in_row >= img_in->height)
	return GGRAPH_ERROR;
    if (out_row < 0 || out_row >= img_out->rows_per_block)
	return GGRAPH_ERROR;

    p_in = img_in->pixels + (in_row * img_in->scanline_width);
    p_out = img_out->pixels + (out_row * img_out->scanline_width);
    memcpy (p_out, p_in, img_in->scanline_width);
    img_out->current_available_rows = out_row + 1;

    return GGRAPH_OK;
}

static unsigned char
feed_palette (gGraphImagePtr img, unsigned char red, unsigned char green,
	      unsigned char blue)
{
/* updating a palette */
    unsigned char i;
    for (i = 0; i < img->max_palette; i++)
      {
	  /* testing if this color is already present */
	  if (img->palette_red[i] == red && img->palette_green[i] == green
	      && img->palette_blue[i] == blue)
	      return i;
      }
/* inserting a new color */
    i = img->max_palette;
    img->palette_red[i] = red;
    img->palette_green[i] = green;
    img->palette_blue[i] = blue;
    img->max_palette += 1;
    return i;
}

GGRAPH_DECLARE int
gGraphInputPixelsFromStripImage (const void *ptr_in, const void *ptr_out,
				 int in_col)
{
/* copying pixels from a Strip Image into a memory image */
    unsigned char *p_in;
    unsigned char *p_out;
    int y;
    int x;
    int is_monochrome = 0;
    int is_monochrome_minisblack = 0;
    gGraphStripImagePtr img_in = (gGraphStripImagePtr) ptr_in;
    gGraphImagePtr img_out = (gGraphImagePtr) ptr_out;

    if (img_in == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_out == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_out->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->rows_per_block < img_out->height)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->pixel_format != img_out->pixel_format)
	return GGRAPH_INVALID_IMAGE;

    if (in_col < 0 || in_col >= img_in->width)
	return GGRAPH_ERROR;

    if (img_in->pixel_format == GG_PIXEL_PALETTE)
      {
	  is_monochrome = 1;
	  if (img_in->max_palette == 2 && img_in->palette_red[0] == 0
	      && img_in->palette_green[0] == 0
	      && img_in->palette_blue[0] == 0
	      && img_in->palette_red[1] == 255
	      && img_in->palette_green[1] == 255
	      && img_in->palette_blue[1] == 255)
	      is_monochrome_minisblack = 1;
	  if (img_in->max_palette == 2 && img_in->palette_red[0] == 255
	      && img_in->palette_green[0] == 255
	      && img_in->palette_blue[0] == 255
	      && img_in->palette_red[1] == 0
	      && img_in->palette_green[1] == 0 && img_in->palette_blue[1] == 0)
	      is_monochrome_minisblack = 0;
      }

    if (is_monochrome)
      {
	  /* feeding a monochrome image */
	  for (y = 0; y < img_out->height; y++)
	    {
		p_in = img_in->pixels + (y * img_in->scanline_width) + in_col;
		p_out = img_out->pixels + (y * img_out->scanline_width);
		for (x = 0; x < img_out->width; x++)
		    *p_out++ = *p_in++;
	    }
	  img_out->max_palette = 2;
	  if (is_monochrome_minisblack)
	    {
		img_out->palette_red[0] = 0;
		img_out->palette_green[0] = 0;
		img_out->palette_blue[0] = 0;
		img_out->palette_red[1] = 255;
		img_out->palette_green[1] = 255;
		img_out->palette_blue[1] = 255;
	    }
	  else
	    {
		img_out->palette_red[0] = 255;
		img_out->palette_green[0] = 255;
		img_out->palette_blue[0] = 255;
		img_out->palette_red[1] = 0;
		img_out->palette_green[1] = 0;
		img_out->palette_blue[1] = 0;
	    }
      }
    else if (img_in->pixel_format == GG_PIXEL_PALETTE)
      {
	  /* feeding a palette image */
	  for (y = 0; y < img_out->height; y++)
	    {
		p_in = img_in->pixels + (y * img_in->scanline_width) + in_col;
		p_out = img_out->pixels + (y * img_out->scanline_width);
		for (x = 0; x < img_out->width; x++)
		  {
		      int i_idx = *p_in++;
		      *p_out++ =
			  feed_palette (img_out, img_in->palette_red[i_idx],
					img_in->palette_green[i_idx],
					img_in->palette_blue[i_idx]);
		  }
	    }
      }
    else
      {
	  for (y = 0; y < img_out->height; y++)
	    {
		p_in =
		    img_in->pixels + (y * img_in->scanline_width) +
		    (in_col * img_in->pixel_size);
		p_out = img_out->pixels + (y * img_out->scanline_width);
		memcpy (p_out, p_in, img_out->scanline_width);
	    }
      }

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromStripImage (const void *ptr_in, int color_space,
			   int sample_format, int bits_per_sample,
			   int samples_per_pixel, int start_line,
			   const void **ptr_out)
{
/* creating an Image from a StripImage (may be, converting pixel format) */
    int width;
    int height;
    int pixel_format;
    int x;
    int y;
    gGraphImagePtr img_out = NULL;
    gGraphStripImagePtr img_in = (gGraphStripImagePtr) ptr_in;

    *ptr_out = NULL;
    if (img_in == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    width = img_in->width;
    height = img_in->current_available_rows;
    switch (color_space)
      {
      case GGRAPH_COLORSPACE_MONOCHROME:
      case GGRAPH_COLORSPACE_PALETTE:
	  pixel_format = GG_PIXEL_PALETTE;
	  break;
      case GGRAPH_COLORSPACE_GRAYSCALE:
	  pixel_format = GG_PIXEL_GRAYSCALE;
	  break;
      case GGRAPH_COLORSPACE_TRUECOLOR:
      case GGRAPH_COLORSPACE_TRUECOLOR_ALPHA:
	  pixel_format = GG_PIXEL_RGB;
	  break;
      case GGRAPH_COLORSPACE_GRID:
	  pixel_format = GG_PIXEL_GRID;
	  break;
      default:
	  pixel_format = GG_PIXEL_UNKNOWN;
	  break;
      };
    if (pixel_format == GG_PIXEL_UNKNOWN)
	return GGRAPH_ERROR;
    if (img_in->pixel_format == GG_PIXEL_GRID || pixel_format == GG_PIXEL_GRID)
      {
	  if (img_in->pixel_format == GG_PIXEL_GRID
	      && pixel_format == GG_PIXEL_GRID)
	      ;
	  else
	      return GGRAPH_ERROR;
      }
    if (pixel_format == GG_PIXEL_RGB)
      {
	  if (sample_format == GGRAPH_SAMPLE_UINT && bits_per_sample == 8
	      && samples_per_pixel == 3)
	      ;
	  else
	      return GGRAPH_ERROR;
      }
    if (pixel_format == GG_PIXEL_GRAYSCALE)
      {
	  if (sample_format == GGRAPH_SAMPLE_UINT && bits_per_sample == 8
	      && samples_per_pixel == 1)
	      ;
	  else
	      return GGRAPH_ERROR;
      }
    if (pixel_format == GG_PIXEL_PALETTE)
      {
	  if (sample_format == GGRAPH_SAMPLE_UINT && samples_per_pixel == 1)
	    {
		if (bits_per_sample == 1 || bits_per_sample == 2
		    || bits_per_sample == 4 || bits_per_sample == 8)
		    ;
		else
		    return GGRAPH_ERROR;
	    }
	  else
	      return GGRAPH_ERROR;
      }
    if (pixel_format == GG_PIXEL_GRID)
      {
	  if (sample_format == GGRAPH_SAMPLE_UINT && bits_per_sample == 16
	      && samples_per_pixel == 1)
	      ;
	  else if (sample_format == GGRAPH_SAMPLE_UINT && bits_per_sample == 32
		   && samples_per_pixel == 1)
	      ;
	  else if (sample_format == GGRAPH_SAMPLE_INT && bits_per_sample == 16
		   && samples_per_pixel == 1)
	      ;
	  else if (sample_format == GGRAPH_SAMPLE_INT && bits_per_sample == 32
		   && samples_per_pixel == 1)
	      ;
	  else if (sample_format == GGRAPH_SAMPLE_FLOAT && bits_per_sample == 32
		   && samples_per_pixel == 1)
	      ;
	  else if (sample_format == GGRAPH_SAMPLE_FLOAT && bits_per_sample == 64
		   && samples_per_pixel == 1)
	      ;
	  else
	      return GGRAPH_ERROR;
      }

    img_out =
	gg_image_create (pixel_format, width, height, bits_per_sample,
			 samples_per_pixel, sample_format, NULL, NULL);
    if (!img_out)
	return GGRAPH_INSUFFICIENT_MEMORY;
    if (img_in->is_georeferenced)
      {
	  /* setting up georeferencing infos */
	  img_out->srid = img_in->srid;
	  img_out->pixel_x_size = img_in->pixel_x_size;
	  img_out->pixel_y_size = img_in->pixel_y_size;
	  img_out->upper_left_x = img_in->upper_left_x;
	  img_out->upper_left_y =
	      img_in->upper_left_y -
	      ((double) start_line * img_in->pixel_y_size);
	  img_out->is_georeferenced = 1;
      }

    if (img_out->pixel_format == GG_PIXEL_GRID)
	;
    else if (img_out->pixel_format == GG_PIXEL_PALETTE)
      {
	  if (img_in->bits_per_sample <= img_out->bits_per_sample)
	      ;
	  else
	      return GGRAPH_ERROR;
      }

    for (y = 0; y < height; y++)
      {
	  unsigned char *p_in = img_in->pixels + (y * img_in->scanline_width);
	  unsigned char *p_out =
	      img_out->pixels + (y * img_out->scanline_width);
	  unsigned char red;
	  unsigned char green;
	  unsigned char blue;
	  unsigned char idx;
	  for (x = 0; x < width; x++)
	    {
		/* fetching the input pixel */
		switch (img_in->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      break;
		  case GG_PIXEL_RGBA:
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      p_in++;	/* skipping alpha */
		      break;
		  case GG_PIXEL_ARGB:
		      p_in++;	/* skipping alpha */
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      break;
		  case GG_PIXEL_BGR:
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      break;
		  case GG_PIXEL_BGRA:
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      p_in++;	/* skipping alpha */
		      break;
		  case GG_PIXEL_GRAYSCALE:
		      red = *p_in++;
		      green = red;
		      blue = red;
		      break;
		  case GG_PIXEL_PALETTE:
		      idx = *p_in++;
		      red = img_in->palette_red[idx];
		      green = img_in->palette_green[idx];
		      blue = img_in->palette_blue[idx];
		      break;
		  };
		/* storing the output pixel */
		switch (img_out->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		      break;
		  case GG_PIXEL_GRAYSCALE:
		      *p_out++ = to_grayscale2 (red, green, blue);
		      break;
		  case GG_PIXEL_PALETTE:
		      *p_out++ = feed_palette (img_out, red, green, blue);
		      break;
		  };
	    }
      }

    *ptr_out = img_out;
    return GGRAPH_OK;
}
