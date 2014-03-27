/* 
/ gaiagraphics_image.c
/
/ image methods
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
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <limits.h>

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

GGRAPH_PRIVATE gGraphImageInfosPtr
gg_image_infos_create (int pixel_format, int width, int height,
		       int bits_per_sample, int samples_per_pixel,
		       int sample_format, const char *srs_name,
		       const char *proj4text)
{
/* creating an image infos struct */
    gGraphImageInfosPtr img;
    int len;
    char *SrsName = NULL;
    char *Proj4Text = NULL;

/* checking PIXEL_FORMAT */
    if (pixel_format == GG_PIXEL_RGB || pixel_format == GG_PIXEL_RGBA
	|| pixel_format == GG_PIXEL_ARGB || pixel_format == GG_PIXEL_BGR
	|| pixel_format == GG_PIXEL_GRAYSCALE
	|| pixel_format == GG_PIXEL_PALETTE || pixel_format == GG_PIXEL_GRID
	|| pixel_format == GG_PIXEL_UNKNOWN)
	;
    else
	return NULL;

    if (srs_name)
      {
	  len = strlen (srs_name);
	  SrsName = malloc (len + 1);
	  if (!SrsName)
	      return NULL;
	  strcpy (SrsName, srs_name);
      }
    if (proj4text)
      {
	  len = strlen (proj4text);
	  Proj4Text = malloc (len + 1);
	  if (!Proj4Text)
	    {
		if (SrsName)
		    free (SrsName);
		return NULL;
	    }
	  strcpy (Proj4Text, proj4text);
      }

/* allocating the image INFOS struct */
    img = malloc (sizeof (gGraphImage));
    if (!img)
	return NULL;

    img->signature = GG_IMAGE_INFOS_MAGIC_SIGNATURE;
    img->width = width;
    img->height = height;
    img->bits_per_sample = bits_per_sample;
    img->samples_per_pixel = samples_per_pixel;
    img->sample_format = sample_format;
    img->pixel_format = pixel_format;
    img->max_palette = 0;
    img->is_transparent = 0;
    img->tile_width = -1;
    img->tile_height = -1;
    img->rows_per_strip = -1;
    img->compression = GGRAPH_TIFF_COMPRESSION_NONE;
    img->scale_1_2 = 0;
    img->scale_1_4 = 0;
    img->scale_1_8 = 0;
    img->is_georeferenced = 0;
    img->srid = -1;
    img->srs_name = SrsName;
    img->proj4text = Proj4Text;
    img->upper_left_x = DBL_MAX;
    img->upper_left_y = DBL_MAX;
    img->pixel_x_size = 0.0;
    img->pixel_y_size = 0.0;
    img->no_data_value = 0.0 - DBL_MAX;
    img->min_value = DBL_MAX;
    img->max_value = 0.0 - DBL_MAX;

    return img;
}

GGRAPH_PRIVATE void
gg_image_infos_destroy (gGraphImageInfosPtr img)
{
/* destroying an image INFOS struct */
    if (!img)
	return;
    if (img->srs_name)
	free (img->srs_name);
    if (img->proj4text)
	free (img->proj4text);
    free (img);
}

GGRAPH_PRIVATE gGraphImagePtr
gg_image_create (int pixel_format, int width, int height, int bits_per_sample,
		 int samples_per_pixel, int sample_format, const char *srs_name,
		 const char *proj4text)
{
/* creating a generic image */
    gGraphImagePtr img;
    int len;
    char *SrsName = NULL;
    char *Proj4Text = NULL;

/* checking PIXEL_FORMAT */
    if (pixel_format == GG_PIXEL_RGB || pixel_format == GG_PIXEL_RGBA
	|| pixel_format == GG_PIXEL_ARGB || pixel_format == GG_PIXEL_BGR
	|| pixel_format == GG_PIXEL_GRAYSCALE
	|| pixel_format == GG_PIXEL_PALETTE || pixel_format == GG_PIXEL_GRID)
	;
    else
	return NULL;
    if (sample_format == GGRAPH_SAMPLE_UINT
	|| sample_format == GGRAPH_SAMPLE_INT
	|| sample_format == GGRAPH_SAMPLE_FLOAT)
	;
    else
	return NULL;

    if (srs_name)
      {
	  len = strlen (srs_name);
	  if (len > 0)
	    {
		SrsName = malloc (len + 1);
		if (!SrsName)
		    return NULL;
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
		      if (SrsName)
			  free (SrsName);
		      return NULL;
		  }
		strcpy (Proj4Text, proj4text);
	    }
      }

/* allocating the image struct */
    img = malloc (sizeof (gGraphImage));
    if (!img)
	return NULL;

    img->signature = GG_IMAGE_MAGIC_SIGNATURE;
    img->pixels = NULL;
    img->width = width;
    img->height = height;
    img->bits_per_sample = bits_per_sample;
    img->samples_per_pixel = samples_per_pixel;
    img->sample_format = sample_format;
    img->pixel_format = pixel_format;
    img->max_palette = 0;
    img->is_transparent = 0;
    img->tile_width = -1;
    img->tile_height = -1;
    img->rows_per_strip = -1;
    img->compression = GGRAPH_TIFF_COMPRESSION_NONE;
    img->is_georeferenced = 0;
    img->srid = -1;
    img->srs_name = SrsName;
    img->proj4text = Proj4Text;
    img->upper_left_x = DBL_MAX;
    img->upper_left_y = DBL_MAX;
    img->pixel_x_size = 0.0;
    img->pixel_y_size = 0.0;
    img->no_data_value = 0.0 - DBL_MAX;
    img->min_value = DBL_MAX;
    img->max_value = 0.0 - DBL_MAX;

/* computing the scanline size */
    if (pixel_format == GG_PIXEL_GRAYSCALE || pixel_format == GG_PIXEL_PALETTE)
      {
	  img->scanline_width = width;
	  img->pixel_size = 1;
      }
    else if (pixel_format == GG_PIXEL_RGB || pixel_format == GG_PIXEL_BGR)
      {
	  img->scanline_width = width * 3;
	  img->pixel_size = 3;
      }
    else if (pixel_format == GG_PIXEL_GRID)
      {
	  if (sample_format == GGRAPH_SAMPLE_FLOAT)
	    {
		if (bits_per_sample == 32)
		  {
		      /* float */
		      img->scanline_width = width * 4;
		      img->pixel_size = 8;
		  }
		else
		  {
		      /* double */
		      img->scanline_width = width * 8;
		      img->pixel_size = 8;
		  }
	    }
	  else
	    {
		/* UINT or INT */
		if (bits_per_sample == 8)
		  {
		      /* 8bits int */
		      img->scanline_width = width;
		      img->pixel_size = 1;
		  }
		else if (bits_per_sample == 16)
		  {
		      /* 16bits int */
		      img->scanline_width = width * 2;
		      img->pixel_size = 2;
		  }
		else
		  {
		      /* 32bits int */
		      img->scanline_width = width * 4;
		      img->pixel_size = 4;
		  }
	    }
      }
    else
      {
	  /* RGBA, ARGB or BGRA */
	  img->scanline_width = width * 4;
	  img->pixel_size = 4;
      }

/* allocating the pixel buffer */
    img->pixels = malloc (img->scanline_width * height);
    if (!img->pixels)
      {
	  free (img);
	  return NULL;
      }
    return img;
}

GGRAPH_PRIVATE gGraphImagePtr
gg_image_create_from_bitmap (unsigned char *bitmap, int pixel_format, int width,
			     int height, int bits_per_sample,
			     int samples_per_pixel, int sample_format,
			     const char *srs_name, const char *proj4text)
{
/* creating a generic image */
    gGraphImagePtr img;
    int len;
    char *SrsName = NULL;
    char *Proj4Text = NULL;

/* checking PIXEL_FORMAT */
    if (pixel_format == GG_PIXEL_RGB || pixel_format == GG_PIXEL_RGBA
	|| pixel_format == GG_PIXEL_ARGB || pixel_format == GG_PIXEL_BGR
	|| pixel_format == GG_PIXEL_GRAYSCALE
	|| pixel_format == GG_PIXEL_PALETTE || pixel_format == GG_PIXEL_GRID)
	;
    else
	return NULL;
    if (sample_format == GGRAPH_SAMPLE_UINT
	|| sample_format == GGRAPH_SAMPLE_INT
	|| sample_format == GGRAPH_SAMPLE_FLOAT)
	;
    else
	return NULL;

    if (srs_name)
      {
	  len = strlen (srs_name);
	  if (len > 0)
	    {
		SrsName = malloc (len + 1);
		if (!SrsName)
		    return NULL;
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
		      if (SrsName)
			  free (SrsName);
		      return NULL;
		  }
		strcpy (Proj4Text, proj4text);
	    }
      }

/* allocating the image struct */
    img = malloc (sizeof (gGraphImage));
    if (!img)
	return NULL;

    img->signature = GG_IMAGE_MAGIC_SIGNATURE;
    img->pixels = NULL;
    img->width = width;
    img->height = height;
    img->bits_per_sample = bits_per_sample;
    img->samples_per_pixel = samples_per_pixel;
    img->sample_format = sample_format;
    img->pixel_format = pixel_format;
    img->max_palette = 0;
    img->is_transparent = 0;
    img->tile_width = -1;
    img->tile_height = -1;
    img->rows_per_strip = -1;
    img->compression = GGRAPH_TIFF_COMPRESSION_NONE;
    img->is_georeferenced = 0;
    img->srid = -1;
    img->srs_name = SrsName;
    img->proj4text = Proj4Text;
    img->upper_left_x = DBL_MAX;
    img->upper_left_y = DBL_MAX;
    img->pixel_x_size = 0.0;
    img->pixel_y_size = 0.0;
    img->no_data_value = 0.0 - DBL_MAX;
    img->min_value = DBL_MAX;
    img->max_value = 0.0 - DBL_MAX;

/* computing the scanline size */
    if (pixel_format == GG_PIXEL_GRAYSCALE || pixel_format == GG_PIXEL_PALETTE)
      {
	  img->scanline_width = width;
	  img->pixel_size = 1;
      }
    else if (pixel_format == GG_PIXEL_RGB || pixel_format == GG_PIXEL_BGR)
      {
	  img->scanline_width = width * 3;
	  img->pixel_size = 3;
      }
    else if (pixel_format == GG_PIXEL_GRID)
      {
	  if (sample_format == GGRAPH_SAMPLE_FLOAT)
	    {
		if (bits_per_sample == 32)
		  {
		      /* float */
		      img->scanline_width = width * 4;
		      img->pixel_size = 8;
		  }
		else
		  {
		      /* double */
		      img->scanline_width = width * 8;
		      img->pixel_size = 8;
		  }
	    }
	  else
	    {
		/* UINT or INT */
		if (bits_per_sample == 8)
		  {
		      /* 8bits int */
		      img->scanline_width = width;
		      img->pixel_size = 1;
		  }
		else if (bits_per_sample == 16)
		  {
		      /* 16bits int */
		      img->scanline_width = width * 2;
		      img->pixel_size = 2;
		  }
		else
		  {
		      /* 32bits int */
		      img->scanline_width = width * 4;
		      img->pixel_size = 4;
		  }
	    }
      }
    else
      {
	  /* RGBA, ARGB or BGRA */
	  img->scanline_width = width * 4;
	  img->pixel_size = 4;
      }

/* setting up the pixel buffer */
    img->pixels = bitmap;
    return img;
}

GGRAPH_PRIVATE void
gg_image_destroy (gGraphImagePtr img)
{
/* destroying a generic image */
    if (!img)
	return;
    if (img->pixels)
	free (img->pixels);
    if (img->srs_name)
	free (img->srs_name);
    if (img->proj4text)
	free (img->proj4text);
    free (img);
}

GGRAPH_PRIVATE gGraphStripImagePtr
gg_strip_image_create (FILE * file_handle, int codec_id, int pixel_format,
		       int width, int height, int bits_per_sample,
		       int samples_per_pixel, int sample_format,
		       const char *srs_name, const char *proj4text)
{
/* creating a file-based image implementing access by strips */
    gGraphStripImagePtr img;
    int len;
    char *SrsName = NULL;
    char *Proj4Text = NULL;

/* checking PIXEL_FORMAT */
    if (pixel_format == GG_PIXEL_RGB || pixel_format == GG_PIXEL_RGBA
	|| pixel_format == GG_PIXEL_ARGB || pixel_format == GG_PIXEL_BGR
	|| pixel_format == GG_PIXEL_GRAYSCALE
	|| pixel_format == GG_PIXEL_PALETTE || pixel_format == GG_PIXEL_GRID)
	;
    else
	return NULL;
    if (sample_format == GGRAPH_SAMPLE_UINT
	|| sample_format == GGRAPH_SAMPLE_INT
	|| sample_format == GGRAPH_SAMPLE_FLOAT)
	;
    else
	return NULL;

    if (srs_name)
      {
	  len = strlen (srs_name);
	  if (len > 0)
	    {
		SrsName = malloc (len + 1);
		if (!SrsName)
		    return NULL;
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
		      if (SrsName)
			  free (SrsName);
		      return NULL;
		  }
		strcpy (Proj4Text, proj4text);
	    }
      }

/* allocating the image struct */
    img = malloc (sizeof (gGraphStripImage));
    if (!img)
	return NULL;

    img->signature = GG_STRIP_IMAGE_MAGIC_SIGNATURE;
    img->file_handle = file_handle;
    img->codec_id = codec_id;
    img->rows_per_block = 0;
    img->current_available_rows = 0;
    img->codec_data = NULL;
    img->pixels = NULL;
    img->next_row = 0;
    img->codec_data = NULL;
    img->width = width;
    img->height = height;
    img->bits_per_sample = bits_per_sample;
    img->samples_per_pixel = samples_per_pixel;
    img->sample_format = sample_format;
    img->pixel_format = pixel_format;
    img->max_palette = 0;
    img->is_transparent = 0;
    img->tile_width = -1;
    img->tile_height = -1;
    img->rows_per_strip = -1;
    img->compression = GGRAPH_TIFF_COMPRESSION_NONE;
    img->is_georeferenced = 0;
    img->srid = -1;
    img->srs_name = SrsName;
    img->proj4text = Proj4Text;
    img->upper_left_x = DBL_MAX;
    img->upper_left_y = DBL_MAX;
    img->pixel_x_size = 0.0;
    img->pixel_y_size = 0.0;
    img->no_data_value = 0.0 - DBL_MAX;
    img->min_value = DBL_MAX;
    img->max_value = 0.0 - DBL_MAX;

/* computing the scanline size */
    if (pixel_format == GG_PIXEL_GRAYSCALE || pixel_format == GG_PIXEL_PALETTE)
      {
	  img->scanline_width = width;
	  img->pixel_size = 1;
      }
    else if (pixel_format == GG_PIXEL_RGB || pixel_format == GG_PIXEL_BGR)
      {
	  img->scanline_width = width * 3;
	  img->pixel_size = 3;
      }
    else if (pixel_format == GG_PIXEL_GRID)
      {
	  if (sample_format == GGRAPH_SAMPLE_FLOAT)
	    {
		if (bits_per_sample == 32)
		  {
		      /* float */
		      img->scanline_width = width * 4;
		      img->pixel_size = 8;
		  }
		else
		  {
		      /* double */
		      img->scanline_width = width * 8;
		      img->pixel_size = 8;
		  }
	    }
	  else
	    {
		/* UINT or INT */
		if (bits_per_sample == 8)
		  {
		      /* 8bits int */
		      img->scanline_width = width;
		      img->pixel_size = 1;
		  }
		else if (bits_per_sample == 16)
		  {
		      /* 16bits int */
		      img->scanline_width = width * 2;
		      img->pixel_size = 2;
		  }
		else
		  {
		      /* 32bits int */
		      img->scanline_width = width * 4;
		      img->pixel_size = 4;
		  }
	    }
      }
    else
      {
	  /* RGBA, ARGB or BGRA */
	  img->scanline_width = width * 4;
	  img->pixel_size = 4;
      }

    return img;
}

GGRAPH_PRIVATE void
gg_strip_image_destroy (gGraphStripImagePtr img)
{
/* destroying a file-based image implementing access by strips */
    if (!img)
	return;
    if (img->codec_id == GGRAPH_IMAGE_PNG)
	gg_png_codec_destroy (img->codec_data);
    if (img->codec_id == GGRAPH_IMAGE_JPEG)
	gg_jpeg_codec_destroy (img->codec_data);
    if (img->codec_id == GGRAPH_IMAGE_TIFF
	|| img->codec_id == GGRAPH_IMAGE_GEOTIFF)
	gg_tiff_codec_destroy (img->codec_data);
    if (img->codec_id == GGRAPH_IMAGE_HGT)
	gg_grid_codec_destroy (img->codec_data);
    if (img->file_handle)
	fclose (img->file_handle);
    if (img->pixels)
	free (img->pixels);
    if (img->srs_name)
	free (img->srs_name);
    if (img->proj4text)
	free (img->proj4text);
    free (img);
}

GGRAPH_PRIVATE void
gg_image_clone_georeferencing (const gGraphImagePtr dst,
			       const gGraphImagePtr src)
{
/* adjusting georeferencing infos between two images */
    double size_x;
    double size_y;
    int len;
    char *SrsName = NULL;
    char *Proj4Text = NULL;
    if (src->is_georeferenced)
      {
	  if (src->srs_name)
	    {
		len = strlen (src->srs_name);
		if (len > 0)
		  {
		      SrsName = malloc (len + 1);
		      if (SrsName)
			  strcpy (SrsName, src->srs_name);
		  }
	    }
	  if (src->proj4text)
	    {
		len = strlen (src->proj4text);
		if (len > 0)
		  {
		      Proj4Text = malloc (len + 1);
		      if (Proj4Text)
			  strcpy (Proj4Text, src->proj4text);
		  }
	    }
      }

/* cleaning up destination georeferencing */
    dst->is_georeferenced = 0;
    dst->srid = -1;
    if (dst->srs_name)
	free (dst->srs_name);
    if (dst->proj4text)
	free (dst->proj4text);
    dst->srs_name = NULL;
    dst->proj4text = NULL;
    dst->upper_left_x = DBL_MAX;
    dst->upper_left_y = DBL_MAX;
    dst->pixel_x_size = 0.0;
    dst->pixel_y_size = 0.0;
    if (src->is_georeferenced == 0)
	return;

/* setting up destination georeferencing */
    dst->is_georeferenced = 1;
    dst->srid = src->srid;
    dst->srs_name = SrsName;
    dst->proj4text = Proj4Text;
    dst->upper_left_x = src->upper_left_x;
    dst->upper_left_y = src->upper_left_y;
    size_x = (double) (src->width) * src->pixel_x_size;
    size_y = (double) (src->height) * src->pixel_y_size;
    dst->pixel_x_size = size_x / (double) (dst->width);
    dst->pixel_y_size = size_y / (double) (dst->height);

}

static void
sub_set_georeferencing (const gGraphImagePtr dst, const gGraphImagePtr src,
			int upper_left_x, int upper_left_y)
{
/* adjusting georeferencing infos between two images [ImageSubSet] */
    int len;
    char *SrsName = NULL;
    char *Proj4Text = NULL;
    if (src->is_georeferenced)
      {
	  if (src->srs_name)
	    {
		len = strlen (src->srs_name);
		if (len > 0)
		  {
		      SrsName = malloc (len + 1);
		      if (SrsName)
			  strcpy (SrsName, src->srs_name);
		  }
	    }
	  if (src->proj4text)
	    {
		len = strlen (src->proj4text);
		if (len > 0)
		  {
		      Proj4Text = malloc (len + 1);
		      if (Proj4Text)
			  strcpy (Proj4Text, src->proj4text);
		  }
	    }
      }

/* cleaning up destination georeferencing */
    dst->is_georeferenced = 0;
    dst->srid = -1;
    if (dst->srs_name)
	free (dst->srs_name);
    if (dst->proj4text)
	free (dst->proj4text);
    dst->srs_name = NULL;
    dst->proj4text = NULL;
    dst->upper_left_x = DBL_MAX;
    dst->upper_left_y = DBL_MAX;
    dst->pixel_x_size = 0.0;
    dst->pixel_y_size = 0.0;
    if (src->is_georeferenced == 0)
	return;

/* setting up destination georeferencing */
    dst->is_georeferenced = 1;
    dst->srid = src->srid;
    dst->srs_name = SrsName;
    dst->proj4text = Proj4Text;
    dst->upper_left_x =
	src->upper_left_x + ((double) upper_left_x * src->pixel_x_size);
    dst->upper_left_y =
	src->upper_left_y - ((double) upper_left_y * src->pixel_y_size);
    dst->pixel_x_size = src->pixel_x_size;
    dst->pixel_y_size = src->pixel_y_size;

}

GGRAPH_PRIVATE void
gg_image_sub_set (const gGraphImagePtr dst, const gGraphImagePtr src,
		  int upper_left_x, int upper_left_y)
{
/* creating an ImageSubSet */
    int x;
    int y;
    int dx;
    int dy = 0;
    int c;
    unsigned char *p_in;
    unsigned char *p_out;

    if (src->pixel_format != dst->pixel_format)
      {
	  /* an absolutely unexpected condition !!!! */
	  return;
      }

    for (y = upper_left_y; y < src->height; y++, dy++)
      {
	  if (dy >= dst->height)
	      break;
	  dx = 0;
	  p_in = src->pixels + (y * src->scanline_width);
	  p_in += upper_left_x * src->pixel_size;
	  p_out = dst->pixels + (dy * dst->scanline_width);
	  for (x = upper_left_x; x < src->width; x++, dx++)
	    {
		if (dx >= dst->width)
		    break;
		/* copying pixel's components */
		for (c = 0; c < src->pixel_size; c++)
		    *p_out++ = *p_in++;
	    }
      }
    if (src->pixel_format == GG_PIXEL_PALETTE)
      {
	  /* copying palette entries */
	  for (x = 0; x < src->max_palette; x++)
	    {
		dst->palette_red[x] = src->palette_red[x];
		dst->palette_green[x] = src->palette_green[x];
		dst->palette_blue[x] = src->palette_blue[x];
	    }
	  dst->max_palette = src->max_palette;
      }

/* computing georeferencing */
    sub_set_georeferencing (dst, src, upper_left_x, upper_left_y);
}

static unsigned char
to_grayscale (unsigned char r, unsigned char g, unsigned char b)
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

GGRAPH_PRIVATE void
gg_image_fill (const gGraphImagePtr img, unsigned char r, unsigned char g,
	       unsigned char b, unsigned char alpha)
{
/* filling the image with given color */
    int x;
    int y;
    unsigned char *p;
    unsigned char gray;

/* resetting the palette anyway */
    img->max_palette = 1;
    img->palette_red[0] = r;
    img->palette_green[0] = g;
    img->palette_blue[0] = b;

/* precomputing a grayscale value anyway */
    if (r == g && g == b)
	gray = r;
    else
	gray = to_grayscale (r, g, b);

    for (y = 0; y < img->height; y++)
      {
	  /* setting pixels by scanline */
	  p = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		/* setting pixels */
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      *p++ = r;
		      *p++ = g;
		      *p++ = b;
		  }
		else if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      *p++ = r;
		      *p++ = g;
		      *p++ = b;
		      *p++ = alpha;
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      *p++ = alpha;
		      *p++ = r;
		      *p++ = g;
		      *p++ = b;
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      *p++ = b;
		      *p++ = g;
		      *p++ = r;
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      *p++ = b;
		      *p++ = g;
		      *p++ = r;
		      *p++ = alpha;
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		    *p++ = gray;
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		    *p++ = 0;
	    }
      }
}

GGRAPH_PRIVATE unsigned char
gg_match_palette (const gGraphImagePtr img, unsigned char r, unsigned char g,
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

static void
shrink_by (const gGraphImagePtr dst, const gGraphImagePtr src)
{
/*
/ this code is widely base upon the original wxWidgets gwxImage wxImage::ShrinkBy(() function
*/
    int xFactor = src->width / dst->width;
    int yFactor = src->height / dst->height;
    int x;
    int y;
    int x1;
    int y1;
    int y_offset;
    int x_offset;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char gray;
    unsigned char *p;
    for (y = 0; y < dst->height; y++)
      {
	  for (x = 0; x < dst->width; x++)
	    {
		/* determine average */
		unsigned int avgRed = 0;
		unsigned int avgGreen = 0;
		unsigned int avgBlue = 0;
		unsigned int counter = 0;
		for (y1 = 0; y1 < yFactor; ++y1)
		  {
		      y_offset = (y * yFactor + y1);
		      for (x1 = 0; x1 < xFactor; ++x1)
			{
			    x_offset = (x * xFactor) + x1;
			    p = src->pixels + (y_offset * src->scanline_width);
			    if (src->pixel_format == GG_PIXEL_RGB)
			      {
				  p += x_offset * 3;
				  red = *p++;
				  green = *p++;
				  blue = *p;
			      }
			    else if (src->pixel_format == GG_PIXEL_RGBA)
			      {
				  p += x_offset * 4;
				  red = *p++;
				  green = *p++;
				  blue = *p;
			      }
			    else if (src->pixel_format == GG_PIXEL_ARGB)
			      {
				  p += x_offset * 4;
				  p++;
				  red = *p++;
				  green = *p++;
				  blue = *p;
			      }
			    else if (src->pixel_format == GG_PIXEL_BGR)
			      {
				  p += x_offset * 3;
				  blue = *p++;
				  green = *p++;
				  red = *p;
			      }
			    else if (src->pixel_format == GG_PIXEL_BGRA)
			      {
				  p += x_offset * 4;
				  blue = *p++;
				  green = *p++;
				  red = *p;
			      }
			    else if (src->pixel_format == GG_PIXEL_GRAYSCALE)
			      {
				  p += x_offset;
				  red = *p;
				  green = *p;
				  blue = *p;
			      }
			    else if (src->pixel_format == GG_PIXEL_PALETTE)
			      {
				  p += x_offset;
				  red = src->palette_red[*p];
				  green = src->palette_green[*p];
				  blue = src->palette_blue[*p];
			      }
			    avgRed += red;
			    avgGreen += green;
			    avgBlue += blue;
			    counter++;
			}
		  }
		/* storing the pixel into destination */
		red = avgRed / counter;
		green = avgGreen / counter;
		blue = avgBlue / counter;
		p = dst->pixels + (y * dst->scanline_width);
		if (dst->pixel_format == GG_PIXEL_RGB)
		  {
		      p += x * 3;
		      *p++ = red;
		      *p++ = green;
		      *p = blue;
		  }
		else if (dst->pixel_format == GG_PIXEL_RGBA)
		  {
		      p += x * 4;
		      *p++ = red;
		      *p++ = green;
		      *p++ = blue;
		      *p = 255;	/* opaque */
		  }
		else if (dst->pixel_format == GG_PIXEL_ARGB)
		  {
		      p += x * 4;
		      *p++ = 255;	/* opaque */
		      *p++ = red;
		      *p++ = green;
		      *p = blue;
		  }
		else if (dst->pixel_format == GG_PIXEL_BGR)
		  {
		      p += x * 3;
		      *p++ = blue;
		      *p++ = green;
		      *p = red;
		  }
		else if (dst->pixel_format == GG_PIXEL_BGRA)
		  {
		      p += x * 4;
		      *p++ = blue;
		      *p++ = green;
		      *p++ = red;
		      *p = 255;	/* opaque */
		  }
		else if (dst->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      p += x;
		      if (red == green && green == blue)
			  gray = red;
		      else
			  gray = to_grayscale (red, green, blue);
		      *p = gray;
		  }
		else if (dst->pixel_format == GG_PIXEL_PALETTE)
		  {
		      unsigned char index =
			  gg_match_palette (dst, red, green, blue);
		      p += x;
		      *p = index;
		  }
	    }
      }
}

GGRAPH_PRIVATE void
gg_image_resize (const gGraphImagePtr dst, const gGraphImagePtr src)
{
/*
/ this function builds an ordinary quality resized image, applying pixel replication
/
/ this code is widely base upon the original wxWidgets gwxImage wxImage::Scale(() function
/ wxIMAGE_QUALITY_NORMAL
*/
    int x_delta;
    int y_delta;
    int y;
    int j;
    int x;
    int i;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    unsigned char gray;
    unsigned char *p;

    if ((src->width % dst->width) == 0 && src->width >= dst->width
	&& (src->height % dst->height) == 0 && src->height >= dst->height)
      {
	  shrink_by (dst, src);
	  return;
      }
    x = src->width;
    y = src->height;
    x_delta = (x << 16) / dst->width;
    y_delta = (y << 16) / dst->height;
    y = 0;
    for (j = 0; j < dst->height; j++)
      {
	  x = 0;
	  for (i = 0; i < dst->width; i++)
	    {
		/* retrieving the origin pixel */
		p = src->pixels + ((y >> 16) * src->scanline_width);
		if (src->pixel_format == GG_PIXEL_RGB)
		  {
		      p += (x >> 16) * 3;
		      red = *p++;
		      green = *p++;
		      blue = *p;
		      alpha = 255;
		  }
		else if (src->pixel_format == GG_PIXEL_RGBA)
		  {
		      p += (x >> 16) * 4;
		      red = *p++;
		      green = *p++;
		      blue = *p++;
		      alpha = *p;
		  }
		else if (src->pixel_format == GG_PIXEL_ARGB)
		  {
		      p += (x >> 16) * 4;
		      alpha = *p++;
		      red = *p++;
		      green = *p++;
		      blue = *p;
		  }
		else if (src->pixel_format == GG_PIXEL_BGR)
		  {
		      p += (x >> 16) * 3;
		      blue = *p++;
		      green = *p++;
		      red = *p;
		      alpha = 255;
		  }
		else if (src->pixel_format == GG_PIXEL_BGRA)
		  {
		      p += (x >> 16) * 4;
		      blue = *p++;
		      green = *p++;
		      red = *p++;
		      alpha = *p;
		  }
		else if (src->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      p += (x >> 16);
		      red = *p;
		      green = *p;
		      blue = *p;
		      alpha = 255;
		  }
		else if (src->pixel_format == GG_PIXEL_PALETTE)
		  {
		      p += (x >> 16);
		      red = src->palette_red[*p];
		      green = src->palette_green[*p];
		      blue = src->palette_blue[*p];
		      alpha = 255;
		  }
		/* setting the destination pixel */
		p = dst->pixels + (j * dst->scanline_width);
		if (dst->pixel_format == GG_PIXEL_RGB)
		  {
		      p += i * 3;
		      *p++ = red;
		      *p++ = green;
		      *p = blue;
		  }
		else if (dst->pixel_format == GG_PIXEL_RGBA)
		  {
		      p += i * 4;
		      *p++ = red;
		      *p++ = green;
		      *p++ = blue;
		      *p = alpha;
		  }
		else if (dst->pixel_format == GG_PIXEL_ARGB)
		  {
		      p += i * 4;
		      *p++ = alpha;
		      *p++ = red;
		      *p++ = green;
		      *p = blue;
		  }
		else if (dst->pixel_format == GG_PIXEL_BGR)
		  {
		      p += i * 3;
		      *p++ = blue;
		      *p++ = green;
		      *p = red;
		  }
		else if (dst->pixel_format == GG_PIXEL_BGRA)
		  {
		      p += i * 4;
		      *p++ = blue;
		      *p++ = green;
		      *p++ = red;
		      *p = alpha;
		  }
		else if (dst->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      p += i;
		      if (red == green && green == blue)
			  gray = red;
		      else
			  gray = to_grayscale (red, green, blue);
		      *p = gray;
		  }
		else if (dst->pixel_format == GG_PIXEL_PALETTE)
		  {
		      unsigned char index =
			  gg_match_palette (dst, red, green, blue);
		      p += i;
		      *p = index;
		  }
		x += x_delta;
	    }
	  y += y_delta;
      }
}

static void
shrink_grid_by (const gGraphImagePtr dst, const gGraphImagePtr src)
{
/*
/ this code is widely base upon the original wxWidgets gwxImage wxImage::ShrinkBy(() function
*/
    int xFactor = src->width / dst->width;
    int yFactor = src->height / dst->height;
    int x;
    int y;
    int x1;
    int y1;
    int y_offset;
    int x_offset;
    short short_value;
    unsigned short ushort_value;
    int int_value;
    unsigned int uint_value;
    float float_value;
    double double_value;
    short *p_short;
    unsigned short *p_ushort;
    int *p_int;
    unsigned int *p_uint;
    float *p_float;
    double *p_double;
    for (y = 0; y < dst->height; y++)
      {
	  for (x = 0; x < dst->width; x++)
	    {
		/* determine average */
		int is_nodata = 0;
		double avg;
		unsigned int counter = 0;
		unsigned char *p;
		for (y1 = 0; y1 < yFactor; ++y1)
		  {
		      y_offset = (y * yFactor + y1);
		      for (x1 = 0; x1 < xFactor; ++x1)
			{
			    x_offset = (x * xFactor) + x1;
			    p = src->pixels + (y_offset * src->scanline_width);
			    if (src->pixel_format == GG_PIXEL_GRID)
			      {
				  p += x_offset * (src->bits_per_sample / 8);
				  switch (src->sample_format)
				    {
				    case GGRAPH_SAMPLE_INT:
					if (src->bits_per_sample == 16)
					  {
					      p_short = (short *) p;
					      short_value = *p_short;
					      if (short_value ==
						  src->no_data_value)
						  is_nodata = 1;
					      if (!is_nodata)
						  avg += short_value;
					  }
					else
					  {
					      p_int = (int *) p;
					      int_value = *p_int;
					      if (int_value ==
						  src->no_data_value)
						  is_nodata = 1;
					      if (!is_nodata)
						  avg += int_value;
					  }
					break;
				    case GGRAPH_SAMPLE_UINT:
					if (src->bits_per_sample == 16)
					  {
					      p_ushort = (unsigned short *) p;
					      ushort_value = *p_ushort;
					      if (ushort_value ==
						  src->no_data_value)
						  is_nodata = 1;
					      if (!is_nodata)
						  avg += ushort_value;
					  }
					else
					  {
					      p_uint = (unsigned int *) p;
					      uint_value = *p_uint;
					      if (uint_value ==
						  src->no_data_value)
						  is_nodata = 1;
					      if (!is_nodata)
						  avg += uint_value;
					  }
					break;
				    case GGRAPH_SAMPLE_FLOAT:
					if (src->bits_per_sample == 32)
					  {
					      p_float = (float *) p;
					      float_value = *p_float;
					      if (float_value ==
						  src->no_data_value)
						  is_nodata = 1;
					      if (!is_nodata)
						  avg += float_value;
					  }
					else
					  {
					      p_double = (double *) p;
					      double_value = *p_double;
					      if (double_value ==
						  src->no_data_value)
						  is_nodata = 1;
					      if (!is_nodata)
						  avg += double_value;
					  }
					break;
				    };
			      }
			    counter++;
			}
		  }
		/* storing the pixel into destination */
		p = dst->pixels + (y * dst->scanline_width);
		if (dst->pixel_format == GG_PIXEL_GRID)
		  {
		      p += x * (dst->bits_per_sample / 8);
		      switch (dst->sample_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (dst->bits_per_sample == 16)
			      {
				  p_short = (short *) p;
				  if (is_nodata)
				      *p_short = (short) (dst->no_data_value);
				  else
				      *p_short = (short) (avg / counter);
			      }
			    else
			      {
				  p_int = (int *) p;
				  if (is_nodata)
				      *p_int = (int) (dst->no_data_value);
				  else
				      *p_int = (int) (avg / counter);
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (dst->bits_per_sample == 16)
			      {
				  p_ushort = (unsigned short *) p;
				  if (is_nodata)
				      *p_ushort =
					  (unsigned short) (dst->no_data_value);
				  else
				      *p_ushort =
					  (unsigned short) (avg / counter);
			      }
			    else
			      {
				  p_uint = (unsigned int *) p;
				  if (is_nodata)
				      *p_uint =
					  (unsigned int) (dst->no_data_value);
				  else
				      *p_uint = (unsigned int) (avg / counter);
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (dst->bits_per_sample == 32)
			      {
				  p_float = (float *) p;
				  if (is_nodata)
				      *p_float = (float) (dst->no_data_value);
				  else
				      *p_float = (float) (avg / counter);
			      }
			    else
			      {
				  p_double = (double *) p;
				  if (is_nodata)
				      *p_double = dst->no_data_value;
				  else
				      *p_double = avg / counter;
			      }
			    break;
			};
		  }
	    }
      }
}

GGRAPH_PRIVATE void
gg_grid_resize (const gGraphImagePtr dst, const gGraphImagePtr src)
{
/*
/ this function builds an ordinary quality resized GRID, applying pixel replication
/
/ this code is widely base upon the original wxWidgets gwxImage wxImage::Scale(() function
/ wxIMAGE_QUALITY_NORMAL
*/
    int x_delta;
    int y_delta;
    int y;
    int j;
    int x;
    int i;
    short short_value;
    unsigned short ushort_value;
    int int_value;
    unsigned int uint_value;
    float float_value;
    double double_value;
    short *p_short;
    unsigned short *p_ushort;
    int *p_int;
    unsigned int *p_uint;
    float *p_float;
    double *p_double;

    if ((src->width % dst->width) == 0 && src->width >= dst->width
	&& (src->height % dst->height) == 0 && src->height >= dst->height)
      {
	  shrink_grid_by (dst, src);
	  return;
      }
    x = src->width;
    y = src->height;
    x_delta = (x << 16) / dst->width;
    y_delta = (y << 16) / dst->height;
    y = 0;
    for (j = 0; j < dst->height; j++)
      {
	  x = 0;
	  for (i = 0; i < dst->width; i++)
	    {
		/* retrieving the origin pixel */
		unsigned char *p =
		    src->pixels + ((y >> 16) * src->scanline_width);
		if (src->pixel_format == GG_PIXEL_GRID)
		  {
		      p += (x >> 16) * (src->bits_per_sample / 8);
		      switch (src->sample_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (src->bits_per_sample == 16)
			      {
				  p_short = (short *) p;
				  short_value = *p_short;
				  p += sizeof (short);
			      }
			    else
			      {
				  p_int = (int *) p;
				  int_value = *p_int;
				  p += sizeof (int);
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (src->bits_per_sample == 16)
			      {
				  p_ushort = (unsigned short *) p;
				  ushort_value = *p_ushort;
				  p += sizeof (unsigned short);
			      }
			    else
			      {
				  p_uint = (unsigned int *) p;
				  uint_value = *p_uint;
				  p += sizeof (unsigned int);
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (src->bits_per_sample == 32)
			      {
				  p_float = (float *) p;
				  float_value = *p_float;
				  p += sizeof (float);
			      }
			    else
			      {
				  p_double = (double *) p;
				  double_value = *p_double;
				  p += sizeof (double);
			      }
			    break;
			};
		  }
		/* setting the destination pixel */
		p = dst->pixels + (j * dst->scanline_width);
		if (dst->pixel_format == GG_PIXEL_GRID)
		  {
		      p += i * (dst->bits_per_sample / 8);
		      switch (dst->sample_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (dst->bits_per_sample == 16)
			      {
				  p_short = (short *) p;
				  *p_short = short_value;
			      }
			    else
			      {
				  p_int = (int *) p;
				  *p_int = int_value;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (dst->bits_per_sample == 16)
			      {
				  p_ushort = (unsigned short *) p;
				  *p_ushort = ushort_value;
			      }
			    else
			      {
				  p_uint = (unsigned int *) p;
				  *p_uint = uint_value;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (dst->bits_per_sample == 32)
			      {
				  p_float = (float *) p;
				  *p_float = float_value;
			      }
			    else
			      {
				  p_double = (double *) p;
				  *p_double = double_value;
			      }
			    break;
			};
		  }
		x += x_delta;
	    }
	  y += y_delta;
      }
}

#define floor2(exp) ((long) exp)

GGRAPH_PRIVATE void
gg_make_thumbnail (const gGraphImagePtr thumbnail, const gGraphImagePtr image)
{
/*
/ this function builds an high quality thumbnail image, applying pixel interpolation
/
/ this code is widely base upon the original GD gdImageCopyResampled() function
*/
    int x, y;
    double sy1, sy2, sx1, sx2;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
    unsigned char gray;
    unsigned char *p;
    for (y = 0; y < thumbnail->height; y++)
      {
	  sy1 =
	      ((double) y) * (double) image->height /
	      (double) thumbnail->height;
	  sy2 =
	      ((double) (y + 1)) * (double) image->height /
	      (double) thumbnail->height;
	  for (x = 0; x < thumbnail->width; x++)
	    {
		double sx, sy;
		double spixels = 0;
		double red = 0.0, green = 0.0, blue = 0.0, alpha = 0.0;
		sx1 =
		    ((double) x) * (double) image->width /
		    (double) thumbnail->width;
		sx2 =
		    ((double) (x + 1)) * (double) image->width /
		    (double) thumbnail->width;
		sy = sy1;
		do
		  {
		      double yportion;
		      if (floor2 (sy) == floor2 (sy1))
			{
			    yportion = 1.0 - (sy - floor2 (sy));
			    if (yportion > sy2 - sy1)
			      {
				  yportion = sy2 - sy1;
			      }
			    sy = floor2 (sy);
			}
		      else if (sy == floor2 (sy2))
			{
			    yportion = sy2 - floor2 (sy2);
			}
		      else
			{
			    yportion = 1.0;
			}
		      sx = sx1;
		      do
			{
			    double xportion;
			    double pcontribution;
			    if (floor2 (sx) == floor2 (sx1))
			      {
				  xportion = 1.0 - (sx - floor2 (sx));
				  if (xportion > sx2 - sx1)
				    {
					xportion = sx2 - sx1;
				    }
				  sx = floor2 (sx);
			      }
			    else if (sx == floor2 (sx2))
			      {
				  xportion = sx2 - floor2 (sx2);
			      }
			    else
			      {
				  xportion = 1.0;
			      }
			    pcontribution = xportion * yportion;
			    /* retrieving the origin pixel */
			    p = image->pixels +
				((int) sy * image->scanline_width);
			    if (image->pixel_format == GG_PIXEL_RGB)
			      {
				  p += (int) sx *3;
				  r = *p++;
				  g = *p++;
				  b = *p;
				  a = 255;
			      }
			    else if (image->pixel_format == GG_PIXEL_RGBA)
			      {
				  p += (int) sx *4;
				  r = *p++;
				  g = *p++;
				  b = *p++;
				  a = *p;
			      }
			    else if (image->pixel_format == GG_PIXEL_ARGB)
			      {
				  p += (int) sx *4;
				  a = *p++;
				  r = *p++;
				  g = *p++;
				  b = *p;
			      }
			    else if (image->pixel_format == GG_PIXEL_BGR)
			      {
				  p += (int) sx *3;
				  b = *p++;
				  g = *p++;
				  r = *p;
				  a = 255;
			      }
			    else if (image->pixel_format == GG_PIXEL_BGRA)
			      {
				  p += (int) sx *4;
				  b = *p++;
				  g = *p++;
				  r = *p++;
				  a = *p;
			      }
			    else if (image->pixel_format == GG_PIXEL_GRAYSCALE)
			      {
				  p += (int) sx;
				  r = *p;
				  g = *p;
				  b = *p;
				  a = 255;
			      }
			    else if (image->pixel_format == GG_PIXEL_PALETTE)
			      {
				  p += (int) sx;
				  r = image->palette_red[*p];
				  g = image->palette_green[*p];
				  b = image->palette_blue[*p];
				  a = 255;
			      }
			    red += r * pcontribution;
			    green += g * pcontribution;
			    blue += b * pcontribution;
			    alpha += a * pcontribution;
			    spixels += xportion * yportion;
			    sx += 1.0;
			}
		      while (sx < sx2);
		      sy += 1.0;
		  }
		while (sy < sy2);
		if (spixels != 0.0)
		  {
		      red /= spixels;
		      green /= spixels;
		      blue /= spixels;
		      alpha /= spixels;
		  }
		if (red > 255.0)
		    red = 255.0;
		if (green > 255.0)
		    green = 255.0;
		if (blue > 255.0)
		    blue = 255.0;
		if (alpha > 255.0)
		    alpha = 255.0;
		/* setting the destination pixel */
		p = thumbnail->pixels + (y * thumbnail->scanline_width);
		if (thumbnail->pixel_format == GG_PIXEL_RGB)
		  {
		      p += x * 3;
		      *p++ = (unsigned char) red;
		      *p++ = (unsigned char) green;
		      *p = (unsigned char) blue;
		  }
		else if (thumbnail->pixel_format == GG_PIXEL_RGBA)
		  {
		      p += x * 4;
		      *p++ = (unsigned char) red;
		      *p++ = (unsigned char) green;
		      *p++ = (unsigned char) blue;
		      *p = (unsigned char) alpha;
		  }
		else if (thumbnail->pixel_format == GG_PIXEL_ARGB)
		  {
		      p += x * 4;
		      *p++ = (unsigned char) alpha;
		      *p++ = (unsigned char) red;
		      *p++ = (unsigned char) green;
		      *p = (unsigned char) blue;
		  }
		else if (thumbnail->pixel_format == GG_PIXEL_BGR)
		  {
		      p += x * 3;
		      *p++ = (unsigned char) blue;
		      *p++ = (unsigned char) green;
		      *p = (unsigned char) red;
		  }
		else if (thumbnail->pixel_format == GG_PIXEL_BGRA)
		  {
		      p += x * 4;
		      *p++ = (unsigned char) blue;
		      *p++ = (unsigned char) green;
		      *p++ = (unsigned char) red;
		      *p = (unsigned char) alpha;
		  }
		else if (thumbnail->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      p += x;
		      if (red == green && green == blue)
			  gray = (unsigned char) red;
		      else
			  gray =
			      to_grayscale ((unsigned char) red,
					    (unsigned char) green,
					    (unsigned char) blue);
		      *p = gray;
		  }
		else if (thumbnail->pixel_format == GG_PIXEL_PALETTE)
		  {
		      unsigned char index =
			  gg_match_palette (thumbnail, (unsigned char) red,
					    (unsigned char) green,
					    (unsigned char) blue);
		      p += x;
		      *p = index;
		  }
	    }
      }
}

GGRAPH_PRIVATE void
gg_make_grid_thumbnail (const gGraphImagePtr thumbnail,
			const gGraphImagePtr image)
{
/*
/ this function builds an high quality thumbnail image, applying pixel interpolation
/
/ this code is widely base upon the original GD gdImageCopyResampled() function
*/
    int x, y;
    double sy1, sy2, sx1, sx2;
    short short_value;
    unsigned short ushort_value;
    int int_value;
    unsigned int uint_value;
    float float_value;
    double double_value;
    short *p_short;
    unsigned short *p_ushort;
    int *p_int;
    unsigned int *p_uint;
    float *p_float;
    double *p_double;
    unsigned char *p;
    for (y = 0; y < thumbnail->height; y++)
      {
	  sy1 =
	      ((double) y) * (double) image->height /
	      (double) thumbnail->height;
	  sy2 =
	      ((double) (y + 1)) * (double) image->height /
	      (double) thumbnail->height;
	  for (x = 0; x < thumbnail->width; x++)
	    {
		double sx, sy;
		double spixels = 0;
		double avg = 0.0;
		int is_nodata = 0;
		sx1 =
		    ((double) x) * (double) image->width /
		    (double) thumbnail->width;
		sx2 =
		    ((double) (x + 1)) * (double) image->width /
		    (double) thumbnail->width;
		sy = sy1;
		do
		  {
		      double yportion;
		      if (floor2 (sy) == floor2 (sy1))
			{
			    yportion = 1.0 - (sy - floor2 (sy));
			    if (yportion > sy2 - sy1)
			      {
				  yportion = sy2 - sy1;
			      }
			    sy = floor2 (sy);
			}
		      else if (sy == floor2 (sy2))
			{
			    yportion = sy2 - floor2 (sy2);
			}
		      else
			{
			    yportion = 1.0;
			}
		      sx = sx1;
		      do
			{
			    double xportion;
			    double pcontribution;
			    if (floor2 (sx) == floor2 (sx1))
			      {
				  xportion = 1.0 - (sx - floor2 (sx));
				  if (xportion > sx2 - sx1)
				    {
					xportion = sx2 - sx1;
				    }
				  sx = floor2 (sx);
			      }
			    else if (sx == floor2 (sx2))
			      {
				  xportion = sx2 - floor2 (sx2);
			      }
			    else
			      {
				  xportion = 1.0;
			      }
			    pcontribution = xportion * yportion;
			    /* retrieving the origin pixel */
			    p = image->pixels +
				((int) sy * image->scanline_width);
			    if (image->pixel_format == GG_PIXEL_GRID)
			      {
				  p += (int) sx *(image->bits_per_sample / 8);
				  switch (image->sample_format)
				    {
				    case GGRAPH_SAMPLE_INT:
					if (image->bits_per_sample == 16)
					  {
					      p_short = (short *) p;
					      short_value = *p_short;
					      if (short_value ==
						  image->no_data_value)
						  is_nodata = 1;
					      else
						  avg +=
						      (double) short_value
						      * pcontribution;
					  }
					else
					  {
					      p_int = (int *) p;
					      int_value = *p_int;
					      if (int_value ==
						  image->no_data_value)
						  is_nodata = 1;
					      else
						  avg +=
						      (double) int_value
						      * pcontribution;
					  }
					break;
				    case GGRAPH_SAMPLE_UINT:
					if (image->bits_per_sample == 16)
					  {
					      p_ushort = (unsigned short *) p;
					      ushort_value = *p_ushort;
					      if (ushort_value ==
						  image->no_data_value)
						  is_nodata = 1;
					      else
						  avg +=
						      (double) ushort_value
						      * pcontribution;
					  }
					else
					  {
					      p_uint = (unsigned int *) p;
					      uint_value = *p_uint;
					      if (uint_value ==
						  image->no_data_value)
						  is_nodata = 1;
					      else
						  avg +=
						      (double) uint_value
						      * pcontribution;
					  }
					break;
				    case GGRAPH_SAMPLE_FLOAT:
					if (image->bits_per_sample == 32)
					  {
					      p_float = (float *) p;
					      float_value = *p_float;
					      if (float_value ==
						  image->no_data_value)
						  is_nodata = 1;
					      else
						  avg +=
						      (double) float_value
						      * pcontribution;
					  }
					else
					  {
					      p_double = (double *) p;
					      double_value = *p_double;
					      if (double_value ==
						  image->no_data_value)
						  is_nodata = 1;
					      else
						  avg +=
						      double_value *
						      pcontribution;
					  }
					break;
				    };
			      }
			    spixels += xportion * yportion;
			    sx += 1.0;
			}
		      while (sx < sx2);
		      sy += 1.0;
		  }
		while (sy < sy2);
		if (spixels != 0.0)
		  {
		      if (!is_nodata)
			  avg /= spixels;
		  }
		/* setting the destination pixel */
		p = thumbnail->pixels + (y * thumbnail->scanline_width);
		if (thumbnail->pixel_format == GG_PIXEL_GRID)
		  {
		      p += x * (thumbnail->bits_per_sample / 8);
		      switch (thumbnail->sample_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (thumbnail->bits_per_sample == 16)
			      {
				  p_short = (short *) p;
				  if (is_nodata)
				      *p_short =
					  (short) (thumbnail->no_data_value);
				  else
				      *p_short = (short) avg;
			      }
			    else
			      {
				  p_int = (int *) p;
				  if (is_nodata)
				      *p_int = (int) (thumbnail->no_data_value);
				  else
				      *p_int = (int) avg;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (thumbnail->bits_per_sample == 16)
			      {
				  p_ushort = (unsigned short *) p;
				  if (is_nodata)
				      *p_ushort =
					  (unsigned
					   short) (thumbnail->no_data_value);
				  else
				      *p_ushort = (unsigned short) avg;
			      }
			    else
			      {
				  p_uint = (unsigned int *) p;
				  if (is_nodata)
				      *p_uint =
					  (unsigned
					   int) (thumbnail->no_data_value);
				  else
				      *p_uint = (unsigned int) avg;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (thumbnail->bits_per_sample == 32)
			      {
				  p_float = (float *) p;
				  if (is_nodata)
				      *p_float =
					  (float) (thumbnail->no_data_value);
				  else
				      *p_float = (float) avg;
			      }
			    else
			      {
				  p_double = (double *) p;
				  if (is_nodata)
				      *p_double = thumbnail->no_data_value;
				  else
				      *p_double = avg;
			      }
			    break;
			};
		  }
	    }
      }
}

GGRAPH_PRIVATE int
gg_convert_image_to_grid_int16 (const gGraphImagePtr img)
{
/* converting this grid-image to Int16 */
    int x;
    int y;
    void *pixels;
    short value;
    unsigned char *p_in;
    short *p_out;

    if (img->pixel_format != GG_PIXEL_GRID)
	return GGRAPH_INVALID_IMAGE;

    if (img->sample_format == GGRAPH_SAMPLE_INT && img->bits_per_sample == 16)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * sizeof (short));
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
		if (img->sample_format == GGRAPH_SAMPLE_UINT
		    && img->bits_per_sample == 16)
		  {
		      value = (short) *((unsigned short *) p_in);
		      p_in += sizeof (unsigned short);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_INT
		    && img->bits_per_sample == 32)
		  {
		      value = (short) *((int *) p_in);
		      p_in += sizeof (int);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_UINT
		    && img->bits_per_sample == 32)
		  {
		      value = (short) *((unsigned int *) p_in);
		      p_in += sizeof (unsigned int);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_FLOAT
		    && img->bits_per_sample == 32)
		  {
		      value = (short) *((float *) p_in);
		      p_in += sizeof (float);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_FLOAT
		    && img->bits_per_sample == 64)
		  {
		      value = (short) *((double *) p_in);
		      p_in += sizeof (double);
		  }
		/* setting the destination pixel */
		*p_out++ = value;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_GRID;
    img->scanline_width = img->width * sizeof (short);
    img->pixel_size = sizeof (short);
    img->sample_format = GGRAPH_SAMPLE_INT;
    img->bits_per_sample = 16;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_grid_uint16 (const gGraphImagePtr img)
{
/* converting this grid-image to UInt16 */
    int x;
    int y;
    void *pixels;
    unsigned short value;
    unsigned char *p_in;
    unsigned short *p_out;

    if (img->pixel_format != GG_PIXEL_GRID)
	return GGRAPH_INVALID_IMAGE;

    if (img->sample_format == GGRAPH_SAMPLE_UINT && img->bits_per_sample == 16)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * sizeof (unsigned short));
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
		if (img->sample_format == GGRAPH_SAMPLE_INT
		    && img->bits_per_sample == 16)
		  {
		      value = (unsigned short) *((short *) p_in);
		      p_in += sizeof (short);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_INT
		    && img->bits_per_sample == 32)
		  {
		      value = (unsigned short) *((int *) p_in);
		      p_in += sizeof (int);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_UINT
		    && img->bits_per_sample == 32)
		  {
		      value = (unsigned short) *((unsigned int *) p_in);
		      p_in += sizeof (unsigned int);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_FLOAT
		    && img->bits_per_sample == 32)
		  {
		      value = (unsigned short) *((float *) p_in);
		      p_in += sizeof (float);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_FLOAT
		    && img->bits_per_sample == 64)
		  {
		      value = (unsigned short) *((double *) p_in);
		      p_in += sizeof (double);
		  }
		/* setting the destination pixel */
		*p_out++ = value;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_GRID;
    img->scanline_width = img->width * sizeof (unsigned short);
    img->pixel_size = sizeof (unsigned short);
    img->sample_format = GGRAPH_SAMPLE_UINT;
    img->bits_per_sample = 16;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_grid_int32 (const gGraphImagePtr img)
{
/* converting this grid-image to Int32 */
    int x;
    int y;
    void *pixels;
    int value;
    unsigned char *p_in;
    int *p_out;

    if (img->pixel_format != GG_PIXEL_GRID)
	return GGRAPH_INVALID_IMAGE;

    if (img->sample_format == GGRAPH_SAMPLE_INT && img->bits_per_sample == 32)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * sizeof (int));
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
		if (img->sample_format == GGRAPH_SAMPLE_INT
		    && img->bits_per_sample == 16)
		  {
		      value = (int) *((short *) p_in);
		      p_in += sizeof (short);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_UINT
		    && img->bits_per_sample == 16)
		  {
		      value = (int) *((unsigned short *) p_in);
		      p_in += sizeof (unsigned short);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_UINT
		    && img->bits_per_sample == 32)
		  {
		      value = (int) *((unsigned int *) p_in);
		      p_in += sizeof (unsigned int);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_FLOAT
		    && img->bits_per_sample == 32)
		  {
		      value = (int) *((float *) p_in);
		      p_in += sizeof (float);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_FLOAT
		    && img->bits_per_sample == 64)
		  {
		      value = (int) *((double *) p_in);
		      p_in += sizeof (double);
		  }
		/* setting the destination pixel */
		*p_out++ = value;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_GRID;
    img->scanline_width = img->width * sizeof (int);
    img->pixel_size = sizeof (int);
    img->sample_format = GGRAPH_SAMPLE_INT;
    img->bits_per_sample = 32;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_grid_uint32 (const gGraphImagePtr img)
{
/* converting this grid-image to UInt32 */
    int x;
    int y;
    void *pixels;
    unsigned int value;
    unsigned char *p_in;
    unsigned int *p_out;

    if (img->pixel_format != GG_PIXEL_GRID)
	return GGRAPH_INVALID_IMAGE;

    if (img->sample_format == GGRAPH_SAMPLE_UINT && img->bits_per_sample == 32)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * sizeof (unsigned short));
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
		if (img->sample_format == GGRAPH_SAMPLE_INT
		    && img->bits_per_sample == 16)
		  {
		      value = (unsigned int) *((short *) p_in);
		      p_in += sizeof (short);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_UINT
		    && img->bits_per_sample == 16)
		  {
		      value = (unsigned int) *((unsigned short *) p_in);
		      p_in += sizeof (unsigned short);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_INT
		    && img->bits_per_sample == 32)
		  {
		      value = (unsigned int) *((int *) p_in);
		      p_in += sizeof (int);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_FLOAT
		    && img->bits_per_sample == 32)
		  {
		      value = (unsigned int) *((float *) p_in);
		      p_in += sizeof (float);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_FLOAT
		    && img->bits_per_sample == 64)
		  {
		      value = (unsigned int) *((double *) p_in);
		      p_in += sizeof (double);
		  }
		/* setting the destination pixel */
		*p_out++ = value;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_GRID;
    img->scanline_width = img->width * sizeof (unsigned int);
    img->pixel_size = sizeof (unsigned int);
    img->sample_format = GGRAPH_SAMPLE_UINT;
    img->bits_per_sample = 32;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_grid_float (const gGraphImagePtr img)
{
/* converting this grid-image to Float */
    int x;
    int y;
    void *pixels;
    float value;
    unsigned char *p_in;
    float *p_out;

    if (img->pixel_format != GG_PIXEL_GRID)
	return GGRAPH_INVALID_IMAGE;

    if (img->sample_format == GGRAPH_SAMPLE_FLOAT && img->bits_per_sample == 32)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * sizeof (short));
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
		if (img->sample_format == GGRAPH_SAMPLE_INT
		    && img->bits_per_sample == 16)
		  {
		      value = (float) *((short *) p_in);
		      p_in += sizeof (short);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_UINT
		    && img->bits_per_sample == 16)
		  {
		      value = (float) *((unsigned short *) p_in);
		      p_in += sizeof (unsigned short);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_INT
		    && img->bits_per_sample == 32)
		  {
		      value = (float) *((int *) p_in);
		      p_in += sizeof (int);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_UINT
		    && img->bits_per_sample == 32)
		  {
		      value = (float) *((unsigned int *) p_in);
		      p_in += sizeof (unsigned int);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_FLOAT
		    && img->bits_per_sample == 64)
		  {
		      value = (float) *((double *) p_in);
		      p_in += sizeof (double);
		  }
		/* setting the destination pixel */
		*p_out++ = value;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_GRID;
    img->scanline_width = img->width * sizeof (float);
    img->pixel_size = sizeof (float);
    img->sample_format = GGRAPH_SAMPLE_INT;
    img->bits_per_sample = 32;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_grid_double (const gGraphImagePtr img)
{
/* converting this grid-image to Double */
    int x;
    int y;
    void *pixels;
    double value;
    unsigned char *p_in;
    double *p_out;

    if (img->pixel_format != GG_PIXEL_GRID)
	return GGRAPH_INVALID_IMAGE;

    if (img->sample_format == GGRAPH_SAMPLE_FLOAT && img->bits_per_sample == 64)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * sizeof (short));
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
		if (img->sample_format == GGRAPH_SAMPLE_INT
		    && img->bits_per_sample == 16)
		  {
		      value = (double) *((short *) p_in);
		      p_in += sizeof (short);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_UINT
		    && img->bits_per_sample == 16)
		  {
		      value = (double) *((unsigned short *) p_in);
		      p_in += sizeof (unsigned short);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_INT
		    && img->bits_per_sample == 32)
		  {
		      value = (double) *((int *) p_in);
		      p_in += sizeof (int);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_UINT
		    && img->bits_per_sample == 32)
		  {
		      value = (double) *((unsigned int *) p_in);
		      p_in += sizeof (unsigned int);
		  }
		if (img->sample_format == GGRAPH_SAMPLE_FLOAT
		    && img->bits_per_sample == 32)
		  {
		      value = (double) *((float *) p_in);
		      p_in += sizeof (float);
		  }
		/* setting the destination pixel */
		*p_out++ = value;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_GRID;
    img->scanline_width = img->width * sizeof (double);
    img->pixel_size = sizeof (double);
    img->sample_format = GGRAPH_SAMPLE_FLOAT;
    img->bits_per_sample = 64;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_rgb (const gGraphImagePtr img)
{
/* converting this image to RGB */
    int x;
    int y;
    void *pixels;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char *p_in;
    unsigned char *p_out;

    if (img->pixel_format == GG_PIXEL_RGB || img->pixel_format == GG_PIXEL_RGBA
	|| img->pixel_format == GG_PIXEL_ARGB
	|| img->pixel_format == GG_PIXEL_BGR
	|| img->pixel_format == GG_PIXEL_BGRA
	|| img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_PALETTE)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (img->pixel_format == GG_PIXEL_RGB)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * 3);
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width * 3);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
		if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;	/* skipping alpha */
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      red = *p_in++;
		      green = red;
		      blue = red;

		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img->palette_red[index];
		      green = img->palette_green[index];
		      blue = img->palette_blue[index];
		  }
		/* setting the destination pixel */
		*p_out++ = red;
		*p_out++ = green;
		*p_out++ = blue;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_RGB;
    img->scanline_width = img->width * 3;
    img->pixel_size = 3;
    img->max_palette = 0;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_rgba (const gGraphImagePtr img)
{
/* converting this image to RGBA */
    int x;
    int y;
    void *pixels;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    unsigned char *p_in;
    unsigned char *p_out;

    if (img->pixel_format == GG_PIXEL_RGB || img->pixel_format == GG_PIXEL_RGBA
	|| img->pixel_format == GG_PIXEL_ARGB
	|| img->pixel_format == GG_PIXEL_BGR
	|| img->pixel_format == GG_PIXEL_BGRA
	|| img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_PALETTE)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (img->pixel_format == GG_PIXEL_RGBA)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * 4);
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width * 4);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      alpha = 255;
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      alpha = *p_in++;
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      alpha = 255;
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      alpha = *p_in++;
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      red = *p_in++;
		      green = red;
		      blue = red;
		      alpha = 255;
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img->palette_red[index];
		      green = img->palette_green[index];
		      blue = img->palette_blue[index];
		      alpha = 255;
		  }
		/* setting the destination pixel */
		*p_out++ = red;
		*p_out++ = green;
		*p_out++ = blue;
		*p_out++ = alpha;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_RGBA;
    img->scanline_width = img->width * 4;
    img->pixel_size = 4;
    img->max_palette = 0;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_argb (const gGraphImagePtr img)
{
/* converting this image to ARGB */
    int x;
    int y;
    void *pixels;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    unsigned char *p_in;
    unsigned char *p_out;

    if (img->pixel_format == GG_PIXEL_RGB || img->pixel_format == GG_PIXEL_RGBA
	|| img->pixel_format == GG_PIXEL_ARGB
	|| img->pixel_format == GG_PIXEL_BGR
	|| img->pixel_format == GG_PIXEL_BGRA
	|| img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_PALETTE)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (img->pixel_format == GG_PIXEL_ARGB)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * 4);
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width * 4);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      alpha = 255;
		  }
		else if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      alpha = *p_in++;
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      alpha = 255;
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      alpha = *p_in++;
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      red = *p_in++;
		      green = red;
		      blue = red;
		      alpha = 255;
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img->palette_red[index];
		      green = img->palette_green[index];
		      blue = img->palette_blue[index];
		      alpha = 255;
		  }
		/* setting the destination pixel */
		*p_out++ = alpha;
		*p_out++ = red;
		*p_out++ = green;
		*p_out++ = blue;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_ARGB;
    img->scanline_width = img->width * 4;
    img->pixel_size = 4;
    img->max_palette = 0;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_bgr (const gGraphImagePtr img)
{
/* converting this image to BGR */
    int x;
    int y;
    void *pixels;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char *p_in;
    unsigned char *p_out;

    if (img->pixel_format == GG_PIXEL_RGB || img->pixel_format == GG_PIXEL_RGBA
	|| img->pixel_format == GG_PIXEL_ARGB
	|| img->pixel_format == GG_PIXEL_BGR
	|| img->pixel_format == GG_PIXEL_BGRA
	|| img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_PALETTE)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (img->pixel_format == GG_PIXEL_BGR)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * 3);
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width * 3);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;	/* skipping alpha */
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      red = *p_in++;
		      green = red;
		      blue = red;
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img->palette_red[index];
		      green = img->palette_green[index];
		      blue = img->palette_blue[index];
		  }
		/* setting the destination pixel */
		*p_out++ = blue;
		*p_out++ = green;
		*p_out++ = red;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_BGR;
    img->scanline_width = img->width * 3;
    img->pixel_size = 3;
    img->max_palette = 0;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_bgra (const gGraphImagePtr img)
{
/* converting this image to BGRA */
    int x;
    int y;
    void *pixels;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    unsigned char *p_in;
    unsigned char *p_out;

    if (img->pixel_format == GG_PIXEL_RGB || img->pixel_format == GG_PIXEL_RGBA
	|| img->pixel_format == GG_PIXEL_ARGB
	|| img->pixel_format == GG_PIXEL_BGR
	|| img->pixel_format == GG_PIXEL_BGRA
	|| img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_PALETTE)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (img->pixel_format == GG_PIXEL_ARGB)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height * 4);
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width * 4);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      alpha = 255;
		  }
		else if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      alpha = *p_in++;
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      alpha = *p_in++;
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      alpha = 255;
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      red = *p_in++;
		      green = red;
		      blue = red;
		      alpha = 255;
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img->palette_red[index];
		      green = img->palette_green[index];
		      blue = img->palette_blue[index];
		      alpha = 255;
		  }
		/* setting the destination pixel */
		*p_out++ = blue;
		*p_out++ = green;
		*p_out++ = red;
		*p_out++ = alpha;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_BGRA;
    img->scanline_width = img->width * 4;
    img->pixel_size = 4;
    img->max_palette = 0;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_grayscale (const gGraphImagePtr img)
{
/* converting this image to GRAYSCALE */
    int x;
    int y;
    void *pixels;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char gray;
    unsigned char *p_in;
    unsigned char *p_out;

    if (img->pixel_format == GG_PIXEL_RGB || img->pixel_format == GG_PIXEL_RGBA
	|| img->pixel_format == GG_PIXEL_ARGB
	|| img->pixel_format == GG_PIXEL_BGR
	|| img->pixel_format == GG_PIXEL_BGRA
	|| img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_PALETTE)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (img->pixel_format == GG_PIXEL_GRAYSCALE)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height);
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;	/* skipping alpha */
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img->palette_red[index];
		      green = img->palette_green[index];
		      blue = img->palette_blue[index];
		  }
		/* setting the destination pixel */
		gray = to_grayscale (red, green, blue);
		*p_out++ = gray;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_GRAYSCALE;
    img->scanline_width = img->width;
    img->pixel_size = 1;
    img->max_palette = 0;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_palette (const gGraphImagePtr img)
{
/* converting this image to PALETTE */
    int x;
    int y;
    void *pixels;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char index;
    unsigned char *p_in;
    unsigned char *p_out;

    if (img->pixel_format == GG_PIXEL_RGB || img->pixel_format == GG_PIXEL_RGBA
	|| img->pixel_format == GG_PIXEL_ARGB
	|| img->pixel_format == GG_PIXEL_BGR
	|| img->pixel_format == GG_PIXEL_BGRA
	|| img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_PALETTE)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (img->pixel_format == GG_PIXEL_PALETTE)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height);
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    img->max_palette = 0;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;	/* skipping alpha */
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      red = *p_in++;
		      green = red;
		      blue = red;
		  }
		/* setting the destination pixel */
		index = gg_match_palette (img, red, green, blue);
		*p_out++ = index;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_PALETTE;
    img->scanline_width = img->width;
    img->pixel_size = 1;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_convert_image_to_monochrome (const gGraphImagePtr img)
{
/* converting this image to MONOCHROME */
    int x;
    int y;
    void *pixels;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char index;
    unsigned char *p_in;
    unsigned char *p_out;

    if (img->pixel_format == GG_PIXEL_RGB || img->pixel_format == GG_PIXEL_RGBA
	|| img->pixel_format == GG_PIXEL_ARGB
	|| img->pixel_format == GG_PIXEL_BGR
	|| img->pixel_format == GG_PIXEL_BGRA
	|| img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_PALETTE)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (img->pixel_format == GG_PIXEL_PALETTE
	&& gg_is_image_monochrome_ready (img) == GGRAPH_TRUE)
	return GGRAPH_OK;

    pixels = malloc (img->width * img->height);
    if (!pixels)
	return GGRAPH_INSUFFICIENT_MEMORY;
    for (y = 0; y < img->height; y++)
      {
	  /* processing any scanline */
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = pixels;
	  p_out += (y * img->width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving the origin pixel */
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;	/* skipping alpha */
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		  }
		else if (img->pixel_format == GG_PIXEL_BGR)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      red = *p_in++;
		      green = red;
		      blue = red;
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img->palette_red[index];
		      green = img->palette_green[index];
		      blue = img->palette_blue[index];
		  }
		/* setting the destination pixel */
		if (red == 0 && green == 0 && blue == 0)
		    index = 0;
		else if (red == 255 && green == 255 && blue == 255)
		    index = 1;
		else
		  {
		      /* neither WHITE nor BLACK */
		      unsigned char gray = to_grayscale (red, green, blue);
		      if (gray < 128)
			{
			    /* defaulting to BLACK */
			    index = 0;
			}
		      else
			{
			    /* defaulting to WHITE */
			    index = 1;
			}
		  }
		*p_out++ = index;
	    }
      }

    free (img->pixels);
    img->pixels = pixels;
    img->pixel_format = GG_PIXEL_PALETTE;
    img->scanline_width = img->width;
    img->pixel_size = 1;
    img->max_palette = 2;
    img->palette_red[0] = 0;
    img->palette_green[0] = 0;
    img->palette_blue[0] = 0;
    img->palette_red[1] = 255;
    img->palette_green[1] = 255;
    img->palette_blue[1] = 255;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_is_image_monochrome_ready (gGraphImagePtr img)
{
/* 
/ checking if this Image is into the Monochrome colorspace and ready to be used
/ expected: palette, 2 colors, black&white
*/
    int i;
    int black = 0;
    int white = 0;
    if (img->pixel_format != GG_PIXEL_PALETTE)
	return GGRAPH_FALSE;
    if (img->max_palette != 2)
	return GGRAPH_FALSE;
    for (i = 0; i < img->max_palette; i++)
      {
	  if (img->palette_red[i] == 0 && img->palette_green[i] == 0
	      && img->palette_blue[i] == 0)
	      black = 1;
	  if (img->palette_red[i] == 255 && img->palette_green[i] == 255
	      && img->palette_blue[i] == 255)
	      white = 1;
      }
    if (black && white)
	return GGRAPH_TRUE;
    return GGRAPH_FALSE;
}

GGRAPH_PRIVATE int
gg_is_image_monochrome (gGraphImagePtr img)
{
/* checking if this Image is into the Monochrome colorspace */
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char *p_in;
    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving a pixel */
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;	/* skipping alpha */
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      red = *p_in++;
		      green = red;
		      blue = red;
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img->palette_red[index];
		      green = img->palette_green[index];
		      blue = img->palette_blue[index];
		  }
		if (red == 0 && green == 0 && blue == 0)
		    continue;
		if (red == 255 && green == 255 && blue == 255)
		    continue;
		return GGRAPH_FALSE;
	    }
      }
    return GGRAPH_TRUE;
}

GGRAPH_PRIVATE int
gg_is_image_grayscale (gGraphImagePtr img)
{
/* checking if this Image is into the GrayScale colorspace */
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char *p_in;
    if (img->pixel_format == GG_PIXEL_GRAYSCALE)
	return GGRAPH_TRUE;
    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving a pixel */
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;	/* skipping alpha */
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img->palette_red[index];
		      green = img->palette_green[index];
		      blue = img->palette_blue[index];
		  }
		if (red == green && green == blue)
		    continue;
		return GGRAPH_FALSE;
	    }
      }
    return GGRAPH_TRUE;
}

static int
palette_check (gGraphImagePtr img, unsigned char r, unsigned char g,
	       unsigned char b)
{
    unsigned char index;
    for (index = 0; index < img->max_palette; index++)
      {
	  /* searching if already defined */
	  if (img->palette_red[index] == r && img->palette_green[index] == g
	      && img->palette_blue[index] == b)
	      return GGRAPH_TRUE;
      }
    if (img->max_palette < 255)
      {
	  /* inserting a new palette entry */
	  unsigned char i = img->max_palette;
	  img->max_palette += 1;
	  img->palette_red[i] = r;
	  img->palette_green[i] = g;
	  img->palette_blue[i] = b;
	  return GGRAPH_TRUE;
      }
    return GGRAPH_FALSE;
}

GGRAPH_PRIVATE int
gg_is_image_palette256 (gGraphImagePtr img)
{
/* checking if this Image may be represented using a 256 colors palette */
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char *p_in;
    if (img->pixel_format == GG_PIXEL_PALETTE
	|| img->pixel_format == GG_PIXEL_GRAYSCALE)
	return GGRAPH_TRUE;
    img->max_palette = 0;
    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		/* retrieving a pixel */
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
		      p_in++;	/* skipping alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;	/* skipping alpha */
		      blue = *p_in++;
		      green = *p_in++;
		      red = *p_in++;
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
		      p_in++;	/* skipping alpha */
		  }
		if (palette_check (img, red, green, blue) == GGRAPH_FALSE)
		    return GGRAPH_FALSE;
	    }
      }
    return GGRAPH_TRUE;
}

static gGraphImagePtr
raw_monochrome (unsigned char *p, int width, int height)
{
/* creating a visible image from RAW MONOCHROME */
    int x;
    int y;
    int line_width;
    gGraphImagePtr img = gg_image_create (GG_PIXEL_PALETTE, width, height, 8, 1,
					  GGRAPH_SAMPLE_UINT, NULL, NULL);

    if (!img)
	return NULL;

    img->max_palette = 2;
    img->palette_red[0] = 255;
    img->palette_green[0] = 255;
    img->palette_blue[0] = 255;
    img->palette_red[1] = 0;
    img->palette_green[1] = 0;
    img->palette_blue[1] = 0;

    line_width = width / 8;
    if ((line_width * 8) < width)
	line_width++;

    for (y = 0; y < height; y++)
      {
	  unsigned char *p_in = p + (y * line_width);
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
	  unsigned char byte = 0x00;
	  unsigned char px;
	  int pixel;
	  int pos = 0;
	  for (x = 0; x < width; x++)
	    {
		if (pos == 0)
		    byte = *p_in++;
		pixel = 0;
		switch (pos)
		  {
		  case 0:
		      px = byte & 0x80;
		      if (px == 0x80)
			  pixel = 1;
		      break;
		  case 1:
		      px = byte & 0x40;
		      if (px == 0x40)
			  pixel = 1;
		      break;
		  case 2:
		      px = byte & 0x20;
		      if (px == 0x20)
			  pixel = 1;
		      break;
		  case 3:
		      px = byte & 0x10;
		      if (px == 0x10)
			  pixel = 1;
		      break;
		  case 4:
		      px = byte & 0x08;
		      if (px == 0x08)
			  pixel = 1;
		      break;
		  case 5:
		      px = byte & 0x04;
		      if (px == 0x04)
			  pixel = 1;
		      break;
		  case 6:
		      px = byte & 0x02;
		      if (px == 0x02)
			  pixel = 1;
		      break;
		  case 7:
		      px = byte & 0x01;
		      if (px == 0x01)
			  pixel = 1;
		      break;
		  };
		*p_out++ = pixel;
		pos++;
		if (pos > 7)
		  {
		      byte = 0x00;
		      pos = 0;
		  }
	    }
      }
    return img;
}

static gGraphImagePtr
raw_rgb (unsigned char *p, int width, int height)
{
/* creating a visible image from RAW RGB */
    int y;
    gGraphImagePtr img =
	gg_image_create (GG_PIXEL_RGB, width, height, 8, 3, GGRAPH_SAMPLE_UINT,
			 NULL, NULL);

    if (!img)
	return NULL;

    for (y = 0; y < height; y++)
      {
	  unsigned char *p_in = p + (y * img->scanline_width);
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
	  memcpy (p_out, p_in, img->scanline_width);
      }
    return img;
}

static gGraphImagePtr
raw_grayscale (unsigned char *p, int width, int height)
{
/* creating a visible image from RAW GRAYSCALE */
    int y;
    gGraphImagePtr img =
	gg_image_create (GG_PIXEL_GRAYSCALE, width, height, 8, 1,
			 GGRAPH_SAMPLE_UINT, NULL, NULL);

    if (!img)
	return NULL;

    for (y = 0; y < height; y++)
      {
	  unsigned char *p_in = p + (y * img->scanline_width);
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
	  memcpy (p_out, p_in, img->scanline_width);
      }
    return img;
}

static gGraphImagePtr
raw_palette (unsigned char *p, int width, int height)
{
/* creating a visible image from RAW PALETTE */
    int x;
    int y;
    gGraphImagePtr img = gg_image_create (GG_PIXEL_PALETTE, width, height, 8, 1,
					  GGRAPH_SAMPLE_UINT, NULL, NULL);

    if (!img)
	return NULL;

    srand (1);
    for (y = 0; y < height; y++)
      {
	  unsigned char *p_in = p + (y * img->scanline_width);
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < width; x++)
	    {
		unsigned int col;
		int idx = *p_in++;
		*p_out++ = idx;
		if (img->max_palette < (idx + 1))
		  {
		      img->max_palette = idx + 1;
		      col = 255 - (unsigned int) rand () % 256;
		      img->palette_red[idx] = col;
		      col = (unsigned int) rand () % 256;
		      img->palette_green[idx] = col;
		      col = 255 - (unsigned int) rand () % 256;
		      img->palette_blue[idx] = col;
		  }
	    }
      }
    return img;
}

static gGraphImagePtr
raw_int16 (unsigned char *p, int width, int height, int endian)
{
/* creating a visible image from RAW GRID-INT16 */
    int x;
    int y;
    int endian_arch = gg_endian_arch ();
    unsigned char *p_in;
    short value;
    short min = SHRT_MAX;
    short max = SHRT_MIN;
    double step;
    double gray;
    gGraphImagePtr img =
	gg_image_create (GG_PIXEL_GRAYSCALE, width, height, 8, 1,
			 GGRAPH_SAMPLE_UINT, NULL, NULL);

    if (!img)
	return NULL;

    for (y = 0; y < height; y++)
      {
	  /* identifying Min/Max values */
	  p_in = p + (y * width * sizeof (short));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_int16 (p_in, endian, endian_arch);
		p_in += sizeof (short);
		if (value > max)
		    max = value;
		if (value < min)
		    min = value;
	    }
      }
    step = (double) (max - min) / 256.0;
    for (y = 0; y < height; y++)
      {
	  /* visualizing as Grayscale */
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
	  p_in = p + (y * width * sizeof (short));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_int16 (p_in, endian, endian_arch);
		p_in += sizeof (short);
		gray = (double) (value - min) / step;
		if (gray < 0.0)
		    gray = 0.0;
		if (gray > 255)
		    gray = 255.0;
		*p_out++ = (short) gray;
	    }
      }
    return img;
}

static gGraphImagePtr
raw_int32 (unsigned char *p, int width, int height, int endian)
{
/* creating a visible image from RAW GRID-INT32 */
    int x;
    int y;
    int endian_arch = gg_endian_arch ();
    unsigned char *p_in;
    int value;
    int min = INT_MAX;
    int max = INT_MIN;
    double step;
    double gray;
    gGraphImagePtr img =
	gg_image_create (GG_PIXEL_GRAYSCALE, width, height, 8, 1,
			 GGRAPH_SAMPLE_UINT, NULL, NULL);

    if (!img)
	return NULL;

    for (y = 0; y < height; y++)
      {
	  /* identifying Min/Max values */
	  p_in = p + (y * width * sizeof (int));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_int32 (p_in, endian, endian_arch);
		p_in += sizeof (int);
		if (value > max)
		    max = value;
		if (value < min)
		    min = value;
	    }
      }
    step = (double) (max - min) / 256.0;
    for (y = 0; y < height; y++)
      {
	  /* visualizing as Grayscale */
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
	  p_in = p + (y * width * sizeof (int));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_int32 (p_in, endian, endian_arch);
		p_in += sizeof (int);
		gray = (double) (value - min) / step;
		if (gray < 0.0)
		    gray = 0.0;
		if (gray > 255)
		    gray = 255.0;
		*p_out++ = (short) gray;
	    }
      }
    return img;
}

static gGraphImagePtr
raw_uint16 (unsigned char *p, int width, int height, int endian)
{
/* creating a visible image from RAW GRID-UINT16 */
    int x;
    int y;
    int endian_arch = gg_endian_arch ();
    unsigned char *p_in;
    unsigned short value;
    unsigned short min = USHRT_MAX;
    unsigned short max = 0;
    double step;
    double gray;
    gGraphImagePtr img =
	gg_image_create (GG_PIXEL_GRAYSCALE, width, height, 8, 1,
			 GGRAPH_SAMPLE_UINT, NULL, NULL);

    if (!img)
	return NULL;

    for (y = 0; y < height; y++)
      {
	  /* identifying Min/Max values */
	  p_in = p + (y * width * sizeof (unsigned short));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_uint16 (p_in, endian, endian_arch);
		p_in += sizeof (unsigned short);
		if (value > max)
		    max = value;
		if (value < min)
		    min = value;
	    }
      }
    step = (double) (max - min) / 256.0;
    for (y = 0; y < height; y++)
      {
	  /* visualizing as Grayscale */
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
	  p_in = p + (y * width * sizeof (unsigned short));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_uint16 (p_in, endian, endian_arch);
		p_in += sizeof (unsigned short);
		gray = (double) (value - min) / step;
		if (gray < 0.0)
		    gray = 0.0;
		if (gray > 255)
		    gray = 255.0;
		*p_out++ = (short) gray;
	    }
      }
    return img;
}

static gGraphImagePtr
raw_uint32 (unsigned char *p, int width, int height, int endian)
{
/* creating a visible image from RAW GRID-UINT32 */
    int x;
    int y;
    int endian_arch = gg_endian_arch ();
    unsigned char *p_in;
    unsigned int value;
    unsigned int min = UINT_MAX;
    unsigned int max = 0;
    double step;
    double gray;
    gGraphImagePtr img =
	gg_image_create (GG_PIXEL_GRAYSCALE, width, height, 8, 1,
			 GGRAPH_SAMPLE_UINT, NULL, NULL);

    if (!img)
	return NULL;

    for (y = 0; y < height; y++)
      {
	  /* identifying Min/Max values */
	  p_in = p + (y * width * sizeof (int));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_uint32 (p_in, endian, endian_arch);
		p_in += sizeof (unsigned int);
		if (value > max)
		    max = value;
		if (value < min)
		    min = value;
	    }
      }
    step = (double) (max - min) / 256.0;
    for (y = 0; y < height; y++)
      {
	  /* visualizing as Grayscale */
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
	  p_in = p + (y * width * sizeof (unsigned int));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_uint32 (p_in, endian, endian_arch);
		p_in += sizeof (unsigned int);
		gray = (double) (value - min) / step;
		if (gray < 0.0)
		    gray = 0.0;
		if (gray > 255)
		    gray = 255.0;
		*p_out++ = (short) gray;
	    }
      }
    return img;
}

static gGraphImagePtr
raw_float (unsigned char *p, int width, int height, int endian)
{
/* creating a visible image from RAW GRID-FLOAT */
    int x;
    int y;
    int endian_arch = gg_endian_arch ();
    unsigned char *p_in;
    float value;
    float min = FLT_MAX;
    float max = 0.0 - FLT_MAX;
    double step;
    double gray;
    gGraphImagePtr img =
	gg_image_create (GG_PIXEL_GRAYSCALE, width, height, 8, 1,
			 GGRAPH_SAMPLE_UINT, NULL, NULL);

    if (!img)
	return NULL;

    for (y = 0; y < height; y++)
      {
	  /* identifying Min/Max values */
	  p_in = p + (y * width * sizeof (float));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_float (p_in, endian, endian_arch);
		p_in += sizeof (float);
		if (value > max)
		    max = value;
		if (value < min)
		    min = value;
	    }
      }
    step = (double) (max - min) / 256.0;
    for (y = 0; y < height; y++)
      {
	  /* visualizing as Grayscale */
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
	  p_in = p + (y * width * sizeof (float));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_float (p_in, endian, endian_arch);
		p_in += sizeof (float);
		gray = (double) (value - min) / step;
		if (gray < 0.0)
		    gray = 0.0;
		if (gray > 255)
		    gray = 255.0;
		*p_out++ = (short) gray;
	    }
      }
    return img;
}

static gGraphImagePtr
raw_double (unsigned char *p, int width, int height, int endian)
{
/* creating a visible image from RAW GRID-DOUBLE */
    int x;
    int y;
    int endian_arch = gg_endian_arch ();
    unsigned char *p_in;
    double value;
    double min = DBL_MAX;
    double max = 0.0 - DBL_MAX;
    double step;
    double gray;
    gGraphImagePtr img =
	gg_image_create (GG_PIXEL_GRAYSCALE, width, height, 8, 1,
			 GGRAPH_SAMPLE_UINT, NULL, NULL);

    if (!img)
	return NULL;

    for (y = 0; y < height; y++)
      {
	  /* identifying Min/Max values */
	  p_in = p + (y * width * sizeof (float));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_double (p_in, endian, endian_arch);
		p_in += sizeof (double);
		if (value > max)
		    max = value;
		if (value < min)
		    min = value;
	    }
      }
    step = (double) (max - min) / 256.0;
    for (y = 0; y < height; y++)
      {
	  /* visualizing as Grayscale */
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
	  p_in = p + (y * width * sizeof (double));
	  for (x = 0; x < width; x++)
	    {
		value = gg_import_double (p_in, endian, endian_arch);
		p_in += sizeof (double);
		gray = (value - min) / step;
		if (gray < 0.0)
		    gray = 0.0;
		if (gray > 255)
		    gray = 255.0;
		*p_out++ = (short) gray;
	    }
      }
    return img;
}

GGRAPH_PRIVATE int
gg_image_from_raw (int mem_buf_size, const void *mem_buf,
		   gGraphImagePtr * image_handle)
{
/* importing a RAW image */
    gGraphImagePtr img = NULL;
    int width;
    int height;
    int line_width;
    unsigned char *p = (unsigned char *) mem_buf;
    short start_signature;
    short end_signature;
    int endian_arch = gg_endian_arch ();

    *image_handle = NULL;
    if (mem_buf_size < (int) (sizeof (short) * 4))
	return GGRAPH_ERROR;

/* checking the magic signature */
    start_signature = gg_import_int16 (p, 1, endian_arch);
    p = (unsigned char *) mem_buf + mem_buf_size - sizeof (short);
    end_signature = gg_import_int16 (p, 1, endian_arch);
    if (start_signature == GG_MONOCHROME_START
	&& end_signature == GG_MONOCHROME_END)
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  line_width = width / 8;
	  if ((line_width * 8) < width)
	      line_width++;
	  if ((line_width * height) !=
	      (int) (mem_buf_size - (4 * sizeof (short))))
	      return GGRAPH_ERROR;
	  img = raw_monochrome (p, width, height);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_1_RGB_START
	 && end_signature == GG_ADAM7_1_RGB_END) ||
	(start_signature == GG_ADAM7_3_RGB_START
	 && end_signature == GG_ADAM7_3_RGB_END) ||
	(start_signature == GG_ADAM7_5_RGB_START
	 && end_signature == GG_ADAM7_5_RGB_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  line_width = width * 3;
	  if ((line_width * height) !=
	      (int) (mem_buf_size - (4 * sizeof (short))))
	      return GGRAPH_ERROR;
	  img = raw_rgb (p, width, height);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_1_GRAYSCALE_START
	 && end_signature == GG_ADAM7_1_GRAYSCALE_END) ||
	(start_signature == GG_ADAM7_3_GRAYSCALE_START
	 && end_signature == GG_ADAM7_3_GRAYSCALE_END) ||
	(start_signature == GG_ADAM7_5_GRAYSCALE_START
	 && end_signature == GG_ADAM7_5_GRAYSCALE_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  line_width = width;
	  if ((line_width * height) !=
	      (int) (mem_buf_size - (4 * sizeof (short))))
	      return GGRAPH_ERROR;
	  img = raw_grayscale (p, width, height);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_1_PALETTE_START
	 && end_signature == GG_ADAM7_1_PALETTE_END) ||
	(start_signature == GG_ADAM7_3_PALETTE_START
	 && end_signature == GG_ADAM7_3_PALETTE_END) ||
	(start_signature == GG_ADAM7_5_PALETTE_START
	 && end_signature == GG_ADAM7_5_PALETTE_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  line_width = width;
	  if ((line_width * height) !=
	      (int) (mem_buf_size - (4 * sizeof (short))))
	      return GGRAPH_ERROR;
	  img = raw_palette (p, width, height);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_1_INT16_START
	 && end_signature == GG_ADAM7_1_INT16_END) ||
	(start_signature == GG_ADAM7_3_INT16_START
	 && end_signature == GG_ADAM7_3_INT16_END) ||
	(start_signature == GG_ADAM7_5_INT16_START
	 && end_signature == GG_ADAM7_5_INT16_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  line_width = width * sizeof (short);
	  if ((line_width * height) !=
	      (int) (mem_buf_size - (4 * sizeof (short))))
	      return GGRAPH_ERROR;
	  img = raw_int16 (p, width, height, 1);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_1_UINT16_START
	 && end_signature == GG_ADAM7_1_UINT16_END) ||
	(start_signature == GG_ADAM7_3_UINT16_START
	 && end_signature == GG_ADAM7_3_UINT16_END) ||
	(start_signature == GG_ADAM7_5_UINT16_START
	 && end_signature == GG_ADAM7_5_UINT16_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  line_width = width * sizeof (unsigned short);
	  if ((line_width * height) !=
	      (int) (mem_buf_size - (4 * sizeof (short))))
	      return GGRAPH_ERROR;
	  img = raw_uint16 (p, width, height, 1);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_1_INT32_START
	 && end_signature == GG_ADAM7_1_INT32_END) ||
	(start_signature == GG_ADAM7_3_INT32_START
	 && end_signature == GG_ADAM7_3_INT32_END) ||
	(start_signature == GG_ADAM7_5_INT32_START
	 && end_signature == GG_ADAM7_5_INT32_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  line_width = width * sizeof (int);
	  if ((line_width * height) !=
	      (int) (mem_buf_size - (4 * sizeof (short))))
	      return GGRAPH_ERROR;
	  img = raw_int32 (p, width, height, 1);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_1_UINT32_START
	 && end_signature == GG_ADAM7_1_UINT32_END) ||
	(start_signature == GG_ADAM7_3_UINT32_START
	 && end_signature == GG_ADAM7_3_UINT32_END) ||
	(start_signature == GG_ADAM7_5_UINT32_START
	 && end_signature == GG_ADAM7_5_UINT32_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  line_width = width * sizeof (unsigned int);
	  if ((line_width * height) !=
	      (int) (mem_buf_size - (4 * sizeof (short))))
	      return GGRAPH_ERROR;
	  img = raw_uint32 (p, width, height, 1);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_1_FLOAT_START
	 && end_signature == GG_ADAM7_1_FLOAT_END) ||
	(start_signature == GG_ADAM7_3_FLOAT_START
	 && end_signature == GG_ADAM7_3_FLOAT_END) ||
	(start_signature == GG_ADAM7_5_FLOAT_START
	 && end_signature == GG_ADAM7_5_FLOAT_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  line_width = width * sizeof (float);
	  if ((line_width * height) !=
	      (int) (mem_buf_size - (4 * sizeof (short))))
	      return GGRAPH_ERROR;
	  img = raw_float (p, width, height, 1);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_1_DOUBLE_START
	 && end_signature == GG_ADAM7_1_DOUBLE_END) ||
	(start_signature == GG_ADAM7_3_DOUBLE_START
	 && end_signature == GG_ADAM7_3_DOUBLE_END) ||
	(start_signature == GG_ADAM7_5_DOUBLE_START
	 && end_signature == GG_ADAM7_5_DOUBLE_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 1, endian_arch);
	  p += sizeof (short);
	  line_width = width * sizeof (double);
	  if ((line_width * height) !=
	      (int) (mem_buf_size - (4 * sizeof (short))))
	      return GGRAPH_ERROR;
	  img = raw_double (p, width, height, 1);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }

    p = (unsigned char *) mem_buf;
    start_signature = gg_import_int16 (p, 0, endian_arch);
    p = (unsigned char *) mem_buf + mem_buf_size - sizeof (short);
    end_signature = gg_import_int16 (p, 0, endian_arch);
    if ((start_signature == GG_ADAM7_0_RGB_START
	 && end_signature == GG_ADAM7_0_RGB_END) ||
	(start_signature == GG_ADAM7_2_RGB_START
	 && end_signature == GG_ADAM7_2_RGB_END) ||
	(start_signature == GG_ADAM7_4_RGB_START
	 && end_signature == GG_ADAM7_4_RGB_END) ||
	(start_signature == GG_ADAM7_6_RGB_START
	 && end_signature == GG_ADAM7_6_RGB_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  img = raw_rgb (p, width, height);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_0_GRAYSCALE_START
	 && end_signature == GG_ADAM7_0_GRAYSCALE_END) ||
	(start_signature == GG_ADAM7_2_GRAYSCALE_START
	 && end_signature == GG_ADAM7_2_GRAYSCALE_END) ||
	(start_signature == GG_ADAM7_4_GRAYSCALE_START
	 && end_signature == GG_ADAM7_4_GRAYSCALE_END) ||
	(start_signature == GG_ADAM7_6_GRAYSCALE_START
	 && end_signature == GG_ADAM7_6_GRAYSCALE_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  img = raw_grayscale (p, width, height);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_0_PALETTE_START
	 && end_signature == GG_ADAM7_0_PALETTE_END) ||
	(start_signature == GG_ADAM7_2_PALETTE_START
	 && end_signature == GG_ADAM7_2_PALETTE_END) ||
	(start_signature == GG_ADAM7_4_PALETTE_START
	 && end_signature == GG_ADAM7_4_PALETTE_END) ||
	(start_signature == GG_ADAM7_6_PALETTE_START
	 && end_signature == GG_ADAM7_6_PALETTE_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  img = raw_palette (p, width, height);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_0_INT16_START
	 && end_signature == GG_ADAM7_0_INT16_END) ||
	(start_signature == GG_ADAM7_2_INT16_START
	 && end_signature == GG_ADAM7_2_INT16_END) ||
	(start_signature == GG_ADAM7_4_INT16_START
	 && end_signature == GG_ADAM7_4_INT16_END) ||
	(start_signature == GG_ADAM7_6_INT16_START
	 && end_signature == GG_ADAM7_6_INT16_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  img = raw_int16 (p, width, height, 0);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_0_UINT16_START
	 && end_signature == GG_ADAM7_0_UINT16_END) ||
	(start_signature == GG_ADAM7_2_UINT16_START
	 && end_signature == GG_ADAM7_2_UINT16_END) ||
	(start_signature == GG_ADAM7_4_UINT16_START
	 && end_signature == GG_ADAM7_4_UINT16_END) ||
	(start_signature == GG_ADAM7_6_UINT16_START
	 && end_signature == GG_ADAM7_6_UINT16_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  img = raw_uint16 (p, width, height, 0);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_0_INT32_START
	 && end_signature == GG_ADAM7_0_INT32_END) ||
	(start_signature == GG_ADAM7_2_INT32_START
	 && end_signature == GG_ADAM7_2_INT32_END) ||
	(start_signature == GG_ADAM7_4_INT32_START
	 && end_signature == GG_ADAM7_4_INT32_END) ||
	(start_signature == GG_ADAM7_6_INT32_START
	 && end_signature == GG_ADAM7_6_INT32_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  img = raw_int32 (p, width, height, 0);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_0_UINT32_START
	 && end_signature == GG_ADAM7_0_UINT32_END) ||
	(start_signature == GG_ADAM7_2_UINT32_START
	 && end_signature == GG_ADAM7_2_UINT32_END) ||
	(start_signature == GG_ADAM7_4_UINT32_START
	 && end_signature == GG_ADAM7_4_UINT32_END) ||
	(start_signature == GG_ADAM7_6_UINT32_START
	 && end_signature == GG_ADAM7_6_UINT32_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  img = raw_uint32 (p, width, height, 0);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_0_FLOAT_START
	 && end_signature == GG_ADAM7_0_FLOAT_END) ||
	(start_signature == GG_ADAM7_2_FLOAT_START
	 && end_signature == GG_ADAM7_2_FLOAT_END) ||
	(start_signature == GG_ADAM7_4_FLOAT_START
	 && end_signature == GG_ADAM7_4_FLOAT_END) ||
	(start_signature == GG_ADAM7_6_FLOAT_START
	 && end_signature == GG_ADAM7_6_FLOAT_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  img = raw_float (p, width, height, 0);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    if ((start_signature == GG_ADAM7_0_DOUBLE_START
	 && end_signature == GG_ADAM7_0_DOUBLE_END) ||
	(start_signature == GG_ADAM7_2_DOUBLE_START
	 && end_signature == GG_ADAM7_2_DOUBLE_END) ||
	(start_signature == GG_ADAM7_4_DOUBLE_START
	 && end_signature == GG_ADAM7_4_DOUBLE_END) ||
	(start_signature == GG_ADAM7_6_DOUBLE_START
	 && end_signature == GG_ADAM7_6_DOUBLE_END))
      {
	  p = (unsigned char *) mem_buf + sizeof (short);
	  width = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  height = gg_import_int16 (p, 0, endian_arch);
	  p += sizeof (short);
	  img = raw_double (p, width, height, 0);
	  if (!img)
	      return GGRAPH_ERROR;
	  *image_handle = img;
	  return GGRAPH_OK;
      }

    return GGRAPH_ERROR;
}

static int
is_near_transparent (unsigned char r, unsigned char g, unsigned char b,
		     const gGraphImagePtr img)
{
/* checking if this one is a near-transparent value */
    int diff;
    if (r == img->transparent_red && g == img->transparent_green
	&& b == img->transparent_blue)
	return 0;
    diff = r - img->transparent_red;
    if (diff <= 8 && diff >= -8)
	;
    else
	return 0;
    diff = g - img->transparent_green;
    if (diff <= 8 && diff >= -8)
	;
    else
	return 0;
    diff = b - img->transparent_blue;
    if (diff <= 8 && diff >= -8)
	;
    else
	return 0;
    return 1;
}

GGRAPH_PRIVATE int
gg_resample_transparent_rgb (const gGraphImagePtr img)
{
/* 
/ resampling colors so to make near-transparent values
/ to be really transparent
*/
    int x;
    int y;
    unsigned char *p_in;
    unsigned char *p_out;
    unsigned char r;
    unsigned char g;
    unsigned char b;

    if (img->pixel_format != GG_PIXEL_RGB)
	return GGRAPH_ERROR;

    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		r = *p_in++;
		g = *p_in++;
		b = *p_in++;
		if (is_near_transparent (r, g, b, img))
		  {
		      *p_out++ = img->transparent_red;
		      *p_out++ = img->transparent_green;
		      *p_out++ = img->transparent_blue;
		  }
		else
		    p_out += 3;
	    }
      }
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_resample_transparent_rgba (const gGraphImagePtr img)
{
/* 
/ resampling colors so to make near-transparent values
/ to be really transparent
*/
    int x;
    int y;
    unsigned char *p_in;
    unsigned char *p_out;
    unsigned char r;
    unsigned char g;
    unsigned char b;

    if (img->pixel_format != GG_PIXEL_RGBA)
	return GGRAPH_ERROR;

    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		r = *p_in++;
		g = *p_in++;
		b = *p_in++;
		p_in++;
		if (is_near_transparent (r, g, b, img))
		  {
		      *p_out++ = img->transparent_red;
		      *p_out++ = img->transparent_green;
		      *p_out++ = img->transparent_blue;
		      *p_out++ = 0;
		  }
		else
		    p_out += 4;
	    }
      }
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_resample_transparent_grayscale (const gGraphImagePtr img)
{
/* 
/ resampling colors so to make near-transparent values
/ to be really transparent
*/
    int x;
    int y;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char *p_in;
    unsigned char *p_out;

    if (img->pixel_format != GG_PIXEL_GRAYSCALE)
	return GGRAPH_ERROR;
    if (img->transparent_red == img->transparent_green
	&& img->transparent_red == img->transparent_blue)
	;
    else
	return GGRAPH_OK;

    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		r = *p_in++;
		g = r;
		b = r;
		if (is_near_transparent (r, g, b, img))
		    *p_out = img->transparent_red;
		p_out++;
	    }
      }
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_resample_transparent_palette (const gGraphImagePtr img)
{
/* 
/ resampling colors so to make near-transparent values
/ to be really transparent
*/
    int x;
    int y;
    int idx;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    int transparent_idx = -1;
    unsigned char *p_in;
    unsigned char *p_out;

    if (img->pixel_format != GG_PIXEL_RGB)
	return GGRAPH_ERROR;
    for (idx = 0; idx < img->max_palette; idx++)
      {
	  if (img->palette_red[idx] == img->transparent_red &&
	      img->palette_green[idx] == img->transparent_green &&
	      img->palette_blue[idx] == img->transparent_blue)
	    {
		transparent_idx = idx;
		break;
	    }
      }
    if (transparent_idx < 0)
	return GGRAPH_OK;

    for (y = 0; y < img->height; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  p_out = p_in;
	  for (x = 0; x < img->width; x++)
	    {
		idx = *p_in++;
		r = img->palette_red[idx];
		g = img->palette_green[idx];
		b = img->palette_blue[idx];
		if (is_near_transparent (r, g, b, img))
		    *p_out = transparent_idx;
		p_out++;
	    }
      }
    return GGRAPH_OK;
}
