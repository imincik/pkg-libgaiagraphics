/* 
/ gaiagraphics_jpeg.c
/
/ JPEG auxiliary helpers
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
#include <setjmp.h>
#include <limits.h>
#include <string.h>

#include <jpeglib.h>
#include <jerror.h>

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

struct jpeg_codec_data
{
/* a struct used by JPEG codec */
    int is_writer;
    struct jpeg_compress_struct cmp_cinfo;
    struct jpeg_decompress_struct dec_cinfo;
    JSAMPROW row;
    xgdIOCtx *io_ctx;
};

/* 
/
/ DISCLAIMER:
/ the following code is vaguely derived from the original GD lib 
/ code, which was originally released under a BSD-like license
/
*/

#define OUTPUT_BUF_SIZE  4096

#ifdef HAVE_BOOLEAN
typedef boolean safeboolean;
#else
typedef int safeboolean;
#endif /* HAVE_BOOLEAN */

typedef struct _jmpbuf_wrapper
{
    jmp_buf jmpbuf;
}
jmpbuf_wrapper;

typedef struct
{
    struct jpeg_destination_mgr pub;
    xgdIOCtx *outfile;
    unsigned char *buffer;
}
my_destination_mgr;

typedef struct
{
    struct jpeg_source_mgr pub;
    xgdIOCtx *infile;
    unsigned char *buffer;
    safeboolean start_of_file;
}
my_source_mgr;

typedef my_source_mgr *my_src_ptr;

#define INPUT_BUF_SIZE  4096

typedef my_destination_mgr *my_dest_ptr;

static void
fatal_jpeg_error (j_common_ptr cinfo)
{
    jmpbuf_wrapper *jmpbufw;

    fprintf (stderr,
	     "jpeg-wrapper: JPEG library reports unrecoverable error: ");
    (*cinfo->err->output_message) (cinfo);
    fflush (stderr);
    jmpbufw = (jmpbuf_wrapper *) cinfo->client_data;
    jpeg_destroy (cinfo);
    if (jmpbufw != 0)
      {
	  longjmp (jmpbufw->jmpbuf, 1);
	  fprintf (stderr, "jpeg-wrappeg: EXTREMELY fatal error: longjmp"
		   " returned control; terminating\n");
      }
    else
      {
	  fprintf (stderr, "jpeg-wrappeg: EXTREMELY fatal error: jmpbuf"
		   " unrecoverable; terminating\n");
      }
    fflush (stderr);
    exit (99);
}

static void
init_source (j_decompress_ptr cinfo)
{
    my_src_ptr src = (my_src_ptr) cinfo->src;
    src->start_of_file = TRUE;
}

#define END_JPEG_SEQUENCE "\r\n[*]--:END JPEG:--[*]\r\n"
static safeboolean
fill_input_buffer (j_decompress_ptr cinfo)
{
    my_src_ptr src = (my_src_ptr) cinfo->src;
    int nbytes = 0;
    memset (src->buffer, 0, INPUT_BUF_SIZE);
    while (nbytes < INPUT_BUF_SIZE)
      {
	  int got = xgdGetBuf (src->buffer + nbytes,
			       INPUT_BUF_SIZE - nbytes,
			       src->infile);
	  if ((got == EOF) || (got == 0))
	    {
		if (!nbytes)
		  {
		      nbytes = -1;
		  }
		break;
	    }
	  nbytes += got;
      }
    if (nbytes <= 0)
      {
	  if (src->start_of_file)
	      ERREXIT (cinfo, JERR_INPUT_EMPTY);
	  WARNMS (cinfo, JWRN_JPEG_EOF);
	  src->buffer[0] = (unsigned char) 0xFF;
	  src->buffer[1] = (unsigned char) JPEG_EOI;
	  nbytes = 2;
      }
    src->pub.next_input_byte = (JOCTET *) (src->buffer);
    src->pub.bytes_in_buffer = nbytes;
    src->start_of_file = FALSE;
    return TRUE;
}

static void
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    my_src_ptr src = (my_src_ptr) cinfo->src;
    if (num_bytes > 0)
      {
	  while (num_bytes > (long) src->pub.bytes_in_buffer)
	    {
		num_bytes -= (long) src->pub.bytes_in_buffer;
		(void) fill_input_buffer (cinfo);
	    }
	  src->pub.next_input_byte += (size_t) num_bytes;
	  src->pub.bytes_in_buffer -= (size_t) num_bytes;
      }
}

static void
term_source (j_decompress_ptr cinfo)
{
    if (cinfo)
	return;			/* does absolutely nothing - required in order to suppress warnings */
}

static void
init_destination (j_compress_ptr cinfo)
{
    my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
    dest->buffer = (unsigned char *)
	(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_IMAGE,
				    OUTPUT_BUF_SIZE * sizeof (unsigned char));
    dest->pub.next_output_byte = (JOCTET *) (dest->buffer);
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}

static safeboolean
empty_output_buffer (j_compress_ptr cinfo)
{
    my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
    if (xgdPutBuf (dest->buffer, OUTPUT_BUF_SIZE, dest->outfile) !=
	(size_t) OUTPUT_BUF_SIZE)
	ERREXIT (cinfo, JERR_FILE_WRITE);
    dest->pub.next_output_byte = (JOCTET *) (dest->buffer);
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
    return TRUE;
}

static void
term_destination (j_compress_ptr cinfo)
{
    my_dest_ptr dest = (my_dest_ptr) cinfo->dest;
    size_t datacount = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;
    if (datacount > 0)
      {
	  if (xgdPutBuf (dest->buffer, datacount, dest->outfile) !=
	      (int) datacount)
	      ERREXIT (cinfo, JERR_FILE_WRITE);
      }
}

static void
jpeg_xgdIOCtx_src (j_decompress_ptr cinfo, xgdIOCtx * infile)
{
    my_src_ptr src;
    if (cinfo->src == NULL)
      {
	  cinfo->src = (struct jpeg_source_mgr *)
	      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
					  sizeof (my_source_mgr));
	  src = (my_src_ptr) cinfo->src;
	  src->buffer = (unsigned char *)
	      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
					  INPUT_BUF_SIZE *
					  sizeof (unsigned char));

      }
    src = (my_src_ptr) cinfo->src;
    src->pub.init_source = init_source;
    src->pub.fill_input_buffer = fill_input_buffer;
    src->pub.skip_input_data = skip_input_data;
    src->pub.resync_to_restart = jpeg_resync_to_restart;
    src->pub.term_source = term_source;
    src->infile = infile;
    src->pub.bytes_in_buffer = 0;
    src->pub.next_input_byte = NULL;
}

static void
jpeg_xgdIOCtx_dest (j_compress_ptr cinfo, xgdIOCtx * outfile)
{
    my_dest_ptr dest;
    if (cinfo->dest == NULL)
      {
	  cinfo->dest = (struct jpeg_destination_mgr *)
	      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
					  sizeof (my_destination_mgr));
      }
    dest = (my_dest_ptr) cinfo->dest;
    dest->pub.init_destination = init_destination;
    dest->pub.empty_output_buffer = empty_output_buffer;
    dest->pub.term_destination = term_destination;
    dest->outfile = outfile;
}

static int
xgdImageJpegCtx (gGraphImagePtr img, xgdIOCtx * outfile, int quality)
{
/* compressing a JPEG image */
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int i, j, jidx;
    volatile JSAMPROW row = 0;
    JSAMPROW rowptr[1];
    jmpbuf_wrapper jmpbufw;
    JDIMENSION nlines;
    char comment[255];
    memset (&cinfo, 0, sizeof (cinfo));
    memset (&jerr, 0, sizeof (jerr));
    cinfo.err = jpeg_std_error (&jerr);
    cinfo.client_data = &jmpbufw;
    if (setjmp (jmpbufw.jmpbuf) != 0)
      {
	  if (row)
	      free (row);
	  return GGRAPH_JPEG_CODEC_ERROR;
      }
    cinfo.err->error_exit = fatal_jpeg_error;
    jpeg_create_compress (&cinfo);
    cinfo.image_width = img->width;
    cinfo.image_height = img->height;
    if (img->pixel_format == GG_PIXEL_GRAYSCALE)
      {
	  /* GRAYSCALE */
	  cinfo.input_components = 1;
	  cinfo.in_color_space = JCS_GRAYSCALE;
      }
    else
      {
	  /* RGB */
	  cinfo.input_components = 3;
	  cinfo.in_color_space = JCS_RGB;
      }
    jpeg_set_defaults (&cinfo);
    if (quality >= 0)
	jpeg_set_quality (&cinfo, quality, TRUE);
    jpeg_xgdIOCtx_dest (&cinfo, outfile);
    row = (JSAMPROW) calloc (1, cinfo.image_width * cinfo.input_components
			     * sizeof (JSAMPLE));
    if (row == 0)
      {
	  jpeg_destroy_compress (&cinfo);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    rowptr[0] = row;
    jpeg_start_compress (&cinfo, TRUE);
    sprintf (comment, "CREATOR: jpeg-wrapper (using IJG JPEG v%d),",
	     JPEG_LIB_VERSION);
    if (quality >= 0)
	sprintf (comment + strlen (comment), " quality = %d\n", quality);
    else
	strcat (comment + strlen (comment), " default quality\n");
    jpeg_write_marker (&cinfo, JPEG_COM, (unsigned char *) comment,
		       (unsigned int) strlen (comment));
#if BITS_IN_JSAMPLE == 12
    fprintf (stderr,
	     "jpeg-wrapper: error: jpeg library was compiled for 12-bit\n");
    goto error;
#endif /* BITS_IN_JSAMPLE == 12 */
    for (i = 0; i < img->height; i++)
      {
	  unsigned char *p_in = img->pixels + (i * img->scanline_width);
	  for (jidx = 0, j = 0; j < img->width; j++)
	    {
		if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      /* GRAYSCALE */
		      row[jidx++] = *p_in++;
		  }
		else
		  {
		      /* RGB */
		      unsigned char r;
		      unsigned char g;
		      unsigned char b;
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
		      else if (img->pixel_format == GG_PIXEL_PALETTE)
			{
			    int index = *p_in++;
			    r = img->palette_red[index];
			    g = img->palette_green[index];
			    b = img->palette_blue[index];
			}
		      row[jidx++] = r;
		      row[jidx++] = g;
		      row[jidx++] = b;
		  }
	    }
	  nlines = jpeg_write_scanlines (&cinfo, rowptr, 1);
	  if (nlines != 1)
	      fprintf (stderr, "jpeg-wrapper: warning: jpeg_write_scanlines"
		       " returns %u -- expected 1\n", nlines);
      }
    jpeg_finish_compress (&cinfo);
    jpeg_destroy_compress (&cinfo);
    free (row);
    return GGRAPH_OK;
}

GGRAPH_PRIVATE void
gg_jpeg_codec_destroy (void *p)
{
/* destroyng the JPEG codec data */
    jmpbuf_wrapper jmpbufw;
    struct jpeg_codec_data *codec = (struct jpeg_codec_data *) p;

    if (!codec)
	return;

    if (codec->is_writer)
      {
	  codec->cmp_cinfo.client_data = &jmpbufw;
	  if (setjmp (jmpbufw.jmpbuf) != 0)
	    {
		fprintf (stderr, "SetJump\n");
		fflush (stderr);
		return;
	    }
	  jpeg_finish_compress (&(codec->cmp_cinfo));
	  jpeg_destroy_compress (&(codec->cmp_cinfo));
      }
    else
      {
	  codec->dec_cinfo.client_data = &jmpbufw;
	  if (setjmp (jmpbufw.jmpbuf) != 0)
	    {
		fprintf (stderr, "SetJump\n");
		fflush (stderr);
		return;
	    }
	  jpeg_destroy_decompress (&(codec->dec_cinfo));
      }
    free (codec->row);
    codec->io_ctx->xgd_free (codec->io_ctx);
    free (codec);
}

static int
xgdStripImageJpegCtx (gGraphStripImagePtr img, xgdIOCtx * outfile, int quality)
{
/* preparing to compress a JPEG image (by strip) */
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    volatile JSAMPROW row = 0;
    jmpbuf_wrapper jmpbufw;
    char comment[255];
    struct jpeg_codec_data *jpeg_codec;
    memset (&cinfo, 0, sizeof (cinfo));
    memset (&jerr, 0, sizeof (jerr));
    cinfo.err = jpeg_std_error (&jerr);
    cinfo.client_data = &jmpbufw;
    if (setjmp (jmpbufw.jmpbuf) != 0)
      {
	  if (row)
	      free (row);
	  return GGRAPH_JPEG_CODEC_ERROR;
      }
    cinfo.err->error_exit = fatal_jpeg_error;
    jpeg_create_compress (&cinfo);
    cinfo.image_width = img->width;
    cinfo.image_height = img->height;
    if (img->pixel_format == GG_PIXEL_GRAYSCALE)
      {
	  /* GRAYSCALE */
	  cinfo.input_components = 1;
	  cinfo.in_color_space = JCS_GRAYSCALE;
      }
    else
      {
	  /* RGB */
	  cinfo.input_components = 3;
	  cinfo.in_color_space = JCS_RGB;
      }
    jpeg_set_defaults (&cinfo);
    if (quality >= 0)
	jpeg_set_quality (&cinfo, quality, TRUE);
    jpeg_xgdIOCtx_dest (&cinfo, outfile);
    row = (JSAMPROW) calloc (1, cinfo.image_width * cinfo.input_components
			     * sizeof (JSAMPLE));
    if (row == 0)
      {
	  jpeg_destroy_compress (&cinfo);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    jpeg_start_compress (&cinfo, TRUE);
    sprintf (comment, "CREATOR: jpeg-wrapper (using IJG JPEG v%d),",
	     JPEG_LIB_VERSION);
    if (quality >= 0)
	sprintf (comment + strlen (comment), " quality = %d\n", quality);
    else
	strcat (comment + strlen (comment), " default quality\n");
    jpeg_write_marker (&cinfo, JPEG_COM, (unsigned char *) comment,
		       (unsigned int) strlen (comment));

/* setting up the JPEG codec struct */
    jpeg_codec = malloc (sizeof (struct jpeg_codec_data));
    if (!jpeg_codec)
      {
	  jpeg_destroy_compress (&cinfo);
	  free (row);
	  gg_strip_image_destroy (img);
	  return GGRAPH_INSUFFICIENT_MEMORY;
      }
    jpeg_codec->is_writer = 1;
    memcpy (&(jpeg_codec->cmp_cinfo), &cinfo,
	    sizeof (struct jpeg_compress_struct));
    jpeg_codec->row = row;
    jpeg_codec->io_ctx = outfile;
    img->codec_data = jpeg_codec;

    return GGRAPH_OK;
}

static int
xgdStripImageJpegWriteCtx (gGraphStripImagePtr img)
{
/* compressing a JPEG image (by strip) */
    int i, j, jidx;
    int height;
    struct jpeg_codec_data *jpeg_codec =
	(struct jpeg_codec_data *) (img->codec_data);
    JSAMPROW rowptr[1];
    JDIMENSION nlines;
    volatile JSAMPROW row = jpeg_codec->row;
    jmpbuf_wrapper jmpbufw;

    rowptr[0] = jpeg_codec->row;
    if (img->next_row >= img->height)
      {
	  /* EOF condition */
	  fprintf (stderr, "png-wrapper error: attempting to write beyond EOF");
	  return GGRAPH_PNG_CODEC_ERROR;
      }
    height = img->current_available_rows;

    jpeg_codec->cmp_cinfo.client_data = &jmpbufw;
    if (setjmp (jmpbufw.jmpbuf) != 0)
	return GGRAPH_JPEG_CODEC_ERROR;
    for (i = 0; i < height; i++)
      {
	  unsigned char *p_in = img->pixels + (i * img->scanline_width);
	  for (jidx = 0, j = 0; j < img->width; j++)
	    {
		if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      /* GRAYSCALE */
		      row[jidx++] = *p_in++;
		  }
		else
		  {
		      /* RGB */
		      unsigned char r;
		      unsigned char g;
		      unsigned char b;
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
		      else if (img->pixel_format == GG_PIXEL_PALETTE)
			{
			    int index = *p_in++;
			    r = img->palette_red[index];
			    g = img->palette_green[index];
			    b = img->palette_blue[index];
			}
		      row[jidx++] = r;
		      row[jidx++] = g;
		      row[jidx++] = b;
		  }
	    }
	  nlines = jpeg_write_scanlines (&(jpeg_codec->cmp_cinfo), rowptr, 1);
	  if (nlines != 1)
	      fprintf (stderr, "jpeg-wrapper: warning: jpeg_write_scanlines"
		       " returns %u -- expected 1\n", nlines);
      }
    img->next_row += height;
    return GGRAPH_OK;
}

static void
CMYKToRGB (int c, int m, int y, int k, int inverted, unsigned char *red,
	   unsigned char *green, unsigned char *blue)
{
    if (inverted)
      {
	  c = 255 - c;
	  m = 255 - m;
	  y = 255 - y;
	  k = 255 - k;
      }
    *red = (255 - c) * (255 - k) / 255;
    *green = (255 - m) * (255 - k) / 255;
    *blue = (255 - y) * (255 - k) / 255;
}

static gGraphImageInfosPtr
xgdImageInspectJpegCtx (xgdIOCtx * infile, int *errcode)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    volatile gGraphImageInfosPtr infos = 0;
    int retval;
    jmpbuf_wrapper jmpbufw;
    memset (&cinfo, 0, sizeof (cinfo));
    memset (&jerr, 0, sizeof (jerr));
    cinfo.err = jpeg_std_error (&jerr);
    cinfo.client_data = &jmpbufw;
    if (setjmp (jmpbufw.jmpbuf) != 0)
      {
	  if (infos)
	      gg_image_infos_destroy (infos);
	  *errcode = GGRAPH_JPEG_CODEC_ERROR;
	  return NULL;
      }
    cinfo.err->error_exit = fatal_jpeg_error;
    jpeg_create_decompress (&cinfo);
    jpeg_xgdIOCtx_src (&cinfo, infile);
    jpeg_save_markers (&cinfo, JPEG_APP0 + 14, 256);
    retval = jpeg_read_header (&cinfo, TRUE);
    if (retval != JPEG_HEADER_OK)
	fprintf (stderr, "jpeg-wrapper: warning: jpeg_read_header returns"
		 " %d, expected %d\n", retval, JPEG_HEADER_OK);
    if (cinfo.image_height > INT_MAX)
	fprintf (stderr,
		 "jpeg-wrapper: warning: JPEG image height (%u) is greater than INT_MAX\n",
		 cinfo.image_height);
    if (cinfo.image_width > INT_MAX)
	fprintf (stderr,
		 "jpeg-wrapper: warning: JPEG image width (%u) is greater than INT_MAX\n",
		 cinfo.image_width);

    if ((cinfo.jpeg_color_space == JCS_CMYK) ||
	(cinfo.jpeg_color_space == JCS_YCCK))
	cinfo.out_color_space = JCS_CMYK;
    else if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
	cinfo.out_color_space = JCS_GRAYSCALE;
    else
	cinfo.out_color_space = JCS_RGB;

    if (cinfo.out_color_space == JCS_GRAYSCALE)
	infos =
	    gg_image_infos_create (GG_PIXEL_GRAYSCALE, (int) cinfo.image_width,
				   (int) cinfo.image_height, 8, 1,
				   GGRAPH_SAMPLE_UINT, NULL, NULL);
    else
	infos =
	    gg_image_infos_create (GG_PIXEL_RGB, (int) cinfo.image_width,
				   (int) cinfo.image_height, 8, 3,
				   GGRAPH_SAMPLE_UINT, NULL, NULL);
    if (infos == NULL)
      {
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }
    infos->compression = GGRAPH_TIFF_COMPRESSION_JPEG;
    infos->scale_1_2 = 1;
    infos->scale_1_4 = 1;
    infos->scale_1_8 = 1;
    return infos;
  error:
    if (infos)
	gg_image_infos_destroy (infos);
    return NULL;
}

static gGraphImagePtr
xgdImageCreateFromJpegCtx (xgdIOCtx * infile, int *errcode, int scale)
{
/* decompressing a JPEG image */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    jmpbuf_wrapper jmpbufw;
    volatile JSAMPROW row = 0;
    volatile gGraphImagePtr img = 0;
    JSAMPROW rowptr[1];
    int i, j, retval;
    JDIMENSION nrows;
    int channels = 3;
    int inverted = 0;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    memset (&cinfo, 0, sizeof (cinfo));
    memset (&jerr, 0, sizeof (jerr));
    cinfo.err = jpeg_std_error (&jerr);
    cinfo.client_data = &jmpbufw;
    if (setjmp (jmpbufw.jmpbuf) != 0)
      {
	  if (row)
	      free (row);
	  if (img)
	      gg_image_destroy (img);
	  *errcode = GGRAPH_JPEG_CODEC_ERROR;
	  return NULL;
      }
    cinfo.err->error_exit = fatal_jpeg_error;
    jpeg_create_decompress (&cinfo);
    jpeg_xgdIOCtx_src (&cinfo, infile);
    jpeg_save_markers (&cinfo, JPEG_APP0 + 14, 256);
    retval = jpeg_read_header (&cinfo, TRUE);
    if (retval != JPEG_HEADER_OK)
	fprintf (stderr, "jpeg-wrapper: warning: jpeg_read_header returns"
		 " %d, expected %d\n", retval, JPEG_HEADER_OK);
    if (cinfo.image_height > INT_MAX)
	fprintf (stderr,
		 "jpeg-wrapper: warning: JPEG image height (%u) is greater than INT_MAX\n",
		 cinfo.image_height);
    if (cinfo.image_width > INT_MAX)
	fprintf (stderr,
		 "jpeg-wrapper: warning: JPEG image width (%u) is greater than INT_MAX\n",
		 cinfo.image_width);

    if (scale == 8)
      {
	  /* requesting 1:8 scaling */
	  cinfo.scale_num = 1;
      }
    else if (scale == 4)
      {
	  /* requesting 1:4 scaling */
	  cinfo.scale_num = 2;
      }
    else if (scale == 2)
      {
	  /* requesting 1:2 scaling */
	  cinfo.scale_num = 4;
      }
    else
      {
	  /* no scaling, full dimension */
	  cinfo.scale_num = 8;
      }
    cinfo.scale_denom = 8;
    if ((cinfo.jpeg_color_space == JCS_CMYK) ||
	(cinfo.jpeg_color_space == JCS_YCCK))
	cinfo.out_color_space = JCS_CMYK;
    else if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
	cinfo.out_color_space = JCS_GRAYSCALE;
    else
	cinfo.out_color_space = JCS_RGB;

    if (jpeg_start_decompress (&cinfo) != TRUE)
	fprintf (stderr,
		 "jpeg-wrapper: warning: jpeg_start_decompress reports suspended data source\n");
    if (cinfo.out_color_space == JCS_GRAYSCALE)
	img =
	    gg_image_create (GG_PIXEL_GRAYSCALE, (int) cinfo.output_width,
			     (int) cinfo.output_height, 8, 1,
			     GGRAPH_SAMPLE_UINT, NULL, NULL);
    else
	img =
	    gg_image_create (GG_PIXEL_RGB, (int) cinfo.output_width,
			     (int) cinfo.output_height, 8, 3,
			     GGRAPH_SAMPLE_UINT, NULL, NULL);
    if (img == NULL)
      {
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }
    if (cinfo.out_color_space == JCS_RGB)
      {
	  if (cinfo.output_components != 3)
	    {
		fprintf (stderr,
			 "jpeg-wrapper: error: JPEG color output_components == %d\n",
			 cinfo.output_components);
		*errcode = GGRAPH_JPEG_CODEC_ERROR;
		goto error;
	    }
	  channels = 3;
      }
    else if (cinfo.out_color_space == JCS_GRAYSCALE)
      {
	  if (cinfo.output_components != 1)
	    {
		fprintf (stderr,
			 "jpeg-wrapper: error: JPEG color output_components == %d\n",
			 cinfo.output_components);
		*errcode = GGRAPH_JPEG_CODEC_ERROR;
		goto error;
	    }
	  channels = 1;
      }
    else if (cinfo.out_color_space == JCS_CMYK)
      {
	  jpeg_saved_marker_ptr marker;
	  if (cinfo.output_components != 4)
	    {
		fprintf (stderr,
			 "jpeg-wrapper: error: JPEG output_components == %d\n",
			 cinfo.output_components);
		*errcode = GGRAPH_JPEG_CODEC_ERROR;
		goto error;
	    }
	  channels = 4;
	  marker = cinfo.marker_list;
	  while (marker)
	    {
		if ((marker->marker == (JPEG_APP0 + 14)) &&
		    (marker->data_length >= 12)
		    && (!strncmp ((const char *) marker->data, "Adobe", 5)))
		  {
		      inverted = 1;
		      break;
		  }
		marker = marker->next;
	    }
      }
    else
      {
	  fprintf (stderr, "jpeg-wrapper: error: unexpected colorspace\n");
	  *errcode = GGRAPH_JPEG_CODEC_ERROR;
	  goto error;
      }
#if BITS_IN_JSAMPLE == 12
    fprintf (stderr,
	     "jpeg-wrapper: error: jpeg library was compiled for 12-bit\n");
    *errcode = GGRAPH_JPEG_CODEC_ERROR;
    goto error;
#endif /* BITS_IN_JSAMPLE == 12 */
    row = calloc (cinfo.output_width * channels, sizeof (JSAMPLE));
    if (row == 0)
      {
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }
    rowptr[0] = row;
    if (cinfo.out_color_space == JCS_CMYK)
      {
	  for (i = 0; i < (int) cinfo.output_height; i++)
	    {
		register JSAMPROW currow = row;
		unsigned char *p_out = img->pixels + (i * img->scanline_width);
		nrows = jpeg_read_scanlines (&cinfo, rowptr, 1);
		if (nrows != 1)
		  {
		      fprintf (stderr,
			       "jpeg-wrapper: error: jpeg_read_scanlines returns %u, expected 1\n",
			       nrows);
		      *errcode = GGRAPH_JPEG_CODEC_ERROR;
		      goto error;
		  }
		for (j = 0; j < (int) cinfo.output_width; j++, currow += 4)
		  {
		      CMYKToRGB (currow[0], currow[1], currow[2], currow[3],
				 inverted, &red, &green, &blue);
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
	    }
      }
    else if (cinfo.out_color_space == JCS_GRAYSCALE)
      {
	  for (i = 0; i < (int) cinfo.output_height; i++)
	    {
		register JSAMPROW currow = row;
		unsigned char *p_out = img->pixels + (i * img->scanline_width);
		nrows = jpeg_read_scanlines (&cinfo, rowptr, 1);
		if (nrows != 1)
		  {
		      fprintf (stderr,
			       "jpeg-wrapper: error: jpeg_read_scanlines returns %u, expected 1\n",
			       nrows);
		      *errcode = GGRAPH_JPEG_CODEC_ERROR;
		      goto error;
		  }
		for (j = 0; j < (int) cinfo.output_width; j++, currow++)
		    *p_out++ = currow[0];
	    }
      }
    else
      {
	  for (i = 0; i < (int) cinfo.output_height; i++)
	    {
		register JSAMPROW currow = row;
		unsigned char *p_out = img->pixels + (i * img->scanline_width);
		nrows = jpeg_read_scanlines (&cinfo, rowptr, 1);
		if (nrows != 1)
		  {
		      fprintf (stderr,
			       "jpeg-wrapper: error: jpeg_read_scanlines returns %u, expected 1\n",
			       nrows);
		      *errcode = GGRAPH_JPEG_CODEC_ERROR;
		      goto error;
		  }
		for (j = 0; j < (int) cinfo.output_width; j++, currow += 3)
		  {
		      *p_out++ = currow[0];
		      *p_out++ = currow[1];
		      *p_out++ = currow[2];
		  }
	    }
      }
    if (jpeg_finish_decompress (&cinfo) != TRUE)
	fprintf (stderr,
		 "jpeg-wrapper: warning: jpeg_finish_decompress reports suspended data source\n");
    jpeg_destroy_decompress (&cinfo);
    free (row);
    return img;
  error:
    jpeg_destroy_decompress (&cinfo);
    if (row)
	free (row);
    if (img)
	gg_image_destroy (img);
    return NULL;
}

static gGraphStripImagePtr
xgdStripImageCreateFromJpegCtx (xgdIOCtx * infile, int *errcode, FILE * file)
{
/* preparing to decompress a JPEG image [by strips] */
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    jmpbuf_wrapper jmpbufw;
    volatile JSAMPROW row = 0;
    volatile gGraphStripImagePtr img = 0;
    int retval;
    int channels = 3;
    struct jpeg_codec_data *jpeg_codec;
    memset (&cinfo, 0, sizeof (cinfo));
    memset (&jerr, 0, sizeof (jerr));
    cinfo.err = jpeg_std_error (&jerr);
    cinfo.client_data = &jmpbufw;
    if (setjmp (jmpbufw.jmpbuf) != 0)
      {
	  if (row)
	      free (row);
	  if (img)
	      gg_strip_image_destroy (img);
	  *errcode = GGRAPH_JPEG_CODEC_ERROR;
	  return NULL;
      }
    cinfo.err->error_exit = fatal_jpeg_error;
    jpeg_create_decompress (&cinfo);
    jpeg_xgdIOCtx_src (&cinfo, infile);
    jpeg_save_markers (&cinfo, JPEG_APP0 + 14, 256);
    retval = jpeg_read_header (&cinfo, TRUE);
    if (retval != JPEG_HEADER_OK)
	fprintf (stderr, "jpeg-wrapper: warning: jpeg_read_header returns"
		 " %d, expected %d\n", retval, JPEG_HEADER_OK);
    if (cinfo.image_height > INT_MAX)
	fprintf (stderr,
		 "jpeg-wrapper: warning: JPEG image height (%u) is greater than INT_MAX\n",
		 cinfo.image_height);
    if (cinfo.image_width > INT_MAX)
	fprintf (stderr,
		 "jpeg-wrapper: warning: JPEG image width (%u) is greater than INT_MAX\n",
		 cinfo.image_width);
    cinfo.scale_num = 8;
    cinfo.scale_denom = 8;
    if ((cinfo.jpeg_color_space == JCS_CMYK) ||
	(cinfo.jpeg_color_space == JCS_YCCK))
	cinfo.out_color_space = JCS_CMYK;
    else if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
	cinfo.out_color_space = JCS_GRAYSCALE;
    else
	cinfo.out_color_space = JCS_RGB;

    if (jpeg_start_decompress (&cinfo) != TRUE)
	fprintf (stderr,
		 "jpeg-wrapper: warning: jpeg_start_decompress reports suspended data source\n");
    if (cinfo.out_color_space == JCS_GRAYSCALE)
	img =
	    gg_strip_image_create (file, GGRAPH_IMAGE_JPEG, GG_PIXEL_GRAYSCALE,
				   cinfo.output_width, cinfo.output_height, 8,
				   1, GGRAPH_SAMPLE_UINT, NULL, NULL);
    else
	img =
	    gg_strip_image_create (file, GGRAPH_IMAGE_JPEG, GG_PIXEL_RGB,
				   cinfo.output_width, cinfo.output_height, 8,
				   3, GGRAPH_SAMPLE_UINT, NULL, NULL);
    if (img == NULL)
      {
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }
    if (cinfo.out_color_space == JCS_RGB)
      {
	  if (cinfo.output_components != 3)
	    {
		fprintf (stderr,
			 "jpeg-wrapper: error: JPEG color output_components == %d\n",
			 cinfo.output_components);
		*errcode = GGRAPH_JPEG_CODEC_ERROR;
		goto error;
	    }
	  channels = 3;
      }
    else if (cinfo.out_color_space == JCS_GRAYSCALE)
      {
	  if (cinfo.output_components != 1)
	    {
		fprintf (stderr,
			 "jpeg-wrapper: error: JPEG color output_components == %d\n",
			 cinfo.output_components);
		*errcode = GGRAPH_JPEG_CODEC_ERROR;
		goto error;
	    }
	  channels = 1;
      }
    else if (cinfo.out_color_space == JCS_CMYK)
      {
	  jpeg_saved_marker_ptr marker;
	  if (cinfo.output_components != 4)
	    {
		fprintf (stderr,
			 "jpeg-wrapper: error: JPEG output_components == %d\n",
			 cinfo.output_components);
		*errcode = GGRAPH_JPEG_CODEC_ERROR;
		goto error;
	    }
	  channels = 4;
	  marker = cinfo.marker_list;
	  while (marker)
	    {
		if ((marker->marker == (JPEG_APP0 + 14)) &&
		    (marker->data_length >= 12)
		    && (!strncmp ((const char *) marker->data, "Adobe", 5)))
		  {
		      break;
		  }
		marker = marker->next;
	    }
      }
    else
      {
	  fprintf (stderr, "jpeg-wrapper: error: unexpected colorspace\n");
	  *errcode = GGRAPH_JPEG_CODEC_ERROR;
	  goto error;
      }
#if BITS_IN_JSAMPLE == 12
    fprintf (stderr,
	     "jpeg-wrapper: error: jpeg library was compiled for 12-bit\n");
    *errcode = GGRAPH_JPEG_CODEC_ERROR;
    goto error;
#endif /* BITS_IN_JSAMPLE == 12 */
    row = calloc (cinfo.output_width * channels, sizeof (JSAMPLE));
    if (row == 0)
      {
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }

/* setting up the JPEG codec struct */
    jpeg_codec = malloc (sizeof (struct jpeg_codec_data));
    if (!jpeg_codec)
      {
	  *errcode = GGRAPH_INSUFFICIENT_MEMORY;
	  goto error;
      }
    jpeg_codec->is_writer = 0;
    memcpy (&(jpeg_codec->dec_cinfo), &cinfo,
	    sizeof (struct jpeg_decompress_struct));
    jpeg_codec->row = row;
    jpeg_codec->io_ctx = infile;
    img->codec_data = jpeg_codec;

    return img;
  error:
    jpeg_destroy_decompress (&cinfo);
    if (row)
	free (row);
    if (img)
	gg_strip_image_destroy (img);
    return NULL;
}

static int
xgdStripImageReadFromJpegCtx (gGraphStripImagePtr img)
{
/* decompressing a JPEG image (by strip) */
    struct jpeg_codec_data *jpeg_codec =
	(struct jpeg_codec_data *) (img->codec_data);
    jmpbuf_wrapper jmpbufw;
    JSAMPROW rowptr[1];
    int i, j;
    JDIMENSION nrows;
    int inverted = 0;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    int height = img->rows_per_block;

    if (img->next_row >= img->height)
      {
	  /* EOF condition */
	  fprintf (stderr, "jpeg-wrapper error: attempting to read beyond EOF");
	  return GGRAPH_JPEG_CODEC_ERROR;
      }
    if ((img->next_row + img->rows_per_block) >= img->height)
	height = img->height - img->next_row;
    img->current_available_rows = height;

    jpeg_codec->dec_cinfo.client_data = &jmpbufw;
    if (setjmp (jmpbufw.jmpbuf) != 0)
	return GGRAPH_JPEG_CODEC_ERROR;

    rowptr[0] = jpeg_codec->row;
    if (jpeg_codec->dec_cinfo.out_color_space == JCS_CMYK)
      {
	  for (i = 0; i < height; i++)
	    {
		register JSAMPROW currow = jpeg_codec->row;
		unsigned char *p_out = img->pixels + (i * img->scanline_width);
		nrows =
		    jpeg_read_scanlines (&(jpeg_codec->dec_cinfo), rowptr, 1);
		if (nrows != 1)
		  {
		      fprintf (stderr,
			       "jpeg-wrapper: error: jpeg_read_scanlines returns %u, expected 1\n",
			       nrows);
		      return GGRAPH_JPEG_CODEC_ERROR;
		  }
		for (j = 0; j < (int) jpeg_codec->dec_cinfo.output_width;
		     j++, currow += 4)
		  {
		      CMYKToRGB (currow[0], currow[1], currow[2], currow[3],
				 inverted, &red, &green, &blue);
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
	    }
      }
    else if (jpeg_codec->dec_cinfo.out_color_space == JCS_GRAYSCALE)
      {
	  for (i = 0; i < height; i++)
	    {
		register JSAMPROW currow = jpeg_codec->row;
		unsigned char *p_out = img->pixels + (i * img->scanline_width);
		nrows =
		    jpeg_read_scanlines (&(jpeg_codec->dec_cinfo), rowptr, 1);
		if (nrows != 1)
		  {
		      fprintf (stderr,
			       "jpeg-wrapper: error: jpeg_read_scanlines returns %u, expected 1\n",
			       nrows);
		      return GGRAPH_JPEG_CODEC_ERROR;
		  }
		for (j = 0; j < (int) jpeg_codec->dec_cinfo.output_width;
		     j++, currow++)
		    *p_out++ = currow[0];
	    }
      }
    else
      {
	  for (i = 0; i < height; i++)
	    {
		register JSAMPROW currow = jpeg_codec->row;
		unsigned char *p_out = img->pixels + (i * img->scanline_width);
		nrows =
		    jpeg_read_scanlines (&(jpeg_codec->dec_cinfo), rowptr, 1);
		if (nrows != 1)
		  {
		      fprintf (stderr,
			       "jpeg-wrapper: error: jpeg_read_scanlines returns %u, expected 1\n",
			       nrows);
		      return GGRAPH_JPEG_CODEC_ERROR;
		  }
		for (j = 0; j < (int) jpeg_codec->dec_cinfo.output_width;
		     j++, currow += 3)
		  {
		      *p_out++ = currow[0];
		      *p_out++ = currow[1];
		      *p_out++ = currow[2];
		  }
	    }
      }
    img->next_row += height;
    return GGRAPH_OK;
}

GGRAPH_PRIVATE int
gg_image_to_jpeg (const gGraphImagePtr img, void **mem_buf, int *mem_buf_size,
		  FILE * file, int dest_type, int quality)
{
/* compressing an image as JPEG */
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
    ret = xgdImageJpegCtx (img, out, quality);
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
gg_image_prepare_to_jpeg_by_strip (const gGraphStripImagePtr img, FILE * file,
				   int quality)
{
/* preparing to compress an image as JPEG [by strip] */
    xgdIOCtx *out;

/* checkings args for validity */
    if (!file)
	return GGRAPH_ERROR;

    out = xgdNewDynamicCtx (0, file, GG_TARGET_IS_FILE);
    return xgdStripImageJpegCtx (img, out, quality);
}

GGRAPH_PRIVATE int
gg_image_write_to_jpeg_by_strip (const gGraphStripImagePtr img, int *progress)
{
/* scanline(s) JPEG compression [by strip] */
    int ret = xgdStripImageJpegWriteCtx (img);
    if (ret == GGRAPH_OK && progress != NULL)
	*progress =
	    (int) (((double) (img->next_row + 1) * 100.0) /
		   (double) (img->height));
    return ret;
}

GGRAPH_PRIVATE int
gg_image_from_jpeg (int size, const void *data, int source_type,
		    gGraphImagePtr * image_handle, int scale)
{
/* uncompressing a JPEG */
    int errcode = GGRAPH_OK;
    gGraphImagePtr img;
    xgdIOCtx *in =
	xgdNewDynamicCtxEx (size, data, XGD_CTX_DONT_FREE, source_type);
    img = xgdImageCreateFromJpegCtx (in, &errcode, scale);
    in->xgd_free (in);
    *image_handle = img;
    return errcode;
}

GGRAPH_PRIVATE int
gg_image_infos_from_jpeg (int size, const void *data, int source_type,
			  gGraphImageInfosPtr * infos_handle)
{
/* image infos from JPEG */
    int errcode = GGRAPH_OK;
    gGraphImageInfosPtr infos;
    xgdIOCtx *in =
	xgdNewDynamicCtxEx (size, data, XGD_CTX_DONT_FREE, source_type);
    infos = xgdImageInspectJpegCtx (in, &errcode);
    in->xgd_free (in);
    *infos_handle = infos;
    return errcode;
}

GGRAPH_PRIVATE int
gg_image_strip_prepare_from_jpeg (FILE * file,
				  gGraphStripImagePtr * image_handle)
{
/* preparing to uncompress a JPEG [by strips] */
    int errcode = GGRAPH_OK;
    gGraphStripImagePtr img;
    xgdIOCtx *in =
	xgdNewDynamicCtxEx (0, file, XGD_CTX_DONT_FREE, GG_TARGET_IS_FILE);
    img = xgdStripImageCreateFromJpegCtx (in, &errcode, file);
    *image_handle = img;
    return errcode;
}

GGRAPH_PRIVATE int
gg_image_strip_read_from_jpeg (gGraphStripImagePtr img, int *progress)
{
/* uncompressing a JPEG [by strips] */
    int ret = xgdStripImageReadFromJpegCtx (img);
    if (ret == GGRAPH_OK && progress != NULL)
	*progress =
	    (int) (((double) (img->next_row + 1) * 100.0) /
		   (double) (img->height));
    return ret;
}
