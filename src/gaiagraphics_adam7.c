/* 
/ gaiagraphics_adam7.c
/
/ Adam7 encoding and deconding
/
/ version 1.0, 2010 September 20
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

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

GGRAPH_DECLARE int
gGraphImageToAdam7 (const void *ptr_img, void *mem_bufs[7],
		    int mem_buf_sizes[7], void **palette, int *palette_size)
{
/* encoding an image as Adam7 */
    unsigned char *p_in;
    unsigned char *p_out;
    void *out;
    int size;
    int scanline_width;
    int pixel_size;
    int i;
    int x;
    int y;
    short out_width;
    short out_height;
    int x_base;
    int y_base;
    int x_step;
    int y_step;
    int endian_arch = gg_endian_arch ();
    int endian;
    int is_palette = 0;
    short start_marker[7];
    short end_marker[7];
    gGraphImagePtr img = (gGraphImagePtr) ptr_img;

    for (i = 0; i < 7; i++)
      {
	  mem_bufs[i] = NULL;
	  mem_buf_sizes[i] = 0;
      }
    *palette = NULL;
    *palette_size = 0;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;

    pixel_size = img->pixel_size;
    switch (img->pixel_format)
      {
      case GG_PIXEL_RGB:
	  start_marker[0] = GG_ADAM7_0_RGB_START;
	  end_marker[0] = GG_ADAM7_0_RGB_END;
	  start_marker[1] = GG_ADAM7_1_RGB_START;
	  end_marker[1] = GG_ADAM7_1_RGB_END;
	  start_marker[2] = GG_ADAM7_2_RGB_START;
	  end_marker[2] = GG_ADAM7_2_RGB_END;
	  start_marker[3] = GG_ADAM7_3_RGB_START;
	  end_marker[3] = GG_ADAM7_3_RGB_END;
	  start_marker[4] = GG_ADAM7_4_RGB_START;
	  end_marker[4] = GG_ADAM7_4_RGB_END;
	  start_marker[5] = GG_ADAM7_5_RGB_START;
	  end_marker[5] = GG_ADAM7_5_RGB_END;
	  start_marker[6] = GG_ADAM7_6_RGB_START;
	  end_marker[6] = GG_ADAM7_6_RGB_END;
	  break;
      case GG_PIXEL_GRAYSCALE:
	  start_marker[0] = GG_ADAM7_0_GRAYSCALE_START;
	  end_marker[0] = GG_ADAM7_0_GRAYSCALE_END;
	  start_marker[1] = GG_ADAM7_1_GRAYSCALE_START;
	  end_marker[1] = GG_ADAM7_1_GRAYSCALE_END;
	  start_marker[2] = GG_ADAM7_2_GRAYSCALE_START;
	  end_marker[2] = GG_ADAM7_2_GRAYSCALE_END;
	  start_marker[3] = GG_ADAM7_3_GRAYSCALE_START;
	  end_marker[3] = GG_ADAM7_3_GRAYSCALE_END;
	  start_marker[4] = GG_ADAM7_4_GRAYSCALE_START;
	  end_marker[4] = GG_ADAM7_4_GRAYSCALE_END;
	  start_marker[5] = GG_ADAM7_5_GRAYSCALE_START;
	  end_marker[5] = GG_ADAM7_5_GRAYSCALE_END;
	  start_marker[6] = GG_ADAM7_6_GRAYSCALE_START;
	  end_marker[6] = GG_ADAM7_6_GRAYSCALE_END;
	  break;
      case GG_PIXEL_PALETTE:
	  start_marker[0] = GG_ADAM7_0_PALETTE_START;
	  end_marker[0] = GG_ADAM7_0_PALETTE_END;
	  start_marker[1] = GG_ADAM7_1_PALETTE_START;
	  end_marker[1] = GG_ADAM7_1_PALETTE_END;
	  start_marker[2] = GG_ADAM7_2_PALETTE_START;
	  end_marker[2] = GG_ADAM7_2_PALETTE_END;
	  start_marker[3] = GG_ADAM7_3_PALETTE_START;
	  end_marker[3] = GG_ADAM7_3_PALETTE_END;
	  start_marker[4] = GG_ADAM7_4_PALETTE_START;
	  end_marker[4] = GG_ADAM7_4_PALETTE_END;
	  start_marker[5] = GG_ADAM7_5_PALETTE_START;
	  end_marker[5] = GG_ADAM7_5_PALETTE_END;
	  start_marker[6] = GG_ADAM7_6_PALETTE_START;
	  end_marker[6] = GG_ADAM7_6_PALETTE_END;
	  is_palette = 1;
	  break;
      case GG_PIXEL_GRID:
	  switch (img->sample_format)
	    {
	    case GGRAPH_SAMPLE_INT:
		if (img->bits_per_sample == 16)
		  {
		      start_marker[0] = GG_ADAM7_0_INT16_START;
		      end_marker[0] = GG_ADAM7_0_INT16_END;
		      start_marker[1] = GG_ADAM7_1_INT16_START;
		      end_marker[1] = GG_ADAM7_1_INT16_END;
		      start_marker[2] = GG_ADAM7_2_INT16_START;
		      end_marker[2] = GG_ADAM7_2_INT16_END;
		      start_marker[3] = GG_ADAM7_3_INT16_START;
		      end_marker[3] = GG_ADAM7_3_INT16_END;
		      start_marker[4] = GG_ADAM7_4_INT16_START;
		      end_marker[4] = GG_ADAM7_4_INT16_END;
		      start_marker[5] = GG_ADAM7_5_INT16_START;
		      end_marker[5] = GG_ADAM7_5_INT16_END;
		      start_marker[6] = GG_ADAM7_6_INT16_START;
		      end_marker[6] = GG_ADAM7_6_INT16_END;
		  }
		else
		  {
		      start_marker[0] = GG_ADAM7_0_INT32_START;
		      end_marker[0] = GG_ADAM7_0_INT32_END;
		      start_marker[1] = GG_ADAM7_1_INT32_START;
		      end_marker[1] = GG_ADAM7_1_INT32_END;
		      start_marker[2] = GG_ADAM7_2_INT32_START;
		      end_marker[2] = GG_ADAM7_2_INT32_END;
		      start_marker[3] = GG_ADAM7_3_INT32_START;
		      end_marker[3] = GG_ADAM7_3_INT32_END;
		      start_marker[4] = GG_ADAM7_4_INT32_START;
		      end_marker[4] = GG_ADAM7_4_INT32_END;
		      start_marker[5] = GG_ADAM7_5_INT32_START;
		      end_marker[5] = GG_ADAM7_5_INT32_END;
		      start_marker[6] = GG_ADAM7_6_INT32_START;
		      end_marker[6] = GG_ADAM7_6_INT32_END;
		  }
		break;
	    case GGRAPH_SAMPLE_UINT:
		if (img->bits_per_sample == 16)
		  {
		      start_marker[0] = GG_ADAM7_0_UINT16_START;
		      end_marker[0] = GG_ADAM7_0_UINT16_END;
		      start_marker[1] = GG_ADAM7_1_UINT16_START;
		      end_marker[1] = GG_ADAM7_1_UINT16_END;
		      start_marker[2] = GG_ADAM7_2_UINT16_START;
		      end_marker[2] = GG_ADAM7_2_UINT16_END;
		      start_marker[3] = GG_ADAM7_3_UINT16_START;
		      end_marker[3] = GG_ADAM7_3_UINT16_END;
		      start_marker[4] = GG_ADAM7_4_UINT16_START;
		      end_marker[4] = GG_ADAM7_4_UINT16_END;
		      start_marker[5] = GG_ADAM7_5_UINT16_START;
		      end_marker[5] = GG_ADAM7_5_UINT16_END;
		      start_marker[6] = GG_ADAM7_6_UINT16_START;
		      end_marker[6] = GG_ADAM7_6_UINT16_END;
		  }
		else
		  {
		      start_marker[0] = GG_ADAM7_0_UINT32_START;
		      end_marker[0] = GG_ADAM7_0_UINT32_END;
		      start_marker[1] = GG_ADAM7_1_UINT32_START;
		      end_marker[1] = GG_ADAM7_1_UINT32_END;
		      start_marker[2] = GG_ADAM7_2_UINT32_START;
		      end_marker[2] = GG_ADAM7_2_UINT32_END;
		      start_marker[3] = GG_ADAM7_3_UINT32_START;
		      end_marker[3] = GG_ADAM7_3_UINT32_END;
		      start_marker[4] = GG_ADAM7_4_UINT32_START;
		      end_marker[4] = GG_ADAM7_4_UINT32_END;
		      start_marker[5] = GG_ADAM7_5_UINT32_START;
		      end_marker[5] = GG_ADAM7_5_UINT32_END;
		      start_marker[6] = GG_ADAM7_6_UINT32_START;
		      end_marker[6] = GG_ADAM7_6_UINT32_END;
		  }
		break;
	    case GGRAPH_SAMPLE_FLOAT:
		if (img->bits_per_sample == 32)
		  {
		      start_marker[0] = GG_ADAM7_0_FLOAT_START;
		      end_marker[0] = GG_ADAM7_0_FLOAT_END;
		      start_marker[1] = GG_ADAM7_1_FLOAT_START;
		      end_marker[1] = GG_ADAM7_1_FLOAT_END;
		      start_marker[2] = GG_ADAM7_2_FLOAT_START;
		      end_marker[2] = GG_ADAM7_2_FLOAT_END;
		      start_marker[3] = GG_ADAM7_3_FLOAT_START;
		      end_marker[3] = GG_ADAM7_3_FLOAT_END;
		      start_marker[4] = GG_ADAM7_4_FLOAT_START;
		      end_marker[4] = GG_ADAM7_4_FLOAT_END;
		      start_marker[5] = GG_ADAM7_5_FLOAT_START;
		      end_marker[5] = GG_ADAM7_5_FLOAT_END;
		      start_marker[6] = GG_ADAM7_6_FLOAT_START;
		      end_marker[6] = GG_ADAM7_6_FLOAT_END;
		  }
		else
		  {
		      start_marker[0] = GG_ADAM7_0_DOUBLE_START;
		      end_marker[0] = GG_ADAM7_0_DOUBLE_END;
		      start_marker[1] = GG_ADAM7_1_DOUBLE_START;
		      end_marker[1] = GG_ADAM7_1_DOUBLE_END;
		      start_marker[2] = GG_ADAM7_2_DOUBLE_START;
		      end_marker[2] = GG_ADAM7_2_DOUBLE_END;
		      start_marker[3] = GG_ADAM7_3_DOUBLE_START;
		      end_marker[3] = GG_ADAM7_3_DOUBLE_END;
		      start_marker[4] = GG_ADAM7_4_DOUBLE_START;
		      end_marker[4] = GG_ADAM7_4_DOUBLE_END;
		      start_marker[5] = GG_ADAM7_5_DOUBLE_START;
		      end_marker[5] = GG_ADAM7_5_DOUBLE_END;
		      start_marker[6] = GG_ADAM7_6_DOUBLE_START;
		      end_marker[6] = GG_ADAM7_6_DOUBLE_END;
		  }
		break;
	    };
	  break;
      };

    for (i = 0; i < 7; i++)
      {
	  /* Adam7 steps */
	  switch (i)
	    {
	    case 0:
		x_base = 0;
		y_base = 0;
		x_step = 8;
		y_step = 8;
		break;
	    case 1:
		x_base = 4;
		y_base = 0;
		x_step = 8;
		y_step = 8;
		break;
	    case 2:
		x_base = 0;
		y_base = 4;
		x_step = 4;
		y_step = 8;
		break;
	    case 3:
		x_base = 2;
		y_base = 0;
		x_step = 4;
		y_step = 4;
		break;
	    case 4:
		x_base = 0;
		y_base = 2;
		x_step = 2;
		y_step = 4;
		break;
	    case 5:
		x_base = 1;
		y_base = 0;
		x_step = 2;
		y_step = 2;
		break;
	    case 6:
		x_base = 0;
		y_base = 1;
		x_step = 1;
		y_step = 2;
		break;
	    };
	  out_width = (img->width - x_base) / x_step;
	  if ((out_width * x_step) < (img->width - x_base))
	      out_width++;
	  out_height = (img->height - y_base) / y_step;
	  if ((out_height * y_step) < (img->height - y_base))
	      out_height++;
	  scanline_width = out_width * pixel_size;
	  size = out_height * scanline_width;
	  size += 4 * sizeof (short);
	  out = malloc (size);
	  if (!out)
	      goto stop;
	  mem_bufs[i] = out;
	  mem_buf_sizes[i] = size;
	  p_out = out;
	  /*
	   * tiles are alternatively encoded as LITTLE or BIG-ENDIAN
	   * in order to ensure an arch-neutral approach
	   */
	  if (i == 0 || i == 2 || i == 4 || i == 6)
	      endian = 0;
	  else
	      endian = 1;
	  gg_export_int16 (start_marker[i], p_out, endian, endian_arch);
	  p_out += sizeof (short);
	  gg_export_int16 (out_width, p_out, endian, endian_arch);
	  p_out += sizeof (short);
	  gg_export_int16 (out_height, p_out, endian, endian_arch);
	  p_out += sizeof (short);
	  for (y = y_base; y < img->height; y += y_step)
	    {
		p_in =
		    (unsigned char *) (img->pixels) +
		    (y * img->scanline_width) + (x_base * pixel_size);
		for (x = x_base; x < img->width; x += x_step)
		  {
		      if (img->pixel_format == GG_PIXEL_GRID)
			{
			    switch (img->sample_format)
			      {
			      case GGRAPH_SAMPLE_INT:
				  if (img->bits_per_sample == 16)
				    {
					short value = *((short *) p_in);
					gg_export_int16 (value, p_out, endian,
							 endian_arch);
					p_in += x_step * sizeof (short);
					p_out += sizeof (short);
				    }
				  else
				    {
					int value = *((int *) p_in);
					gg_export_int32 (value, p_out, endian,
							 endian_arch);
					p_in += x_step * sizeof (int);
					p_out += sizeof (int);
				    }
				  break;
			      case GGRAPH_SAMPLE_UINT:
				  if (img->bits_per_sample == 16)
				    {
					unsigned short value =
					    *((unsigned short *) p_in);
					gg_export_uint16 (value, p_out, endian,
							  endian_arch);
					p_in +=
					    x_step * sizeof (unsigned short);
					p_out += sizeof (unsigned short);
				    }
				  else
				    {
					unsigned int value =
					    *((unsigned int *) p_in);
					gg_export_uint32 (value, p_out, endian,
							  endian_arch);
					p_in += x_step * sizeof (unsigned int);
					p_out += sizeof (unsigned int);
				    }
				  break;
			      case GGRAPH_SAMPLE_FLOAT:
				  if (img->bits_per_sample == 32)
				    {
					float value = *((float *) p_in);
					gg_export_float (value, p_out, endian,
							 endian_arch);
					p_in += x_step * sizeof (float);
					p_out += sizeof (float);
				    }
				  else
				    {
					double value = *((double *) p_in);
					gg_export_double (value, p_out, endian,
							  endian_arch);
					p_in += x_step * sizeof (double);
					p_out += sizeof (double);
				    }
				  break;
			      };
			}
		      else
			{
			    memcpy (p_out, p_in, pixel_size);
			    p_in += x_step * pixel_size;
			    p_out += pixel_size;
			}
		  }
	    }
	  p_out = (unsigned char *) out + size - sizeof (short);
	  gg_export_int16 (end_marker[i], p_out, endian, endian_arch);
      }

    if (is_palette)
      {
	  /* a palette is required */
	  size = (3 * img->max_palette) + (2 * sizeof (short)) + 1;
	  out = malloc (size);
	  if (out == NULL)
	      goto stop;
	  p_out = out;
	  /* palette markers always are LITTLE-ENDIAN */
	  gg_export_int16 (GG_ADAM7_PALETTE_START, p_out, 1, endian_arch);
	  p_out += sizeof (short);
	  *p_out++ = (unsigned char) (img->max_palette);
	  for (i = 0; i < img->max_palette; i++)
	    {
		*p_out++ = img->palette_red[i];
		*p_out++ = img->palette_green[i];
		*p_out++ = img->palette_blue[i];
	    }
	  gg_export_int16 (GG_ADAM7_PALETTE_END, p_out, 1, endian_arch);
	  *palette = out;
	  *palette_size = size;
      }

    return GGRAPH_OK;

  stop:
    for (i = 0; i < 7; i++)
      {
	  if (mem_bufs[i])
	      free (mem_bufs[i]);
	  mem_bufs[i] = NULL;
	  mem_buf_sizes[i] = 0;
      }
    return GGRAPH_INSUFFICIENT_MEMORY;
}

static gGraphImagePtr
adam7_decode (int img_no, void *mem_buf, int mem_buf_size)
{
/* decoding an Adam7 sub-image */
    unsigned char *p_in;
    int endian_arch = gg_endian_arch ();
    int endian;
    short start_signature;
    short end_signature;
    short width;
    short height;
    int x;
    int y;
    int pixel_format = GG_PIXEL_UNKNOWN;
    int sample_format;
    int bits_per_sample;
    int samples_per_pixel = 1;
    int size;
    gGraphImagePtr img = NULL;

    if (mem_buf_size < (int) (sizeof (short) * 4))
	return NULL;
/*
* tiles are alternatively encoded as LITTLE or BIG-ENDIAN
* in order to ensure an arch-neutral approach
*/
    if (img_no == 0 || img_no == 2 || img_no == 4 || img_no == 6)
	endian = 0;
    else
	endian = 1;

/* checking signatures */
    p_in = mem_buf;
    start_signature = gg_import_int16 (p_in, endian, endian_arch);
    p_in = (unsigned char *) mem_buf + mem_buf_size - sizeof (short);
    end_signature = gg_import_int16 (p_in, endian, endian_arch);

    switch (img_no)
      {
      case 0:
	  if (start_signature == GG_ADAM7_0_RGB_START
	      && end_signature == GG_ADAM7_0_RGB_END)
	    {
		pixel_format = GG_PIXEL_RGB;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
		samples_per_pixel = 3;
	    }
	  if (start_signature == GG_ADAM7_0_GRAYSCALE_START
	      && end_signature == GG_ADAM7_0_GRAYSCALE_END)
	    {
		pixel_format = GG_PIXEL_GRAYSCALE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_0_PALETTE_START
	      && end_signature == GG_ADAM7_0_PALETTE_END)
	    {
		pixel_format = GG_PIXEL_PALETTE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_0_INT16_START
	      && end_signature == GG_ADAM7_0_INT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_0_INT32_START
	      && end_signature == GG_ADAM7_0_INT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_0_UINT16_START
	      && end_signature == GG_ADAM7_0_UINT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_0_UINT32_START
	      && end_signature == GG_ADAM7_0_UINT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_0_FLOAT_START
	      && end_signature == GG_ADAM7_0_FLOAT_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_0_DOUBLE_START
	      && end_signature == GG_ADAM7_0_DOUBLE_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 64;
	    }
	  break;
      case 1:
	  if (start_signature == GG_ADAM7_1_RGB_START
	      && end_signature == GG_ADAM7_1_RGB_END)
	    {
		pixel_format = GG_PIXEL_RGB;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
		samples_per_pixel = 3;
	    }
	  if (start_signature == GG_ADAM7_1_GRAYSCALE_START
	      && end_signature == GG_ADAM7_1_GRAYSCALE_END)
	    {
		pixel_format = GG_PIXEL_GRAYSCALE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_1_PALETTE_START
	      && end_signature == GG_ADAM7_1_PALETTE_END)
	    {
		pixel_format = GG_PIXEL_PALETTE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_1_INT16_START
	      && end_signature == GG_ADAM7_1_INT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_1_INT32_START
	      && end_signature == GG_ADAM7_1_INT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_1_UINT16_START
	      && end_signature == GG_ADAM7_1_UINT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_1_UINT32_START
	      && end_signature == GG_ADAM7_1_UINT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_1_FLOAT_START
	      && end_signature == GG_ADAM7_1_FLOAT_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_1_DOUBLE_START
	      && end_signature == GG_ADAM7_1_DOUBLE_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 64;
	    }
	  break;
      case 2:
	  if (start_signature == GG_ADAM7_2_RGB_START
	      && end_signature == GG_ADAM7_2_RGB_END)
	    {
		pixel_format = GG_PIXEL_RGB;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
		samples_per_pixel = 3;
	    }
	  if (start_signature == GG_ADAM7_2_GRAYSCALE_START
	      && end_signature == GG_ADAM7_2_GRAYSCALE_END)
	    {
		pixel_format = GG_PIXEL_GRAYSCALE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_2_PALETTE_START
	      && end_signature == GG_ADAM7_2_PALETTE_END)
	    {
		pixel_format = GG_PIXEL_PALETTE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_2_INT16_START
	      && end_signature == GG_ADAM7_2_INT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_2_INT32_START
	      && end_signature == GG_ADAM7_2_INT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_2_UINT16_START
	      && end_signature == GG_ADAM7_2_UINT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_2_UINT32_START
	      && end_signature == GG_ADAM7_2_UINT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_2_FLOAT_START
	      && end_signature == GG_ADAM7_2_FLOAT_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_2_DOUBLE_START
	      && end_signature == GG_ADAM7_2_DOUBLE_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 64;
	    }
	  break;
      case 3:
	  if (start_signature == GG_ADAM7_3_RGB_START
	      && end_signature == GG_ADAM7_3_RGB_END)
	    {
		pixel_format = GG_PIXEL_RGB;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
		samples_per_pixel = 3;
	    }
	  if (start_signature == GG_ADAM7_3_GRAYSCALE_START
	      && end_signature == GG_ADAM7_3_GRAYSCALE_END)
	    {
		pixel_format = GG_PIXEL_GRAYSCALE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_3_PALETTE_START
	      && end_signature == GG_ADAM7_3_PALETTE_END)
	    {
		pixel_format = GG_PIXEL_PALETTE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_3_INT16_START
	      && end_signature == GG_ADAM7_3_INT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_3_INT32_START
	      && end_signature == GG_ADAM7_3_INT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_3_UINT16_START
	      && end_signature == GG_ADAM7_3_UINT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_3_UINT32_START
	      && end_signature == GG_ADAM7_3_UINT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_3_FLOAT_START
	      && end_signature == GG_ADAM7_3_FLOAT_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_3_DOUBLE_START
	      && end_signature == GG_ADAM7_3_DOUBLE_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 64;
	    }
	  break;
      case 4:
	  if (start_signature == GG_ADAM7_4_RGB_START
	      && end_signature == GG_ADAM7_4_RGB_END)
	    {
		pixel_format = GG_PIXEL_RGB;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
		samples_per_pixel = 3;
	    }
	  if (start_signature == GG_ADAM7_4_GRAYSCALE_START
	      && end_signature == GG_ADAM7_4_GRAYSCALE_END)
	    {
		pixel_format = GG_PIXEL_GRAYSCALE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_4_PALETTE_START
	      && end_signature == GG_ADAM7_4_PALETTE_END)
	    {
		pixel_format = GG_PIXEL_PALETTE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_4_INT16_START
	      && end_signature == GG_ADAM7_4_INT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_4_INT32_START
	      && end_signature == GG_ADAM7_4_INT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_4_UINT16_START
	      && end_signature == GG_ADAM7_4_UINT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_4_UINT32_START
	      && end_signature == GG_ADAM7_4_UINT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_4_FLOAT_START
	      && end_signature == GG_ADAM7_4_FLOAT_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_4_DOUBLE_START
	      && end_signature == GG_ADAM7_4_DOUBLE_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 64;
	    }
	  break;
      case 5:
	  if (start_signature == GG_ADAM7_5_RGB_START
	      && end_signature == GG_ADAM7_5_RGB_END)
	    {
		pixel_format = GG_PIXEL_RGB;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
		samples_per_pixel = 3;
	    }
	  if (start_signature == GG_ADAM7_5_GRAYSCALE_START
	      && end_signature == GG_ADAM7_5_GRAYSCALE_END)
	    {
		pixel_format = GG_PIXEL_GRAYSCALE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_5_PALETTE_START
	      && end_signature == GG_ADAM7_5_PALETTE_END)
	    {
		pixel_format = GG_PIXEL_PALETTE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_5_INT16_START
	      && end_signature == GG_ADAM7_5_INT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_5_INT32_START
	      && end_signature == GG_ADAM7_5_INT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_5_UINT16_START
	      && end_signature == GG_ADAM7_5_UINT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_5_UINT32_START
	      && end_signature == GG_ADAM7_5_UINT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_5_FLOAT_START
	      && end_signature == GG_ADAM7_5_FLOAT_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_5_DOUBLE_START
	      && end_signature == GG_ADAM7_5_DOUBLE_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 64;
	    }
	  break;
      case 6:
	  if (start_signature == GG_ADAM7_6_RGB_START
	      && end_signature == GG_ADAM7_6_RGB_END)
	    {
		pixel_format = GG_PIXEL_RGB;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
		samples_per_pixel = 3;
	    }
	  if (start_signature == GG_ADAM7_6_GRAYSCALE_START
	      && end_signature == GG_ADAM7_6_GRAYSCALE_END)
	    {
		pixel_format = GG_PIXEL_GRAYSCALE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_6_PALETTE_START
	      && end_signature == GG_ADAM7_6_PALETTE_END)
	    {
		pixel_format = GG_PIXEL_PALETTE;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 8;
	    }
	  if (start_signature == GG_ADAM7_6_INT16_START
	      && end_signature == GG_ADAM7_6_INT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_6_INT32_START
	      && end_signature == GG_ADAM7_6_INT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_INT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_6_UINT16_START
	      && end_signature == GG_ADAM7_6_UINT16_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 16;
	    }
	  if (start_signature == GG_ADAM7_6_UINT32_START
	      && end_signature == GG_ADAM7_6_UINT32_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_UINT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_6_FLOAT_START
	      && end_signature == GG_ADAM7_6_FLOAT_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 32;
	    }
	  if (start_signature == GG_ADAM7_6_DOUBLE_START
	      && end_signature == GG_ADAM7_6_DOUBLE_END)
	    {
		pixel_format = GG_PIXEL_GRID;
		sample_format = GGRAPH_SAMPLE_FLOAT;
		bits_per_sample = 64;
	    }
	  break;
      };
    if (pixel_format == GG_PIXEL_UNKNOWN)
	return NULL;

/* retrieving image dimensions */
    p_in = mem_buf;
    p_in += sizeof (short);
    width = gg_import_int16 (p_in, endian, endian_arch);
    p_in += sizeof (short);
    height = gg_import_int16 (p_in, endian, endian_arch);
    p_in += sizeof (short);
    if (width == 0 || height == 0)
	return NULL;

/* checking the pix-buffer size */
    switch (pixel_format)
      {
      case GG_PIXEL_RGB:
      case GG_PIXEL_GRAYSCALE:
	  size = width * height * samples_per_pixel;
	  break;
      case GG_PIXEL_PALETTE:
	  size = width * height;
	  break;
      case GG_PIXEL_GRID:
	  switch (sample_format)
	    {
	    case GGRAPH_SAMPLE_INT:
		if (bits_per_sample == 16)
		    size = width * height * sizeof (short);
		else
		    size = width * height * sizeof (int);
		break;
	    case GGRAPH_SAMPLE_UINT:
		if (bits_per_sample == 16)
		    size = width * height * sizeof (unsigned short);
		else
		    size = width * height * sizeof (unsigned int);
		break;
	    case GGRAPH_SAMPLE_FLOAT:
		if (bits_per_sample == 32)
		    size = width * height * sizeof (float);
		else
		    size = width * height * sizeof (double);
		break;
	    };
	  break;
      };
    if (size != mem_buf_size - (4 * (int) sizeof (short)))
	return NULL;

/* allocating a new image */
    img =
	gg_image_create (pixel_format, width, height, bits_per_sample,
			 samples_per_pixel, sample_format, NULL, NULL);
    if (!img)
	return NULL;

    if (pixel_format == GG_PIXEL_GRID)
      {
	  /* extracting GRID-values accordingly to endianness */
	  short *p_short = (short *) (img->pixels);
	  int *p_int = (int *) (img->pixels);
	  unsigned short *p_ushort = (unsigned short *) (img->pixels);
	  unsigned int *p_uint = (unsigned int *) (img->pixels);
	  float *p_float = (float *) (img->pixels);
	  double *p_double = (double *) (img->pixels);
	  for (y = 0; y < height; y++)
	    {
		for (x = 0; x < width; x++)
		  {
		      switch (sample_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (bits_per_sample == 16)
			      {
				  short value = gg_import_int16 (p_in, endian,
								 endian_arch);
				  p_in += sizeof (short);
				  *p_short++ = value;
			      }
			    else
			      {
				  int value = gg_import_int32 (p_in, endian,
							       endian_arch);
				  p_in += sizeof (int);
				  *p_int++ = value;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (bits_per_sample == 16)
			      {
				  unsigned short value =
				      gg_import_uint16 (p_in, endian,
							endian_arch);
				  p_in += sizeof (unsigned short);
				  *p_ushort++ = value;
			      }
			    else
			      {
				  unsigned int value =
				      gg_import_uint32 (p_in, endian,
							endian_arch);
				  p_in += sizeof (unsigned int);
				  *p_uint++ = value;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (bits_per_sample == 32)
			      {
				  float value = gg_import_float (p_in, endian,
								 endian_arch);
				  p_in += sizeof (float);
				  *p_float++ = value;
			      }
			    else
			      {
				  double value = gg_import_double (p_in, endian,
								   endian_arch);
				  p_in += sizeof (double);
				  *p_double++ = value;
			      }
			    break;
			};
		  }
	    }
      }
    else
	memcpy (img->pixels, p_in, size);
    return img;
}

static int
check_adam7_subs (gGraphImagePtr im1, gGraphImagePtr im2)
{
/* checks if two Adam7 images are of the same type */
    if (im1->pixel_format != im2->pixel_format)
	return 0;
    if (im1->sample_format != im2->sample_format)
	return 0;
    if (im1->samples_per_pixel != im2->samples_per_pixel)
	return 0;
    if (im1->bits_per_sample != im2->bits_per_sample)
	return 0;
    return 1;
}

static int
get_adam7_dims_4 (gGraphImagePtr img_0, gGraphImagePtr img_1,
		  gGraphImagePtr img_2, int *width, int *height)
{
/* checking/computing Adam7 dimensions [1:4 scale] */
    int w;
    int h;
    int out_width;
    int out_height;

    out_width = (img_0->width * 2) - 1;
    out_height = (img_0->height * 2) - 1;

    w = img_1->width * 2;
    h = (img_0->height * 2) - 1;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    w = img_2->width;
    h = img_2->height * 2;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    *width = out_width;
    *height = out_height;
    return 1;
}

static int
get_adam7_dims_2 (gGraphImagePtr img_0, gGraphImagePtr img_1,
		  gGraphImagePtr img_2, gGraphImagePtr img_3,
		  gGraphImagePtr img_4, int *width, int *height)
{
/* checking/computing Adam7 dimensions [1:2 scale] */
    int w;
    int h;
    int out_width;
    int out_height;

    out_width = (img_0->width * 4) - 3;
    out_height = (img_0->height * 4) - 3;

    w = (img_1->width * 4) - 1;
    h = (img_0->height * 4) - 3;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    w = (img_2->width * 2) - 1;
    h = (img_2->height * 4) - 1;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    w = img_3->width * 2;
    h = (img_3->height * 2) - 1;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    w = img_4->width;
    h = img_4->height * 2;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    *width = out_width;
    *height = out_height;
    return 1;
}

static int
get_adam7_dims_1 (gGraphImagePtr img_0, gGraphImagePtr img_1,
		  gGraphImagePtr img_2, gGraphImagePtr img_3,
		  gGraphImagePtr img_4, gGraphImagePtr img_5,
		  gGraphImagePtr img_6, int *width, int *height)
{
/* computing Adam7 dimensions [1:1 scale] */
    int w;
    int h;
    int out_width;
    int out_height;

    out_width = (img_0->width * 8) - 7;
    out_height = (img_0->height * 8) - 7;

    w = (img_1->width * 8) - 3;
    h = (img_0->height * 8) - 7;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    w = (img_2->width * 4) - 3;
    h = (img_2->height * 8) - 3;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    w = (img_3->width * 4) - 1;
    h = (img_3->height * 4) - 3;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    w = (img_4->width * 2) - 1;
    h = (img_4->height * 4) - 1;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    w = img_5->width * 2;
    h = (img_5->height * 2) - 1;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    w = img_6->width;
    h = img_6->height * 2;
    if (w > out_width)
	out_width = w;
    if (h > out_height)
	out_height = h;

    *width = out_width;
    *height = out_height;
    return 1;
}

static gGraphImagePtr
merge_adam7_scale_1 (gGraphImagePtr img_0, gGraphImagePtr img_1,
		     gGraphImagePtr img_2, gGraphImagePtr img_3,
		     gGraphImagePtr img_4, gGraphImagePtr img_5,
		     gGraphImagePtr img_6, int width, int height)
{
/* decoding an Adam7 image (1:1) */
    int x;
    int y;
    unsigned char *p_in;
    unsigned char *p_out;
    gGraphImagePtr img = gg_image_create (img_0->pixel_format, width, height,
					  img_0->bits_per_sample,
					  img_0->samples_per_pixel,
					  img_0->sample_format, NULL,
					  NULL);
    if (!img)
	return NULL;

/* expanding SubImage-0 */
    for (y = 0; y < img_0->height; y++)
      {
	  p_in = img_0->pixels + (y * img_0->scanline_width);
	  p_out = img->pixels + ((y * 8) * img->scanline_width);
	  for (x = 0; x < img_0->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += (7 * 3);
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 8;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 8;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 8;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 8;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 8;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 8;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 8;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-1 */
    for (y = 0; y < img_1->height; y++)
      {
	  p_in = img_1->pixels + (y * img_1->scanline_width);
	  p_out =
	      img->pixels + ((y * 8) * img->scanline_width) +
	      (4 * img->pixel_size);
	  for (x = 0; x < img_1->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += (7 * 3);
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 8;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 8;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 8;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 8;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 8;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 8;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 8;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-2 */
    for (y = 0; y < img_2->height; y++)
      {
	  p_in = img_2->pixels + (y * img_2->scanline_width);
	  p_out = img->pixels + (((y * 8) + 4) * img->scanline_width);
	  for (x = 0; x < img_2->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += (3 * 3);
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 4;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 4;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 4;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 4;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 4;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 4;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 4;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-3 */
    for (y = 0; y < img_3->height; y++)
      {
	  p_in = img_3->pixels + (y * img_3->scanline_width);
	  p_out =
	      img->pixels + ((y * 4) * img->scanline_width) +
	      (2 * img->pixel_size);
	  for (x = 0; x < img_3->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += (3 * 3);
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 4;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 4;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 4;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 4;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 4;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 4;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 4;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-4 */
    for (y = 0; y < img_4->height; y++)
      {
	  p_in = img_4->pixels + (y * img_4->scanline_width);
	  p_out = img->pixels + (((y * 4) + 2) * img->scanline_width);
	  for (x = 0; x < img_4->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += 3;
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 2;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 2;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 2;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 2;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 2;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-5 */
    for (y = 0; y < img_5->height; y++)
      {
	  p_in = img_5->pixels + (y * img_5->scanline_width);
	  p_out =
	      img->pixels + ((y * 2) * img->scanline_width) + img->pixel_size;
	  for (x = 0; x < img_5->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += 3;
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 2;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 2;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 2;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 2;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 2;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-6 */
    for (y = 0; y < img_6->height; y++)
      {
	  p_in = img_6->pixels + (y * img_6->scanline_width);
	  p_out = img->pixels + (((y * 2) + 1) * img->scanline_width);
	  for (x = 0; x < img_6->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out++ = *p_in++;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short);
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int);
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short);
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int);
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float);
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double);
			      }
			    break;
			};
		      break;
		  };
	    }
      }

    return img;
}

static gGraphImagePtr
merge_adam7_scale_2 (gGraphImagePtr img_0, gGraphImagePtr img_1,
		     gGraphImagePtr img_2, gGraphImagePtr img_3,
		     gGraphImagePtr img_4, int width, int height)
{
/* decoding an Adam7 image (1:2) */
    int x;
    int y;
    unsigned char *p_in;
    unsigned char *p_out;
    gGraphImagePtr img = gg_image_create (img_0->pixel_format, width, height,
					  img_0->bits_per_sample,
					  img_0->samples_per_pixel,
					  img_0->sample_format, NULL,
					  NULL);
    if (!img)
	return NULL;

/* expanding SubImage-0 */
    for (y = 0; y < img_0->height; y++)
      {
	  p_in = img_0->pixels + (y * img_0->scanline_width);
	  p_out = img->pixels + ((y * 4) * img->scanline_width);
	  for (x = 0; x < img_0->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += (3 * 3);
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 4;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 4;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 4;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 4;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 4;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 4;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 4;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-1 */
    for (y = 0; y < img_1->height; y++)
      {
	  p_in = img_1->pixels + (y * img_1->scanline_width);
	  p_out =
	      img->pixels + ((y * 4) * img->scanline_width) +
	      (2 * img->pixel_size);
	  for (x = 0; x < img_1->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += (3 * 3);
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 4;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 4;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 4;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 4;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 4;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 4;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 4;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-2 */
    for (y = 0; y < img_2->height; y++)
      {
	  p_in = img_2->pixels + (y * img_2->scanline_width);
	  p_out = img->pixels + (((y * 4) + 2) * img->scanline_width);
	  for (x = 0; x < img_2->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += 3;
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 2;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 2;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 2;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 2;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 2;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-3 */
    for (y = 0; y < img_3->height; y++)
      {
	  p_in = img_3->pixels + (y * img_3->scanline_width);
	  p_out =
	      img->pixels + ((y * 2) * img->scanline_width) + img->pixel_size;
	  for (x = 0; x < img_3->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += 3;
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 2;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 2;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 2;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 2;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 2;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-4 */
    for (y = 0; y < img_4->height; y++)
      {
	  p_in = img_4->pixels + (y * img_4->scanline_width);
	  p_out = img->pixels + (((y * 2) + 1) * img->scanline_width);
	  for (x = 0; x < img_4->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out++ = *p_in++;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short);
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int);
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short);
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int);
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float);
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double);
			      }
			    break;
			};
		      break;
		  };
	    }
      }

    return img;
}

static gGraphImagePtr
merge_adam7_scale_4 (gGraphImagePtr img_0, gGraphImagePtr img_1,
		     gGraphImagePtr img_2, int width, int height)
{
/* decoding an Adam7 image (1:4) */
    int x;
    int y;
    unsigned char *p_in;
    unsigned char *p_out;
    gGraphImagePtr img = gg_image_create (img_0->pixel_format, width, height,
					  img_0->bits_per_sample,
					  img_0->samples_per_pixel,
					  img_0->sample_format, NULL,
					  NULL);
    if (!img)
	return NULL;

/* expanding SubImage-0 */
    for (y = 0; y < img_0->height; y++)
      {
	  p_in = img_0->pixels + (y * img_0->scanline_width);
	  p_out = img->pixels + ((y * 2) * img->scanline_width);
	  for (x = 0; x < img_0->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += 3;
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 2;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 2;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 2;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 2;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 2;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-1 */
    for (y = 0; y < img_1->height; y++)
      {
	  p_in = img_1->pixels + (y * img_1->scanline_width);
	  p_out =
	      img->pixels + ((y * 2) * img->scanline_width) + img->pixel_size;
	  for (x = 0; x < img_1->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      p_out += 3;
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out = *p_in++;
		      p_out += 2;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short) * 2;
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short) * 2;
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int) * 2;
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float) * 2;
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double) * 2;
			      }
			    break;
			};
		      break;
		  };
	    }
      }

/* expanding SubImage-2 */
    for (y = 0; y < img_2->height; y++)
      {
	  p_in = img_2->pixels + (y * img_2->scanline_width);
	  p_out = img->pixels + (((y * 2) + 1) * img->scanline_width);
	  for (x = 0; x < img_2->width; x++)
	    {
		switch (img->pixel_format)
		  {
		  case GG_PIXEL_RGB:
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      *p_out++ = *p_in++;
		      break;
		  case GG_PIXEL_GRAYSCALE:
		  case GG_PIXEL_PALETTE:
		      *p_out++ = *p_in++;
		      break;
		  case GG_PIXEL_GRID:
		      switch (img_0->pixel_format)
			{
			case GGRAPH_SAMPLE_INT:
			    if (img_0->bits_per_sample == 16)
			      {
				  short *p = (short *) p_in;
				  short value = *p;
				  p_in += sizeof (short);
				  p = (short *) p_out;
				  *p = value;
				  p_out += sizeof (short);
			      }
			    else
			      {
				  int *p = (int *) p_in;
				  int value = *p;
				  p_in += sizeof (int);
				  p = (int *) p_out;
				  *p = value;
				  p_out += sizeof (int);
			      }
			    break;
			case GGRAPH_SAMPLE_UINT:
			    if (img_0->bits_per_sample == 16)
			      {
				  unsigned short *p = (unsigned short *) p_in;
				  unsigned short value = *p;
				  p_in += sizeof (unsigned short);
				  p = (unsigned short *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned short);
			      }
			    else
			      {
				  unsigned int *p = (unsigned int *) p_in;
				  int value = *p;
				  p_in += sizeof (unsigned int);
				  p = (unsigned int *) p_out;
				  *p = value;
				  p_out += sizeof (unsigned int);
			      }
			    break;
			case GGRAPH_SAMPLE_FLOAT:
			    if (img_0->bits_per_sample == 32)
			      {
				  float *p = (float *) p_in;
				  float value = *p;
				  p_in += sizeof (float);
				  p = (float *) p_out;
				  *p = value;
				  p_out += sizeof (float);
			      }
			    else
			      {
				  double *p = (double *) p_in;
				  double value = *p;
				  p_in += sizeof (double);
				  p = (double *) p_out;
				  *p = value;
				  p_out += sizeof (double);
			      }
			    break;
			};
		      break;
		  };
	    }
      }

    return img;
}

static int
adam7_set_palette (gGraphImagePtr img, void *palette, int palette_size)
{
/* setting up a palette */
    unsigned char *p_in;
    int endian_arch = gg_endian_arch ();
    short palette_start;
    short palette_end;
    int i;

    if (img->pixel_format != GG_PIXEL_PALETTE)
	return 0;

/* checking signatures */
    if (palette_size < (int) ((sizeof (short) * 2) + 1))
	return 0;
    p_in = palette;
    palette_start = gg_import_int16 (p_in, 1, endian_arch);
    p_in = (unsigned char *) palette + palette_size - sizeof (short);
    palette_end = gg_import_int16 (p_in, 1, endian_arch);
    if (palette_start == GG_ADAM7_PALETTE_START
	&& palette_end == GG_ADAM7_PALETTE_END)
	;
    else
	return 0;

    p_in = (unsigned char *) palette + sizeof (short);
    img->max_palette = *p_in++;
    for (i = 0; i < img->max_palette; i++)
      {
	  img->palette_red[i] = *p_in++;
	  img->palette_green[i] = *p_in++;
	  img->palette_blue[i] = *p_in++;
      }
    return 1;
}

GGRAPH_DECLARE int
gGraphImageFromAdam7 (void *mem_bufs[7], int mem_buf_sizes[7], void *palette,
		      int palette_size, const void **image_handle, int scale)
{
/* decoding an image from Adam7 */
    gGraphImagePtr img_0 = NULL;
    gGraphImagePtr img_1 = NULL;
    gGraphImagePtr img_2 = NULL;
    gGraphImagePtr img_3 = NULL;
    gGraphImagePtr img_4 = NULL;
    gGraphImagePtr img_5 = NULL;
    gGraphImagePtr img_6 = NULL;
    gGraphImagePtr img;
    int width;
    int height;

    *image_handle = NULL;

/* checking sub-images */
    if (scale == 8)
      {
	  if (mem_bufs[0] == NULL || mem_buf_sizes[0] <= 0)
	      goto stop;
	  img_0 = adam7_decode (0, mem_bufs[0], mem_buf_sizes[0]);
	  if (!img_0)
	      goto stop;
	  if (img_0->pixel_format == GG_PIXEL_PALETTE)
	    {
		if (!adam7_set_palette (img_0, palette, palette_size))
		    goto stop;
	    }
	  *image_handle = img_0;
	  return GGRAPH_OK;
      }
    else if (scale == 4)
      {
	  if (mem_bufs[0] == NULL || mem_buf_sizes[0] <= 0)
	      goto stop;
	  if (mem_bufs[1] == NULL || mem_buf_sizes[1] <= 0)
	      goto stop;
	  if (mem_bufs[2] == NULL || mem_buf_sizes[2] <= 0)
	      goto stop;
	  img_0 = adam7_decode (0, mem_bufs[0], mem_buf_sizes[0]);
	  if (!img_0)
	      goto stop;
	  img_1 = adam7_decode (1, mem_bufs[1], mem_buf_sizes[1]);
	  if (!img_1)
	      goto stop;
	  img_2 = adam7_decode (2, mem_bufs[2], mem_buf_sizes[2]);
	  if (!img_2)
	      goto stop;
	  if (!check_adam7_subs (img_0, img_1))
	      goto stop;
	  if (!check_adam7_subs (img_0, img_2))
	      goto stop;
	  if (!get_adam7_dims_4 (img_0, img_1, img_2, &width, &height))
	      goto stop;
	  img = merge_adam7_scale_4 (img_0, img_1, img_2, width, height);
	  if (!img)
	      goto stop;
	  gGraphDestroyImage (img_0);
	  gGraphDestroyImage (img_1);
	  gGraphDestroyImage (img_2);
	  if (img->pixel_format == GG_PIXEL_PALETTE)
	    {
		if (!adam7_set_palette (img, palette, palette_size))
		    goto stop;
	    }
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    else if (scale == 2)
      {
	  if (mem_bufs[0] == NULL || mem_buf_sizes[0] <= 0)
	      goto stop;
	  if (mem_bufs[1] == NULL || mem_buf_sizes[1] <= 0)
	      goto stop;
	  if (mem_bufs[2] == NULL || mem_buf_sizes[2] <= 0)
	      goto stop;
	  if (mem_bufs[3] == NULL || mem_buf_sizes[3] <= 0)
	      goto stop;
	  if (mem_bufs[4] == NULL || mem_buf_sizes[4] <= 0)
	      goto stop;
	  img_0 = adam7_decode (0, mem_bufs[0], mem_buf_sizes[0]);
	  if (!img_0)
	      goto stop;
	  img_1 = adam7_decode (1, mem_bufs[1], mem_buf_sizes[1]);
	  if (!img_1)
	      goto stop;
	  img_2 = adam7_decode (2, mem_bufs[2], mem_buf_sizes[2]);
	  if (!img_2)
	      goto stop;
	  img_3 = adam7_decode (3, mem_bufs[3], mem_buf_sizes[3]);
	  if (!img_3)
	      goto stop;
	  img_4 = adam7_decode (4, mem_bufs[4], mem_buf_sizes[4]);
	  if (!img_4)
	      goto stop;
	  if (!check_adam7_subs (img_0, img_1))
	      goto stop;
	  if (!check_adam7_subs (img_0, img_2))
	      goto stop;
	  if (!check_adam7_subs (img_0, img_3))
	      goto stop;
	  if (!check_adam7_subs (img_0, img_4))
	      goto stop;
	  if (!get_adam7_dims_2
	      (img_0, img_1, img_2, img_3, img_4, &width, &height))
	      goto stop;
	  img =
	      merge_adam7_scale_2 (img_0, img_1, img_2, img_3, img_4, width,
				   height);
	  if (!img)
	      goto stop;
	  gGraphDestroyImage (img_0);
	  gGraphDestroyImage (img_1);
	  gGraphDestroyImage (img_2);
	  gGraphDestroyImage (img_3);
	  gGraphDestroyImage (img_4);
	  if (img->pixel_format == GG_PIXEL_PALETTE)
	    {
		if (!adam7_set_palette (img, palette, palette_size))
		    goto stop;
	    }
	  *image_handle = img;
	  return GGRAPH_OK;
      }
    else
      {
	  if (mem_bufs[0] == NULL || mem_buf_sizes[0] <= 0)
	      goto stop;
	  if (mem_bufs[1] == NULL || mem_buf_sizes[1] <= 0)
	      goto stop;
	  if (mem_bufs[2] == NULL || mem_buf_sizes[2] <= 0)
	      goto stop;
	  if (mem_bufs[3] == NULL || mem_buf_sizes[3] <= 0)
	      goto stop;
	  if (mem_bufs[4] == NULL || mem_buf_sizes[4] <= 0)
	      goto stop;
	  if (mem_bufs[5] == NULL || mem_buf_sizes[5] <= 0)
	      goto stop;
	  if (mem_bufs[6] == NULL || mem_buf_sizes[6] <= 0)
	      goto stop;
	  img_0 = adam7_decode (0, mem_bufs[0], mem_buf_sizes[0]);
	  if (!img_0)
	      goto stop;
	  img_1 = adam7_decode (1, mem_bufs[1], mem_buf_sizes[1]);
	  if (!img_1)
	      goto stop;
	  img_2 = adam7_decode (2, mem_bufs[2], mem_buf_sizes[2]);
	  if (!img_2)
	      goto stop;
	  img_3 = adam7_decode (3, mem_bufs[3], mem_buf_sizes[3]);
	  if (!img_3)
	      goto stop;
	  img_4 = adam7_decode (4, mem_bufs[4], mem_buf_sizes[4]);
	  if (!img_4)
	      goto stop;
	  img_5 = adam7_decode (5, mem_bufs[5], mem_buf_sizes[5]);
	  if (!img_5)
	      goto stop;
	  img_6 = adam7_decode (6, mem_bufs[6], mem_buf_sizes[6]);
	  if (!img_6)
	      goto stop;
	  if (!check_adam7_subs (img_0, img_1))
	      goto stop;
	  if (!check_adam7_subs (img_0, img_2))
	      goto stop;
	  if (!check_adam7_subs (img_0, img_3))
	      goto stop;
	  if (!check_adam7_subs (img_0, img_4))
	      goto stop;
	  if (!check_adam7_subs (img_0, img_5))
	      goto stop;
	  if (!check_adam7_subs (img_0, img_6))
	      goto stop;
	  if (!get_adam7_dims_1
	      (img_0, img_1, img_2, img_3, img_4, img_5, img_6, &width,
	       &height))
	      goto stop;
	  img =
	      merge_adam7_scale_1 (img_0, img_1, img_2, img_3, img_4, img_5,
				   img_6, width, height);
	  if (!img)
	      goto stop;
	  gGraphDestroyImage (img_0);
	  gGraphDestroyImage (img_1);
	  gGraphDestroyImage (img_2);
	  gGraphDestroyImage (img_3);
	  gGraphDestroyImage (img_4);
	  gGraphDestroyImage (img_5);
	  gGraphDestroyImage (img_6);
	  if (img->pixel_format == GG_PIXEL_PALETTE)
	    {
		if (!adam7_set_palette (img, palette, palette_size))
		    goto stop;
	    }
	  *image_handle = img;
	  return GGRAPH_OK;
      }

  stop:
    if (img_0)
	gGraphDestroyImage (img_0);
    if (img_1)
	gGraphDestroyImage (img_1);
    if (img_2)
	gGraphDestroyImage (img_2);
    if (img_3)
	gGraphDestroyImage (img_3);
    if (img_4)
	gGraphDestroyImage (img_4);
    if (img_5)
	gGraphDestroyImage (img_5);
    if (img_6)
	gGraphDestroyImage (img_6);
    return GGRAPH_INVALID_IMAGE;
}

GGRAPH_DECLARE int
gGraphImageToMonochrome (const void *ptr_img, void **mem_buf, int *mem_buf_size)
{
/* encoding an image as Monochrome */
    int line_width;
    int size;
    void *out;
    int endian_arch = gg_endian_arch ();
    unsigned char *p_in;
    unsigned char *p_out;
    int x;
    int y;
    gGraphImagePtr img = (gGraphImagePtr) ptr_img;

    *mem_buf = NULL;
    *mem_buf_size = 0;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img->pixel_format != GG_PIXEL_PALETTE)
	return GGRAPH_INVALID_IMAGE;

    if (img->max_palette != 2)
	return GGRAPH_INVALID_IMAGE;
    if (img->palette_red[0] == 0 && img->palette_green[0] == 0
	&& img->palette_blue[0] == 0 && img->palette_red[1] == 255
	&& img->palette_green[1] == 255 && img->palette_blue[1] == 255)
	;
    else if (img->palette_red[0] == 255 && img->palette_green[0] == 255
	     && img->palette_blue[0] == 255 && img->palette_red[1] == 0
	     && img->palette_green[1] == 0 && img->palette_blue[1] == 0)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    line_width = img->width / 8;
    if ((line_width * 8) < img->width)
	line_width++;
    size = line_width * img->height;
    size += 4 * sizeof (short);
    out = malloc (size);
    if (!out)
	goto stop;
    p_out = out;

/* 
* monochrome tile signatures are encoded as LITTLE-ENDIAN 
* but Width/Height are BID-ENDIAN encoded
*/
    gg_export_int16 (GG_MONOCHROME_START, p_out, 1, endian_arch);
    p_out += sizeof (short);
    gg_export_int16 (img->width, p_out, 0, endian_arch);
    p_out += sizeof (short);
    gg_export_int16 (img->height, p_out, 0, endian_arch);
    p_out += sizeof (short);

    for (y = 0; y < img->height; y++)
      {
	  unsigned char byte = 0x00;
	  int pos = 0;
	  p_in = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		int idx = *p_in++;
		if (img->palette_red[idx] == 0)
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
		      *p_out++ = byte;
		      byte = 0x00;
		      pos = 0;
		  }
	    }
	  if (pos > 0)		/* exporting the last octet */
	      *p_out++ = byte;
      }
    gg_export_int16 (GG_MONOCHROME_END, p_out, 1, endian_arch);

    *mem_buf = out;
    *mem_buf_size = size;
    return GGRAPH_OK;

  stop:
    return GGRAPH_INSUFFICIENT_MEMORY;
}

GGRAPH_DECLARE int
gGraphImageFromMonochrome (const void *mem_buf, int mem_buf_size,
			   const void **image_handle)
{
/* decoding an image from Monochrome */
    const unsigned char *p_in;
    int endian_arch = gg_endian_arch ();
    short start_signature;
    short end_signature;
    short width;
    short height;
    int x;
    int y;
    int line_width;
    gGraphImagePtr img;

    *image_handle = NULL;

    if (mem_buf_size < (int) (sizeof (short) * 4))
	return GGRAPH_INVALID_IMAGE;
/* 
* monochrome tile signatures are encoded as LITTLE-ENDIAN 
* but Width/Height are BID-ENDIAN encoded
*/

/* checking signatures */
    p_in = mem_buf;
    start_signature = gg_import_int16 (p_in, 1, endian_arch);
    p_in = (unsigned char *) mem_buf + mem_buf_size - sizeof (short);
    end_signature = gg_import_int16 (p_in, 1, endian_arch);
    if (start_signature == GG_MONOCHROME_START
	&& end_signature == GG_MONOCHROME_END)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    p_in = mem_buf;
    p_in += sizeof (short);
    width = gg_import_int16 (p_in, 0, endian_arch);
    p_in += sizeof (short);
    height = gg_import_int16 (p_in, 0, endian_arch);
    p_in += sizeof (short);

    line_width = width / 8;
    if ((line_width * 8) < width)
	line_width++;
    if ((line_width * height) != (int) (mem_buf_size - (4 * sizeof (short))))
	return GGRAPH_INVALID_IMAGE;

    img =
	gg_image_create (GG_PIXEL_PALETTE, width, height, 8, 1,
			 GGRAPH_SAMPLE_UINT, NULL, NULL);
    if (!img)
	return GGRAPH_INSUFFICIENT_MEMORY;

    img->max_palette = 2;
    img->palette_red[0] = 255;
    img->palette_green[0] = 255;
    img->palette_blue[0] = 255;
    img->palette_red[1] = 0;
    img->palette_green[1] = 0;
    img->palette_blue[1] = 0;

    for (y = 0; y < height; y++)
      {
	  unsigned char byte = 0x00;
	  unsigned char px;
	  int pixel;
	  int pos = 0;
	  unsigned char *p_out = img->pixels + (y * img->scanline_width);
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

    *image_handle = img;
    return GGRAPH_OK;

}
