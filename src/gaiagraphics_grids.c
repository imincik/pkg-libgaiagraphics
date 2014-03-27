/* 
/ gaiagraphics_grids.c
/
/ GRIDS auxiliary helpers
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
#include <float.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

#define GRID_HGT_3	1
#define GRID_HGT_1	2
#define GRID_BIN_HDR	3
#define GRID_FLT_HDR	4
#define GRID_DEM_HDR	5
#define GRID_ASCII	6

struct grid_codec_data
{
/* a struct used by GRID codec */
    int grid_type;
    int is_writer;
    int little_endian;
    void *grid_buffer;
    long *row_offsets;
};

GGRAPH_PRIVATE int
gg_image_strip_prepare_from_hgt (FILE * in, int lon, int lat,
				 gGraphStripImagePtr * image_handle)
{
/* preparing to decode an HGT-GRID [by strips] */
    gGraphStripImagePtr img = NULL;
    struct grid_codec_data *grid_codec = NULL;
    long file_length;
    int type;
    int width;
    int height;
    double pixel_size;
    double half_pixel;
    int buf_size;
    void *grid_buffer = NULL;
    int ret = GGRAPH_HGT_CODEC_ERROR;
    *image_handle = NULL;

/* retrieving the HGT dimensions */
    if (fseek (in, 0, SEEK_END) != 0)
	return GGRAPH_HGT_CODEC_ERROR;
    file_length = ftell (in);
    if (file_length == (1201 * 1201 * sizeof (short)))
      {
	  type = GRID_HGT_3;
	  width = 1201;
	  height = 1201;
	  pixel_size = 1.0 / 1200.0;
      }
    else if (file_length == (3601 * 3601 * sizeof (short)))
      {
	  type = GRID_HGT_1;
	  width = 3601;
	  height = 3601;
	  pixel_size = 1.0 / 3600.0;
      }
    else
	return GGRAPH_HGT_CODEC_ERROR;

    img =
	gg_strip_image_create (in, GGRAPH_IMAGE_HGT, GG_PIXEL_GRID, width,
			       height, 16, 1, GGRAPH_SAMPLE_INT, "WGS 84",
			       "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
    if (!img)
      {
	  ret = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }
/* setting up georeferencing infos */
    half_pixel = pixel_size / 2.0;
    img->is_georeferenced = 1;
    img->srid = 4326;
    img->upper_left_x = (double) lon - half_pixel;
    img->upper_left_y = (double) lat + 1.0 + half_pixel;
    img->pixel_x_size = pixel_size;
    img->pixel_y_size = pixel_size;
    img->no_data_value = SHRT_MIN;

/* setting up the GRID codec struct */
    grid_codec = malloc (sizeof (struct grid_codec_data));
    if (!grid_codec)
	goto error;
    grid_codec->grid_type = type;
    grid_codec->is_writer = 0;
    grid_codec->grid_buffer = NULL;
    grid_codec->row_offsets = NULL;
    img->codec_data = grid_codec;

/* allocating the GRID read buffer */
    buf_size = sizeof (short) * width;
    grid_buffer = malloc (buf_size);
    if (!grid_buffer)
	goto error;
    grid_codec->grid_buffer = grid_buffer;

    *image_handle = img;
    return GGRAPH_OK;

  error:
    if (img)
	gGraphDestroyImage (img);
    return ret;
}

GGRAPH_PRIVATE void
gg_grid_codec_destroy (void *p)
{
/* destroyng the GRID codec data */
    struct grid_codec_data *codec = (struct grid_codec_data *) p;
    if (!codec)
	return;
    if (codec->grid_buffer)
	free (codec->grid_buffer);
    if (codec->row_offsets)
	free (codec->row_offsets);
    free (codec);
}

static int
read_from_hgt (FILE * in, gGraphStripImagePtr img, void *scanline)
{
/* decoding an HGT-GRID [by strip] */
    int width = img->width;
    int height = img->height;
    size_t scan_size = width * 2;
    int row;
    int incr = 0;
    int x;
    int endian_arch = gg_endian_arch ();
    off_t pos = img->next_row * scan_size;
    unsigned char *p_in;
    short *p_out;
    short cell_value;

/* positioning on the start scanline */
    if (fseek (in, pos, SEEK_SET) != 0)
	return GGRAPH_HGT_CODEC_ERROR;
    for (row = 0; row < img->rows_per_block; row++)
      {
	  /* reading the required number of scanlines */
	  if ((row + img->next_row) >= height)
	      break;
	  if (fread (scanline, 1, scan_size, in) != scan_size)
	      return GGRAPH_HGT_CODEC_ERROR;
	  p_in = scanline;
	  p_out = (short *) (img->pixels);
	  p_out += (row * width);
	  for (x = 0; x < width; x++)
	    {
		cell_value = gg_import_int16 (p_in, 0, endian_arch);
		*p_out++ = cell_value;
		p_in += sizeof (short);
	    }
	  incr++;
      }
    img->next_row += incr;
    img->current_available_rows = incr;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_strip_read_from_hgt (gGraphStripImagePtr img, int *progress)
{
/* decoding an HGT-GRID [by strip] */
    struct grid_codec_data *grid_codec =
	(struct grid_codec_data *) (img->codec_data);
    FILE *in = img->file_handle;

    if (grid_codec->grid_type == GRID_HGT_1
	|| grid_codec->grid_type == GRID_HGT_3)
      {
	  int ret = read_from_hgt (in, img, grid_codec->grid_buffer);
	  if (ret == GGRAPH_OK && progress != NULL)
	      *progress =
		  (int) (((double) (img->next_row + 1) * 100.0) /
			 (double) (img->height));
	  return ret;
      }
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphCheckHgtPath (const char *path, int *lat, int *lon)
{
/* checking for HGT format - retrieving coord from path */
    char file_name[1024];
    char coord_lat[8];
    char coord_lon[8];
    int lat_sign = 1;
    int lon_sign = 1;
    int i;
    int start = 0;

    for (i = strlen (path) - 1; i >= 0; i--)
      {
	  /* attempting to extract the file name */
	  if (path[i] == '/' || path[i] == '\\')
	    {
		start = i + 1;
		break;
	    }
      }
    strcpy (file_name, path + start);

    if (strlen (file_name) != 11)
	return GGRAPH_ERROR;
    if (file_name[0] == 'N')
	;
    else if (file_name[0] == 'S')
	lat_sign = -1;
    else
	return GGRAPH_ERROR;
    if (file_name[1] >= '0' && file_name[1] <= '9')
	coord_lat[0] = file_name[1];
    else
	return GGRAPH_ERROR;
    if (file_name[2] >= '0' && file_name[2] <= '9')
      {
	  coord_lat[1] = file_name[2];
	  coord_lat[2] = '\0';
      }
    else
	return GGRAPH_ERROR;
    if (file_name[3] == 'E')
	;
    else if (file_name[3] == 'W')
	lon_sign = -1;
    else
	return GGRAPH_ERROR;
    if (file_name[4] >= '0' && file_name[4] <= '9')
	coord_lon[0] = file_name[4];
    else
	return GGRAPH_ERROR;
    if (file_name[5] >= '0' && file_name[5] <= '9')
	coord_lon[1] = file_name[5];
    if (file_name[6] >= '0' && file_name[6] <= '9')
      {
	  coord_lon[2] = file_name[6];
	  coord_lon[3] = '\0';
      }
    else
	return GGRAPH_ERROR;
    if (file_name[7] != '.')
	return GGRAPH_ERROR;
    if (file_name[8] != 'h')
	return GGRAPH_ERROR;
    if (file_name[9] != 'g')
	return GGRAPH_ERROR;
    if (file_name[10] != 't')
	return GGRAPH_ERROR;
/* ok, it's an HGT file */
    *lat = atoi (coord_lat) * lat_sign;
    *lon = atoi (coord_lon) * lon_sign;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageInfosFromHgtFile (const char *path, int lat, int lon,
			     const void **infos_handle)
{
/* retrieving Image infos from HGT file */
    FILE *in = NULL;
    gGraphImageInfosPtr infos = NULL;
    long file_length;
    int width;
    int height;
    double pixel_size;
    double half_pixel;

    *infos_handle = NULL;

/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

/* retrieving the HGT dimensions */
    if (fseek (in, 0, SEEK_END) != 0)
	return GGRAPH_HGT_CODEC_ERROR;
    file_length = ftell (in);
    if (file_length == (1201 * 1201 * sizeof (short)))
      {
	  width = 1201;
	  height = 1201;
	  pixel_size = 1.0 / 1200.0;
      }
    else if (file_length == (3601 * 3601 * sizeof (short)))
      {
	  width = 3601;
	  height = 3601;
	  pixel_size = 1.0 / 3600.0;
      }
    else
      {
	  fclose (in);
	  return GGRAPH_HGT_CODEC_ERROR;
      }
    fclose (in);

    infos =
	gg_image_infos_create (GG_PIXEL_GRID, width, height, 16, 1,
			       GGRAPH_SAMPLE_INT, "WGS 84",
			       "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
    if (!infos)
	return GGRAPH_INSUFFICIENT_MEMORY;

/* setting up georeferencing infos */
    half_pixel = pixel_size / 2.0;
    infos->is_georeferenced = 1;
    infos->srid = 4326;
    infos->upper_left_x = (double) lon - half_pixel;
    infos->upper_left_y = (double) lat + 1.0 + half_pixel;
    infos->pixel_x_size = pixel_size;
    infos->pixel_y_size = pixel_size;
    infos->no_data_value = SHRT_MIN;

    *infos_handle = infos;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphCheckBinPath (const char *path, char *hdr_path, int dont_test)
{
/* checking for BIN+HDR format */
    char naked_path[1024];
    int i;
    int start = 0;
    FILE *in;

    *hdr_path = '\0';
    for (i = strlen (path) - 1; i >= 0; i--)
      {
	  /* attempting to extract the file name */
	  if (path[i] == '.')
	    {
		start = i;
		break;
	    }
      }
    strcpy (naked_path, path);
    if (strcasecmp (path + start, ".bin") != 0)
	return GGRAPH_ERROR;
    naked_path[start] = '\0';
    strcat (naked_path, ".hdr");
    if (dont_test)
      {
	  strcpy (hdr_path, naked_path);
	  return GGRAPH_OK;
      }

    in = fopen (naked_path, "rb");
    if (in != NULL)
      {
	  fclose (in);
	  strcpy (hdr_path, naked_path);
	  return GGRAPH_OK;
      }
    naked_path[start] = '\0';
    strcat (naked_path, ".HDR");
    in = fopen (naked_path, "rb");
    if (in != NULL)
      {
	  fclose (in);
	  strcpy (hdr_path, naked_path);
	  return GGRAPH_OK;
      }

    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphCheckFltPath (const char *path, char *hdr_path, int dont_test)
{
/* checking for FLT+HDR format */
    char naked_path[1024];
    int i;
    int start = 0;
    FILE *in;

    *hdr_path = '\0';
    for (i = strlen (path) - 1; i >= 0; i--)
      {
	  /* attempting to extract the file name */
	  if (path[i] == '.')
	    {
		start = i;
		break;
	    }
      }
    strcpy (naked_path, path);
    if (strcasecmp (path + start, ".flt") != 0)
	return GGRAPH_ERROR;
    naked_path[start] = '\0';
    strcat (naked_path, ".hdr");
    if (dont_test)
      {
	  strcpy (hdr_path, naked_path);
	  return GGRAPH_OK;
      }

    in = fopen (naked_path, "rb");
    if (in != NULL)
      {
	  fclose (in);
	  strcpy (hdr_path, naked_path);
	  return GGRAPH_OK;
      }
    naked_path[start] = '\0';
    strcat (naked_path, ".HDR");
    in = fopen (naked_path, "rb");
    if (in != NULL)
      {
	  fclose (in);
	  strcpy (hdr_path, naked_path);
	  return GGRAPH_OK;
      }

    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphCheckDemPath (const char *path, char *hdr_path, int dont_test)
{
/* checking for DEM+HDR format */
    char naked_path[1024];
    int i;
    int start = 0;
    FILE *in;

    *hdr_path = '\0';
    for (i = strlen (path) - 1; i >= 0; i--)
      {
	  /* attempting to extract the file name */
	  if (path[i] == '.')
	    {
		start = i;
		break;
	    }
      }
    strcpy (naked_path, path);
    if (strcasecmp (path + start, ".dem") != 0)
	return GGRAPH_ERROR;
    naked_path[start] = '\0';
    strcat (naked_path, ".hdr");
    if (dont_test)
      {
	  strcpy (hdr_path, naked_path);
	  return GGRAPH_OK;
      }

    in = fopen (naked_path, "rb");
    if (in != NULL)
      {
	  fclose (in);
	  strcpy (hdr_path, naked_path);
	  return GGRAPH_OK;
      }
    naked_path[start] = '\0';
    strcat (naked_path, ".HDR");
    in = fopen (naked_path, "rb");
    if (in != NULL)
      {
	  fclose (in);
	  strcpy (hdr_path, naked_path);
	  return GGRAPH_OK;
      }

    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphCheckAscPath (const char *path)
{
/* checking for ASC format */
    int i;
    int start = 0;

    for (i = strlen (path) - 1; i >= 0; i--)
      {
	  /* attempting to extract the file name */
	  if (path[i] == '.')
	    {
		start = i;
		break;
	    }
      }
    if (strcasecmp (path + start, ".asc") != 0)
	return GGRAPH_ERROR;
    return GGRAPH_OK;
}

static char *
string_tokenizer (char *string, char *delimiters, char **ptr)
{
/* breaking a string into distinct tokens */
    char *start = NULL;
    char *p;
    if (*ptr != NULL)
	p = *ptr;
    else
	p = string;

    if (p == NULL)
	return NULL;

    while (1)
      {
	  int sep = 0;
	  char *pd = delimiters;

	  if (*p == '\0')
	    {
		/* last token end */
		*ptr = p;
		break;
	    }

	  while (*pd != '\0')
	    {
		if (*p == *pd)
		  {
		      /* found a separator char */
		      sep = 1;
		      break;
		  }
		pd++;
	    }
	  if (sep)
	    {
		if (start)
		  {
		      /* token end */
		      *p = '\0';
		      *ptr = p + 1;
		      break;
		  }
	    }
	  else
	    {
		if (!start)
		  {
		      /* token start */
		      start = p;
		  }
	    }
	  p++;
      }

    return start;
}

static int
cvtToInt (const char *str, int *value)
{
/* attempting to convert from string to int */
    const char *p = str;
    int error = 0;
    int digits = 0;
    int sign = 0;
    while (*p != '\0')
      {
	  if (*p >= '0' && *p <= '9')
	      digits++;
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
    if (digits > 0 && sign < 2 && error == 0)
      {
	  *value = atoi (str);
	  return 1;
      }
    return 0;
}

static int
cvtToDouble (const char *str, double *value)
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

static int
parse_binflt_hdr (const char *hdr_path, int *width, int *height,
		  int *bits_per_sample, int *sample, int *endian,
		  double *no_data, double *min, double *max, double *ulx,
		  double *uly, double *pixel_x_size, double *pixel_y_size,
		  int *expected_length)
{
/* parsing an HDR file [BIN or FLT format] */
    FILE *hdr = NULL;
    int s_width = INT_MIN;
    int s_height = INT_MIN;
    int sample_format = GGRAPH_SAMPLE_UNKNOWN;
    int sample_bits = INT_MIN;
    double no_data_value = DBL_MAX;
    double x_pixel_size = DBL_MAX;
    double y_pixel_size = DBL_MAX;
    double lower_left_x = DBL_MAX;
    double lower_left_y = DBL_MAX;
    double min_value = DBL_MAX;
    double max_value = DBL_MIN;
    int little_endian = 0;
    int c;
    int i_tk;
    char *saveptr;
    char hdr_buf[1024];
    char *token;
    char keyword[1024];
    char value[1024];
    char *p_hdr;
    int cvt_int;
    double cvt_dbl;
    int x_centered;
    int y_centered;
    double half_pixel;

/* opening the HDR header file */
    hdr = fopen (hdr_path, "rb");
    if (hdr == NULL)
	return 0;

    p_hdr = hdr_buf;
    while ((c = getc (hdr)) != EOF)
      {
	  /* parsing the Header file */
	  if (c == '\r')
	      continue;
	  if (c == '\n')
	    {
		*p_hdr = '\0';
		if (*hdr_buf == '\0')
		  {
		      /* skipping empty lines */
		      p_hdr = hdr_buf;
		      continue;
		  }
		/* breaking the string into separate tokens */
		i_tk = 0;
		saveptr = NULL;
		while (1)
		  {
		      token = string_tokenizer (hdr_buf, " \t", &saveptr);
		      if (token == NULL)
			  break;
		      if (*token == '\0')
			  continue;
		      if (i_tk == 0)
			{
			    strcpy (keyword, token);
			    i_tk++;
			    continue;
			}

		      strcpy (value, token);
		      i_tk++;
		  }

		if (i_tk != 2)
		    goto stop;
		if (strcasecmp (keyword, "BYTEORDER") == 0)
		  {
		      if (strcasecmp (value, "LSBFIRST") == 0)
			  little_endian = 1;
		      else
			  little_endian = 0;
		  }
		else if (strcasecmp (keyword, "NROWS") == 0)
		  {
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      s_height = cvt_int;
		  }
		else if (strcasecmp (keyword, "NCOLS") == 0)
		  {
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      s_width = cvt_int;
		  }
		else if (strcasecmp (keyword, "NUMBERTYPE") == 0)
		  {
		      if (strcasecmp (value, "2_BYTE_INTEGER") == 0)
			{
			    sample_format = GGRAPH_SAMPLE_INT;
			    sample_bits = 16;
			}
		      else if (strcasecmp (value, "4_BYTE_INTEGER") == 0)
			{
			    sample_format = GGRAPH_SAMPLE_INT;
			    sample_bits = 32;
			}
		      else if (strcasecmp (value, "4_BYTE_FLOAT") == 0)
			{
			    sample_format = GGRAPH_SAMPLE_FLOAT;
			    sample_bits = 32;
			}
		      else if (strcasecmp (value, "8_BYTE_FLOAT") == 0)
			{
			    sample_format = GGRAPH_SAMPLE_FLOAT;
			    sample_bits = 64;
			}
		      else
			  goto stop;
		  }
		else if (strcasecmp (keyword, "NODATA_VALUE") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      no_data_value = cvt_dbl;
		  }
		else if (strcasecmp (keyword, "XLLCENTER") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      lower_left_x = cvt_dbl;
		      x_centered = 1;
		  }
		else if (strcasecmp (keyword, "XLLCORNER") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      lower_left_x = cvt_dbl;
		      x_centered = 0;
		  }
		else if (strcasecmp (keyword, "YLLCENTER") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      lower_left_y = cvt_dbl;
		      y_centered = 1;
		  }
		else if (strcasecmp (keyword, "YLLCORNER") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      lower_left_y = cvt_dbl;
		      y_centered = 0;
		  }
		else if (strcasecmp (keyword, "CELLSIZE") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      x_pixel_size = cvt_dbl;
		      y_pixel_size = cvt_dbl;
		  }
		else if (strcasecmp (keyword, "ZUNITS") == 0)
		  {
		      /* ignoring */
		  }
		else if (strcasecmp (keyword, "MIN_VALUE") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      min_value = cvt_dbl;
		  }
		else if (strcasecmp (keyword, "MAX_VALUE") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      max_value = cvt_dbl;
		  }
		else
		  {
		      /* unrecognized tag */
		      goto stop;
		  }
		p_hdr = hdr_buf;
		continue;
	    }
	  *p_hdr++ = c;
	  if (p_hdr - hdr_buf > 1024)
	      goto stop;
      }
    fclose (hdr);
    hdr = NULL;

    if (s_width <= 0 || s_height <= 0 || sample_bits <= 0)
	goto stop;
    if (sample_format == GGRAPH_SAMPLE_UNKNOWN)
	goto stop;
    if (no_data_value == INT_MAX)
	goto stop;
    if (x_pixel_size == DBL_MAX || y_pixel_size == DBL_MAX)
	goto stop;
    if (lower_left_x == DBL_MAX || lower_left_y == DBL_MAX)
	goto stop;
    if (min_value == DBL_MAX || max_value == DBL_MIN)
	goto stop;

    *width = s_width;
    *height = s_height;
    *sample = sample_format;
    *bits_per_sample = sample_bits;
    *endian = little_endian;
    *no_data = no_data_value;
    *ulx = lower_left_x;
    half_pixel = x_pixel_size / 2.0;
    if (x_centered)
	*ulx -= half_pixel;
    *uly = lower_left_y + ((double) s_height * y_pixel_size);
    half_pixel = y_pixel_size / 2.0;
    if (y_centered)
	*uly += half_pixel;
    *pixel_x_size = x_pixel_size;
    *pixel_y_size = y_pixel_size;
    *min = min_value;
    *max = max_value;
    if (sample_bits == 16)
	*expected_length = s_width * s_height * 2;
    else if (sample_bits == 32)
	*expected_length = s_width * s_height * 4;
    else if (sample_bits == 64)
	*expected_length = s_width * s_height * 8;
    else
	*expected_length = -1;
    return 1;

  stop:
    if (hdr)
	fclose (hdr);
    return 0;
}

GGRAPH_DECLARE int
gGraphImageInfosFromBinFile (const char *path, const char *hdr_path,
			     const void **infos_handle)
{
/* retrieving Image infos from BIN+HDR file */
    FILE *in = NULL;
    gGraphImageInfosPtr infos = NULL;
    long file_length;
    int expected_length;
    int width;
    int height;
    double ulx;
    double uly;
    double pixel_x_size;
    double pixel_y_size;
    int sample;
    int bits_per_sample;
    int endian;
    double no_data;
    double min;
    double max;

    *infos_handle = NULL;

    if (!parse_binflt_hdr
	(hdr_path, &width, &height, &bits_per_sample, &sample, &endian,
	 &no_data, &min, &max, &ulx, &uly, &pixel_x_size, &pixel_y_size,
	 &expected_length))
	return GGRAPH_BIN_CODEC_ERROR;

/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

/* retrieving the BIN dimensions */
    if (fseek (in, 0, SEEK_END) != 0)
	return GGRAPH_BIN_CODEC_ERROR;
    file_length = ftell (in);
    if (file_length != expected_length)
      {
	  fclose (in);
	  return GGRAPH_BIN_CODEC_ERROR;
      }
    fclose (in);

    infos =
	gg_image_infos_create (GG_PIXEL_GRID, width, height, bits_per_sample, 1,
			       sample, NULL, NULL);
    if (!infos)
	return GGRAPH_INSUFFICIENT_MEMORY;

/* setting up georeferencing infos */
    infos->is_georeferenced = 1;
    infos->upper_left_x = ulx;
    infos->upper_left_y = uly;
    infos->pixel_x_size = pixel_x_size;
    infos->pixel_y_size = pixel_y_size;
    infos->no_data_value = no_data;
    infos->min_value = min;
    infos->max_value = max;

    *infos_handle = infos;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageInfosFromFltFile (const char *path, const char *hdr_path,
			     const void **infos_handle)
{
/* retrieving Image infos from FLT+HDR file */
    FILE *in = NULL;
    gGraphImageInfosPtr infos = NULL;
    long file_length;
    int expected_length;
    int width;
    int height;
    double ulx;
    double uly;
    double pixel_x_size;
    double pixel_y_size;
    int sample;
    int bits_per_sample;
    int endian;
    double no_data;
    double min;
    double max;

    *infos_handle = NULL;

    if (!parse_binflt_hdr
	(hdr_path, &width, &height, &bits_per_sample, &sample, &endian,
	 &no_data, &min, &max, &ulx, &uly, &pixel_x_size, &pixel_y_size,
	 &expected_length))
	return GGRAPH_FLT_CODEC_ERROR;

/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

/* retrieving the FLT dimensions */
    if (fseek (in, 0, SEEK_END) != 0)
	return GGRAPH_FLT_CODEC_ERROR;
    file_length = ftell (in);
    if (file_length != expected_length)
      {
	  fclose (in);
	  return GGRAPH_FLT_CODEC_ERROR;
      }
    fclose (in);

    infos =
	gg_image_infos_create (GG_PIXEL_GRID, width, height, bits_per_sample, 1,
			       sample, NULL, NULL);
    if (!infos)
	return GGRAPH_INSUFFICIENT_MEMORY;

/* setting up georeferencing infos */
    infos->is_georeferenced = 1;
    infos->upper_left_x = ulx;
    infos->upper_left_y = uly;
    infos->pixel_x_size = pixel_x_size;
    infos->pixel_y_size = pixel_y_size;
    infos->no_data_value = no_data;
    infos->min_value = min;
    infos->max_value = max;

    *infos_handle = infos;
    return GGRAPH_OK;
}

static int
parse_dem_hdr (const char *hdr_path, int *width, int *height,
	       int *bits_per_sample, int *sample, int *endian, double *no_data,
	       double *ulx, double *uly, double *pixel_x_size,
	       double *pixel_y_size, int *expected_length)
{
/* parsing an HDR file [DEM format] */
    FILE *hdr = NULL;
    int s_width = INT_MIN;
    int s_height = INT_MIN;
    int sample_format = GGRAPH_SAMPLE_UNKNOWN;
    int sample_bits = INT_MIN;
    double no_data_value = DBL_MAX;
    double x_pixel_size = DBL_MAX;
    double y_pixel_size = DBL_MAX;
    double upper_left_x = DBL_MAX;
    double upper_left_y = DBL_MAX;
    int band_row_bytes = INT_MIN;
    int total_row_bytes = INT_MIN;
    int little_endian = 0;
    int c;
    int i_tk;
    char *saveptr;
    char hdr_buf[1024];
    char *token;
    char keyword[1024];
    char value[1024];
    char *p_hdr;
    int cvt_int;
    double cvt_dbl;

/* opening the HDR header file */
    hdr = fopen (hdr_path, "rb");
    if (hdr == NULL)
	return 0;

    p_hdr = hdr_buf;
    while ((c = getc (hdr)) != EOF)
      {
	  /* parsing the Header file */
	  if (c == '\r' || c == '\n')
	    {
		*p_hdr = '\0';
		if (*hdr_buf == '\0')
		  {
		      /* skipping empty lines */
		      p_hdr = hdr_buf;
		      continue;
		  }
		/* breaking the string into separate tokens */
		i_tk = 0;
		saveptr = NULL;
		while (1)
		  {
		      token = string_tokenizer (hdr_buf, " \t", &saveptr);
		      if (token == NULL)
			  break;
		      if (*token == '\0')
			  continue;
		      if (i_tk == 0)
			{
			    strcpy (keyword, token);
			    i_tk++;
			    continue;
			}

		      strcpy (value, token);
		      i_tk++;
		  }

		if (i_tk != 2)
		    goto stop;
		if (strcasecmp (keyword, "BYTEORDER") == 0)
		  {
		      if (strcasecmp (value, "M") == 0)
			  little_endian = 0;
		      else
			  little_endian = 1;
		  }
		else if (strcasecmp (keyword, "LAYOUT") == 0)
		  {
		      /* Bands organization: only Band Interleave by Line is supported */
		      if (strcasecmp (value, "BIL") != 0)
			  goto stop;
		  }
		else if (strcasecmp (keyword, "NROWS") == 0)
		  {
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      s_height = cvt_int;
		  }
		else if (strcasecmp (keyword, "NCOLS") == 0)
		  {
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      s_width = cvt_int;
		  }
		else if (strcasecmp (keyword, "NBANDS") == 0)
		  {
		      /* expected: 1 band */
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      if (cvt_int != 1)
			  goto stop;
		  }
		else if (strcasecmp (keyword, "NBITS") == 0)
		  {
		      /* expected: 16 or 32 bit ints */
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      if (cvt_int == 16)
			{
			    sample_format = GGRAPH_SAMPLE_INT;
			    sample_bits = 16;
			}
		      else if (cvt_int == 32)
			{
			    sample_format = GGRAPH_SAMPLE_INT;
			    sample_bits = 32;
			}
		      else
			  goto stop;
		  }
		else if (strcasecmp (keyword, "BANDROWBYTES") == 0)
		  {
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      band_row_bytes = cvt_int;
		  }
		else if (strcasecmp (keyword, "TOTALROWBYTES") == 0)
		  {
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      total_row_bytes = cvt_int;
		  }
		else if (strcasecmp (keyword, "BANDGAPBYTES") == 0)
		  {
		      /* expected: 0 bytes between bands */
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      if (cvt_int != 0)
			  goto stop;
		  }
		else if (strcasecmp (keyword, "NODATA") == 0)
		  {
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      no_data_value = cvt_int;
		  }
		else if (strcasecmp (keyword, "ULXMAP") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      upper_left_x = cvt_dbl;
		  }
		else if (strcasecmp (keyword, "ULYMAP") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      upper_left_y = cvt_dbl;
		  }
		else if (strcasecmp (keyword, "XDIM") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      x_pixel_size = cvt_dbl;
		  }
		else if (strcasecmp (keyword, "YDIM") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      y_pixel_size = cvt_dbl;
		  }
		else
		  {
		      /* unrecognized tag */
		      goto stop;
		  }
		p_hdr = hdr_buf;
		continue;
	    }
	  *p_hdr++ = c;
	  if (p_hdr - hdr_buf > 1024)
	      goto stop;
      }
    fclose (hdr);
    hdr = NULL;

    if (s_width <= 0 || s_height <= 0 || sample_bits <= 0)
	goto stop;
    if (sample_format == GGRAPH_SAMPLE_UNKNOWN)
	goto stop;
    if (no_data_value == INT_MAX)
	goto stop;
    if (x_pixel_size == DBL_MAX || y_pixel_size == DBL_MAX)
	goto stop;
    if (upper_left_x == DBL_MAX || upper_left_y == DBL_MAX)
	goto stop;
    if (sample_bits == 16)
      {
	  if (2 * s_width != band_row_bytes)
	      goto stop;
	  if (2 * s_width != total_row_bytes)
	      goto stop;
      }
    if (sample_bits == 32)
      {
	  if (4 * s_width != band_row_bytes)
	      goto stop;
	  if (4 * s_width != total_row_bytes)
	      goto stop;
      }
    *width = s_width;
    *height = s_height;
    *sample = sample_format;
    *bits_per_sample = sample_bits;
    *endian = little_endian;
    *no_data = no_data_value;
    *ulx = upper_left_x;
    *uly = upper_left_y;
    *pixel_x_size = x_pixel_size;
    *pixel_y_size = y_pixel_size;
    if (sample_bits == 16)
	*expected_length = s_width * s_height * 2;
    else if (sample_bits == 32)
	*expected_length = s_width * s_height * 4;
    else
	*expected_length = -1;
    return 1;

  stop:
    if (hdr)
	fclose (hdr);
    return 0;
}

GGRAPH_DECLARE int
gGraphImageInfosFromDemFile (const char *path, const char *hdr_path,
			     const void **infos_handle)
{
/* retrieving Image infos from DEM+HDR file */
    FILE *in = NULL;
    gGraphImageInfosPtr infos = NULL;
    long file_length;
    int expected_length;
    int width;
    int height;
    double ulx;
    double uly;
    double pixel_x_size;
    double pixel_y_size;
    int sample;
    int bits_per_sample;
    int endian;
    double no_data;

    *infos_handle = NULL;

    if (!parse_dem_hdr
	(hdr_path, &width, &height, &bits_per_sample, &sample, &endian,
	 &no_data, &ulx, &uly, &pixel_x_size, &pixel_y_size, &expected_length))
	return GGRAPH_FLT_CODEC_ERROR;

/* attempting to open the image file */
    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

/* retrieving the FLT dimensions */
    if (fseek (in, 0, SEEK_END) != 0)
	return GGRAPH_FLT_CODEC_ERROR;
    file_length = ftell (in);
    if (file_length != expected_length)
      {
	  fclose (in);
	  return GGRAPH_FLT_CODEC_ERROR;
      }
    fclose (in);

    infos =
	gg_image_infos_create (GG_PIXEL_GRID, width, height, bits_per_sample, 1,
			       sample, NULL, NULL);
    if (!infos)
	return GGRAPH_INSUFFICIENT_MEMORY;

/* setting up georeferencing infos */
    infos->is_georeferenced = 1;
    infos->upper_left_x = ulx;
    infos->upper_left_y = uly;
    infos->pixel_x_size = pixel_x_size;
    infos->pixel_y_size = pixel_y_size;
    infos->no_data_value = no_data;

    *infos_handle = infos;
    return GGRAPH_OK;
}

static int
parse_asc_hdr (const char *path, FILE * file, int *width, int *height,
	       double *ulx, double *uly, double *pixel_x_size,
	       double *pixel_y_size, double *no_data)
{
/* parsing an ASC file [ASCII GRID format] */
    FILE *hdr = NULL;
    int s_width = INT_MIN;
    int s_height = INT_MIN;
    double x_pixel_size = DBL_MAX;
    double y_pixel_size = DBL_MAX;
    double lower_left_x = DBL_MAX;
    double lower_left_y = DBL_MAX;
    double no_data_value = DBL_MAX;
    int c;
    int i_tk;
    char *saveptr;
    char hdr_buf[1024];
    char *token;
    char keyword[1024];
    char value[1024];
    char *p_hdr;
    int cvt_int;
    double cvt_dbl;
    int x_centered;
    int y_centered;
    double half_pixel;
    int row;

    if (path != NULL)
      {
	  /* opening the ASC header file */
	  hdr = fopen (path, "rb");
      }
    else
	hdr = file;
    if (hdr == NULL)
	return 0;
    row = 0;
    p_hdr = hdr_buf;
    while ((c = getc (hdr)) != EOF)
      {
	  /* parsing the Header file */
	  if (c == '\r')
	      continue;
	  if (c == '\n')
	    {
		row++;
		*p_hdr = '\0';
		if (*hdr_buf == '\0')
		  {
		      /* skipping empty lines */
		      p_hdr = hdr_buf;
		      continue;
		  }
		/* breaking the string into separate tokens */
		i_tk = 0;
		saveptr = NULL;
		while (1)
		  {
		      token = string_tokenizer (hdr_buf, " \t", &saveptr);
		      if (token == NULL)
			  break;
		      if (*token == '\0')
			  continue;
		      if (i_tk == 0)
			{
			    strcpy (keyword, token);
			    i_tk++;
			    continue;
			}

		      strcpy (value, token);
		      i_tk++;
		  }

		if (i_tk != 2)
		    goto stop;
		if (strcasecmp (keyword, "NROWS") == 0)
		  {
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      s_height = cvt_int;
		  }
		else if (strcasecmp (keyword, "NCOLS") == 0)
		  {
		      if (!cvtToInt (value, &cvt_int))
			  goto stop;
		      s_width = cvt_int;
		  }
		else if (strcasecmp (keyword, "NODATA_VALUE") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      no_data_value = cvt_dbl;
		  }
		else if (strcasecmp (keyword, "XLLCENTER") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      lower_left_x = cvt_dbl;
		      x_centered = 1;
		  }
		else if (strcasecmp (keyword, "XLLCORNER") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      lower_left_x = cvt_dbl;
		      x_centered = 0;
		  }
		else if (strcasecmp (keyword, "YLLCENTER") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      lower_left_y = cvt_dbl;
		      y_centered = 1;
		  }
		else if (strcasecmp (keyword, "YLLCORNER") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      lower_left_y = cvt_dbl;
		      y_centered = 0;
		  }
		else if (strcasecmp (keyword, "CELLSIZE") == 0)
		  {
		      if (!cvtToDouble (value, &cvt_dbl))
			  goto stop;
		      x_pixel_size = cvt_dbl;
		      y_pixel_size = x_pixel_size;
		  }
		else
		  {
		      /* unrecognized tag */
		      goto stop;
		  }
		p_hdr = hdr_buf;
		if (row == 6)
		    break;
		continue;
	    }
	  *p_hdr++ = c;
	  if (p_hdr - hdr_buf > 1024)
	      goto stop;
      }
    if (path != NULL)
	fclose (hdr);
    hdr = NULL;

    if (s_width <= 0 || s_height <= 0)
	goto stop;
    if (no_data_value == INT_MAX)
	goto stop;
    if (x_pixel_size == DBL_MAX || y_pixel_size == DBL_MAX)
	goto stop;
    if (lower_left_x == DBL_MAX || lower_left_y == DBL_MAX)
	goto stop;
    *width = s_width;
    *height = s_height;
    *no_data = no_data_value;
    *ulx = lower_left_x;
    half_pixel = x_pixel_size / 2.0;
    if (x_centered)
	*ulx -= half_pixel;
    *uly = lower_left_y + ((double) s_height * y_pixel_size);
    half_pixel = y_pixel_size / 2.0;
    if (y_centered)
	*uly += half_pixel;
    *pixel_x_size = x_pixel_size;
    *pixel_y_size = y_pixel_size;
    return 1;

  stop:
    if (path != NULL)
      {
	  if (hdr)
	      fclose (hdr);
      }
    return 0;
}

GGRAPH_DECLARE int
gGraphImageInfosFromAscFile (const char *path, const void **infos_handle)
{
/* retrieving Image infos from ASC file */
    gGraphImageInfosPtr infos = NULL;
    int width;
    int height;
    double ulx;
    double uly;
    double pixel_x_size;
    double pixel_y_size;
    double no_data;

    *infos_handle = NULL;

    if (!parse_asc_hdr
	(path, NULL, &width, &height, &ulx, &uly, &pixel_x_size, &pixel_y_size,
	 &no_data))
	return GGRAPH_ASCII_CODEC_ERROR;

    infos =
	gg_image_infos_create (GG_PIXEL_GRID, width, height, 32, 1,
			       GGRAPH_SAMPLE_FLOAT, NULL, NULL);
    if (!infos)
	return GGRAPH_INSUFFICIENT_MEMORY;

/* setting up georeferencing infos */
    infos->is_georeferenced = 1;
    infos->upper_left_x = ulx;
    infos->upper_left_y = uly;
    infos->pixel_x_size = pixel_x_size;
    infos->pixel_y_size = pixel_y_size;
    infos->no_data_value = no_data;

    *infos_handle = infos;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_strip_prepare_from_bin_hdr (FILE * in, const char *hdr_path,
				     gGraphStripImagePtr * image_handle)
{
/* preparing to decode an BIN-GRID [by strips] */
    gGraphStripImagePtr img = NULL;
    struct grid_codec_data *grid_codec = NULL;
    long file_length;
    int expected_length;
    int width;
    int height;
    double ulx;
    double uly;
    double pixel_x_size;
    double pixel_y_size;
    int sample;
    int bits_per_sample;
    int endian;
    double no_data;
    double min;
    double max;
    int buf_size;
    void *grid_buffer = NULL;
    int ret = GGRAPH_BIN_CODEC_ERROR;
    *image_handle = NULL;

    if (!parse_binflt_hdr
	(hdr_path, &width, &height, &bits_per_sample, &sample, &endian,
	 &no_data, &min, &max, &ulx, &uly, &pixel_x_size, &pixel_y_size,
	 &expected_length))
	return GGRAPH_BIN_CODEC_ERROR;

/* retrieving the BIN dimensions */
    if (fseek (in, 0, SEEK_END) != 0)
	return GGRAPH_BIN_CODEC_ERROR;
    file_length = ftell (in);
    if (file_length != expected_length)
	return GGRAPH_BIN_CODEC_ERROR;

    img =
	gg_strip_image_create (in, GGRAPH_IMAGE_BIN_HDR, GG_PIXEL_GRID, width,
			       height, bits_per_sample, 1, sample, NULL, NULL);
    if (!img)
      {
	  ret = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }

/* setting up georeferencing infos */
    img->is_georeferenced = 1;
    img->upper_left_x = ulx;
    img->upper_left_y = uly;
    img->pixel_x_size = pixel_x_size;
    img->pixel_y_size = pixel_y_size;
    img->no_data_value = no_data;
    img->min_value = min;
    img->max_value = max;

/* setting up the GRID codec struct */
    grid_codec = malloc (sizeof (struct grid_codec_data));
    if (!grid_codec)
	goto error;
    grid_codec->grid_type = GRID_BIN_HDR;
    grid_codec->is_writer = 0;
    grid_codec->grid_buffer = NULL;
    grid_codec->row_offsets = NULL;
    grid_codec->little_endian = endian;
    img->codec_data = grid_codec;

/* allocating the GRID read buffer */
    if (sample == GGRAPH_SAMPLE_INT)
      {
	  if (bits_per_sample == 16)
	      buf_size = sizeof (short) * width;
	  if (bits_per_sample == 32)
	      buf_size = sizeof (int) * width;
      }
    if (sample == GGRAPH_SAMPLE_FLOAT)
      {
	  if (bits_per_sample == 32)
	      buf_size = sizeof (float) * width;
	  if (bits_per_sample == 64)
	      buf_size = sizeof (double) * width;
      }
    grid_buffer = malloc (buf_size);
    if (!grid_buffer)
	goto error;
    grid_codec->grid_buffer = grid_buffer;

    *image_handle = img;
    return GGRAPH_OK;

  error:
    if (img)
	gGraphDestroyImage (img);
    return ret;
}

GGRAPH_PRIVATE int
gg_image_strip_prepare_from_flt_hdr (FILE * in, const char *hdr_path,
				     gGraphStripImagePtr * image_handle)
{
/* preparing to decode an FLT-GRID [by strips] */
    gGraphStripImagePtr img = NULL;
    struct grid_codec_data *grid_codec = NULL;
    long file_length;
    int expected_length;
    int width;
    int height;
    double ulx;
    double uly;
    double pixel_x_size;
    double pixel_y_size;
    int sample;
    int bits_per_sample;
    int endian;
    double no_data;
    double min;
    double max;
    int buf_size;
    void *grid_buffer = NULL;
    int ret = GGRAPH_FLT_CODEC_ERROR;
    *image_handle = NULL;

    if (!parse_binflt_hdr
	(hdr_path, &width, &height, &bits_per_sample, &sample, &endian,
	 &no_data, &min, &max, &ulx, &uly, &pixel_x_size, &pixel_y_size,
	 &expected_length))
	return GGRAPH_FLT_CODEC_ERROR;

/* retrieving the FLT dimensions */
    if (fseek (in, 0, SEEK_END) != 0)
	return GGRAPH_FLT_CODEC_ERROR;
    file_length = ftell (in);
    if (file_length != expected_length)
	return GGRAPH_FLT_CODEC_ERROR;

    img =
	gg_strip_image_create (in, GGRAPH_IMAGE_FLT_HDR, GG_PIXEL_GRID, width,
			       height, bits_per_sample, 1, sample, NULL, NULL);
    if (!img)
      {
	  ret = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }

/* setting up georeferencing infos */
    img->is_georeferenced = 1;
    img->upper_left_x = ulx;
    img->upper_left_y = uly;
    img->pixel_x_size = pixel_x_size;
    img->pixel_y_size = pixel_y_size;
    img->no_data_value = no_data;
    img->min_value = min;
    img->max_value = max;

/* setting up the GRID codec struct */
    grid_codec = malloc (sizeof (struct grid_codec_data));
    if (!grid_codec)
	goto error;
    grid_codec->grid_type = GRID_FLT_HDR;
    grid_codec->is_writer = 0;
    grid_codec->grid_buffer = NULL;
    grid_codec->row_offsets = NULL;
    grid_codec->little_endian = endian;
    img->codec_data = grid_codec;

/* allocating the GRID read buffer */
    if (sample == GGRAPH_SAMPLE_INT)
      {
	  if (bits_per_sample == 16)
	      buf_size = sizeof (short) * width;
	  if (bits_per_sample == 32)
	      buf_size = sizeof (int) * width;
      }
    if (sample == GGRAPH_SAMPLE_FLOAT)
      {
	  if (bits_per_sample == 32)
	      buf_size = sizeof (float) * width;
	  if (bits_per_sample == 64)
	      buf_size = sizeof (double) * width;
      }
    grid_buffer = malloc (buf_size);
    if (!grid_buffer)
	goto error;
    grid_codec->grid_buffer = grid_buffer;

    *image_handle = img;
    return GGRAPH_OK;

  error:
    if (img)
	gGraphDestroyImage (img);
    return ret;
}

GGRAPH_PRIVATE int
gg_image_strip_prepare_from_dem_hdr (FILE * in, const char *hdr_path,
				     gGraphStripImagePtr * image_handle)
{
/* preparing to decode an DEM-GRID [by strips] */
    gGraphStripImagePtr img = NULL;
    struct grid_codec_data *grid_codec = NULL;
    long file_length;
    int expected_length;
    int width;
    int height;
    double ulx;
    double uly;
    double pixel_x_size;
    double pixel_y_size;
    int sample;
    int bits_per_sample;
    int endian;
    double no_data;
    int buf_size;
    void *grid_buffer = NULL;
    int ret = GGRAPH_DEM_CODEC_ERROR;
    *image_handle = NULL;

    if (!parse_dem_hdr
	(hdr_path, &width, &height, &bits_per_sample, &sample, &endian,
	 &no_data, &ulx, &uly, &pixel_x_size, &pixel_y_size, &expected_length))
	return GGRAPH_DEM_CODEC_ERROR;

/* retrieving the DEM dimensions */
    if (fseek (in, 0, SEEK_END) != 0)
	return GGRAPH_DEM_CODEC_ERROR;
    file_length = ftell (in);
    if (file_length != expected_length)
	return GGRAPH_DEM_CODEC_ERROR;

    img =
	gg_strip_image_create (in, GGRAPH_IMAGE_DEM_HDR, GG_PIXEL_GRID, width,
			       height, bits_per_sample, 1, sample, NULL, NULL);
    if (!img)
      {
	  ret = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }

/* setting up georeferencing infos */
    img->is_georeferenced = 1;
    img->upper_left_x = ulx;
    img->upper_left_y = uly;
    img->pixel_x_size = pixel_x_size;
    img->pixel_y_size = pixel_y_size;
    img->no_data_value = no_data;

/* setting up the GRID codec struct */
    grid_codec = malloc (sizeof (struct grid_codec_data));
    if (!grid_codec)
	goto error;
    grid_codec->grid_type = GRID_DEM_HDR;
    grid_codec->is_writer = 0;
    grid_codec->grid_buffer = NULL;
    grid_codec->row_offsets = NULL;
    grid_codec->little_endian = endian;
    img->codec_data = grid_codec;

/* allocating the GRID read buffer */
    if (bits_per_sample == 16)
	buf_size = sizeof (short) * width;
    if (bits_per_sample == 32)
	buf_size = sizeof (int) * width;
    grid_buffer = malloc (buf_size);
    if (!grid_buffer)
	goto error;
    grid_codec->grid_buffer = grid_buffer;

    *image_handle = img;
    return GGRAPH_OK;

  error:
    if (img)
	gGraphDestroyImage (img);
    return ret;
}

static int
parse_asc_offsets (FILE * in, int height, long **row_offsets)
{
/* retrieving row-offsets from an ASC file [ASCII GRID format] */
    int c;
    int row = 0;
    int ind = 0;
    long *offsets = *row_offsets;
    long current = 0;

    rewind (in);
    while ((c = getc (in)) != EOF)
      {
	  /* parsing the Header file */
	  if (c == '\n')
	    {
		row++;
		if (row >= 6)
		  {
		      if (ind < height)
			  *(offsets + ind) = current;
		      ind++;
		  }
	    }
	  current++;
      }
    if (ind != height + 1)
	return 0;
    return 1;
}

GGRAPH_PRIVATE int
gg_image_strip_prepare_from_ascii_grid (FILE * in,
					gGraphStripImagePtr * image_handle)
{
/* preparing to decode an ASCII-GRID [by strips] */
    gGraphStripImagePtr img = NULL;
    struct grid_codec_data *grid_codec = NULL;
    int width;
    int height;
    double ulx;
    double uly;
    double pixel_x_size;
    double pixel_y_size;
    double no_data;
    long *row_offsets = NULL;
    int ret = GGRAPH_ASCII_CODEC_ERROR;
    *image_handle = NULL;

    if (!parse_asc_hdr
	(NULL, in, &width, &height, &ulx, &uly, &pixel_x_size, &pixel_y_size,
	 &no_data))
	return GGRAPH_ASCII_CODEC_ERROR;

/* preparing the row-offset array */
    row_offsets = malloc (sizeof (long) * height);
    if (!row_offsets)
	return GGRAPH_ASCII_CODEC_ERROR;
    if (!parse_asc_offsets (in, height, &row_offsets))
      {
	  free (row_offsets);
	  return GGRAPH_ASCII_CODEC_ERROR;
      }

    img =
	gg_strip_image_create (in, GGRAPH_IMAGE_ASCII_GRID, GG_PIXEL_GRID,
			       width, height, 32, 1, GGRAPH_SAMPLE_FLOAT, NULL,
			       NULL);
    if (!img)
      {
	  ret = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }

/* setting up georeferencing infos */
    img->is_georeferenced = 1;
    img->upper_left_x = ulx;
    img->upper_left_y = uly;
    img->pixel_x_size = pixel_x_size;
    img->pixel_y_size = pixel_y_size;
    img->no_data_value = no_data;

/* setting up the GRID codec struct */
    grid_codec = malloc (sizeof (struct grid_codec_data));
    if (!grid_codec)
	goto error;
    grid_codec->grid_type = GRID_ASCII;
    grid_codec->is_writer = 0;
    grid_codec->grid_buffer = NULL;
    grid_codec->row_offsets = row_offsets;
    img->codec_data = grid_codec;

    *image_handle = img;
    return GGRAPH_OK;

  error:
    if (img)
	gGraphDestroyImage (img);
    return ret;
}

static int
read_from_bin_grid (FILE * in, gGraphStripImagePtr img, int sample,
		    int bits_per_pixel, int type, void *scanline, int endian)
{
/* decoding a BIN-GRID or a FLT-GRID [by strip] */
    int width = img->width;
    int height = img->height;
    size_t scan_size;
    int row;
    int incr = 0;
    int x;
    int endian_arch = gg_endian_arch ();
    off_t pos;
    unsigned char *p_in;
    short *p_out_short;
    short cell_value_short;
    int *p_out_int;
    int cell_value_int;
    float *p_out_float;
    float cell_value_float;
    double *p_out_double;
    double cell_value_double;

    if (bits_per_pixel == 16)
	scan_size = width * 2;
    if (bits_per_pixel == 32)
	scan_size = width * 4;
    if (bits_per_pixel == 64)
	scan_size = width * 8;
    pos = img->next_row * scan_size;
/* positioning on the start scanline */
    if (fseek (in, pos, SEEK_SET) != 0)
      {
	  if (type == GRID_BIN_HDR)
	      return GGRAPH_BIN_CODEC_ERROR;
	  if (type == GRID_FLT_HDR)
	      return GGRAPH_FLT_CODEC_ERROR;
	  if (type == GRID_DEM_HDR)
	      return GGRAPH_DEM_CODEC_ERROR;
      }
    for (row = 0; row < img->rows_per_block; row++)
      {
	  /* reading the required number of scanlines */
	  if ((row + img->next_row) >= height)
	      break;
	  if (fread (scanline, 1, scan_size, in) != scan_size)
	      return GGRAPH_HGT_CODEC_ERROR;
	  p_in = scanline;
	  if (sample == GGRAPH_SAMPLE_INT)
	    {
		if (bits_per_pixel == 16)
		  {
		      p_out_short = (short *) (img->pixels);
		      p_out_short += (row * width);
		  }

		if (bits_per_pixel == 32)
		  {
		      p_out_int = (int *) (img->pixels);
		      p_out_int += (row * width);
		  }
	    }
	  if (sample == GGRAPH_SAMPLE_FLOAT)
	    {
		if (bits_per_pixel == 32)
		  {
		      p_out_float = (float *) (img->pixels);
		      p_out_float += (row * width);
		  }

		if (bits_per_pixel == 64)
		  {
		      p_out_double = (double *) (img->pixels);
		      p_out_double += (row * width);
		  }
	    }
	  for (x = 0; x < width; x++)
	    {
		if (sample == GGRAPH_SAMPLE_INT)
		  {
		      if (bits_per_pixel == 16)
			{
			    cell_value_short =
				gg_import_int16 (p_in, endian, endian_arch);
			    *p_out_short++ = cell_value_short;
			    p_in += sizeof (short);
			}
		      if (bits_per_pixel == 32)
			{
			    cell_value_int =
				gg_import_int32 (p_in, endian, endian_arch);
			    *p_out_int++ = cell_value_int;
			    p_in += sizeof (int);
			}
		  }
		if (sample == GGRAPH_SAMPLE_FLOAT)
		  {
		      if (bits_per_pixel == 32)
			{
			    cell_value_float =
				gg_import_float (p_in, endian, endian_arch);
			    *p_out_float++ = cell_value_float;
			    p_in += sizeof (float);
			}
		      if (bits_per_pixel == 64)
			{
			    cell_value_double =
				gg_import_double (p_in, endian, endian_arch);
			    *p_out_double++ = cell_value_double;
			    p_in += sizeof (double);
			}
		  }
	    }
	  incr++;
      }
    img->next_row += incr;
    img->current_available_rows = incr;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_strip_read_from_bin_grid (gGraphStripImagePtr img, int *progress)
{
/* decoding a BIN-GRID or a FLT-GRID [by strip] */
    struct grid_codec_data *grid_codec =
	(struct grid_codec_data *) (img->codec_data);
    FILE *in = img->file_handle;

    if (grid_codec->grid_type == GRID_BIN_HDR
	|| grid_codec->grid_type == GRID_FLT_HDR)
      {
	  int ret = read_from_bin_grid (in, img, img->sample_format,
					img->bits_per_sample,
					grid_codec->grid_type,
					grid_codec->grid_buffer,
					grid_codec->little_endian);
	  if (ret == GGRAPH_OK && progress != NULL)
	      *progress =
		  (int) (((double) (img->next_row + 1) * 100.0) /
			 (double) (img->height));
	  return ret;
      }
    return GGRAPH_ERROR;
}

GGRAPH_PRIVATE int
gg_image_strip_read_from_dem_grid (gGraphStripImagePtr img, int *progress)
{
/* decoding a DEM-GRID [by strip] */
    struct grid_codec_data *grid_codec =
	(struct grid_codec_data *) (img->codec_data);
    FILE *in = img->file_handle;

    if (grid_codec->grid_type == GRID_DEM_HDR)
      {
	  int ret = read_from_bin_grid (in, img, img->sample_format,
					img->bits_per_sample,
					grid_codec->grid_type,
					grid_codec->grid_buffer,
					grid_codec->little_endian);
	  if (ret == GGRAPH_OK && progress != NULL)
	      *progress =
		  (int) (((double) (img->next_row + 1) * 100.0) /
			 (double) (img->height));
	  return ret;
      }
    return GGRAPH_ERROR;
}

static int
read_from_ascii_grid (FILE * in, gGraphStripImagePtr img, long *row_offsets)
{
/* decoding an ASCII-GRID [by strip] */
    int width = img->width;
    int height = img->height;
    int row;
    int incr = 0;
    int col;
    int c;
    float *p_out;
    float cell_value;
    char buf[256];
    char *p;

/* positioning on the start scanline */
    if (fseek (in, row_offsets[img->next_row], SEEK_SET) != 0)
	return GGRAPH_ASCII_CODEC_ERROR;
    if (getc (in) != '\n')
	return GGRAPH_ASCII_CODEC_ERROR;

    for (row = 0; row < img->rows_per_block; row++)
      {
	  /* reading the required number of scanlines */
	  if ((row + img->next_row) >= height)
	      break;
	  p_out = (float *) (img->pixels);
	  p_out += row * width;
	  p = buf;
	  col = 0;
	  while ((c = getc (in)) != EOF)
	    {
		/* parsing cell values */
		if (c == '\r')
		    continue;
		if (c == '\n')
		    break;
		if (c == ' ')
		  {
		      /* cell end */
		      *p = '\0';
		      cell_value = atof (buf);
		      if (col >= width)
			  return GGRAPH_ASCII_CODEC_ERROR;
		      *p_out++ = cell_value;
		      col++;
		      p = buf;
		      continue;
		  }
		if ((p - buf) >= 256)
		    return GGRAPH_ASCII_CODEC_ERROR;
		*p++ = c;
	    }
	  if (col != width)
	      return GGRAPH_ASCII_CODEC_ERROR;
	  incr++;
      }
    img->next_row += incr;
    img->current_available_rows = incr;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_strip_read_from_ascii_grid (gGraphStripImagePtr img, int *progress)
{
/* decoding an ASCII-GRID [by strip] */
    struct grid_codec_data *grid_codec =
	(struct grid_codec_data *) (img->codec_data);
    FILE *in = img->file_handle;

    if (grid_codec->grid_type == GRID_ASCII)
      {
	  int ret = read_from_ascii_grid (in, img, grid_codec->row_offsets);
	  if (ret == GGRAPH_OK && progress != NULL)
	      *progress =
		  (int) (((double) (img->next_row + 1) * 100.0) /
			 (double) (img->height));
	  return ret;
      }
    return GGRAPH_ERROR;
}

GGRAPH_PRIVATE int
gg_image_prepare_to_ascii_grid_by_strip (const gGraphStripImagePtr img,
					 FILE * out)
{
/* preparing to export an ASCII GRID [by strips] */
    char dummy[256];

    fprintf (out, "ncols        %d\r\n", img->width);
    fprintf (out, "nrows        %d\r\n", img->height);
    gGraphSmartPrintf (img->upper_left_x, dummy);
    fprintf (out, "xllcorner    %s\r\n", dummy);
    gGraphSmartPrintf (img->upper_left_y -
		       ((double) (img->height) * img->pixel_y_size), dummy);
    fprintf (out, "yllcorner    %s\r\n", dummy);
    gGraphSmartPrintf (img->pixel_y_size, dummy);
    fprintf (out, "cellsize     %s\r\n", dummy);
    gGraphSmartPrintf (img->no_data_value, dummy);
    fprintf (out, "NODATA_value %s\r\n", dummy);

    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_write_to_ascii_grid_by_strip (const gGraphStripImagePtr img,
				       int *progress)
{
/* scanline(s) ASCII GRID export [by strip] */
    FILE *out = img->file_handle;
    int row;
    int col;
    char *p_in_int8;
    unsigned char *p_in_uint8;
    short *p_in_int16;
    unsigned short *p_in_uint16;
    int *p_in_int32;
    unsigned int *p_in_uint32;
    float *p_in_flt;
    double *p_in_dbl;
    char dummy[256];

    for (row = 0; row < img->current_available_rows; row++)
      {
	  switch (img->sample_format)
	    {
	    case GGRAPH_SAMPLE_UINT:
		switch (img->bits_per_sample)
		  {
		  case 8:
		      p_in_uint8 = (unsigned char *) (img->pixels);
		      p_in_uint8 += row * img->width;
		      break;
		  case 16:
		      p_in_uint16 = (unsigned short *) (img->pixels);
		      p_in_uint16 += row * img->width;
		      break;
		  case 32:
		      p_in_uint32 = (unsigned int *) (img->pixels);
		      p_in_uint32 += row * img->width;
		      break;
		  };
		break;
	    case GGRAPH_SAMPLE_INT:
		switch (img->bits_per_sample)
		  {
		  case 8:
		      p_in_int8 = (char *) (img->pixels);
		      p_in_int8 += row * img->width;
		      break;
		  case 16:
		      p_in_int16 = (short *) (img->pixels);
		      p_in_int16 += row * img->width;
		      break;
		  case 32:
		      p_in_int32 = (int *) (img->pixels);
		      p_in_int32 += row * img->width;
		      break;
		  };
		break;
	    case GGRAPH_SAMPLE_FLOAT:
		switch (img->bits_per_sample)
		  {
		  case 32:
		      p_in_flt = (float *) (img->pixels);
		      p_in_flt += row * img->width;
		      break;
		  case 64:
		      p_in_dbl = (double *) (img->pixels);
		      p_in_dbl += row * img->width;
		      break;
		  };
		break;
	    };
	  for (col = 0; col < img->width; col++)
	    {
		switch (img->sample_format)
		  {
		  case GGRAPH_SAMPLE_UINT:
		      switch (img->bits_per_sample)
			{
			case 8:
			    sprintf (dummy, "%u", *p_in_uint8++);
			    break;
			case 16:
			    sprintf (dummy, "%u", *p_in_uint16++);
			    break;
			case 32:
			    sprintf (dummy, "%u", *p_in_uint32++);
			    break;
			};
		      break;
		  case GGRAPH_SAMPLE_INT:
		      switch (img->bits_per_sample)
			{
			case 8:
			    sprintf (dummy, "%d", *p_in_int8++);
			    break;
			case 16:
			    sprintf (dummy, "%d", *p_in_int16++);
			    break;
			case 32:
			    sprintf (dummy, "%d", *p_in_int32++);
			    break;
			};
		      break;
		  case GGRAPH_SAMPLE_FLOAT:
		      switch (img->bits_per_sample)
			{
			case 32:
			    gGraphSmartPrintf (*p_in_flt++, dummy);
			    break;
			case 64:
			    gGraphSmartPrintf (*p_in_dbl++, dummy);
			    break;
			};
		      break;
		  };
		fprintf (out, "%s ", dummy);
	    }
	  /* terminating a full scanline */
	  fprintf (out, "\r\n");
      }
    img->next_row += img->current_available_rows;

    if (progress != NULL)
	*progress =
	    (int) (((double) (img->next_row + 1) * 100.0) /
		   (double) (img->height));
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_prepare_to_bin_hdr_by_strip (const gGraphStripImagePtr img)
{
/* preparing to export a BIN+HDR GRID [by strips] */
    struct grid_codec_data *grid_codec = NULL;
    int buf_size;
    void *grid_buffer = NULL;

    if (img->sample_format != GGRAPH_SAMPLE_INT)
	return GGRAPH_BIN_CODEC_ERROR;
    if (img->bits_per_sample == 16 || img->bits_per_sample == 32)
	;
    else
	return GGRAPH_BIN_CODEC_ERROR;

/* setting up the GRID codec struct */
    grid_codec = malloc (sizeof (struct grid_codec_data));
    if (!grid_codec)
	return GGRAPH_BIN_CODEC_ERROR;
    grid_codec->grid_type = GRID_BIN_HDR;
    grid_codec->is_writer = 1;
    grid_codec->grid_buffer = NULL;
    grid_codec->row_offsets = NULL;

/* allocating the GRID read buffer */
    if (img->bits_per_sample == 16)
	buf_size = sizeof (short) * img->width;
    else
	buf_size = sizeof (int) * img->width;
    grid_buffer = malloc (buf_size);
    if (!grid_buffer)
      {
	  free (grid_codec);
	  return GGRAPH_BIN_CODEC_ERROR;
      }
    grid_codec->grid_buffer = grid_buffer;
    img->codec_data = grid_codec;

    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_prepare_to_flt_hdr_by_strip (const gGraphStripImagePtr img)
{
/* preparing to export a FLT+HDR GRID [by strips] */
    struct grid_codec_data *grid_codec = NULL;
    int buf_size;
    void *grid_buffer = NULL;

    if (img->sample_format != GGRAPH_SAMPLE_FLOAT)
	return GGRAPH_FLT_CODEC_ERROR;
    if (img->bits_per_sample == 32 || img->bits_per_sample == 64)
	;
    else
	return GGRAPH_FLT_CODEC_ERROR;

/* setting up the GRID codec struct */
    grid_codec = malloc (sizeof (struct grid_codec_data));
    if (!grid_codec)
	return GGRAPH_FLT_CODEC_ERROR;
    grid_codec->grid_type = GRID_FLT_HDR;
    grid_codec->is_writer = 1;
    grid_codec->grid_buffer = NULL;
    grid_codec->row_offsets = NULL;

/* allocating the GRID read buffer */
    if (img->bits_per_sample == 32)
	buf_size = sizeof (float) * img->width;
    else
	buf_size = sizeof (double) * img->width;
    grid_buffer = malloc (buf_size);
    if (!grid_buffer)
      {
	  free (grid_codec);
	  return GGRAPH_FLT_CODEC_ERROR;
      }
    grid_codec->grid_buffer = grid_buffer;
    img->codec_data = grid_codec;

    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_write_to_bin_hdr_by_strip (const gGraphStripImagePtr img,
				    int *progress)
{
/* scanline(s) BIN+HDR GRID export [by strip] */
    FILE *out = img->file_handle;
    struct grid_codec_data *grid_codec =
	(struct grid_codec_data *) (img->codec_data);
    int row;
    int col;
    short *p_in_int16;
    int *p_in_int32;
    unsigned char *p_out;
    size_t sz;
    int endian_arch = gg_endian_arch ();

    for (row = 0; row < img->current_available_rows; row++)
      {
	  switch (img->bits_per_sample)
	    {
	    case 16:
		p_in_int16 = (short *) (img->pixels);
		p_in_int16 += row * img->width;
		sz = img->width * sizeof (short);
		break;
	    case 32:
		p_in_int32 = (int *) (img->pixels);
		p_in_int32 += row * img->width;
		sz = img->width * sizeof (int);
		break;
	    };
	  p_out = grid_codec->grid_buffer;

	  for (col = 0; col < img->width; col++)
	    {
		switch (img->bits_per_sample)
		  {
		  case 16:
		      if (*p_in_int16 < img->min_value)
			  img->min_value = *p_in_int16;
		      if (*p_in_int16 > img->max_value)
			  img->max_value = *p_in_int16;
		      gg_export_int16 (*p_in_int16++, p_out, 1, endian_arch);
		      p_out += sizeof (short);
		      break;
		  case 32:
		      if (*p_in_int32 < img->min_value)
			  img->min_value = *p_in_int32;
		      if (*p_in_int32 > img->max_value)
			  img->max_value = *p_in_int32;
		      gg_export_int32 (*p_in_int32++, p_out, 1, endian_arch);
		      p_out += sizeof (int);
		      break;
		  };
	    }
	  /* terminating a full scanline */
	  if (fwrite (grid_codec->grid_buffer, 1, sz, out) != sz)
	      return GGRAPH_BIN_CODEC_ERROR;
      }
    img->next_row += img->current_available_rows;

    if (progress != NULL)
	*progress =
	    (int) (((double) (img->next_row + 1) * 100.0) /
		   (double) (img->height));
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_write_to_flt_hdr_by_strip (const gGraphStripImagePtr img,
				    int *progress)
{
/* scanline(s) FLT+HDR GRID export [by strip] */
    FILE *out = img->file_handle;
    struct grid_codec_data *grid_codec =
	(struct grid_codec_data *) (img->codec_data);
    int row;
    int col;
    float *p_in_float;
    double *p_in_double;
    unsigned char *p_out;
    size_t sz;
    int endian_arch = gg_endian_arch ();

    for (row = 0; row < img->current_available_rows; row++)
      {
	  switch (img->bits_per_sample)
	    {
	    case 32:
		p_in_float = (float *) (img->pixels);
		p_in_float += row * img->width;
		sz = img->width * sizeof (float);
		break;
	    case 64:
		p_in_double = (double *) (img->pixels);
		p_in_double += row * img->width;
		sz = img->width * sizeof (double);
		break;
	    };
	  p_out = grid_codec->grid_buffer;

	  for (col = 0; col < img->width; col++)
	    {
		switch (img->bits_per_sample)
		  {
		  case 32:
		      if (*p_in_float < img->min_value)
			  img->min_value = *p_in_float;
		      if (*p_in_float > img->max_value)
			  img->max_value = *p_in_float;
		      gg_export_float (*p_in_float++, p_out, 1, endian_arch);
		      p_out += sizeof (float);
		      break;
		  case 64:
		      if (*p_in_double < img->min_value)
			  img->min_value = *p_in_double;
		      if (*p_in_double > img->max_value)
			  img->max_value = *p_in_double;
		      gg_export_double (*p_in_double++, p_out, 1, endian_arch);
		      p_out += sizeof (float);
		      break;
		  };
	    }
	  /* terminating a full scanline */
	  if (fwrite (grid_codec->grid_buffer, 1, sz, out) != sz)
	      return GGRAPH_FLT_CODEC_ERROR;
      }
    img->next_row += img->current_available_rows;

    if (progress != NULL)
	*progress =
	    (int) (((double) (img->next_row + 1) * 100.0) /
		   (double) (img->height));
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphWriteBinHeader (const char *hdr_path, const void *ptr)
{
/* exporting a BIN Header file */
    FILE *out = NULL;
    char dummy[256];
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

/* attempting to open/create the header file */
    out = fopen (hdr_path, "wb");
    if (out == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

    fprintf (out, "NCOLS         %d\r\n", img->width);
    fprintf (out, "NROWS         %d\r\n", img->height);
    gGraphSmartPrintf (img->upper_left_x, dummy);
    fprintf (out, "XLLCORNER     %s\r\n", dummy);
    gGraphSmartPrintf (img->upper_left_y -
		       ((double) (img->height) * img->pixel_y_size), dummy);
    fprintf (out, "YLLCORNER     %s\r\n", dummy);
    gGraphSmartPrintf (img->pixel_y_size, dummy);
    fprintf (out, "CELLSIZE      %s\r\n", dummy);
    gGraphSmartPrintf (img->no_data_value, dummy);
    fprintf (out, "NODATA_VALUE  %s\r\n", dummy);
    fprintf (out, "BYTEORDER     LSBFIRST\r\n");
    if (img->bits_per_sample == 16)
	fprintf (out, "NUMBERTYPE    2_BYTE_INTEGER\r\n");
    else
	fprintf (out, "NUMBERTYPE    4_BYTE_INTEGER\r\n");
    fprintf (out, "ZUNITS        METERS\r\n");
    gGraphSmartPrintf (img->min_value, dummy);
    fprintf (out, "MIN_VALUE     %s\r\n", dummy);
    gGraphSmartPrintf (img->max_value, dummy);
    fprintf (out, "MAX_VALUE     %s\r\n", dummy);

    fclose (out);

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphWriteFltHeader (const char *hdr_path, const void *ptr)
{
/* exporting a FLT Header file */
    FILE *out = NULL;
    char dummy[256];
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

/* attempting to open/create the header file */
    out = fopen (hdr_path, "wb");
    if (out == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

    fprintf (out, "ncols         %d\r\n", img->width);
    fprintf (out, "nrows         %d\r\n", img->height);
    gGraphSmartPrintf (img->upper_left_x, dummy);
    fprintf (out, "xllcorner     %s\r\n", dummy);
    gGraphSmartPrintf (img->upper_left_y -
		       ((double) (img->height) * img->pixel_y_size), dummy);
    fprintf (out, "yllcorner     %s\r\n", dummy);
    gGraphSmartPrintf (img->pixel_y_size, dummy);
    fprintf (out, "cellsize      %s\r\n", dummy);
    gGraphSmartPrintf (img->no_data_value, dummy);
    fprintf (out, "NODATA_value  %s\r\n", dummy);
    fprintf (out, "byteorder     LSBFIRST\r\n");
    if (img->bits_per_sample == 32)
	fprintf (out, "NUMBERTYPE    4_BYTE_FLOAT\r\n");
    else
	fprintf (out, "NUMBERTYPE    8_BYTE_FLOAT\r\n");
    gGraphSmartPrintf (img->min_value, dummy);
    fprintf (out, "MIN_VALUE     %s\r\n", dummy);
    gGraphSmartPrintf (img->max_value, dummy);
    fprintf (out, "MAX_VALUE     %s\r\n", dummy);

    fclose (out);

    return GGRAPH_OK;
}
