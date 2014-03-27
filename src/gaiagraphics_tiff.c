/* 
/ gaiagraphics_tiff.c
/
/ TIFF auxiliary helpers
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
#include <string.h>
#include <stdlib.h>

/*
/ the following patch supporting GeoTiff headers
/ was kindly contributed by Brad Hards: 2011-09-02
*/
#ifdef HAVE_GEOTIFF_GEOTIFF_H
#include <geotiff/geotiff.h>
#include <geotiff/xtiffio.h>
#include <geotiff/geo_tiffp.h>
#include <geotiff/geo_keyp.h>
#include <geotiff/geovalues.h>
#include <geotiff/geo_normalize.h>
#elif HAVE_LIBGEOTIFF_GEOTIFF_H
#include <libgeotiff/geotiff.h>
#include <libgeotiff/xtiffio.h>
#include <libgeotiff/geo_tiffp.h>
#include <libgeotiff/geo_keyp.h>
#include <libgeotiff/geovalues.h>
#include <libgeotiff/geo_normalize.h>
#else
#include <geotiff.h>
#include <xtiffio.h>
#include <geo_tiffp.h>
#include <geo_keyp.h>
#include <geovalues.h>
#include <geo_normalize.h>
#endif

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

#define TIFF_TYPE_MONOCHROME	1
#define TIFF_TYPE_PALETTE		2
#define TIFF_TYPE_GRAYSCALE		3
#define TIFF_TYPE_RGB			4
#define TIFF_TYPE_GRID			5

struct tiff_codec_data
{
/* a struct used by TIFF codec */
    int is_geotiff;
    int is_writer;
    TIFF *tiff_handle;
    GTIF *geotiff_handle;
    void *tiff_buffer;
    int is_tiled;
    int tiff_type;
};

struct memfile
{
/* a struct emulating a file [memory mapped] */
    unsigned char *buffer;
    int malloc_block;
    tsize_t size;
    tsize_t eof;
    toff_t current;
};

static void
memory_realloc (struct memfile *mem, tsize_t req_size)
{
/* expanding the allocated memory */
    unsigned char *new_buffer;
    tsize_t new_size = mem->size;
    while (new_size < req_size)
	new_size += mem->malloc_block;
    new_buffer = realloc (mem->buffer, new_size);
    if (!new_buffer)
	return;
    mem->buffer = new_buffer;
    mem->size = new_size;
}

static tsize_t
memory_readproc (thandle_t clientdata, tdata_t data, tsize_t size)
{
/* emulating the read()  function */
    struct memfile *mem = clientdata;
    tsize_t len;
    if (mem->current >= (toff_t) mem->eof)
	return 0;
    len = size;
    if ((mem->current + size) >= (toff_t) mem->eof)
	len = (tsize_t) (mem->eof - mem->current);
    memcpy (data, mem->buffer + mem->current, len);
    mem->current += len;
    return len;
}

static tsize_t
memory_writeproc (thandle_t clientdata, tdata_t data, tsize_t size)
{
/* emulating the write()  function */
    struct memfile *mem = clientdata;
    if ((mem->current + size) >= (toff_t) mem->size)
	memory_realloc (mem, mem->current + size);
    if ((mem->current + size) >= (toff_t) mem->size)
	return -1;
    memcpy (mem->buffer + mem->current, (unsigned char *) data, size);
    mem->current += size;
    if (mem->current > (toff_t) mem->eof)
	mem->eof = (tsize_t) (mem->current);
    return size;
}

static toff_t
memory_seekproc (thandle_t clientdata, toff_t offset, int whence)
{
/* emulating the lseek()  function */
    struct memfile *mem = clientdata;
    switch (whence)
      {
      case SEEK_CUR:
	  if ((int) (mem->current + offset) < 0)
	      return (toff_t) - 1;
	  mem->current += offset;
	  if ((toff_t) mem->eof < mem->current)
	      mem->eof = (tsize_t) (mem->current);
	  break;
      case SEEK_END:
	  if ((int) (mem->eof + offset) < 0)
	      return (toff_t) - 1;
	  mem->current = mem->eof + offset;
	  if ((toff_t) mem->eof < mem->current)
	      mem->eof = (tsize_t) (mem->current);
	  break;
      case SEEK_SET:
      default:
	  if ((int) offset < 0)
	      return (toff_t) - 1;
	  mem->current = offset;
	  if ((toff_t) mem->eof < mem->current)
	      mem->eof = (tsize_t) (mem->current);
	  break;
      };
    return mem->current;
}

static int
closeproc (thandle_t clientdata)
{
/* emulating the close()  function */
    if (clientdata)
	return 0;		/* does absolutely nothing - required in order to suppress warnings */
    return 0;
}

static toff_t
memory_sizeproc (thandle_t clientdata)
{
/* returning the pseudo-file current size */
    struct memfile *mem = clientdata;
    return mem->eof;
}

static int
mapproc (thandle_t clientdata, tdata_t * data, toff_t * offset)
{
    if (clientdata || data || offset)
	return 0;		/* does absolutely nothing - required in order to suppress warnings */
    return 0;
}

static void
unmapproc (thandle_t clientdata, tdata_t data, toff_t offset)
{
    if (clientdata || data || offset)
	return;			/* does absolutely nothing - required in order to suppress warnings */
    return;
}

GGRAPH_PRIVATE int
gg_image_infos_from_mem_tiff (int size, const void *data,
			      gGraphImageInfosPtr * infos_handle)
{
/* image infos from TIFF (memory cached) */
    gGraphImageInfosPtr infos;
    uint16 bits_per_sample;
    uint16 samples_per_pixel;
    uint16 photometric;
    uint16 compression;
    uint16 sample_format;
    uint16 planar_config;
    uint32 width = 0;
    uint32 height = 0;
    uint32 rows_strip = 0;
    struct memfile clientdata;
    int is_tiled;
    uint32 tile_width;
    uint32 tile_height;
    int type;
    int gg_sample_format;
    TIFF *in = (TIFF *) 0;
    int max_palette = 0;
    unsigned char red[256];
    unsigned char green[256];
    unsigned char blue[256];
    int i;
    *infos_handle = NULL;

/* suppressing TIFF warnings */
    TIFFSetWarningHandler (NULL);

/* reading from memory */
    clientdata.buffer = (unsigned char *) data;
    clientdata.size = size;
    clientdata.eof = size;
    clientdata.current = 0;
    in = TIFFClientOpen ("tiff", "r", &clientdata, memory_readproc,
			 memory_writeproc, memory_seekproc, closeproc,
			 memory_sizeproc, mapproc, unmapproc);
    if (in == NULL)
	return GGRAPH_TIFF_CODEC_ERROR;
    is_tiled = TIFFIsTiled (in);
/* retrieving the TIFF dimensions */
    TIFFGetField (in, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField (in, TIFFTAG_IMAGEWIDTH, &width);
    if (is_tiled)
      {
	  TIFFGetField (in, TIFFTAG_TILEWIDTH, &tile_width);
	  TIFFGetField (in, TIFFTAG_TILELENGTH, &tile_height);
      }
    else
	TIFFGetField (in, TIFFTAG_ROWSPERSTRIP, &rows_strip);
    TIFFGetField (in, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    if (TIFFGetField (in, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  samples_per_pixel = 1;
      }
    TIFFGetField (in, TIFFTAG_PHOTOMETRIC, &photometric);
    TIFFGetField (in, TIFFTAG_COMPRESSION, &compression);
    if (TIFFGetField (in, TIFFTAG_PLANARCONFIG, &planar_config) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  planar_config = 1;
      }
    if (TIFFGetField (in, TIFFTAG_SAMPLEFORMAT, &sample_format) == 0)
	sample_format = SAMPLEFORMAT_UINT;
    if (planar_config == PLANARCONFIG_CONTIG)
      {
	  if (sample_format == SAMPLEFORMAT_UINT)
	    {
		if (bits_per_sample == 1 && samples_per_pixel == 1
		    && photometric < 2)
		  {
		      type = GG_PIXEL_PALETTE;
		      max_palette = 2;
		      if (photometric == PHOTOMETRIC_MINISWHITE)
			{
			    red[0] = 255;
			    green[0] = 255;
			    blue[0] = 255;
			    red[1] = 0;
			    green[1] = 0;
			    blue[1] = 0;
			}
		      else
			{
			    red[0] = 0;
			    green[0] = 0;
			    blue[0] = 0;
			    red[1] = 255;
			    green[1] = 255;
			    blue[1] = 255;
			}
		  }
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric == 3)
		  {
		      uint16 *plt_red;
		      uint16 *plt_green;
		      uint16 *plt_blue;
		      type = GG_PIXEL_PALETTE;
		      TIFFGetField (in, TIFFTAG_COLORMAP, &plt_red, &plt_green,
				    &plt_blue);
		      max_palette = 256;
		      for (i = 0; i < max_palette; i++)
			{
			    if (plt_red[i] < 256)
				red[i] = plt_red[i];
			    else
				red[i] = plt_red[i] / 256;
			    if (plt_green[i] < 256)
				green[i] = plt_green[i];
			    else
				green[i] = plt_green[i] / 256;
			    if (plt_blue[i] < 256)
				blue[i] = plt_blue[i];
			    else
				blue[i] = plt_blue[i] / 256;
			}
		  }
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric < 2)
		    type = GG_PIXEL_GRAYSCALE;
		else if (bits_per_sample == 8 && samples_per_pixel == 3)
		    type = GG_PIXEL_RGB;
		else
		    type = GG_PIXEL_UNKNOWN;
	    }
	  else if (samples_per_pixel == 1)
	      type = GG_PIXEL_GRID;
	  else
	      type = GG_PIXEL_UNKNOWN;
      }
    else
	type = GG_PIXEL_UNKNOWN;
    switch (sample_format)
      {
      case SAMPLEFORMAT_UINT:
	  gg_sample_format = GGRAPH_SAMPLE_UINT;
	  break;
      case SAMPLEFORMAT_INT:
	  gg_sample_format = GGRAPH_SAMPLE_INT;
	  break;
      case SAMPLEFORMAT_IEEEFP:
	  gg_sample_format = GGRAPH_SAMPLE_FLOAT;
	  break;
      default:
	  gg_sample_format = GGRAPH_SAMPLE_UNKNOWN;
	  break;
      };
    infos =
	gg_image_infos_create (type, width, height, bits_per_sample,
			       samples_per_pixel, gg_sample_format, NULL, NULL);
    if (!infos)
	return GGRAPH_INSUFFICIENT_MEMORY;
    if (is_tiled)
      {
	  infos->tile_width = tile_width;
	  infos->tile_height = tile_height;
      }
    else
	infos->rows_per_strip = rows_strip;
    switch (compression)
      {
      case COMPRESSION_NONE:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_NONE;
	  break;
      case COMPRESSION_LZW:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_LZW;
	  break;
      case COMPRESSION_DEFLATE:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_DEFLATE;
	  break;
      case COMPRESSION_JPEG:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_JPEG;
	  break;
      case COMPRESSION_CCITTFAX3:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX3;
	  break;
      case COMPRESSION_CCITTFAX4:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX4;
	  break;
      default:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_UNKNOWN;
	  break;
      };
    for (i = 0; i < max_palette; i++)
      {
	  infos->palette_red[i] = red[i];
	  infos->palette_green[i] = green[i];
	  infos->palette_blue[i] = blue[i];
      }
    infos->max_palette = max_palette;
    TIFFClose (in);
    *infos_handle = infos;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_infos_from_tiff (const char *path, gGraphImageInfosPtr * infos_handle)
{
/* image infos from TIFF */
    gGraphImageInfosPtr infos;
    uint16 bits_per_sample;
    uint16 samples_per_pixel;
    uint16 photometric;
    uint16 compression;
    uint16 sample_format;
    uint16 planar_config;
    uint32 width = 0;
    uint32 height = 0;
    uint32 rows_strip = 0;
    int is_tiled;
    uint32 tile_width;
    uint32 tile_height;
    int type;
    int gg_sample_format;
    TIFF *in = (TIFF *) 0;
    int max_palette = 0;
    unsigned char red[256];
    unsigned char green[256];
    unsigned char blue[256];
    int i;
    *infos_handle = NULL;

/* suppressing TIFF warnings */
    TIFFSetWarningHandler (NULL);

/* reading from file */
    in = TIFFOpen (path, "r");
    if (in == NULL)
	return GGRAPH_TIFF_CODEC_ERROR;
    is_tiled = TIFFIsTiled (in);
/* retrieving the TIFF dimensions */
    TIFFGetField (in, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField (in, TIFFTAG_IMAGEWIDTH, &width);
    if (is_tiled)
      {
	  TIFFGetField (in, TIFFTAG_TILEWIDTH, &tile_width);
	  TIFFGetField (in, TIFFTAG_TILELENGTH, &tile_height);
      }
    else
	TIFFGetField (in, TIFFTAG_ROWSPERSTRIP, &rows_strip);
    TIFFGetField (in, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    if (TIFFGetField (in, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  samples_per_pixel = 1;
      }
    TIFFGetField (in, TIFFTAG_PHOTOMETRIC, &photometric);
    TIFFGetField (in, TIFFTAG_COMPRESSION, &compression);
    if (TIFFGetField (in, TIFFTAG_PLANARCONFIG, &planar_config) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  planar_config = 1;
      }
    if (TIFFGetField (in, TIFFTAG_SAMPLEFORMAT, &sample_format) == 0)
	sample_format = SAMPLEFORMAT_UINT;
    if (planar_config == PLANARCONFIG_CONTIG)
      {
	  if (sample_format == SAMPLEFORMAT_UINT)
	    {
		if (bits_per_sample == 1 && samples_per_pixel == 1
		    && photometric < 2)
		  {
		      type = GG_PIXEL_PALETTE;
		      max_palette = 2;
		      if (photometric == PHOTOMETRIC_MINISWHITE)
			{
			    red[0] = 255;
			    green[0] = 255;
			    blue[0] = 255;
			    red[1] = 0;
			    green[1] = 0;
			    blue[1] = 0;
			}
		      else
			{
			    red[0] = 0;
			    green[0] = 0;
			    blue[0] = 0;
			    red[1] = 255;
			    green[1] = 255;
			    blue[1] = 255;
			}
		  }
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric == 3)
		  {
		      uint16 *plt_red;
		      uint16 *plt_green;
		      uint16 *plt_blue;
		      type = GG_PIXEL_PALETTE;
		      TIFFGetField (in, TIFFTAG_COLORMAP, &plt_red, &plt_green,
				    &plt_blue);
		      max_palette = 256;
		      for (i = 0; i < max_palette; i++)
			{
			    if (plt_red[i] < 256)
				red[i] = plt_red[i];
			    else
				red[i] = plt_red[i] / 256;
			    if (plt_green[i] < 256)
				green[i] = plt_green[i];
			    else
				green[i] = plt_green[i] / 256;
			    if (plt_blue[i] < 256)
				blue[i] = plt_blue[i];
			    else
				blue[i] = plt_blue[i] / 256;
			}
		  }
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric < 2)
		    type = GG_PIXEL_GRAYSCALE;
		else if (bits_per_sample == 8 && samples_per_pixel == 3)
		    type = GG_PIXEL_RGB;
		else
		    type = GG_PIXEL_UNKNOWN;
	    }
	  else if (samples_per_pixel == 1)
	      type = GG_PIXEL_GRID;
	  else
	      type = GG_PIXEL_UNKNOWN;
      }
    else
	type = GG_PIXEL_UNKNOWN;
    switch (sample_format)
      {
      case SAMPLEFORMAT_UINT:
	  gg_sample_format = GGRAPH_SAMPLE_UINT;
	  break;
      case SAMPLEFORMAT_INT:
	  gg_sample_format = GGRAPH_SAMPLE_INT;
	  break;
      case SAMPLEFORMAT_IEEEFP:
	  gg_sample_format = GGRAPH_SAMPLE_FLOAT;
	  break;
      default:
	  gg_sample_format = GGRAPH_SAMPLE_UNKNOWN;
	  break;
      };
    infos =
	gg_image_infos_create (type, width, height, bits_per_sample,
			       samples_per_pixel, gg_sample_format, NULL, NULL);
    if (!infos)
	return GGRAPH_INSUFFICIENT_MEMORY;
    if (is_tiled)
      {
	  infos->tile_width = tile_width;
	  infos->tile_height = tile_height;
      }
    else
	infos->rows_per_strip = rows_strip;
    switch (compression)
      {
      case COMPRESSION_NONE:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_NONE;
	  break;
      case COMPRESSION_LZW:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_LZW;
	  break;
      case COMPRESSION_DEFLATE:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_DEFLATE;
	  break;
      case COMPRESSION_JPEG:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_JPEG;
	  break;
      case COMPRESSION_CCITTFAX3:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX3;
	  break;
      case COMPRESSION_CCITTFAX4:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX4;
	  break;
      default:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_UNKNOWN;
	  break;
      };
    for (i = 0; i < max_palette; i++)
      {
	  infos->palette_red[i] = red[i];
	  infos->palette_green[i] = green[i];
	  infos->palette_blue[i] = blue[i];
      }
    infos->max_palette = max_palette;
    TIFFClose (in);
    *infos_handle = infos;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_infos_from_geo_tiff (const char *path,
			      gGraphImageInfosPtr * infos_handle)
{
/* image infos from GeoTIFF */
    gGraphImageInfosPtr infos;
    uint16 bits_per_sample;
    uint16 samples_per_pixel;
    uint16 photometric;
    uint16 compression;
    uint16 sample_format;
    uint16 planar_config;
    uint32 width = 0;
    uint32 height = 0;
    uint32 rows_strip = 0;
    int is_tiled;
    uint32 tile_width;
    uint32 tile_height;
    int type;
    int gg_sample_format;
    int epsg = -1;
    double cx;
    double cy;
    double upper_left_x;
    double upper_left_y;
    double upper_right_x;
    double lower_left_y;
    double pixel_x;
    double pixel_y;
    char srs_name[1024];
    char proj4text[1024];
    TIFF *in = (TIFF *) 0;
    GTIF *gtif = (GTIF *) 0;
    GTIFDefn definition;
    int max_palette = 0;
    unsigned char red[256];
    unsigned char green[256];
    unsigned char blue[256];
    int i;
    *infos_handle = NULL;

/* suppressing TIFF warnings */
    TIFFSetWarningHandler (NULL);

/* reading from file */
    in = XTIFFOpen (path, "r");
    if (in == NULL)
	return GGRAPH_GEOTIFF_CODEC_ERROR;
    gtif = GTIFNew (in);
    if (gtif == NULL)
	goto error;

    if (!GTIFGetDefn (gtif, &definition))
	goto error;
/* retrieving the EPSG code */
    if (definition.PCS == 32767)
      {
	  if (definition.GCS != 32767)
	      epsg = definition.GCS;
      }
    else
	epsg = definition.PCS;
    *srs_name = '\0';
    if (definition.PCS == 32767)
      {
	  /* Get the GCS name if possible */
	  char *pszName = NULL;
	  GTIFGetGCSInfo (definition.GCS, &pszName, NULL, NULL, NULL);
	  if (pszName != NULL)
	      strcpy (srs_name, pszName);
	  CPLFree (pszName);
      }
    else
      {
	  /* Get the PCS name if possible */
	  char *pszPCSName = NULL;
	  GTIFGetPCSInfo (definition.PCS, &pszPCSName, NULL, NULL, NULL);
	  if (pszPCSName != NULL)
	      strcpy (srs_name, pszPCSName);
	  CPLFree (pszPCSName);
      }
/* retrieving the PROJ.4 params */
    strcpy (proj4text, GTIFGetProj4Defn (&definition));

    is_tiled = TIFFIsTiled (in);
/* retrieving the TIFF dimensions */
    TIFFGetField (in, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField (in, TIFFTAG_IMAGEWIDTH, &width);
    if (is_tiled)
      {
	  TIFFGetField (in, TIFFTAG_TILEWIDTH, &tile_width);
	  TIFFGetField (in, TIFFTAG_TILELENGTH, &tile_height);
      }
    else
	TIFFGetField (in, TIFFTAG_ROWSPERSTRIP, &rows_strip);
    TIFFGetField (in, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    if (TIFFGetField (in, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  samples_per_pixel = 1;
      }
    TIFFGetField (in, TIFFTAG_PHOTOMETRIC, &photometric);
    TIFFGetField (in, TIFFTAG_COMPRESSION, &compression);
    if (TIFFGetField (in, TIFFTAG_PLANARCONFIG, &planar_config) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  planar_config = PLANARCONFIG_CONTIG;
      }
    if (TIFFGetField (in, TIFFTAG_SAMPLEFORMAT, &sample_format) == 0)
	sample_format = SAMPLEFORMAT_UINT;
    if (planar_config == PLANARCONFIG_CONTIG)
      {
	  if (sample_format == SAMPLEFORMAT_UINT)
	    {
		if (bits_per_sample == 1 && samples_per_pixel == 1
		    && photometric < 2)
		  {
		      type = GG_PIXEL_PALETTE;
		      max_palette = 2;
		      if (photometric == PHOTOMETRIC_MINISWHITE)
			{
			    red[0] = 255;
			    green[0] = 255;
			    blue[0] = 255;
			    red[1] = 0;
			    green[1] = 0;
			    blue[1] = 0;
			}
		      else
			{
			    red[0] = 0;
			    green[0] = 0;
			    blue[0] = 0;
			    red[1] = 255;
			    green[1] = 255;
			    blue[1] = 255;
			}
		  }
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric == 3)
		  {
		      uint16 *plt_red;
		      uint16 *plt_green;
		      uint16 *plt_blue;
		      type = GG_PIXEL_PALETTE;
		      TIFFGetField (in, TIFFTAG_COLORMAP, &plt_red, &plt_green,
				    &plt_blue);
		      max_palette = 256;
		      for (i = 0; i < max_palette; i++)
			{
			    if (plt_red[i] < 256)
				red[i] = plt_red[i];
			    else
				red[i] = plt_red[i] / 256;
			    if (plt_green[i] < 256)
				green[i] = plt_green[i];
			    else
				green[i] = plt_green[i] / 256;
			    if (plt_blue[i] < 256)
				blue[i] = plt_blue[i];
			    else
				blue[i] = plt_blue[i] / 256;
			}
		  }
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric < 2)
		    type = GG_PIXEL_GRAYSCALE;
		else if (bits_per_sample == 8 && samples_per_pixel == 3)
		    type = GG_PIXEL_RGB;
		else
		    type = GG_PIXEL_UNKNOWN;
	    }
	  else if (samples_per_pixel == 1)
	      type = GG_PIXEL_GRID;
	  else
	      type = GG_PIXEL_UNKNOWN;
      }
    else
	type = GG_PIXEL_UNKNOWN;
    switch (sample_format)
      {
      case SAMPLEFORMAT_UINT:
	  gg_sample_format = GGRAPH_SAMPLE_UINT;
	  break;
      case SAMPLEFORMAT_INT:
	  gg_sample_format = GGRAPH_SAMPLE_INT;
	  break;
      case SAMPLEFORMAT_IEEEFP:
	  gg_sample_format = GGRAPH_SAMPLE_FLOAT;
	  break;
      default:
	  gg_sample_format = GGRAPH_SAMPLE_UNKNOWN;
	  break;
      };

/* computing the corners coords */
    cx = 0.0;
    cy = 0.0;
    GTIFImageToPCS (gtif, &cx, &cy);
    upper_left_x = cx;
    upper_left_y = cy;
    cx = 0.0;
    cy = height;
    GTIFImageToPCS (gtif, &cx, &cy);
    lower_left_y = cy;
    cx = width;
    cy = 0.0;
    GTIFImageToPCS (gtif, &cx, &cy);
    upper_right_x = cx;
    cx = width;
    cy = height;
    GTIFImageToPCS (gtif, &cx, &cy);
/* computing the pixel size */
    pixel_x = (upper_right_x - upper_left_x) / (double) width;
    pixel_y = (upper_left_y - lower_left_y) / (double) height;

    infos =
	gg_image_infos_create (type, width, height, bits_per_sample,
			       samples_per_pixel, gg_sample_format, srs_name,
			       proj4text);
    if (!infos)
	return GGRAPH_INSUFFICIENT_MEMORY;
    if (is_tiled)
      {
	  infos->tile_width = tile_width;
	  infos->tile_height = tile_height;
      }
    else
	infos->rows_per_strip = rows_strip;
    switch (compression)
      {
      case COMPRESSION_NONE:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_NONE;
	  break;
      case COMPRESSION_LZW:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_LZW;
	  break;
      case COMPRESSION_DEFLATE:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_DEFLATE;
	  break;
      case COMPRESSION_JPEG:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_JPEG;
	  break;
      case COMPRESSION_CCITTFAX3:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX3;
	  break;
      case COMPRESSION_CCITTFAX4:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX4;
	  break;
      default:
	  infos->compression = GGRAPH_TIFF_COMPRESSION_UNKNOWN;
	  break;
      };
    for (i = 0; i < max_palette; i++)
      {
	  infos->palette_red[i] = red[i];
	  infos->palette_green[i] = green[i];
	  infos->palette_blue[i] = blue[i];
      }
    infos->max_palette = max_palette;
    infos->is_georeferenced = 1;
    infos->srid = epsg;
    infos->upper_left_x = upper_left_x;
    infos->upper_left_y = upper_left_y;
    infos->pixel_x_size = pixel_x;
    infos->pixel_y_size = pixel_y;
    TIFFClose (in);
    GTIFFree (gtif);
    *infos_handle = infos;
    return GGRAPH_OK;

  error:
    XTIFFClose (in);
    if (gtif)
	GTIFFree (gtif);
    return GGRAPH_GEOTIFF_CODEC_ERROR;
}

static int
common_read_from_tiff_grid (TIFF * in, gGraphImagePtr img, uint32 width,
			    uint32 height, int is_tiled, int gg_sample_format,
			    uint16 bits_per_sample, uint32 tile_width,
			    uint32 tile_height, uint32 rows_strip)
{
/* common utility: decoding a TIFF raster [GRID] */
    unsigned char *raster = NULL;
    int x;
    int y;
    int i;
    int img_y;
    int strip_no;
    int effective_strip;
    int tile_x;
    int tile_y;
    unsigned char *p_in;
    unsigned char *p_out;
    int ret = GGRAPH_TIFF_CODEC_ERROR;

    if (is_tiled)
      {
	  if (gg_sample_format == GGRAPH_SAMPLE_FLOAT)
	    {
		if (bits_per_sample == 32)
		  {
		      /* float */
		      raster = malloc (4 * tile_width * tile_height);
		  }
		else
		  {
		      /* double */
		      raster = malloc (8 * tile_width * tile_height);
		  }
	    }
	  else
	    {
		if (bits_per_sample == 8)
		  {
		      /* 8bit Int */
		      raster = malloc (tile_width * tile_height);
		  }
		else if (bits_per_sample == 16)
		  {
		      /* 16bit Int */
		      raster = malloc (2 * tile_width * tile_height);
		  }
		else
		  {
		      /* 32bit Int */
		      raster = malloc (4 * tile_width * tile_height);
		  }
	    }
      }
    else
      {
	  if (gg_sample_format == GGRAPH_SAMPLE_FLOAT)
	    {
		if (bits_per_sample == 32)
		  {
		      /* float */
		      raster = malloc (4 * width * rows_strip);
		  }
		else
		  {
		      /* double */
		      raster = malloc (8 * width * rows_strip);
		  }
	    }
	  else
	    {
		if (bits_per_sample == 8)
		  {
		      /* 8bit Int */
		      raster = malloc (width * rows_strip);
		  }
		else if (bits_per_sample == 16)
		  {
		      /* 16bit Int */
		      raster = malloc (2 * width * rows_strip);
		  }
		else
		  {
		      /* 32bit Int */
		      raster = malloc (4 * width * rows_strip);
		  }
	    }
      }

    ret = GGRAPH_TIFF_CODEC_ERROR;
    if (is_tiled)
      {
	  for (tile_y = 0; tile_y < (int) height; tile_y += tile_height)
	    {
		/* scanning tiles by row */
		for (tile_x = 0; tile_x < (int) width; tile_x += tile_width)
		  {
		      /* reading a TIFF tile */
		      if (!TIFFReadTile (in, raster, tile_x, tile_y, 0, 0))
			  goto error;
		      for (y = 0; y < (int) tile_height; y++)
			{
			    img_y = tile_y + y;
			    if (img_y >= (int) height)
				continue;
			    p_in = raster + (tile_width * y * img->pixel_size);
			    p_out =
				img->pixels + (img_y * img->scanline_width) +
				(tile_x * img->pixel_size);
			    for (x = 0; x < (int) tile_width; x++)
			      {
				  if (tile_x + x >= (int) width)
				      break;
				  for (i = 0; i < img->pixel_size; i++)
				      *p_out++ = *p_in++;
			      }
			}
		  }
	    }
      }
    else
      {
	  for (strip_no = 0; strip_no < (int) height; strip_no += rows_strip)
	    {
		/* reading a TIFF strip */
		if (!TIFFReadScanline (in, raster, strip_no, 0))
		    goto error;
		effective_strip = rows_strip;
		if ((strip_no + rows_strip) > height)
		    effective_strip = height - strip_no;

		for (y = 0; y < effective_strip; y++)
		  {
		      img_y = strip_no + y;
		      p_in = raster;
		      p_out = img->pixels + (img_y * img->scanline_width);
		      for (x = 0; x < (int) width; x++)
			{
			    for (i = 0; i < img->pixel_size; i++)
				*p_out++ = *p_in++;
			}
		  }
	    }
      }
    free (raster);
    return GGRAPH_OK;

  error:
    if (raster)
	free (raster);
    return ret;
}

static int
common_read_from_tiff (TIFF * in, gGraphImagePtr img, uint32 width,
		       uint32 height, int is_tiled, int type,
		       int gg_sample_format, uint16 bits_per_sample,
		       uint32 tile_width, uint32 tile_height, uint32 rows_strip)
{
/* common utility: decoding a TIFF raster */
    uint32 *raster = NULL;
    uint32 *scanline;
    int x;
    int y;
    int img_y;
    int strip_no;
    int effective_strip;
    uint32 pixel;
    int tile_x;
    int tile_y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char index;
    unsigned char *p_out;
    int ret = GGRAPH_TIFF_CODEC_ERROR;

    if (type == GG_PIXEL_GRID)
	return common_read_from_tiff_grid (in, img, width, height, is_tiled,
					   gg_sample_format, bits_per_sample,
					   tile_width, tile_height, rows_strip);

/* allocating read buffer [plain ordinary image] */
    if (is_tiled)
	raster = malloc (sizeof (uint32) * tile_width * tile_height);
    else
	raster = malloc (sizeof (uint32) * width * rows_strip);
    if (!raster)
      {
	  gg_image_destroy (img);
	  ret = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }

    ret = GGRAPH_TIFF_CODEC_ERROR;
    if (is_tiled)
      {
	  for (tile_y = 0; tile_y < (int) height; tile_y += tile_height)
	    {
		/* scanning tiles by row */
		for (tile_x = 0; tile_x < (int) width; tile_x += tile_width)
		  {
		      /* reading a TIFF tile */
		      if (!TIFFReadRGBATile (in, tile_x, tile_y, raster))
			  goto error;
		      for (y = 0; y < (int) tile_height; y++)
			{
			    img_y = tile_y + (tile_height - y) - 1;
			    if (img_y >= (int) height)
				continue;
			    scanline = raster + (tile_width * y);
			    for (x = 0; x < (int) tile_width; x++)
			      {
				  if (tile_x + x >= (int) width)
				      break;
				  p_out =
				      img->pixels +
				      (img_y * img->scanline_width) +
				      ((tile_x + x) * img->pixel_size);
				  pixel = scanline[x];
				  red = TIFFGetR (pixel);
				  green = TIFFGetG (pixel);
				  blue = TIFFGetB (pixel);
				  if (img->pixel_format == GG_PIXEL_PALETTE)
				    {
					/* PALETTE image */
					index =
					    gg_match_palette (img, red, green,
							      blue);
					*p_out++ = index;
				    }
				  else if (img->pixel_format ==
					   GG_PIXEL_GRAYSCALE)
				    {
					/* GRAYSCALE  image */
					*p_out++ = red;
				    }
				  else
				    {
					/* should be an RGB image */
					*p_out++ = red;
					*p_out++ = green;
					*p_out++ = blue;
				    }
			      }
			}
		  }
	    }
      }
    else
      {
	  for (strip_no = 0; strip_no < (int) height; strip_no += rows_strip)
	    {
		/* reading a TIFF strip */
		if (!TIFFReadRGBAStrip (in, strip_no, raster))
		    goto error;
		effective_strip = rows_strip;
		if ((strip_no + rows_strip) > height)
		    effective_strip = height - strip_no;

		for (y = 0; y < effective_strip; y++)
		  {
		      img_y = strip_no + ((effective_strip - y) - 1);
		      scanline = raster + (width * y);
		      p_out = img->pixels + (img_y * img->scanline_width);
		      for (x = 0; x < (int) width; x++)
			{
			    pixel = scanline[x];
			    red = TIFFGetR (pixel);
			    green = TIFFGetG (pixel);
			    blue = TIFFGetB (pixel);
			    if (img->pixel_format == GG_PIXEL_PALETTE)
			      {
				  /* PALETTE image */
				  index =
				      gg_match_palette (img, red, green, blue);
				  *p_out++ = index;
			      }
			    else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
			      {
				  /* GRAYSCALE  image */
				  *p_out++ = red;
			      }
			    else
			      {
				  /* should be an RGB image */
				  *p_out++ = red;
				  *p_out++ = green;
				  *p_out++ = blue;
			      }
			}
		  }
	    }
      }
    free (raster);
    return GGRAPH_OK;

  error:
    if (raster)
	free (raster);
    return ret;
}

GGRAPH_PRIVATE int
gg_image_from_mem_tiff (int size, const void *data,
			gGraphImagePtr * image_handle)
{
/* decoding a TIFF (memory cached) */
    gGraphImagePtr img = NULL;
    uint16 bits_per_sample;
    uint16 samples_per_pixel;
    uint16 photometric;
    uint16 compression;
    uint16 planar_config;
    uint16 sample_format;
    uint32 width = 0;
    uint32 height = 0;
    uint32 rows_strip = 0;
    struct memfile clientdata;
    int is_tiled;
    uint32 tile_width;
    uint32 tile_height;
    int type;
    int gg_sample_format;
    int ret = GGRAPH_TIFF_CODEC_ERROR;
    TIFF *in = (TIFF *) 0;
    *image_handle = NULL;

/* suppressing TIFF warnings */
    TIFFSetWarningHandler (NULL);

/* reading from memory */
    clientdata.buffer = (unsigned char *) data;
    clientdata.size = size;
    clientdata.eof = size;
    clientdata.current = 0;
    in = TIFFClientOpen ("tiff", "r", &clientdata, memory_readproc,
			 memory_writeproc, memory_seekproc, closeproc,
			 memory_sizeproc, mapproc, unmapproc);
    if (in == NULL)
	return GGRAPH_TIFF_CODEC_ERROR;
    is_tiled = TIFFIsTiled (in);
/* retrieving the TIFF dimensions */
    TIFFGetField (in, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField (in, TIFFTAG_IMAGEWIDTH, &width);
    if (is_tiled)
      {
	  TIFFGetField (in, TIFFTAG_TILEWIDTH, &tile_width);
	  TIFFGetField (in, TIFFTAG_TILELENGTH, &tile_height);
      }
    else
	TIFFGetField (in, TIFFTAG_ROWSPERSTRIP, &rows_strip);
    TIFFGetField (in, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    if (TIFFGetField (in, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  samples_per_pixel = 1;
      }
    TIFFGetField (in, TIFFTAG_PHOTOMETRIC, &photometric);
    TIFFGetField (in, TIFFTAG_COMPRESSION, &compression);
    if (TIFFGetField (in, TIFFTAG_PLANARCONFIG, &planar_config) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  planar_config = PLANARCONFIG_CONTIG;
      }
    if (TIFFGetField (in, TIFFTAG_SAMPLEFORMAT, &sample_format) == 0)
	sample_format = SAMPLEFORMAT_UINT;
    if (planar_config == PLANARCONFIG_CONTIG)
      {
	  if (sample_format == SAMPLEFORMAT_UINT)
	    {
		if (bits_per_sample == 1 && samples_per_pixel == 1)
		    type = GG_PIXEL_PALETTE;
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric == 3)
		    type = GG_PIXEL_PALETTE;
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric < 2)
		    type = GG_PIXEL_GRAYSCALE;
		else if (bits_per_sample == 8 && samples_per_pixel == 3)
		    type = GG_PIXEL_RGB;
		else
		    type = GG_PIXEL_UNKNOWN;
	    }
	  else if (samples_per_pixel == 1)
	      type = GG_PIXEL_GRID;
	  else
	      type = GG_PIXEL_UNKNOWN;
      }
    else
	type = GG_PIXEL_UNKNOWN;
    switch (sample_format)
      {
      case SAMPLEFORMAT_UINT:
	  gg_sample_format = GGRAPH_SAMPLE_UINT;
	  break;
      case SAMPLEFORMAT_INT:
	  gg_sample_format = GGRAPH_SAMPLE_INT;
	  break;
      case SAMPLEFORMAT_IEEEFP:
	  gg_sample_format = GGRAPH_SAMPLE_FLOAT;
	  break;
      default:
	  gg_sample_format = GGRAPH_SAMPLE_UNKNOWN;
	  break;
      };
    if (type == GG_PIXEL_UNKNOWN)
      {
	  ret = GGRAPH_UNSUPPORTED_TIFF_LAYOUT;
	  goto error;
      }

    img =
	gg_image_create (type, width, height, bits_per_sample,
			 samples_per_pixel, gg_sample_format, NULL, NULL);
    if (!img)
      {
	  ret = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }
    if (is_tiled)
      {
	  img->tile_width = tile_width;
	  img->tile_height = tile_height;
      }
    else
	img->rows_per_strip = rows_strip;
    switch (compression)
      {
      case COMPRESSION_NONE:
	  img->compression = GGRAPH_TIFF_COMPRESSION_NONE;
	  break;
      case COMPRESSION_LZW:
	  img->compression = GGRAPH_TIFF_COMPRESSION_LZW;
	  break;
      case COMPRESSION_DEFLATE:
	  img->compression = GGRAPH_TIFF_COMPRESSION_DEFLATE;
	  break;
      case COMPRESSION_JPEG:
	  img->compression = GGRAPH_TIFF_COMPRESSION_JPEG;
	  break;
      case COMPRESSION_CCITTFAX3:
	  img->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX3;
	  break;
      case COMPRESSION_CCITTFAX4:
	  img->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX4;
	  break;
      default:
	  img->compression = GGRAPH_TIFF_COMPRESSION_UNKNOWN;
	  break;
      };

    ret =
	common_read_from_tiff (in, img, width, height, is_tiled, type,
			       gg_sample_format, bits_per_sample, tile_width,
			       tile_height, rows_strip);
    if (ret != GGRAPH_OK)
	goto error;

    TIFFClose (in);
    *image_handle = img;
    return GGRAPH_OK;

  error:
    TIFFClose (in);
    if (img)
	gGraphDestroyImage (img);
    return ret;
}

GGRAPH_PRIVATE int
gg_image_strip_prepare_from_tiff (const char *path,
				  gGraphStripImagePtr * image_handle)
{
/* preparing to decode a TIFF [by meta-strips] */
    gGraphStripImagePtr img = NULL;
    uint16 bits_per_sample;
    uint16 samples_per_pixel;
    uint16 photometric;
    uint16 compression;
    uint16 planar_config;
    uint16 sample_format;
    uint32 width = 0;
    uint32 height = 0;
    uint32 rows_strip = 0;
    int is_tiled;
    uint32 tile_width;
    uint32 tile_height;
    int type;
    int gg_sample_format;
    struct tiff_codec_data *tiff_codec = NULL;
    tsize_t buf_size;
    void *tiff_buffer = NULL;
    int ret = GGRAPH_TIFF_CODEC_ERROR;
    TIFF *in = (TIFF *) 0;
    *image_handle = NULL;

/* suppressing TIFF warnings */
    TIFFSetWarningHandler (NULL);

/* reading from file */
    in = TIFFOpen (path, "r");
    if (in == NULL)
	return GGRAPH_TIFF_CODEC_ERROR;
    is_tiled = TIFFIsTiled (in);
/* retrieving the TIFF dimensions */
    TIFFGetField (in, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField (in, TIFFTAG_IMAGEWIDTH, &width);
    if (is_tiled)
      {
	  TIFFGetField (in, TIFFTAG_TILEWIDTH, &tile_width);
	  TIFFGetField (in, TIFFTAG_TILELENGTH, &tile_height);
      }
    else
	TIFFGetField (in, TIFFTAG_ROWSPERSTRIP, &rows_strip);
    TIFFGetField (in, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    if (TIFFGetField (in, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  samples_per_pixel = 1;
      }
    TIFFGetField (in, TIFFTAG_PHOTOMETRIC, &photometric);
    TIFFGetField (in, TIFFTAG_COMPRESSION, &compression);
    if (TIFFGetField (in, TIFFTAG_PLANARCONFIG, &planar_config) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  planar_config = PLANARCONFIG_CONTIG;
      }
    if (TIFFGetField (in, TIFFTAG_SAMPLEFORMAT, &sample_format) == 0)
	sample_format = SAMPLEFORMAT_UINT;
    if (planar_config == PLANARCONFIG_CONTIG)
      {
	  if (sample_format == SAMPLEFORMAT_UINT)
	    {
		if (bits_per_sample == 1 && samples_per_pixel == 1)
		    type = GG_PIXEL_PALETTE;
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric == 3)
		    type = GG_PIXEL_PALETTE;
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric < 2)
		    type = GG_PIXEL_GRAYSCALE;
		else if (bits_per_sample == 8 && samples_per_pixel == 3)
		    type = GG_PIXEL_RGB;
		else
		    type = GG_PIXEL_UNKNOWN;
	    }
	  else if (samples_per_pixel == 1)
	      type = GG_PIXEL_GRID;
	  else
	      type = GG_PIXEL_UNKNOWN;
      }
    else
	type = GG_PIXEL_UNKNOWN;
    switch (sample_format)
      {
      case SAMPLEFORMAT_UINT:
	  gg_sample_format = GGRAPH_SAMPLE_UINT;
	  break;
      case SAMPLEFORMAT_INT:
	  gg_sample_format = GGRAPH_SAMPLE_INT;
	  break;
      case SAMPLEFORMAT_IEEEFP:
	  gg_sample_format = GGRAPH_SAMPLE_FLOAT;
	  break;
      default:
	  gg_sample_format = GGRAPH_SAMPLE_UNKNOWN;
	  break;
      };
    if (type == GG_PIXEL_UNKNOWN)
      {
	  ret = GGRAPH_UNSUPPORTED_TIFF_LAYOUT;
	  goto error;
      }

    img =
	gg_strip_image_create (NULL, GGRAPH_IMAGE_TIFF, type, width, height,
			       bits_per_sample, samples_per_pixel,
			       gg_sample_format, NULL, NULL);
    if (!img)
      {
	  ret = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }
    if (is_tiled)
      {
	  img->tile_width = tile_width;
	  img->tile_height = tile_height;
      }
    else
	img->rows_per_strip = rows_strip;
    switch (compression)
      {
      case COMPRESSION_NONE:
	  img->compression = GGRAPH_TIFF_COMPRESSION_NONE;
	  break;
      case COMPRESSION_LZW:
	  img->compression = GGRAPH_TIFF_COMPRESSION_LZW;
	  break;
      case COMPRESSION_DEFLATE:
	  img->compression = GGRAPH_TIFF_COMPRESSION_DEFLATE;
	  break;
      case COMPRESSION_JPEG:
	  img->compression = GGRAPH_TIFF_COMPRESSION_JPEG;
	  break;
      case COMPRESSION_CCITTFAX3:
	  img->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX3;
	  break;
      case COMPRESSION_CCITTFAX4:
	  img->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX4;
	  break;
      default:
	  img->compression = GGRAPH_TIFF_COMPRESSION_UNKNOWN;
	  break;
      };
    if (photometric == PHOTOMETRIC_PALETTE)
      {
	  /* populating the palette */
	  int i;
	  uint16 *red;
	  uint16 *green;
	  uint16 *blue;
	  TIFFGetField (in, TIFFTAG_COLORMAP, &red, &green, &blue);
	  switch (bits_per_sample)
	    {
	    case 1:
		img->max_palette = 2;
		break;
	    case 2:
		img->max_palette = 4;
		break;
	    case 3:
		img->max_palette = 8;
		break;
	    case 4:
		img->max_palette = 16;
		break;
	    case 5:
		img->max_palette = 32;
		break;
	    case 6:
		img->max_palette = 64;
		break;
	    case 7:
		img->max_palette = 128;
		break;
	    default:
		img->max_palette = 256;
		break;
	    };
	  for (i = 0; i < img->max_palette; i++)
	    {
		unsigned char r;
		unsigned char g;
		unsigned char b;
		if (red[i] < 256)
		    r = red[i];
		else
		    r = red[i] / 256;
		if (green[i] < 256)
		    g = green[i];
		else
		    g = green[i] / 256;
		if (blue[i] < 256)
		    b = blue[i];
		else
		    b = blue[i] / 256;
		img->palette_red[i] = r;
		img->palette_green[i] = g;
		img->palette_blue[i] = b;
	    }
      }
    if (bits_per_sample == 1 && img->max_palette == 0)
      {
	  /* populating a monochrome palette */
	  if (photometric == PHOTOMETRIC_MINISBLACK)
	    {
		img->palette_red[0] = 0;
		img->palette_green[0] = 0;
		img->palette_blue[0] = 0;
		img->palette_red[1] = 255;
		img->palette_green[1] = 255;
		img->palette_blue[1] = 255;
		img->max_palette = 2;
	    }
	  else
	    {
		img->palette_red[0] = 255;
		img->palette_green[0] = 255;
		img->palette_blue[0] = 255;
		img->palette_red[1] = 0;
		img->palette_green[1] = 0;
		img->palette_blue[1] = 0;
		img->max_palette = 2;
	    }
      }

/* setting up the TIFF codec struct */
    tiff_codec = malloc (sizeof (struct tiff_codec_data));
    if (!tiff_codec)
	goto error;
    tiff_codec->is_geotiff = 0;
    tiff_codec->is_writer = 0;
    tiff_codec->tiff_handle = in;
    tiff_codec->geotiff_handle = (GTIF *) 0;
    tiff_codec->tiff_buffer = NULL;
    tiff_codec->is_tiled = is_tiled;
    img->codec_data = tiff_codec;

/* allocating the TIFF read buffer */
    if (type == GG_PIXEL_GRID)
      {
	  if (is_tiled)
	      buf_size = TIFFTileSize (in);
	  else
	      buf_size = TIFFScanlineSize (in);
      }
    else
      {
	  if (is_tiled)
	      buf_size = sizeof (uint32) * tile_width * tile_height;
	  else
	      buf_size = sizeof (uint32) * width * rows_strip;
      }
    tiff_buffer = malloc (buf_size);
    if (!tiff_buffer)
	goto error;
    tiff_codec->tiff_buffer = tiff_buffer;

    *image_handle = img;
    return GGRAPH_OK;

  error:
    TIFFClose (in);
    if (img)
	gGraphDestroyImage (img);
    return ret;
}

GGRAPH_PRIVATE int
gg_image_strip_prepare_from_geotiff (const char *path,
				     gGraphStripImagePtr * image_handle)
{
/* preparing to decode a GeoTIFF [by meta-strips] */
    gGraphStripImagePtr img = NULL;
    uint16 bits_per_sample;
    uint16 samples_per_pixel;
    uint16 photometric;
    uint16 compression;
    uint16 planar_config;
    uint16 sample_format;
    uint32 width = 0;
    uint32 height = 0;
    uint32 rows_strip = 0;
    int is_tiled;
    uint32 tile_width;
    uint32 tile_height;
    int type;
    int gg_sample_format;
    int epsg = -1;
    double cx;
    double cy;
    double upper_left_x;
    double upper_left_y;
    double upper_right_x;
    double lower_left_y;
    double pixel_x;
    double pixel_y;
    char srs_name[1024];
    char proj4text[1024];
    struct tiff_codec_data *tiff_codec = NULL;
    tsize_t buf_size;
    void *tiff_buffer = NULL;
    int ret = GGRAPH_TIFF_CODEC_ERROR;
    TIFF *in = (TIFF *) 0;
    GTIF *gtif = (GTIF *) 0;
    GTIFDefn definition;
    *image_handle = NULL;

/* suppressing TIFF warnings */
    TIFFSetWarningHandler (NULL);

/* reading from file */
    in = XTIFFOpen (path, "r");
    if (in == NULL)
	return GGRAPH_GEOTIFF_CODEC_ERROR;
    gtif = GTIFNew (in);
    if (gtif == NULL)
	goto error;

    if (!GTIFGetDefn (gtif, &definition))
	goto error;

/* retrieving the EPSG code */
    if (definition.PCS == 32767)
      {
	  if (definition.GCS != 32767)
	      epsg = definition.GCS;
      }
    else
	epsg = definition.PCS;
    *srs_name = '\0';
    if (definition.PCS == 32767)
      {
	  /* Get the GCS name if possible */
	  char *pszName = NULL;
	  GTIFGetGCSInfo (definition.GCS, &pszName, NULL, NULL, NULL);
	  if (pszName != NULL)
	      strcpy (srs_name, pszName);
	  CPLFree (pszName);
      }
    else
      {
	  /* Get the PCS name if possible */
	  char *pszPCSName = NULL;
	  GTIFGetPCSInfo (definition.PCS, &pszPCSName, NULL, NULL, NULL);
	  if (pszPCSName != NULL)
	      strcpy (srs_name, pszPCSName);
	  CPLFree (pszPCSName);
      }
/* retrieving the PROJ.4 params */
    strcpy (proj4text, GTIFGetProj4Defn (&definition));

    is_tiled = TIFFIsTiled (in);
/* retrieving the TIFF dimensions */
    TIFFGetField (in, TIFFTAG_IMAGELENGTH, &height);
    TIFFGetField (in, TIFFTAG_IMAGEWIDTH, &width);
    if (is_tiled)
      {
	  TIFFGetField (in, TIFFTAG_TILEWIDTH, &tile_width);
	  TIFFGetField (in, TIFFTAG_TILELENGTH, &tile_height);
      }
    else
	TIFFGetField (in, TIFFTAG_ROWSPERSTRIP, &rows_strip);
    TIFFGetField (in, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
    if (TIFFGetField (in, TIFFTAG_SAMPLESPERPIXEL, &samples_per_pixel) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  samples_per_pixel = 1;
      }
    TIFFGetField (in, TIFFTAG_PHOTOMETRIC, &photometric);
    TIFFGetField (in, TIFFTAG_COMPRESSION, &compression);
    if (TIFFGetField (in, TIFFTAG_PLANARCONFIG, &planar_config) == 0)
      {
	  /* attempting to recover badly formatted TIFFs */
	  planar_config = PLANARCONFIG_CONTIG;
      }
    if (TIFFGetField (in, TIFFTAG_SAMPLEFORMAT, &sample_format) == 0)
	sample_format = SAMPLEFORMAT_UINT;
    if (planar_config == PLANARCONFIG_CONTIG)
      {
	  if (sample_format == SAMPLEFORMAT_UINT)
	    {
		if (bits_per_sample == 1 && samples_per_pixel == 1)
		    type = GG_PIXEL_PALETTE;
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric == 3)
		    type = GG_PIXEL_PALETTE;
		else if (bits_per_sample == 8 && samples_per_pixel == 1
			 && photometric < 2)
		    type = GG_PIXEL_GRAYSCALE;
		else if (bits_per_sample == 8 && samples_per_pixel == 3)
		    type = GG_PIXEL_RGB;
		else
		    type = GG_PIXEL_UNKNOWN;
	    }
	  else if (samples_per_pixel == 1)
	      type = GG_PIXEL_GRID;
	  else
	      type = GG_PIXEL_UNKNOWN;
      }
    else
	type = GG_PIXEL_UNKNOWN;
    switch (sample_format)
      {
      case SAMPLEFORMAT_UINT:
	  gg_sample_format = GGRAPH_SAMPLE_UINT;
	  break;
      case SAMPLEFORMAT_INT:
	  gg_sample_format = GGRAPH_SAMPLE_INT;
	  break;
      case SAMPLEFORMAT_IEEEFP:
	  gg_sample_format = GGRAPH_SAMPLE_FLOAT;
	  break;
      default:
	  gg_sample_format = GGRAPH_SAMPLE_UNKNOWN;
	  break;
      };

/* computing the corners coords */
    cx = 0.0;
    cy = 0.0;
    GTIFImageToPCS (gtif, &cx, &cy);
    upper_left_x = cx;
    upper_left_y = cy;
    cx = 0.0;
    cy = height;
    GTIFImageToPCS (gtif, &cx, &cy);
    lower_left_y = cy;
    cx = width;
    cy = 0.0;
    GTIFImageToPCS (gtif, &cx, &cy);
    upper_right_x = cx;
    cx = width;
    cy = height;
    GTIFImageToPCS (gtif, &cx, &cy);
/* computing the pixel size */
    pixel_x = (upper_right_x - upper_left_x) / (double) width;
    pixel_y = (upper_left_y - lower_left_y) / (double) height;
    if (type == GG_PIXEL_UNKNOWN)
      {
	  ret = GGRAPH_UNSUPPORTED_TIFF_LAYOUT;
	  goto error;
      }

    img =
	gg_strip_image_create (NULL, GGRAPH_IMAGE_TIFF, type, width, height,
			       bits_per_sample, samples_per_pixel,
			       gg_sample_format, srs_name, proj4text);
    if (!img)
      {
	  ret = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }
    if (is_tiled)
      {
	  img->tile_width = tile_width;
	  img->tile_height = tile_height;
      }
    else
	img->rows_per_strip = rows_strip;
    img->is_georeferenced = 1;
    img->srid = epsg;
    img->upper_left_x = upper_left_x;
    img->upper_left_y = upper_left_y;
    img->pixel_x_size = pixel_x;
    img->pixel_y_size = pixel_y;
    switch (compression)
      {
      case COMPRESSION_NONE:
	  img->compression = GGRAPH_TIFF_COMPRESSION_NONE;
	  break;
      case COMPRESSION_LZW:
	  img->compression = GGRAPH_TIFF_COMPRESSION_LZW;
	  break;
      case COMPRESSION_DEFLATE:
	  img->compression = GGRAPH_TIFF_COMPRESSION_DEFLATE;
	  break;
      case COMPRESSION_JPEG:
	  img->compression = GGRAPH_TIFF_COMPRESSION_JPEG;
	  break;
      case COMPRESSION_CCITTFAX3:
	  img->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX3;
	  break;
      case COMPRESSION_CCITTFAX4:
	  img->compression = GGRAPH_TIFF_COMPRESSION_CCITTFAX4;
	  break;
      default:
	  img->compression = GGRAPH_TIFF_COMPRESSION_UNKNOWN;
	  break;
      };
    if (photometric == PHOTOMETRIC_PALETTE)
      {
	  /* populating the palette */
	  int i;
	  uint16 *red;
	  uint16 *green;
	  uint16 *blue;
	  TIFFGetField (in, TIFFTAG_COLORMAP, &red, &green, &blue);
	  switch (bits_per_sample)
	    {
	    case 2:
		img->max_palette = 4;
		break;
	    case 3:
		img->max_palette = 8;
		break;
	    case 4:
		img->max_palette = 16;
		break;
	    case 5:
		img->max_palette = 32;
		break;
	    case 6:
		img->max_palette = 64;
		break;
	    case 7:
		img->max_palette = 128;
		break;
	    default:
		img->max_palette = 256;
		break;
	    };
	  for (i = 0; i < img->max_palette; i++)
	    {
		unsigned char r;
		unsigned char g;
		unsigned char b;
		if (red[i] < 256)
		    r = red[i];
		else
		    r = red[i] / 256;
		if (green[i] < 256)
		    g = green[i];
		else
		    g = green[i] / 256;
		if (blue[i] < 256)
		    b = blue[i];
		else
		    b = blue[i] / 256;
		img->palette_red[i] = r;
		img->palette_green[i] = g;
		img->palette_blue[i] = b;
	    }
      }
    if (bits_per_sample == 1 && img->max_palette == 0)
      {
	  /* populating a monochrome palette */
	  if (photometric == PHOTOMETRIC_MINISBLACK)
	    {
		img->palette_red[0] = 0;
		img->palette_green[0] = 0;
		img->palette_blue[0] = 0;
		img->palette_red[1] = 255;
		img->palette_green[1] = 255;
		img->palette_blue[1] = 255;
		img->max_palette = 2;
	    }
	  else
	    {
		img->palette_red[0] = 255;
		img->palette_green[0] = 255;
		img->palette_blue[0] = 255;
		img->palette_red[1] = 0;
		img->palette_green[1] = 0;
		img->palette_blue[1] = 0;
		img->max_palette = 2;
	    }
      }

/* setting up the TIFF codec struct */
    tiff_codec = malloc (sizeof (struct tiff_codec_data));
    if (!tiff_codec)
	goto error;
    tiff_codec->is_geotiff = 1;
    tiff_codec->is_writer = 0;
    tiff_codec->tiff_handle = in;
    tiff_codec->geotiff_handle = gtif;
    tiff_codec->tiff_buffer = NULL;
    tiff_codec->is_tiled = is_tiled;
    img->codec_data = tiff_codec;

/* allocating the TIFF read buffer */
    if (type == GG_PIXEL_GRID)
      {
	  if (is_tiled)
	      buf_size = TIFFTileSize (in);
	  else
	      buf_size = TIFFScanlineSize (in);
      }
    else
      {
	  if (is_tiled)
	      buf_size = sizeof (uint32) * tile_width * tile_height;
	  else
	      buf_size = sizeof (uint32) * width * rows_strip;
      }
    tiff_buffer = malloc (buf_size);
    if (!tiff_buffer)
	goto error;
    tiff_codec->tiff_buffer = tiff_buffer;

    *image_handle = img;
    return GGRAPH_OK;

  error:
    XTIFFClose (in);
    if (gtif)
	GTIFFree (gtif);
    if (img)
	gGraphDestroyImage (img);
    return ret;
}

GGRAPH_PRIVATE void
gg_tiff_codec_destroy (void *p)
{
/* destroyng the TIFF codec data */
    struct tiff_codec_data *codec = (struct tiff_codec_data *) p;
    if (!codec)
	return;
    if (codec->is_geotiff)
      {
	  XTIFFClose (codec->tiff_handle);
	  GTIFFree (codec->geotiff_handle);
      }
    else
	TIFFClose (codec->tiff_handle);
    if (codec->tiff_buffer)
	free (codec->tiff_buffer);
    free (codec);
}

static int
common_strip_read_from_tiff_grid (TIFF * in, void *tiff_buffer,
				  gGraphStripImagePtr img, uint32 width,
				  uint32 height, int is_tiled,
				  uint32 tile_width, uint32 tile_height)
{
/* common utility: decoding a TIFF raster [GRID] by meta-strip */
    void *raster = tiff_buffer;
    int x;
    int y;
    int strip_no;
    int tile_x;
    int tile_y;
    char *p_in_int8;
    char *p_out_int8;
    unsigned char *p_in_uint8;
    unsigned char *p_out_uint8;
    short *p_in_int16;
    short *p_out_int16;
    unsigned short *p_in_uint16;
    unsigned short *p_out_uint16;
    int *p_in_int32;
    int *p_out_int32;
    unsigned int *p_in_uint32;
    unsigned int *p_out_uint32;
    float *p_in_flt;
    float *p_out_flt;
    double *p_in_dbl;
    double *p_out_dbl;
    int begin_row = img->next_row;
    int end_row = img->next_row + img->rows_per_block;

    if (end_row > img->height)
	end_row = img->height;

    if (is_tiled)
      {
	  for (tile_y = 0; tile_y < (int) height; tile_y += tile_height)
	    {
		/* scanning tiles by row */
		if ((tile_y + tile_height) <= (uint32) begin_row)
		    continue;
		if (tile_y >= end_row)
		    break;
		for (tile_x = 0; tile_x < (int) width; tile_x += tile_width)
		  {
		      /* reading a TIFF tile */
		      if (!TIFFReadTile (in, raster, tile_x, tile_y, 0, 0))
			  return GGRAPH_TIFF_CODEC_ERROR;
		      for (y = 0; y < (int) tile_height; y++)
			{
			    if ((tile_y + y) < begin_row)
				continue;
			    if ((tile_y + y) >= end_row)
				continue;
			    switch (img->sample_format)
			      {
			      case GGRAPH_SAMPLE_INT:
				  switch (img->bits_per_sample)
				    {
				    case 8:
					p_in_int8 = raster;
					p_in_int8 += y * tile_width;
					break;
				    case 16:
					p_in_int16 = raster;
					p_in_int16 += y * tile_width;
					break;
				    case 32:
					p_in_int32 = raster;
					p_in_int32 += y * tile_width;
					break;
				    };
				  break;
			      case GGRAPH_SAMPLE_UINT:
				  switch (img->bits_per_sample)
				    {
				    case 8:
					p_in_uint8 = raster;
					p_in_uint8 += y * tile_width;
					break;
				    case 16:
					p_in_uint16 = raster;
					p_in_uint16 += y * tile_width;
					break;
				    case 32:
					p_in_uint32 = raster;
					p_in_uint32 += y * tile_width;
					break;
				    };
				  break;
			      case GGRAPH_SAMPLE_FLOAT:
				  switch (img->bits_per_sample)
				    {
				    case 32:
					p_in_flt = raster;
					p_in_flt += y * tile_width;
					break;
				    case 64:
					p_in_dbl = raster;
					p_in_dbl += y * tile_width;
					break;
				    }
				  break;
			      };
			    for (x = 0; x < (int) tile_width; x++)
			      {
				  if (tile_x + x >= (int) width)
				      break;
				  switch (img->sample_format)
				    {
				    case GGRAPH_SAMPLE_INT:
					switch (img->bits_per_sample)
					  {
					  case 8:
					      p_out_int8 =
						  (char *) (img->pixels);
					      p_out_int8 +=
						  (img->width *
						   ((tile_y + y) - begin_row)) +
						  (tile_x + x);
					      *p_out_int8 = *p_in_int8++;
					      break;
					  case 16:
					      p_out_int16 =
						  (short *) (img->pixels);
					      p_out_int16 +=
						  (img->width *
						   ((tile_y + y) - begin_row)) +
						  (tile_x + x);
					      *p_out_int16 = *p_in_int16++;
					      break;
					  case 32:
					      p_out_int32 =
						  (int *) (img->pixels);
					      p_out_int32 +=
						  (img->width *
						   ((tile_y + y) - begin_row)) +
						  (tile_x + x);
					      *p_out_int32 = *p_in_int32++;
					      break;
					  };
					break;
				    case GGRAPH_SAMPLE_UINT:
					switch (img->bits_per_sample)
					  {
					  case 8:
					      p_out_uint8 =
						  (unsigned char
						   *) (img->pixels);
					      p_out_uint8 +=
						  (img->width *
						   ((tile_y + y) - begin_row)) +
						  (tile_x + x);
					      *p_out_uint8 = *p_in_uint8++;
					      break;
					  case 16:
					      p_out_uint16 =
						  (unsigned short
						   *) (img->pixels);
					      p_out_uint16 +=
						  (img->width *
						   ((tile_y + y) - begin_row)) +
						  (tile_x + x);
					      *p_out_uint16 = *p_in_uint16++;
					      break;
					  case 32:
					      p_out_uint32 =
						  (unsigned int
						   *) (img->pixels);
					      p_out_uint32 +=
						  (img->width *
						   ((tile_y + y) - begin_row)) +
						  (tile_x + x);
					      *p_out_uint32 = *p_in_uint32++;
					      break;
					  }
					break;
				    case GGRAPH_SAMPLE_FLOAT:
					switch (img->bits_per_sample)
					  {
					  case 32:
					      p_out_flt =
						  (float *) (img->pixels);
					      p_out_flt +=
						  (img->width *
						   ((tile_y + y) - begin_row)) +
						  (tile_x + x);
					      *p_out_flt = *p_in_flt++;
					      break;
					  case 64:
					      p_out_dbl =
						  (double *) (img->pixels);
					      p_out_dbl +=
						  (img->width *
						   ((tile_y + y) - begin_row)) +
						  (tile_x + x);
					      *p_out_dbl = *p_in_dbl++;
					      break;
					  };
					break;
				    };
			      }
			}
		  }
	    }
	  img->next_row += end_row - begin_row;
	  img->current_available_rows = end_row - begin_row;
      }
    else
      {
	  for (strip_no = begin_row; strip_no < end_row; strip_no++)
	    {
		/* reading a TIFF strip */
		if (!TIFFReadScanline (in, raster, strip_no, 0))
		    return GGRAPH_TIFF_CODEC_ERROR;
		switch (img->sample_format)
		  {
		  case GGRAPH_SAMPLE_INT:
		      switch (img->bits_per_sample)
			{
			case 8:
			    p_in_int8 = raster;
			    p_out_int8 = (char *) (img->pixels);
			    p_out_int8 += img->width * (strip_no - begin_row);
			    break;
			case 16:
			    p_in_int16 = raster;
			    p_out_int16 = (short *) (img->pixels);
			    p_out_int16 += img->width * (strip_no - begin_row);
			    break;
			case 32:
			    p_in_int32 = raster;
			    p_out_int32 = (int *) (img->pixels);
			    p_out_int32 += img->width * (strip_no - begin_row);
			    break;
			};
		      break;
		  case GGRAPH_SAMPLE_UINT:
		      switch (img->bits_per_sample)
			{
			case 8:
			    p_in_uint8 = raster;
			    p_out_uint8 = (unsigned char *) (img->pixels);
			    p_out_uint8 += img->width * (strip_no - begin_row);
			    break;
			case 16:
			    p_in_uint16 = raster;
			    p_out_uint16 = (unsigned short *) (img->pixels);
			    p_out_uint16 += img->width * (strip_no - begin_row);
			    break;
			case 32:
			    p_in_uint32 = raster;
			    p_out_uint32 = (unsigned int *) (img->pixels);
			    p_out_uint32 += img->width * (strip_no - begin_row);
			    break;
			};
		      break;
		  case GGRAPH_SAMPLE_FLOAT:
		      switch (img->bits_per_sample)
			{
			case 32:
			    p_in_flt = raster;
			    p_out_flt = (float *) (img->pixels);
			    p_out_flt += img->width * (strip_no - begin_row);
			    break;
			case 64:
			    p_in_dbl = raster;
			    p_out_dbl = (double *) (img->pixels);
			    p_out_dbl += img->width * (strip_no - begin_row);
			    break;
			}
		      break;
		  };
		for (x = 0; x < (int) width; x++)
		  {
		      switch (img->sample_format)
			{
			case GGRAPH_SAMPLE_INT:
			    switch (img->bits_per_sample)
			      {
			      case 8:
				  *p_out_int8++ = *p_in_int8++;
				  break;
			      case 16:
				  *p_out_int16++ = *p_in_int16++;
				  break;
			      case 32:
				  *p_out_int32++ = *p_in_int32++;
				  break;
			      };
			    break;
			case GGRAPH_SAMPLE_UINT:
			    switch (img->bits_per_sample)
			      {
			      case 8:
				  *p_out_uint8++ = *p_in_uint8++;
				  break;
			      case 16:
				  *p_out_uint16++ = *p_in_uint16++;
				  break;
			      case 32:
				  *p_out_uint32++ = *p_in_uint32++;
				  break;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    switch (img->bits_per_sample)
			      {
			      case 32:
				  *p_out_flt++ = *p_in_flt++;
				  break;
			      case 64:
				  *p_out_dbl++ = *p_in_dbl++;
				  break;
			      };
			    break;
			};
		  }
	    }
	  img->next_row += end_row - begin_row;
	  img->current_available_rows = end_row - begin_row;
      }
    return GGRAPH_OK;
}

GGRAPH_PRIVATE unsigned char
gg_match_strip_palette (const gGraphStripImagePtr img, unsigned char r,
			unsigned char g, unsigned char b)
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

static int
common_strip_read_from_tiff (TIFF * in, void *tiff_buffer,
			     gGraphStripImagePtr img, uint32 width,
			     uint32 height, int is_tiled, int type,
			     uint32 tile_width, uint32 tile_height,
			     uint32 rows_strip)
{
/* common utility: decoding a TIFF raster [by meta-strip] */
    uint32 *raster = tiff_buffer;
    uint32 *scanline;
    int x;
    int y;
    int img_y;
    int strip_no;
    uint32 pixel;
    int tile_x;
    int tile_y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char index;
    unsigned char *p_out;
    int begin_row = img->next_row;
    int end_row = img->next_row + img->rows_per_block;

    if (type == GG_PIXEL_GRID)
	return common_strip_read_from_tiff_grid (in, tiff_buffer, img, width,
						 height, is_tiled, tile_width,
						 tile_height);

    if (end_row > img->height)
	end_row = img->height;

    if (is_tiled)
      {
	  for (tile_y = 0; tile_y < (int) height; tile_y += tile_height)
	    {
		/* scanning tiles by row */
		if ((tile_y + tile_height) <= (uint32) begin_row)
		    continue;
		if (tile_y >= end_row)
		    break;
		for (tile_x = 0; tile_x < (int) width; tile_x += tile_width)
		  {
		      /* reading a TIFF tile */
		      if (!TIFFReadRGBATile (in, tile_x, tile_y, raster))
			  return GGRAPH_TIFF_CODEC_ERROR;
		      for (y = 0; y < (int) tile_height; y++)
			{
			    img_y = tile_y + (tile_height - y) - 1;
			    if (img_y < begin_row)
				continue;
			    if (img_y >= end_row)
				continue;
			    scanline = raster + (y * tile_width);
			    for (x = 0; x < (int) tile_width; x++)
			      {
				  if (tile_x + x >= (int) width)
				      break;
				  p_out =
				      img->pixels +
				      (img->scanline_width *
				       (img_y - begin_row));
				  p_out += (tile_x + x) * img->pixel_size;
				  pixel = scanline[x];
				  red = TIFFGetR (pixel);
				  green = TIFFGetG (pixel);
				  blue = TIFFGetB (pixel);
				  if (img->pixel_format == GG_PIXEL_PALETTE)
				    {
					/* PALETTE image */
					index =
					    gg_match_strip_palette (img, red,
								    green,
								    blue);
					*p_out++ = index;
				    }
				  else if (img->pixel_format ==
					   GG_PIXEL_GRAYSCALE)
				    {
					/* GRAYSCALE  image */
					*p_out++ = red;
				    }
				  else
				    {
					/* should be an RGB image */
					*p_out++ = red;
					*p_out++ = green;
					*p_out++ = blue;
				    }
			      }
			}
		  }
	    }
	  img->next_row += end_row - begin_row;
	  img->current_available_rows = end_row - begin_row;
      }
    else
      {
	  for (strip_no = 0; strip_no < (int) height; strip_no += rows_strip)
	    {
		/* reading a TIFF strip */
		int effective_strip = rows_strip;
		if ((strip_no + rows_strip) <= (uint32) begin_row)
		    continue;
		if (strip_no >= end_row)
		    break;
		if (!TIFFReadRGBAStrip (in, strip_no, raster))
		    return GGRAPH_TIFF_CODEC_ERROR;
		if ((strip_no + rows_strip) > height)
		    effective_strip = height - strip_no;
		for (y = 0; y < (int) effective_strip; y++)
		  {
		      img_y = (strip_no + effective_strip) - (y + 1);
		      if (img_y < begin_row)
			  continue;
		      if (img_y >= end_row)
			  continue;
		      scanline = raster + (y * width);
		      p_out =
			  img->pixels +
			  (img->scanline_width * (img_y - begin_row));
		      for (x = 0; x < (int) width; x++)
			{
			    pixel = scanline[x];
			    red = TIFFGetR (pixel);
			    green = TIFFGetG (pixel);
			    blue = TIFFGetB (pixel);
			    if (img->pixel_format == GG_PIXEL_PALETTE)
			      {
				  /* PALETTE image */
				  index =
				      gg_match_strip_palette (img, red, green,
							      blue);
				  *p_out++ = index;
			      }
			    else if (img->pixel_format == GG_PIXEL_GRAYSCALE)
			      {
				  /* GRAYSCALE  image */
				  *p_out++ = red;
			      }
			    else
			      {
				  /* should be an RGB image */
				  *p_out++ = red;
				  *p_out++ = green;
				  *p_out++ = blue;
			      }
			}
		  }
	    }
	  img->next_row += end_row - begin_row;
	  img->current_available_rows = end_row - begin_row;
      }
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_strip_read_from_tiff (gGraphStripImagePtr img, int *progress)
{
/* decoding a TIFF [by meta-strip] */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    uint32 width = img->width;
    uint32 height = img->height;
    uint32 rows_strip = img->rows_per_strip;
    int is_tiled = tiff_codec->is_tiled;
    uint32 tile_width = img->tile_width;
    uint32 tile_height = img->tile_height;
    int type = img->pixel_format;
    TIFF *in = tiff_codec->tiff_handle;

    int ret =
	common_strip_read_from_tiff (in, tiff_codec->tiff_buffer, img, width,
				     height, is_tiled, type, tile_width,
				     tile_height, rows_strip);

    if (ret == GGRAPH_OK && progress != NULL)
	*progress =
	    (int) (((double) (img->next_row + 1) * 100.0) /
		   (double) (img->height));
    return ret;
}

GGRAPH_PRIVATE int
gg_image_prepare_to_tiff_by_strip (const gGraphStripImagePtr img,
				   const char *path, int layout, int tile_width,
				   int tile_height, int rows_per_strip,
				   int color_model, int bits_per_sample,
				   int sample_format, int num_palette,
				   unsigned char *red, unsigned char *green,
				   unsigned char *blue, int compression)
{
/* preparing to export a TIFF [by meta-strips] */
    TIFF *out = (TIFF *) 0;
    uint16 r_plt[256];
    uint16 g_plt[256];
    uint16 b_plt[256];
    int i;
    int tiff_type;
    tsize_t buf_size;
    void *tiff_buffer = NULL;
    struct tiff_codec_data *tiff_codec = NULL;

/* suppressing TIFF warnings */
    TIFFSetWarningHandler (NULL);

/* writing to file */
    out = TIFFOpen (path, "w");
    if (out == NULL)
	goto error;

/* setting up the TIFF headers */
    TIFFSetField (out, TIFFTAG_SUBFILETYPE, 0);
    TIFFSetField (out, TIFFTAG_IMAGEWIDTH, img->width);
    TIFFSetField (out, TIFFTAG_IMAGELENGTH, img->height);
    TIFFSetField (out, TIFFTAG_XRESOLUTION, 300.0);
    TIFFSetField (out, TIFFTAG_YRESOLUTION, 300.0);
    TIFFSetField (out, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    TIFFSetField (out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField (out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    if (color_model == GGRAPH_COLORSPACE_PALETTE)
      {
	  if (num_palette == 2)
	    {
		if ((red[0] == 255 && green[0] == 255 && blue[0] == 255
		     && red[1] == 0 && green[1] == 0 && blue[1] == 0)
		    || (red[0] == 0 && green[0] == 0 && blue[0] == 0
			&& red[1] == 255 && green[1] == 255 && blue[1] == 255))
		  {
		      /* MONOCHROME */
		      tiff_type = TIFF_TYPE_MONOCHROME;
		      TIFFSetField (out, TIFFTAG_SAMPLEFORMAT,
				    SAMPLEFORMAT_UINT);
		      TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, 1);
		      TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, 1);
		      TIFFSetField (out, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
		      TIFFSetField (out, TIFFTAG_PHOTOMETRIC,
				    PHOTOMETRIC_MINISWHITE);
		      if (compression == GGRAPH_TIFF_COMPRESSION_CCITTFAX3)
			  TIFFSetField (out, TIFFTAG_COMPRESSION,
					COMPRESSION_CCITTFAX3);
		      else if (compression == GGRAPH_TIFF_COMPRESSION_CCITTFAX4)
			  TIFFSetField (out, TIFFTAG_COMPRESSION,
					COMPRESSION_CCITTFAX4);
		      else
			  TIFFSetField (out, TIFFTAG_COMPRESSION,
					COMPRESSION_NONE);
		      goto header_done;
		  }
	    }
	  /* PALETTE */
	  tiff_type = TIFF_TYPE_PALETTE;
	  for (i = 0; i < img->max_palette; i++)
	    {
		r_plt[i] = img->palette_red[i] * 256;
		g_plt[i] = img->palette_green[i] * 256;
		b_plt[i] = img->palette_blue[i] * 256;
	    }
	  TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	  TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, 1);
	  TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, 8);
	  TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
	  TIFFSetField (out, TIFFTAG_COLORMAP, r_plt, g_plt, b_plt);
	  if (compression == GGRAPH_TIFF_COMPRESSION_LZW)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_DEFLATE)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	  else
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      }
    if (color_model == GGRAPH_COLORSPACE_GRAYSCALE)
      {
	  /* GRAYSCALE */
	  tiff_type = TIFF_TYPE_GRAYSCALE;
	  TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	  TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, 1);
	  TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, 8);
	  TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	  if (compression == GGRAPH_TIFF_COMPRESSION_LZW)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_DEFLATE)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_JPEG)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
	  else
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      }
    if (color_model == GGRAPH_COLORSPACE_TRUECOLOR
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR_ALPHA)
      {
	  /* RGB */
	  tiff_type = TIFF_TYPE_RGB;
	  TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	  TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, 3);
	  TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, 8);
	  TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	  if (compression == GGRAPH_TIFF_COMPRESSION_LZW)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_DEFLATE)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_JPEG)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
	  else
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      }
    if (color_model == GGRAPH_COLORSPACE_GRID)
      {
	  /* GRID data */

	  tiff_type = TIFF_TYPE_GRID;
	  if (sample_format == GGRAPH_SAMPLE_INT)
	      TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
	  if (sample_format == GGRAPH_SAMPLE_UINT)
	      TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	  if (sample_format == GGRAPH_SAMPLE_FLOAT)
	      TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
	  TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, 1);
	  TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
	  TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	  if (compression == GGRAPH_TIFF_COMPRESSION_LZW)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_DEFLATE)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	  else
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      }

  header_done:
    TIFFSetField (out, TIFFTAG_SOFTWARE, "GaiaGraphics-tools");
    if (layout == GGRAPH_TIFF_LAYOUT_TILES)
      {
	  TIFFSetField (out, TIFFTAG_TILEWIDTH, tile_width);
	  TIFFSetField (out, TIFFTAG_TILELENGTH, tile_height);
      }
    else
	TIFFSetField (out, TIFFTAG_ROWSPERSTRIP, rows_per_strip);

/* setting up the TIFF codec struct */
    tiff_codec = malloc (sizeof (struct tiff_codec_data));
    if (!tiff_codec)
	goto error;
    tiff_codec->is_geotiff = 0;
    tiff_codec->is_writer = 1;
    tiff_codec->tiff_handle = out;
    tiff_codec->geotiff_handle = (GTIF *) 0;
    tiff_codec->tiff_buffer = NULL;
    if (layout == GGRAPH_TIFF_LAYOUT_TILES)
	tiff_codec->is_tiled = 1;
    else
	tiff_codec->is_tiled = 0;
    tiff_codec->tiff_type = tiff_type;
    img->codec_data = tiff_codec;

/* allocating the TIFF write buffer */
    if (img->pixel_format == GG_PIXEL_GRID)
      {
	  if (tiff_codec->is_tiled)
	      buf_size = TIFFTileSize (out);
	  else
	      buf_size = TIFFScanlineSize (out);
      }
    else
      {
	  if (tiff_codec->is_tiled)
	      buf_size = TIFFTileSize (out);
	  else
	      buf_size = TIFFScanlineSize (out);
      }
    tiff_buffer = malloc (buf_size);
    if (!tiff_buffer)
	goto error;
    tiff_codec->tiff_buffer = tiff_buffer;

    return GGRAPH_OK;

  error:
    if (out)
	TIFFClose (out);
    return GGRAPH_TIFF_CODEC_ERROR;
}

static int
is_projected_srs (const char *proj4text)
{
/* checks if this one is a PCS SRS */
    if (proj4text == NULL)
	return 1;
    if (strlen (proj4text) > 14)
      {
	  if (strncmp (proj4text, "+proj=longlat ", 14) == 0)
	      return 0;
      }
    return 1;
}

GGRAPH_PRIVATE int
gg_image_prepare_to_geotiff_by_strip (const gGraphStripImagePtr img,
				      const char *path, int layout,
				      int tile_width, int tile_height,
				      int rows_per_strip, int color_model,
				      int bits_per_sample, int sample_format,
				      int num_palette, unsigned char *red,
				      unsigned char *green, unsigned char *blue,
				      int compression)
{
/* preparing to export a GeoTIFF [by meta-strips] */
    TIFF *out = (TIFF *) 0;
    GTIF *gtif = (GTIF *) 0;
    double tiepoint[6];
    double pixsize[3];
    uint16 r_plt[256];
    uint16 g_plt[256];
    uint16 b_plt[256];
    int i;
    int tiff_type;
    tsize_t buf_size;
    void *tiff_buffer = NULL;
    struct tiff_codec_data *tiff_codec = NULL;

/* suppressing TIFF warnings */
    TIFFSetWarningHandler (NULL);

/* writing to file */
    out = XTIFFOpen (path, "w");
    if (out == NULL)
	goto error;
    gtif = GTIFNew (out);
    if (!gtif)
	goto error;

/* setting up the TIFF headers */
    TIFFSetField (out, TIFFTAG_SUBFILETYPE, 0);
    TIFFSetField (out, TIFFTAG_IMAGEWIDTH, img->width);
    TIFFSetField (out, TIFFTAG_IMAGELENGTH, img->height);
    TIFFSetField (out, TIFFTAG_XRESOLUTION, 300.0);
    TIFFSetField (out, TIFFTAG_YRESOLUTION, 300.0);
    TIFFSetField (out, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    TIFFSetField (out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField (out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    if (color_model == GGRAPH_COLORSPACE_PALETTE)
      {
	  if (num_palette == 2)
	    {
		if ((red[0] == 255 && green[0] == 255 && blue[0] == 255
		     && red[1] == 0 && green[1] == 0 && blue[1] == 0)
		    || (red[0] == 0 && green[0] == 0 && blue[0] == 0
			&& red[1] == 255 && green[1] == 255 && blue[1] == 255))
		  {
		      /* MONOCHROME */
		      tiff_type = TIFF_TYPE_MONOCHROME;
		      TIFFSetField (out, TIFFTAG_SAMPLEFORMAT,
				    SAMPLEFORMAT_UINT);
		      TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, 1);
		      TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, 1);
		      TIFFSetField (out, TIFFTAG_FILLORDER, FILLORDER_MSB2LSB);
		      TIFFSetField (out, TIFFTAG_PHOTOMETRIC,
				    PHOTOMETRIC_MINISWHITE);
		      if (compression == GGRAPH_TIFF_COMPRESSION_CCITTFAX3)
			  TIFFSetField (out, TIFFTAG_COMPRESSION,
					COMPRESSION_CCITTFAX3);
		      else if (compression == GGRAPH_TIFF_COMPRESSION_CCITTFAX4)
			  TIFFSetField (out, TIFFTAG_COMPRESSION,
					COMPRESSION_CCITTFAX4);
		      else
			  TIFFSetField (out, TIFFTAG_COMPRESSION,
					COMPRESSION_NONE);
		      goto header_done;
		  }
	    }
	  /* PALETTE */
	  tiff_type = TIFF_TYPE_PALETTE;
	  for (i = 0; i < img->max_palette; i++)
	    {
		r_plt[i] = img->palette_red[i] * 256;
		g_plt[i] = img->palette_green[i] * 256;
		b_plt[i] = img->palette_blue[i] * 256;
	    }
	  TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	  TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, 1);
	  TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, 8);
	  TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE);
	  TIFFSetField (out, TIFFTAG_COLORMAP, r_plt, g_plt, b_plt);
	  if (compression == GGRAPH_TIFF_COMPRESSION_LZW)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_DEFLATE)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	  else
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      }
    if (color_model == GGRAPH_COLORSPACE_GRAYSCALE)
      {
	  /* GRAYSCALE */
	  tiff_type = TIFF_TYPE_GRAYSCALE;
	  TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	  TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, 1);
	  TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, 8);
	  TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	  if (compression == GGRAPH_TIFF_COMPRESSION_LZW)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_DEFLATE)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_JPEG)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
	  else
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      }
    if (color_model == GGRAPH_COLORSPACE_TRUECOLOR
	|| color_model == GGRAPH_COLORSPACE_TRUECOLOR_ALPHA)
      {
	  /* RGB */
	  tiff_type = TIFF_TYPE_RGB;
	  TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	  TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, 3);
	  TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, 8);
	  TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	  if (compression == GGRAPH_TIFF_COMPRESSION_LZW)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_DEFLATE)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_JPEG)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
	  else
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      }
    if (color_model == GGRAPH_COLORSPACE_GRID)
      {
	  /* GRID data */

	  tiff_type = TIFF_TYPE_GRID;
	  if (sample_format == GGRAPH_SAMPLE_INT)
	      TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_INT);
	  if (sample_format == GGRAPH_SAMPLE_UINT)
	      TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
	  if (sample_format == GGRAPH_SAMPLE_FLOAT)
	      TIFFSetField (out, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
	  TIFFSetField (out, TIFFTAG_SAMPLESPERPIXEL, 1);
	  TIFFSetField (out, TIFFTAG_BITSPERSAMPLE, bits_per_sample);
	  TIFFSetField (out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	  if (compression == GGRAPH_TIFF_COMPRESSION_LZW)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
	  else if (compression == GGRAPH_TIFF_COMPRESSION_DEFLATE)
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	  else
	      TIFFSetField (out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
      }

  header_done:
    TIFFSetField (out, TIFFTAG_SOFTWARE, "GaiaGraphics-tools");
    if (layout == GGRAPH_TIFF_LAYOUT_TILES)
      {
	  TIFFSetField (out, TIFFTAG_TILEWIDTH, tile_width);
	  TIFFSetField (out, TIFFTAG_TILELENGTH, tile_height);
      }
    else
	TIFFSetField (out, TIFFTAG_ROWSPERSTRIP, rows_per_strip);

/* setting up the GeoTIFF Tags */
    pixsize[0] = img->pixel_x_size;
    pixsize[1] = img->pixel_y_size;
    pixsize[2] = 0.0;
    TIFFSetField (out, GTIFF_PIXELSCALE, 3, pixsize);
    tiepoint[0] = 0.0;
    tiepoint[1] = 0.0;
    tiepoint[2] = 0.0;
    tiepoint[3] = img->upper_left_x;
    tiepoint[4] = img->upper_left_y;
    tiepoint[5] = 0.0;
    TIFFSetField (out, GTIFF_TIEPOINTS, 6, tiepoint);
    if (img->srs_name != NULL)
	TIFFSetField (out, GTIFF_ASCIIPARAMS, img->srs_name);
    if (img->proj4text != NULL)
	GTIFSetFromProj4 (gtif, img->proj4text);
    if (img->srs_name != NULL)
	GTIFKeySet (gtif, GTCitationGeoKey, TYPE_ASCII, 0, img->srs_name);
    if (is_projected_srs (img->proj4text))
	GTIFKeySet (gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, img->srid);
    GTIFWriteKeys (gtif);

/* setting up the TIFF codec struct */
    tiff_codec = malloc (sizeof (struct tiff_codec_data));
    if (!tiff_codec)
	goto error;
    tiff_codec->is_geotiff = 0;
    tiff_codec->is_writer = 1;
    tiff_codec->tiff_handle = out;
    tiff_codec->geotiff_handle = (GTIF *) 0;
    tiff_codec->tiff_buffer = NULL;
    if (layout == GGRAPH_TIFF_LAYOUT_TILES)
	tiff_codec->is_tiled = 1;
    else
	tiff_codec->is_tiled = 0;
    tiff_codec->tiff_type = tiff_type;
    img->codec_data = tiff_codec;

/* allocating the TIFF write buffer */
    if (img->pixel_format == GG_PIXEL_GRID)
      {
	  if (tiff_codec->is_tiled)
	      buf_size = TIFFTileSize (out);
	  else
	      buf_size = TIFFScanlineSize (out);
      }
    else
      {
	  if (tiff_codec->is_tiled)
	      buf_size = TIFFTileSize (out);
	  else
	      buf_size = TIFFScanlineSize (out);
      }
    tiff_buffer = malloc (buf_size);
    if (!tiff_buffer)
	goto error;
    tiff_codec->tiff_buffer = tiff_buffer;

    return GGRAPH_OK;

  error:
    if (gtif)
	GTIFFree (gtif);
    if (out)
	XTIFFClose (out);
    return GGRAPH_TIFF_CODEC_ERROR;
}

static int
tiff_write_tile_grid (const gGraphStripImagePtr img)
{
/* scanline(s) TIFF compression [tiles] GRID data */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    TIFF *out = tiff_codec->tiff_handle;
    int tile_width = img->tile_width;
    int tile_height = img->tile_height;
    int row;
    int col;
    void *tile = tiff_codec->tiff_buffer;
    char *p_in_int8;
    char *p_out_int8;
    unsigned char *p_in_uint8;
    unsigned char *p_out_uint8;
    short *p_in_int16;
    short *p_out_int16;
    unsigned short *p_in_uint16;
    unsigned short *p_out_uint16;
    int *p_in_int32;
    int *p_out_int32;
    unsigned int *p_in_uint32;
    unsigned int *p_out_uint32;
    float *p_in_flt;
    float *p_out_flt;
    double *p_in_dbl;
    double *p_out_dbl;
    uint32 tile_x;
    uint32 tile_y;
    int num_rows;
    int num_cols;

    tile_y = img->next_row;
    if (tile_y + tile_height > (uint32) (img->height))
	num_rows = img->height - tile_y;
    else
	num_rows = tile_height;
    if (num_rows != img->current_available_rows)
	return GGRAPH_TIFF_CODEC_ERROR;

    for (tile_x = 0; tile_x < (uint32) (img->width); tile_x += tile_width)
      {
	  /* feeding a tile */
	  if (tile_x + tile_width > (uint32) (img->width))
	      num_cols = img->width - tile_x;
	  else
	      num_cols = tile_width;
	  for (row = 0; row < num_rows; row++)
	    {
		switch (img->sample_format)
		  {
		  case GGRAPH_SAMPLE_UINT:
		      switch (img->bits_per_sample)
			{
			case 8:
			    p_out_uint8 = tile;
			    p_out_uint8 += (row * tile_width);
			    p_in_uint8 = (unsigned char *) (img->pixels);
			    p_in_uint8 += (row * img->width) + tile_x;
			    break;
			case 16:
			    p_out_uint16 = tile;
			    p_out_uint16 += (row * tile_width);
			    p_in_uint16 = (unsigned short *) (img->pixels);
			    p_in_uint16 += (row * img->width) + tile_x;
			    break;
			case 32:
			    p_out_uint32 = tile;
			    p_out_uint32 += (row * tile_width);
			    p_in_uint32 = (unsigned int *) (img->pixels);
			    p_in_uint32 += (row * img->width) + tile_x;
			    break;
			};
		      break;
		  case GGRAPH_SAMPLE_INT:
		      switch (img->bits_per_sample)
			{
			case 8:
			    p_out_int8 = tile;
			    p_out_int8 += (row * tile_width);
			    p_in_int8 = (char *) (img->pixels);
			    p_in_int8 += (row * img->width) + tile_x;
			    break;
			case 16:
			    p_out_int16 = tile;
			    p_out_int16 += (row * tile_width);
			    p_in_int16 = (short *) (img->pixels);
			    p_in_int16 += (row * img->width) + tile_x;
			    break;
			case 32:
			    p_out_int32 = tile;
			    p_out_int32 += (row * tile_width);
			    p_in_int32 = (int *) (img->pixels);
			    p_in_int32 += (row * img->width) + tile_x;
			    break;
			};
		      break;
		  case GGRAPH_SAMPLE_FLOAT:
		      switch (img->bits_per_sample)
			{
			case 32:
			    p_out_flt = tile;
			    p_out_flt += (row * tile_width);
			    p_in_flt = (float *) (img->pixels);
			    p_in_flt += (row * img->width) + tile_x;
			    break;
			case 64:
			    p_out_dbl = tile;
			    p_out_dbl += (row * tile_width);
			    p_in_dbl = (double *) (img->pixels);
			    p_in_dbl += (row * img->width) + tile_x;
			    break;
			};
		      break;
		  };
		for (col = 0; col < num_cols; col++)
		  {
		      switch (img->sample_format)
			{
			case GGRAPH_SAMPLE_UINT:
			    switch (img->bits_per_sample)
			      {
			      case 8:
				  *p_out_uint8++ = *p_in_uint8++;
				  break;
			      case 16:
				  *p_out_uint16++ = *p_in_uint16++;
				  break;
			      case 32:
				  *p_out_uint32++ = *p_in_uint32++;
				  break;
			      };
			    break;
			case GGRAPH_SAMPLE_INT:
			    switch (img->bits_per_sample)
			      {
			      case 8:
				  *p_out_int8++ = *p_in_int8++;
				  break;
			      case 16:
				  *p_out_int16++ = *p_in_int16++;
				  break;
			      case 32:
				  *p_out_int32++ = *p_in_int32++;
				  break;
			      };
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    switch (img->bits_per_sample)
			      {
			      case 32:
				  *p_out_flt++ = *p_in_flt++;
				  break;
			      case 16:
				  *p_out_dbl++ = *p_in_dbl++;
				  break;
			      };
			    break;
			};
		  }
	    }
	  if (TIFFWriteTile (out, tile, tile_x, tile_y, 0, 0) < 0)
	      return GGRAPH_TIFF_CODEC_ERROR;
      }
    img->next_row += num_rows;
    return GGRAPH_OK;
}

static int
tiff_write_strip_grid (const gGraphStripImagePtr img)
{
/* scanline(s) TIFF compression [strips] GRID data */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    TIFF *out = tiff_codec->tiff_handle;
    int row;
    int col;
    void *scanline = tiff_codec->tiff_buffer;
    char *p_in_int8;
    char *p_out_int8;
    unsigned char *p_in_uint8;
    unsigned char *p_out_uint8;
    short *p_in_int16;
    short *p_out_int16;
    unsigned short *p_in_uint16;
    unsigned short *p_out_uint16;
    int *p_in_int32;
    int *p_out_int32;
    unsigned int *p_in_uint32;
    unsigned int *p_out_uint32;
    float *p_in_flt;
    float *p_out_flt;
    double *p_in_dbl;
    double *p_out_dbl;

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
		      p_out_uint8 = scanline;
		      break;
		  case 16:
		      p_in_uint16 = (unsigned short *) (img->pixels);
		      p_in_uint16 += row * img->width;
		      p_out_uint16 = scanline;
		      break;
		  case 32:
		      p_in_uint32 = (unsigned int *) (img->pixels);
		      p_in_uint32 += row * img->width;
		      p_out_uint32 = scanline;
		      break;
		  };
		break;
	    case GGRAPH_SAMPLE_INT:
		switch (img->bits_per_sample)
		  {
		  case 8:
		      p_in_int8 = (char *) (img->pixels);
		      p_in_int8 += row * img->width;
		      p_out_int8 = scanline;
		      break;
		  case 16:
		      p_in_int16 = (short *) (img->pixels);
		      p_in_int16 += row * img->width;
		      p_out_int16 = scanline;
		      break;
		  case 32:
		      p_in_int32 = (int *) (img->pixels);
		      p_in_int32 += row * img->width;
		      p_out_int32 = scanline;
		      break;
		  };
		break;
	    case GGRAPH_SAMPLE_FLOAT:
		switch (img->bits_per_sample)
		  {
		  case 32:
		      p_in_flt = (float *) (img->pixels);
		      p_in_flt += row * img->width;
		      p_out_flt = scanline;
		      break;
		  case 64:
		      p_in_dbl = (double *) (img->pixels);
		      p_in_dbl += row * img->width;
		      p_out_dbl = scanline;
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
			    *p_out_uint8++ = *p_in_uint8++;
			    break;
			case 16:
			    *p_out_uint16++ = *p_in_uint16++;
			    break;
			case 32:
			    *p_out_uint32++ = *p_in_uint32++;
			    break;
			};
		      break;
		  case GGRAPH_SAMPLE_INT:
		      switch (img->bits_per_sample)
			{
			case 8:
			    *p_out_int8++ = *p_in_int8++;
			    break;
			case 16:
			    *p_out_int16++ = *p_in_int16++;
			    break;
			case 32:
			    *p_out_int32++ = *p_in_int32++;
			    break;
			};
		      break;
		  case GGRAPH_SAMPLE_FLOAT:
		      switch (img->bits_per_sample)
			{
			case 32:
			    *p_out_flt++ = *p_in_flt++;
			    break;
			case 16:
			    *p_out_dbl++ = *p_in_dbl++;
			    break;
			};
		      break;
		  };
	    }
	  if (TIFFWriteScanline (out, scanline, img->next_row + row, 0) < 0)
	      return GGRAPH_TIFF_CODEC_ERROR;
      }
    img->next_row += img->current_available_rows;
    return GGRAPH_OK;
}

static int
tiff_write_tile_monochrome (const gGraphStripImagePtr img)
{
/* scanline(s) TIFF compression [tiles] MONOCHROME */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    TIFF *out = tiff_codec->tiff_handle;
    int tile_width = img->tile_width;
    int tile_height = img->tile_height;
    int row;
    int col;
    int pixel;
    unsigned char byte;
    int pos;
    unsigned char *tile = tiff_codec->tiff_buffer;
    unsigned char *line_ptr;
    uint32 tile_x;
    uint32 tile_y;
    int num_rows;
    int num_cols;
    tsize_t scan_bytes = TIFFTileSize (out) / tile_height;

    tile_y = img->next_row;
    if (tile_y + tile_height > (uint32) (img->height))
	num_rows = img->height - tile_y;
    else
	num_rows = tile_height;
    if (num_rows != img->current_available_rows)
	return GGRAPH_TIFF_CODEC_ERROR;

    for (tile_x = 0; tile_x < (uint32) (img->width); tile_x += tile_width)
      {
	  /* feeding a tile */
	  if (tile_x + tile_width > (uint32) (img->width))
	      num_cols = img->width - tile_x;
	  else
	      num_cols = tile_width;
	  for (row = 0; row < num_rows; row++)
	    {
		unsigned char *p_in;
		byte = 0x00;
		pos = 0;
		line_ptr = tile + (row * scan_bytes);
		for (col = 0; col < num_cols; col++)
		  {
		      p_in = img->pixels + (row * img->scanline_width) +
			  ((tile_x + col) * img->pixel_size);
		      pixel = img->palette_red[*p_in];	/* expected to be a PALETTE image */
		      if (pixel == 0)
			{
			    /* handling a black pixel */
			    switch (pos)
			      {
			      case 0:
				  byte |= 0x80;
				  break;
			      case 1:
				  byte |= 0x40;
				  break;
			      case 2:
				  byte |= 0x20;
				  break;
			      case 3:
				  byte |= 0x10;
				  break;
			      case 4:
				  byte |= 0x08;
				  break;
			      case 5:
				  byte |= 0x04;
				  break;
			      case 6:
				  byte |= 0x02;
				  break;
			      case 7:
				  byte |= 0x01;
				  break;
			      };
			}
		      pos++;
		      if (pos > 7)
			{
			    /* exporting an octet */
			    *line_ptr++ = byte;
			    byte = 0x00;
			    pos = 0;
			}
		  }
		if (pos > 0)	/* exporting the last octet */
		    *line_ptr++ = byte;
	    }
	  if (TIFFWriteTile (out, tile, tile_x, tile_y, 0, 0) < 0)
	      return GGRAPH_TIFF_CODEC_ERROR;
      }
    img->next_row += num_rows;
    return GGRAPH_OK;
}

static int
tiff_write_strip_monochrome (const gGraphStripImagePtr img)
{
/* scanline(s) TIFF compression [strips] MONOCHROME */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    TIFF *out = tiff_codec->tiff_handle;
    int row;
    int col;
    int pixel;
    unsigned char byte;
    int pos;
    unsigned char *scanline = tiff_codec->tiff_buffer;
    unsigned char *line_ptr;
    for (row = 0; row < img->current_available_rows; row++)
      {
	  unsigned char *p_in = img->pixels + (row * img->scanline_width);
	  line_ptr = scanline;
	  for (col = 0; col < img->width; col++)
	      byte = 0x00;
	  pos = 0;
	  for (col = 0; col < img->width; col++)
	    {
		pixel = img->palette_red[*p_in++];	/* expected to be a PALETTE image */
		if (pixel == 0)
		  {
		      /* handling a black pixel */
		      switch (pos)
			{
			case 0:
			    byte |= 0x80;
			    break;
			case 1:
			    byte |= 0x40;
			    break;
			case 2:
			    byte |= 0x20;
			    break;
			case 3:
			    byte |= 0x10;
			    break;
			case 4:
			    byte |= 0x08;
			    break;
			case 5:
			    byte |= 0x04;
			    break;
			case 6:
			    byte |= 0x02;
			    break;
			case 7:
			    byte |= 0x01;
			    break;
			};
		  }
		pos++;
		if (pos > 7)
		  {
		      /* exporting an octet */
		      *line_ptr++ = byte;
		      byte = 0x00;
		      pos = 0;
		  }
	    }
	  if (pos > 0)		/* exporting the last octet */
	      *line_ptr++ = byte;
	  if (TIFFWriteScanline (out, scanline, img->next_row + row, 0) < 0)
	      return GGRAPH_TIFF_CODEC_ERROR;
      }
    img->next_row += img->current_available_rows;
    return GGRAPH_OK;
}

static int
tiff_write_tile_palette (const gGraphStripImagePtr img)
{
/* scanline(s) TIFF compression [tiles] PALETTE */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    TIFF *out = tiff_codec->tiff_handle;
    int tile_width = img->tile_width;
    int tile_height = img->tile_height;
    int row;
    int col;
    unsigned char *tile = tiff_codec->tiff_buffer;
    unsigned char *line_ptr;
    uint32 tile_x;
    uint32 tile_y;
    int num_rows;
    int num_cols;

    tile_y = img->next_row;
    if (tile_y + tile_height > (uint32) (img->height))
	num_rows = img->height - tile_y;
    else
	num_rows = tile_height;
    if (num_rows != img->current_available_rows)
	return GGRAPH_TIFF_CODEC_ERROR;

    for (tile_x = 0; tile_x < (uint32) (img->width); tile_x += tile_width)
      {
	  /* feeding a tile */
	  if (tile_x + tile_width > (uint32) (img->width))
	      num_cols = img->width - tile_x;
	  else
	      num_cols = tile_width;
	  for (row = 0; row < num_rows; row++)
	    {
		unsigned char *p_in;
		line_ptr = tile + (row * tile_width);
		for (col = 0; col < num_cols; col++)
		  {
		      p_in = img->pixels + (row * img->scanline_width) +
			  ((tile_x + col) * img->pixel_size);
		      *line_ptr++ = *p_in++;	/* expected to be a PALETTE image anyway */
		  }
	    }
	  if (TIFFWriteTile (out, tile, tile_x, tile_y, 0, 0) < 0)
	      return GGRAPH_TIFF_CODEC_ERROR;
      }
    img->next_row += num_rows;
    return GGRAPH_OK;
}

static int
tiff_write_strip_palette (const gGraphStripImagePtr img)
{
/* scanline(s) TIFF compression [strips] PALETTE */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    TIFF *out = tiff_codec->tiff_handle;
    int row;
    int col;
    unsigned char *scanline = tiff_codec->tiff_buffer;
    unsigned char *line_ptr;
    for (row = 0; row < img->current_available_rows; row++)
      {
	  unsigned char *p_in = img->pixels + (row * img->scanline_width);
	  line_ptr = scanline;
	  for (col = 0; col < img->width; col++)
	      *line_ptr++ = *p_in++;	/* expected to be a PALETTE image anyway */
	  if (TIFFWriteScanline (out, scanline, img->next_row + row, 0) < 0)
	      return GGRAPH_TIFF_CODEC_ERROR;
      }
    img->next_row += img->current_available_rows;
    return GGRAPH_OK;
}

static int
tiff_write_tile_grayscale (const gGraphStripImagePtr img)
{
/* scanline(s) TIFF compression [tiles] GRAYSCALE */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    TIFF *out = tiff_codec->tiff_handle;
    int tile_width = img->tile_width;
    int tile_height = img->tile_height;
    int row;
    int col;
    unsigned char *tile = tiff_codec->tiff_buffer;
    unsigned char *line_ptr;
    uint32 tile_x;
    uint32 tile_y;
    int num_rows;
    int num_cols;

    tile_y = img->next_row;
    if (tile_y + tile_height > (uint32) (img->height))
	num_rows = img->height - tile_y;
    else
	num_rows = tile_height;
    if (num_rows != img->current_available_rows)
	return GGRAPH_TIFF_CODEC_ERROR;

    for (tile_x = 0; tile_x < (uint32) (img->width); tile_x += tile_width)
      {
	  /* feeding a tile */
	  if (tile_x + tile_width > (uint32) (img->width))
	      num_cols = img->width - tile_x;
	  else
	      num_cols = tile_width;
	  for (row = 0; row < num_rows; row++)
	    {
		unsigned char *p_in;
		line_ptr = tile + (row * tile_width);
		for (col = 0; col < num_cols; col++)
		  {
		      p_in = img->pixels + (row * img->scanline_width) +
			  ((tile_x + col) * img->pixel_size);
		      *line_ptr++ = *p_in++;	/* expected to be a GRAYSCALE image anyway */
		  }
	    }
	  if (TIFFWriteTile (out, tile, tile_x, tile_y, 0, 0) < 0)
	      return GGRAPH_TIFF_CODEC_ERROR;
      }
    img->next_row += num_rows;
    return GGRAPH_OK;
}

static int
tiff_write_strip_grayscale (const gGraphStripImagePtr img)
{
/* scanline(s) TIFF compression [strips] GRAYSCALE */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    TIFF *out = tiff_codec->tiff_handle;
    int row;
    int col;
    unsigned char *scanline = tiff_codec->tiff_buffer;
    unsigned char *line_ptr;

    for (row = 0; row < img->current_available_rows; row++)
      {
	  unsigned char *p_in = img->pixels + (row * img->scanline_width);
	  line_ptr = scanline;
	  for (col = 0; col < img->width; col++)
	      *line_ptr++ = *p_in++;	/* expected to be a GRAYSCALE image anyway */
	  if (TIFFWriteScanline (out, scanline, img->next_row + row, 0) < 0)
	      return GGRAPH_TIFF_CODEC_ERROR;
      }
    img->next_row += img->current_available_rows;
    return GGRAPH_OK;
}

static int
tiff_write_tile_rgb (const gGraphStripImagePtr img)
{
/* scanline(s) TIFF compression [tiles] RGB */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    TIFF *out = tiff_codec->tiff_handle;
    int tile_width = img->tile_width;
    int tile_height = img->tile_height;
    int row;
    int col;
    unsigned char *tile = tiff_codec->tiff_buffer;
    unsigned char *line_ptr;
    uint32 tile_x;
    uint32 tile_y;
    int num_rows;
    int num_cols;

    tile_y = img->next_row;
    if (tile_y + tile_height > (uint32) (img->height))
	num_rows = img->height - tile_y;
    else
	num_rows = tile_height;
    if (num_rows != img->current_available_rows)
	return GGRAPH_TIFF_CODEC_ERROR;

    for (tile_x = 0; tile_x < (uint32) (img->width); tile_x += tile_width)
      {
	  /* feeding a tile */
	  if (tile_x + tile_width > (uint32) (img->width))
	      num_cols = img->width - tile_x;
	  else
	      num_cols = tile_width;
	  for (row = 0; row < num_rows; row++)
	    {
		unsigned char *p_in;
		unsigned char r;
		unsigned char g;
		unsigned char b;
		line_ptr = tile + (row * tile_width * 3);
		for (col = 0; col < num_cols; col++)
		  {
		      p_in =
			  img->pixels +
			  (row * img->scanline_width) +
			  ((tile_x + col) * img->pixel_size);
		      if (img->pixel_format == GG_PIXEL_RGB)
			{
			    r = *p_in++;
			    g = *p_in++;
			    b = *p_in++;
			}
		      else if (img->pixel_format == GG_PIXEL_RGBA)
			{
			    r = *p_in++;
			    g = *p_in++;
			    b = *p_in++;
			    p_in++;	/* skipping Alpha */
			}
		      else if (img->pixel_format == GG_PIXEL_ARGB)
			{
			    p_in++;	/* skipping Alpha */
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
		      else if (img->pixel_format == GG_PIXEL_BGRA)
			{
			    b = *p_in++;
			    g = *p_in++;
			    r = *p_in++;
			    p_in++;	/* skipping Alpha */
			}
		      *line_ptr++ = r;
		      *line_ptr++ = g;
		      *line_ptr++ = b;
		  }
	    }
	  if (TIFFWriteTile (out, tile, tile_x, tile_y, 0, 0) < 0)
	      return GGRAPH_TIFF_CODEC_ERROR;
      }
    img->next_row += num_rows;
    return GGRAPH_OK;
}

static int
tiff_write_strip_rgb (const gGraphStripImagePtr img)
{
/* scanline(s) TIFF compression [strips] RGB */
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    TIFF *out = tiff_codec->tiff_handle;
    int row;
    int col;
    unsigned char *scanline = tiff_codec->tiff_buffer;
    unsigned char *line_ptr;

    for (row = 0; row < img->current_available_rows; row++)
      {
	  unsigned char *p_in = img->pixels + (row * img->scanline_width);
	  unsigned char r;
	  unsigned char g;
	  unsigned char b;
	  line_ptr = scanline;
	  for (col = 0; col < img->width; col++)
	    {
		if (img->pixel_format == GG_PIXEL_RGB)
		  {
		      r = *p_in++;
		      g = *p_in++;
		      b = *p_in++;
		  }
		else if (img->pixel_format == GG_PIXEL_RGBA)
		  {
		      r = *p_in++;
		      g = *p_in++;
		      b = *p_in++;
		      p_in++;	/* skipping Alpha */
		  }
		else if (img->pixel_format == GG_PIXEL_ARGB)
		  {
		      p_in++;	/* skipping Alpha */
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
		else if (img->pixel_format == GG_PIXEL_BGRA)
		  {
		      b = *p_in++;
		      g = *p_in++;
		      r = *p_in++;
		      p_in++;
		  }
		*line_ptr++ = r;
		*line_ptr++ = g;
		*line_ptr++ = b;
	    }
	  if (TIFFWriteScanline (out, scanline, img->next_row + row, 0) < 0)
	      return GGRAPH_TIFF_CODEC_ERROR;
      }
    img->next_row += img->current_available_rows;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_write_to_tiff_by_strip (const gGraphStripImagePtr img, int *progress)
{
/* scanline(s) TIFF compression [by strip] */
    int ret;
    struct tiff_codec_data *tiff_codec =
	(struct tiff_codec_data *) (img->codec_data);
    if (tiff_codec->tiff_type == TIFF_TYPE_RGB)
      {
	  if (tiff_codec->is_tiled)
	      ret = tiff_write_tile_rgb (img);
	  else
	      ret = tiff_write_strip_rgb (img);
      }
    if (tiff_codec->tiff_type == TIFF_TYPE_GRAYSCALE)
      {
	  if (tiff_codec->is_tiled)
	      ret = tiff_write_tile_grayscale (img);
	  else
	      ret = tiff_write_strip_grayscale (img);
      }
    if (tiff_codec->tiff_type == TIFF_TYPE_PALETTE)
      {
	  if (tiff_codec->is_tiled)
	      ret = tiff_write_tile_palette (img);
	  else
	      ret = tiff_write_strip_palette (img);
      }
    if (tiff_codec->tiff_type == TIFF_TYPE_MONOCHROME)
      {
	  if (tiff_codec->is_tiled)
	      ret = tiff_write_tile_monochrome (img);
	  else
	      ret = tiff_write_strip_monochrome (img);
      }
    if (tiff_codec->tiff_type == TIFF_TYPE_GRID)
      {
	  if (tiff_codec->is_tiled)
	      ret = tiff_write_tile_grid (img);
	  else
	      ret = tiff_write_strip_grid (img);
      }
    if (ret == GGRAPH_OK && progress != NULL)
	*progress =
	    (int) (((double) (img->next_row + 1) * 100.0) /
		   (double) (img->height));
    return ret;
}
