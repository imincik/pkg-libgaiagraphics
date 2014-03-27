/* 
/ gaiagraphics_color_rules.c
/
/ GRIDS color rules helpers
/
/ version 1.0, 2010 August 31
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
#include <string.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <io.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

#define GG_MAX_THREADS		64

#define LANDSAT_RED		1
#define LANDSAT_GREEN	2
#define LANDSAT_BLUE	3
#define LANDSAT_PANCHRO	4

struct thread_grid_render
{
/* a struct used by grid rendering threads */
    gGraphColorMapPtr color_map;
    double no_data_value;
    int sample_format;
    int bits_per_sample;
    void *pixels;
    int base;
    int num_pixels;
    unsigned char *p_rgb;
};

struct thread_shaded_relief_render
{
/* a struct used by shader relief rendering threads */
    gGraphShadedReliefTripleRowPtr triple_row;
    double altRadians;
    double azRadians;
    int base;
    int num_pixels;
    unsigned char *p_rgb;
};

struct thread_landsat_recalibrate
{
/* a struct used by Landsat recalibrate */
    gGraphStripImagePtr img_red;
    gGraphStripImagePtr img_green;
    gGraphStripImagePtr img_blue;
    gGraphStripImagePtr img_out;
    int min_row;
    int max_row;
    int width;
    unsigned char value;
    double lmin_red;
    double lmax_red;
    double qcalmin_red;
    double qcalmax_red;
    int gain_low_red;
    double spectral_irradiance_red;
    double low_gain_factor_red;
    double high_gain_factor_red;
    unsigned char recalibration_min_red;
    unsigned char recalibration_max_red;
    double lmin_green;
    double lmax_green;
    double qcalmin_green;
    double qcalmax_green;
    int gain_low_green;
    double spectral_irradiance_green;
    double low_gain_factor_green;
    double high_gain_factor_green;
    unsigned char recalibration_min_green;
    unsigned char recalibration_max_green;
    double lmin_blue;
    double lmax_blue;
    double qcalmin_blue;
    double qcalmax_blue;
    int gain_low_blue;
    double spectral_irradiance_blue;
    double low_gain_factor_blue;
    double high_gain_factor_blue;
    unsigned char recalibration_min_blue;
    unsigned char recalibration_max_blue;
    double lmin_panchro;
    double lmax_panchro;
    double qcalmin_panchro;
    double qcalmax_panchro;
    int gain_low_panchro;
    double spectral_irradiance_panchro;
    double low_gain_factor_panchro;
    double high_gain_factor_panchro;
    unsigned char recalibration_min_panchro;
    unsigned char recalibration_max_panchro;
    double sun_distance;
    double sun_elevation;
    int band;
};

static int
color_rule_compare1 (double min1, double max1, double min2, double max2)
{
/* evaluting the range */
    if (min1 == min2 && max1 == max2)
	return 0;
    if (min1 > min2)
	return 1;
    if (min1 == min2 && max1 > max2)
	return 1;
    return -1;
}

static int
cmp_color_rules1 (const void *p1, const void *p2)
{
/* compares two Color Entries [for QSORT] */
    gGraphColorMapEntryPtr pR1 = *((gGraphColorMapEntryPtr *) p1);
    gGraphColorMapEntryPtr pR2 = *((gGraphColorMapEntryPtr *) p2);
    return color_rule_compare1 (pR1->min, pR2->max, pR2->min, pR2->max);
}

static int
color_rule_compare2 (double value, double min, double max)
{
/* evaluting the value */
    if (value < min)
	return -1;
    if (value > max)
	return 1;
    return 0;
}

static int
cmp_color_rules2 (const void *p1, const void *p2)
{
/* compares two Color Entries [for BSEARCH] */
    gGraphColorMapEntryPtr pR1 = (gGraphColorMapEntryPtr) p1;
    gGraphColorMapEntryPtr pR2 = *((gGraphColorMapEntryPtr *) p2);
    return color_rule_compare2 (pR1->min, pR2->min, pR2->max);
}

GGRAPH_PRIVATE gGraphColorRulePtr
gg_color_rule_create (void)
{
/* allocating an empty Color Rule */
    gGraphColorRulePtr ptr = malloc (sizeof (gGraphColorRule));
    if (!ptr)
	return NULL;
    ptr->signature = GG_COLOR_RULE_MAGIC_SIGNATURE;
    ptr->first = NULL;
    ptr->last = NULL;
    ptr->needs_range = 0;
    ptr->no_data_red = 255;
    ptr->no_data_green = 255;
    ptr->no_data_blue = 255;
    return ptr;
}

GGRAPH_PRIVATE void
gg_color_rule_destroy (gGraphColorRulePtr ptr)
{
/* destroying a Color Rule */
    gGraphColorRuleItemPtr p;
    gGraphColorRuleItemPtr pn;
    if (!ptr)
	return;
    p = ptr->first;
    while (p)
      {
	  pn = p->next;
	  free (p);
	  p = pn;
      }
    free (ptr);
}

GGRAPH_PRIVATE gGraphColorMapPtr
gg_color_map_create (void)
{
/* allocating an empty Color Map */
    gGraphColorMapPtr ptr = malloc (sizeof (gGraphColorMap));
    if (!ptr)
	return NULL;
    ptr->signature = GG_COLOR_MAP_MAGIC_SIGNATURE;
    ptr->first = NULL;
    ptr->last = NULL;
    ptr->no_data_red = 255;
    ptr->no_data_green = 255;
    ptr->no_data_blue = 255;
    ptr->not_found_red = 255;
    ptr->not_found_green = 255;
    ptr->not_found_blue = 255;
    ptr->num_entries = 0;
    ptr->array = NULL;
    return ptr;
}

GGRAPH_PRIVATE void
gg_color_map_destroy (gGraphColorMapPtr ptr)
{
/* destroying a Color Map */
    gGraphColorMapEntryPtr p;
    gGraphColorMapEntryPtr pn;
    if (!ptr)
	return;
    p = ptr->first;
    while (p)
      {
	  pn = p->next;
	  free (p);
	  p = pn;
      }
    if (ptr->array)
	free (ptr->array);
    free (ptr);
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
is_no_data_token (const char *token)
{
/* checking if this one is a NoData declaration */
    char buf[8];
    int i;
    if (strlen (token) != 2)
	return 0;
    strcpy (buf, token);
    for (i = 0; i < (int) strlen (buf); i++)
      {
	  if (buf[i] >= 'A' && buf[i] <= 'Z')
	      buf[i] = 'a' + (buf[i] - 'A');
      }
    if (strcmp (buf, "nv") == 0)
	return 1;
    return 0;
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

static int
is_percent_token (const char *token, double *perc_value)
{
/* checking if this one is a percentual value */
    if (token[strlen (token) - 1] == '%')
      {
	  double value;
	  char buf[128];
	  strcpy (buf, token);
	  buf[strlen (token) - 1] = '\0';
	  if (!token_to_double (buf, &value))
	      return 0;
	  *perc_value = value;
	  return 1;
      }
    return 0;
}

static int
is_color_name_token (const char *token, unsigned char *red,
		     unsigned char *green, unsigned char *blue)
{
/* checking if this one is a color name */
    char buf[32];
    int i;
    if (strlen (token) > 30)
	return 0;
    strcpy (buf, token);
    for (i = 0; i < (int) strlen (buf); i++)
      {
	  if (buf[i] >= 'A' && buf[i] <= 'Z')
	      buf[i] = 'a' + (buf[i] - 'A');
      }
    if (strcmp (buf, "black") == 0)
      {
	  *red = 0;
	  *green = 0;
	  *blue = 0;
	  return 1;
      }
    if (strcmp (buf, "white") == 0)
      {
	  *red = 255;
	  *green = 255;
	  *blue = 255;
	  return 1;
      }
    if (strcmp (buf, "red") == 0)
      {
	  *red = 255;
	  *green = 0;
	  *blue = 0;
	  return 1;
      }
    if (strcmp (buf, "lime") == 0)
      {
	  *red = 0;
	  *green = 255;
	  *blue = 0;
	  return 1;
      }
    if (strcmp (buf, "blue") == 0)
      {
	  *red = 0;
	  *green = 0;
	  *blue = 255;
	  return 1;
      }
    if (strcmp (buf, "yellow") == 0)
      {
	  *red = 255;
	  *green = 255;
	  *blue = 0;
	  return 1;
      }
    if (strcmp (buf, "magenta") == 0 || strcmp (buf, "fuchsia") == 0)
      {
	  *red = 255;
	  *green = 0;
	  *blue = 255;
	  return 1;
      }
    if (strcmp (buf, "cyan") == 0 || strcmp (buf, "aqua") == 0)
      {
	  *red = 0;
	  *green = 255;
	  *blue = 255;
	  return 1;
      }
    if (strcmp (buf, "orange") == 0)
      {
	  *red = 255;
	  *green = 128;
	  *blue = 0;
	  return 1;
      }
    if (strcmp (buf, "violet") == 0)
      {
	  *red = 128;
	  *green = 0;
	  *blue = 255;
	  return 1;
      }
    if (strcmp (buf, "purple") == 0)
      {
	  *red = 128;
	  *green = 0;
	  *blue = 128;
	  return 1;
      }
    if (strcmp (buf, "brown") == 0 || strcmp (buf, "maroon") == 0)
      {
	  *red = 128;
	  *green = 0;
	  *blue = 0;
	  return 1;
      }
    if (strcmp (buf, "indigo") == 0)
      {
	  *red = 0;
	  *green = 0;
	  *blue = 128;
	  return 1;
      }
    if (strcmp (buf, "green") == 0)
      {
	  *red = 0;
	  *green = 128;
	  *blue = 0;
	  return 1;
      }
    if (strcmp (buf, "navy") == 0)
      {
	  *red = 0;
	  *green = 0;
	  *blue = 128;
	  return 1;
      }
    if (strcmp (buf, "olive") == 0)
      {
	  *red = 128;
	  *green = 128;
	  *blue = 0;
	  return 1;
      }
    if (strcmp (buf, "teal") == 0)
      {
	  *red = 0;
	  *green = 128;
	  *blue = 128;
	  return 1;
      }
    if (strcmp (buf, "silver") == 0)
      {
	  *red = 192;
	  *green = 192;
	  *blue = 192;
	  return 1;
      }
    if (strcmp (buf, "gray") == 0)
      {
	  *red = 128;
	  *green = 128;
	  *blue = 128;
	  return 1;
      }
    return 0;
}

static gGraphColorRuleItemPtr
alloc_color_rule_item (double value, unsigned char percent, unsigned char red,
		       unsigned char green, unsigned char blue)
{
/* allocates and initializes a Color Rule Item */
    gGraphColorRuleItemPtr p = malloc (sizeof (gGraphColorRuleItem));
    if (!p)
	return NULL;
    if (percent)
      {
	  p->value = DBL_MAX;
	  p->percent_value = value;
      }
    else
      {
	  p->value = value;
	  p->percent_value = DBL_MAX;
      }
    p->is_percent = percent;
    p->red = red;
    p->green = green;
    p->blue = blue;
    p->next = NULL;
    return p;
}

static int
add_interval_to_color_rule (gGraphColorRulePtr ptr, double value,
			    unsigned char red, unsigned char green,
			    unsigned char blue)
{
/* adding an Interval [absolute value] to a Color Rule */
    gGraphColorRuleItemPtr p;
    if (!ptr)
	return 0;
    p = alloc_color_rule_item (value, 0, red, green, blue);
    if (!p)
	return 0;
    if (ptr->first == NULL)
	ptr->first = p;
    if (ptr->last != NULL)
	ptr->last->next = p;
    ptr->last = p;
    return 1;
}

static int
add_percent_to_color_rule (gGraphColorRulePtr ptr, double value,
			   unsigned char red, unsigned char green,
			   unsigned char blue)
{
/* adding an Interval [percent] to a Color Rule */
    gGraphColorRuleItemPtr p;
    if (!ptr)
	return 0;
    p = alloc_color_rule_item (value, 1, red, green, blue);
    if (!p)
	return 0;
    ptr->needs_range = 1;
    if (ptr->first == NULL)
	ptr->first = p;
    if (ptr->last != NULL)
	ptr->last->next = p;
    ptr->last = p;
    return 1;
}

GGRAPH_DECLARE int
gGraphColorRuleFromMemBuf (char *buf, const void **color_rule)
{
/* trying to load a Color Rule from a GRASS-like memory block [null terminated] */
    const char *p = buf;
    char in_buf[1024];
    char *token;
    char *p_in;
    int error;
    double threshold;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    int no_data;
    int percent;
    double perc_value;
    int color_name;
    char *saveptr;
    int i_tk;
    double value;
    int ret;
    gGraphColorRulePtr rule = NULL;

    *color_rule = NULL;
    rule = gg_color_rule_create ();
    if (!rule)
	return GGRAPH_INSUFFICIENT_MEMORY;

    p_in = in_buf;
    while (*p != '\0')
      {
	  /* parsing the Color Map block */
	  if (*p == '\r')
	    {
		p++;
		continue;
	    }
	  if (*p == '\n')
	    {
		*p_in = '\0';

		red = 0;
		green = 0;
		blue = 0;
		no_data = 0;
		percent = 0;
		color_name = 0;
		error = 0;

		if (*in_buf == '#')
		  {
		      /* skipping a comment line */
		      p_in = in_buf;
		      continue;
		  }

		i_tk = 0;
		saveptr = NULL;
		while (1)
		  {
		      /* breaking the string into separate tokens */
		      token = string_tokenizer (in_buf, ": \t", &saveptr);
		      if (token == NULL)
			  break;

		      if (i_tk == 0)
			{
			    if (is_no_data_token (token))
			      {
				  no_data = 1;
				  i_tk++;
				  continue;
			      }
			    if (is_percent_token (token, &perc_value))
			      {
				  percent = 1;
				  i_tk++;
				  continue;
			      }
			}
		      else
			{
			    color_name =
				is_color_name_token (token, &red, &green,
						     &blue);
			    if (color_name)
			      {
				  i_tk++;
				  continue;
			      }
			}

		      if (!token_to_double (token, &value))
			  error = 1;
		      else
			{
			    switch (i_tk)
			      {
			      case 0:
				  threshold = value;
				  break;
			      case 1:
				  if (value >= 0.0 && value <= 255.0)
				      red = (unsigned char) value;
				  else
				      error = 1;
				  break;
			      case 2:
				  if (value >= 0.0 && value <= 255.0)
				      green = (unsigned char) value;
				  else
				      error = 1;
				  break;
			      case 3:
				  if (value >= 0.0 && value <= 255.0)
				      blue = (unsigned char) value;
				  else
				      error = 1;
				  break;
			      default:
				  error = 1;
				  break;
			      };
			}

		      i_tk++;
		  }

		if (error)
		  {
		      ret = GGRAPH_ERROR;
		      goto stop;
		  }
		else if (no_data)
		  {
		      rule->no_data_red = red;
		      rule->no_data_green = green;
		      rule->no_data_blue = blue;
		  }
		else if (percent)
		  {
		      if (!add_percent_to_color_rule
			  (rule, perc_value, red, green, blue))
			{
			    ret = GGRAPH_INSUFFICIENT_MEMORY;
			    goto stop;
			}
		  }
		else
		  {
		      if (!add_interval_to_color_rule
			  (rule, threshold, red, green, blue))
			{
			    ret = GGRAPH_INSUFFICIENT_MEMORY;
			    goto stop;
			}
		  }

		p_in = in_buf;
		p++;
		continue;
	    }
	  *p_in++ = *p++;
      }
    *color_rule = rule;
    return GGRAPH_OK;

  stop:
    gg_color_rule_destroy (rule);
    return ret;
}

GGRAPH_DECLARE int
gGraphColorRuleFromFile (const char *path, const void **color_rule)
{
/* trying to load a Color Rule from a GRASS-like file */
    FILE *in = NULL;
    int c;
    char *buf;
    char *p;
    int ret;
    const void *handle;

    *color_rule = NULL;
    buf = malloc (1024 * 1024);
    if (!buf)
	return GGRAPH_INSUFFICIENT_MEMORY;

    in = fopen (path, "rb");
    if (in == NULL)
	return GGRAPH_FILE_OPEN_ERROR;

    p = buf;
    while ((c = getc (in)) != EOF)
      {
	  /* loading the Color Rule file */
	  *p++ = c;
      }
    *p = '\0';
    fclose (in);

    ret = gGraphColorRuleFromMemBuf (buf, &handle);
    free (buf);
    if (ret == GGRAPH_OK)
	*color_rule = handle;
    return ret;
}

GGRAPH_DECLARE void
gGraphDestroyColorRule (const void *ptr)
{
/* destroyng a Color Rule object */
    gGraphColorRulePtr rule = (gGraphColorRulePtr) ptr;

    if (rule == NULL)
	return;
    if (rule->signature == GG_COLOR_RULE_MAGIC_SIGNATURE)
      {
	  gg_color_rule_destroy (rule);
	  return;
      }
}

GGRAPH_DECLARE int
gGraphIsColorRuleRelative (const void *ptr, int *relative)
{
/* checking if this Color Rule is a Relative one */
    gGraphColorRulePtr rule = (gGraphColorRulePtr) ptr;

    if (rule == NULL)
	return GGRAPH_INVALID_COLOR_RULE;
    if (rule->signature != GG_COLOR_RULE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_COLOR_RULE;

    if (rule->needs_range)
	*relative = GGRAPH_TRUE;
    else
	*relative = GGRAPH_FALSE;
    return GGRAPH_OK;
}

static gGraphColorMapEntryPtr
alloc_color_map_entry (double min, double max, unsigned char red,
		       unsigned char green, unsigned char blue)
{
/* allocates and initializes a Color Map Entry object */
    gGraphColorMapEntryPtr p = malloc (sizeof (gGraphColorMapEntry));
    if (!p)
	return NULL;
    p->min = min;
    p->max = max;
    p->red = red;
    p->green = green;
    p->blue = blue;
    p->next = NULL;
    return p;
}

static gGraphColorMapEntryPtr
add_entry_to_color_map (gGraphColorMapPtr ptr, double min,
			double max, unsigned char red, unsigned char green,
			unsigned char blue)
{
/* adding an Entry to a Color Map object */
    gGraphColorMapEntryPtr entry;
    if (!ptr)
	return NULL;

    if (ptr->last)
      {
	  entry = ptr->last;
	  if (entry->red == red && entry->green == green && entry->blue == blue)
	    {
		/* extending the current range */
		if (entry->min > min)
		    entry->min = min;
		if (entry->max < max)
		    entry->max = max;
		return entry;
	    }
      }

    entry = alloc_color_map_entry (min, max, red, green, blue);
    if (!entry)
	return NULL;
    if (ptr->first == NULL)
	ptr->first = entry;
    if (ptr->last != NULL)
	ptr->last->next = entry;
    ptr->last = entry;
    return entry;
}

static int
resolve_interval (gGraphColorMapPtr ptr, double min, double max,
		  unsigned char min_red, unsigned char min_green,
		  unsigned char min_blue, unsigned char max_red,
		  unsigned char max_green, unsigned char max_blue)
{
/* resolving a color gradient */
    double range = max - min;
    double red_range;
    double green_range;
    double blue_range;
    double tik = range / 256.0;
    double red_tik;
    double green_tik;
    double blue_tik;
    int reverse_red;
    int reverse_green;
    int reverse_blue;
    int i;

    if (max_red >= min_red)
      {
	  red_range = max_red - min_red;
	  reverse_red = 0;
      }
    else
      {
	  red_range = min_red - max_red;
	  reverse_red = 1;
      }
    if (max_green >= min_green)
      {
	  green_range = max_green - min_green;
	  reverse_green = 0;
      }
    else
      {
	  green_range = min_green - max_green;
	  reverse_green = 1;
      }
    if (max_blue >= min_blue)
      {
	  blue_range = max_blue - min_blue;
	  reverse_blue = 0;
      }
    else
      {
	  blue_range = min_blue - max_blue;
	  reverse_blue = 1;
      }
    red_tik = red_range / 256.0;
    green_tik = green_range / 256.0;
    blue_tik = blue_range / 256.0;

    for (i = 0; i < 256; i++)
      {
	  /* generating range intervals */
	  double imin = min + (tik * (double) i);
	  double imax = imin + tik;
	  double red;
	  double green;
	  double blue;
	  if (reverse_red)
	      red = (double) min_red - (red_tik * (double) i);
	  else
	      red = (double) min_red + (red_tik * (double) i);
	  if (reverse_green)
	      green = (double) min_green - (green_tik * (double) i);
	  else
	      green = (double) min_green + (green_tik * (double) i);
	  if (reverse_blue)
	      blue = (double) min_blue - (blue_tik * (double) i);
	  else
	      blue = (double) min_blue + (blue_tik * (double) i);
	  if (i == 255)
	    {
		imax = max;
		red = max_red;
		green = max_green;
		blue = max_blue;
	    }
	  if (red < 0.0)
	      red = 0.0;
	  if (red > 255.0)
	      red = 255.0;
	  if (green < 0.0)
	      green = 0.0;
	  if (green > 255.0)
	      green = 255.0;
	  if (blue < 0.0)
	      blue = 0.0;
	  if (blue > 255.0)
	      blue = 255.0;
	  if (add_entry_to_color_map (ptr, imin, imax, (unsigned char) red,
				      (unsigned char) green,
				      (unsigned char) blue) == NULL)
	      return GGRAPH_ERROR;
      }
    return GGRAPH_OK;
}

static void
color_map_prepare (gGraphColorMapPtr map)
{
/* preparing the sorted array for Color Entries */
    int ind;
    gGraphColorMapEntryPtr entry;
    if (map->array)
	free (map->array);
    map->array = NULL;

    map->num_entries = 0;
    entry = map->first;
    while (entry)
      {
	  /* counting how many Entries are there */
	  map->num_entries += 1;
	  entry = entry->next;
      }
    if (map->num_entries <= 0)
	return;

    map->array = malloc (sizeof (gGraphColorMapEntryPtr) * map->num_entries);
    ind = 0;
    entry = map->first;
    while (entry)
      {
	  /* feeding the pointers array */
	  *(map->array + ind) = entry;
	  ind++;
	  entry = entry->next;
      }
/* sorting the array */
    qsort (map->array, map->num_entries, sizeof (gGraphColorMapEntryPtr),
	   cmp_color_rules1);
}

GGRAPH_DECLARE int
gGraphCreateColorMapAbsolute (const void *color_rule,
			      unsigned char background_red,
			      unsigned char background_green,
			      unsigned char background_blue,
			      const void **color_map)
{
/* attempting to create an ABSOLUTE Color Map */
    double last_value = DBL_MAX;
    unsigned char last_red;
    unsigned char last_green;
    unsigned char last_blue;
    gGraphColorMapPtr map;
    gGraphColorRuleItemPtr item;
    gGraphColorRulePtr rule = (gGraphColorRulePtr) color_rule;

    if (rule == NULL)
	return GGRAPH_INVALID_COLOR_RULE;
    if (rule->signature != GG_COLOR_RULE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_COLOR_RULE;
    if (rule->needs_range)
	return GGRAPH_INVALID_COLOR_RULE;

    *color_map = NULL;
    map = gg_color_map_create ();
    if (!map)
	return GGRAPH_INSUFFICIENT_MEMORY;
    map->no_data_red = background_red;
    map->no_data_green = background_green;
    map->no_data_blue = background_blue;
    map->not_found_red = background_red;
    map->not_found_green = background_green;
    map->not_found_blue = background_blue;

    item = rule->first;
    while (item)
      {
	  /* expanding intervals by color */
	  if (last_value != DBL_MAX && last_value != item->value)
	    {
		if (resolve_interval
		    (map, last_value, item->value, last_red, last_green,
		     last_blue, item->red, item->green,
		     item->blue) != GGRAPH_OK)
		    goto error;
	    }
	  last_value = item->value;
	  last_red = item->red;
	  last_green = item->green;
	  last_blue = item->blue;
	  item = item->next;
      }
    color_map_prepare (map);
    if (map->array == NULL)
	goto error;
    *color_map = map;
    return GGRAPH_OK;

  error:
    gg_color_map_destroy (map);
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphCreateColorMapRelative (const void *color_rule, double min, double max,
			      unsigned char background_red,
			      unsigned char background_green,
			      unsigned char background_blue,
			      const void **color_map)
{
/* attempting to create a RELATIVE Color Map */
    double last_value = DBL_MAX;
    unsigned char last_red;
    unsigned char last_green;
    unsigned char last_blue;
    double percent;
    gGraphColorMapPtr map;
    gGraphColorRuleItemPtr item;
    gGraphColorRulePtr rule = (gGraphColorRulePtr) color_rule;

    if (rule == NULL)
	return GGRAPH_INVALID_COLOR_RULE;
    if (rule->signature != GG_COLOR_RULE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_COLOR_RULE;
    if (rule->needs_range == 0)
	return GGRAPH_INVALID_COLOR_RULE;

    *color_map = NULL;
    map = gg_color_map_create ();
    if (!map)
	return GGRAPH_INSUFFICIENT_MEMORY;
    map->no_data_red = background_red;
    map->no_data_green = background_green;
    map->no_data_blue = background_blue;
    map->not_found_red = background_red;
    map->not_found_green = background_green;
    map->not_found_blue = background_blue;

    percent = (max - min) / 100.0;
    item = rule->first;
    while (item)
      {
	  /* tranforming percent values into absolute ones */
	  if (item->is_percent)
	      item->value = min + (percent * item->percent_value);
	  item = item->next;
      }

    item = rule->first;
    while (item)
      {
	  /* expanding intervals by color */
	  if (last_value != DBL_MAX && last_value != item->value)
	    {
		if (resolve_interval
		    (map, last_value, item->value, last_red, last_green,
		     last_blue, item->red, item->green,
		     item->blue) != GGRAPH_OK)
		    goto error;
	    }
	  last_value = item->value;
	  last_red = item->red;
	  last_green = item->green;
	  last_blue = item->blue;
	  item = item->next;
      }
    color_map_prepare (map);
    if (map->array == NULL)
	goto error;
    *color_map = map;
    return GGRAPH_OK;

  error:
    gg_color_map_destroy (map);
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE void
gGraphDestroyColorMap (const void *ptr)
{
/* destroyng a Color Map object */
    gGraphColorMapPtr map = (gGraphColorMapPtr) ptr;

    if (map == NULL)
	return;
    if (map->signature == GG_COLOR_MAP_MAGIC_SIGNATURE)
	gg_color_map_destroy (map);
}

GGRAPH_PRIVATE void
match_color (gGraphColorMapPtr map, double value, double no_data,
	     unsigned char *red, unsigned char *green, unsigned char *blue)
{
/* identifying a color corresponding to a value */
    gGraphColorMapEntryPtr *ret;
    gGraphColorMapEntryPtr p;
    gGraphColorMapEntry val;
    val.min = value;
    val.max = value;
    if (value == no_data)
      {
	  /* NoData */
	  *red = map->no_data_red;
	  *green = map->no_data_green;
	  *blue = map->no_data_blue;
	  return;
      }
/* dicotomic search */
    ret =
	(gGraphColorMapEntryPtr *) bsearch (&val, map->array,
					    map->num_entries,
					    sizeof (gGraphColorMapEntryPtr),
					    cmp_color_rules2);
    if (!ret)
      {
	  /* not found */
	  *red = map->not_found_red;
	  *green = map->not_found_green;
	  *blue = map->not_found_blue;
	  return;
      }
/* ok, match found */
    p = *ret;
    *red = p->red;
    *green = p->green;
    *blue = p->blue;
}

static void
do_grid_render (struct thread_grid_render *params)
{
/* actual function: rendering a GRID pixels block */
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char *p_out;
    short *p_int16;
    unsigned short *p_uint16;
    int *p_int32;
    unsigned int *p_uint32;
    float *p_float;
    double *p_double;
    double value;
    int x;

    if (params->sample_format == GGRAPH_SAMPLE_FLOAT)
      {
	  if (params->bits_per_sample == 32)
	    {
		p_float = (float *) (params->pixels);
		p_float += params->base;
	    }
	  else
	    {
		p_double = (double *) (params->pixels);
		p_double += params->base;
	    }
      }
    if (params->sample_format == GGRAPH_SAMPLE_INT)
      {
	  if (params->bits_per_sample == 32)
	    {
		p_int32 = (int *) (params->pixels);
		p_int32 += params->base;
	    }
	  else
	    {
		p_int16 = (short *) (params->pixels);
		p_int16 += params->base;
	    }
      }
    if (params->sample_format == GGRAPH_SAMPLE_UINT)
      {
	  if (params->bits_per_sample == 32)
	    {
		p_uint32 = (unsigned int *) (params->pixels);
		p_uint32 += params->base;
	    }
	  else
	    {
		p_uint16 = (unsigned short *) (params->pixels);
		p_uint16 += params->base;
	    }
      }
    p_out = params->p_rgb + (params->base * 3);
    for (x = 0; x < params->num_pixels; x++)
      {
	  if (params->sample_format == GGRAPH_SAMPLE_FLOAT)
	    {
		if (params->bits_per_sample == 32)
		    value = *p_float++;
		else
		    value = *p_double++;
	    }
	  if (params->sample_format == GGRAPH_SAMPLE_INT)
	    {
		if (params->bits_per_sample == 32)
		    value = *p_int32++;
		else
		    value = *p_int16++;
	    }
	  if (params->sample_format == GGRAPH_SAMPLE_UINT)
	    {
		if (params->bits_per_sample == 32)
		    value = *p_uint32++;
		else
		    value = *p_uint16++;
	    }
	  match_color (params->color_map, value, params->no_data_value, &red,
		       &green, &blue);
	  *p_out++ = red;
	  *p_out++ = green;
	  *p_out++ = blue;
      }
}

#ifdef _WIN32
static DWORD WINAPI
#else
static void *
#endif
grid_render (void *arg)
{
/* threaded function: rendering a GRID pixels block */
    struct thread_grid_render *params = (struct thread_grid_render *) arg;
    do_grid_render (params);
#ifdef _WIN32
    return 0;
#else
    pthread_exit (NULL);
#endif
}

GGRAPH_DECLARE int
gGraphStripImageRenderGridPixels (const void *in_ptr, const void *out_ptr,
				  const void *map, int num_threads)
{
/* rendering GRID pixels between two images */
    int y;
    short *p_int16;
    unsigned short *p_uint16;
    int *p_int32;
    unsigned int *p_uint32;
    float *p_float;
    double *p_double;
    void *p_void;
    unsigned char *p_out;
    int nt;
    struct thread_grid_render threads[GG_MAX_THREADS];
#ifdef _WIN32
    HANDLE thread_handles[GG_MAX_THREADS];
    DWORD dwThreadIdArray[GG_MAX_THREADS];
#else
    pthread_t thread_ids[GG_MAX_THREADS];
#endif
    gGraphStripImagePtr img_in = (gGraphStripImagePtr) in_ptr;
    gGraphStripImagePtr img_out = (gGraphStripImagePtr) out_ptr;
    gGraphColorMapPtr color_map = (gGraphColorMapPtr) map;

    if (img_in == NULL || img_out == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_out->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (map == NULL)
	return GGRAPH_INVALID_COLOR_MAP;
    if (color_map->signature != GG_COLOR_MAP_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_COLOR_MAP;

    if (num_threads > GG_MAX_THREADS)
	num_threads = GG_MAX_THREADS;
    if (num_threads < 1)
	num_threads = 1;

/* checking if the strip buffers does actually have the same size */
    if (img_in->width == img_out->width
	&& img_in->rows_per_block == img_out->rows_per_block)
	;
    else
	return GGRAPH_ERROR;
    if (img_in->pixel_format != GG_PIXEL_GRID)
	return GGRAPH_INVALID_IMAGE;
    if (img_out->pixel_format != GG_PIXEL_RGB)
	return GGRAPH_INVALID_IMAGE;

    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  if (img_in->sample_format == GGRAPH_SAMPLE_FLOAT)
	    {
		if (img_in->bits_per_sample == 32)
		  {
		      p_float = (float *) (img_in->pixels);
		      p_float += y * img_in->width;
		      p_void = p_float;
		  }
		else
		  {
		      p_double = (double *) (img_in->pixels);
		      p_double += y * img_in->width;
		      p_void = p_double;
		  }
	    }
	  if (img_in->sample_format == GGRAPH_SAMPLE_INT)
	    {
		if (img_in->bits_per_sample == 32)
		  {
		      p_int32 = (int *) (img_in->pixels);
		      p_int32 += y * img_in->width;
		      p_void = p_int32;
		  }
		else
		  {
		      p_int16 = (short *) (img_in->pixels);
		      p_int16 += y * img_in->width;
		      p_void = p_int16;
		  }
	    }
	  if (img_in->sample_format == GGRAPH_SAMPLE_UINT)
	    {
		if (img_in->bits_per_sample == 32)
		  {
		      p_uint32 = (unsigned int *) (img_in->pixels);
		      p_uint32 += y * img_in->width;
		      p_void = p_uint32;
		  }
		else
		  {
		      p_uint16 = (unsigned short *) (img_in->pixels);
		      p_uint16 += y * img_in->width;
		      p_void = p_uint16;
		  }
	    }
	  p_out = img_out->pixels + (y * img_out->scanline_width);

	  if (num_threads < 2)
	    {
		/* not using multithreading */
		threads[0].color_map = color_map;
		threads[0].no_data_value = img_in->no_data_value;
		threads[0].sample_format = img_in->sample_format;
		threads[0].bits_per_sample = img_in->bits_per_sample;
		threads[0].pixels = p_void;
		threads[0].base = 0;
		threads[0].num_pixels = img_in->width;
		threads[0].p_rgb = p_out;
		do_grid_render (&(threads[0]));
	    }
	  else
	    {
		/* using concurrent threads */
		int base = 0;
		int pixels_per_thread = img_in->width / num_threads;
		if ((pixels_per_thread * num_threads) < img_in->width)
		    pixels_per_thread++;
		for (nt = 0; nt < num_threads; nt++)
		  {
		      int num_pixels;
		      threads[nt].color_map = color_map;
		      threads[nt].no_data_value = img_in->no_data_value;
		      threads[nt].sample_format = img_in->sample_format;
		      threads[nt].bits_per_sample = img_in->bits_per_sample;
		      threads[nt].pixels = p_void;
		      threads[nt].base = base;
		      num_pixels = pixels_per_thread;
		      if (base + pixels_per_thread > img_in->width)
			  num_pixels = img_in->width - base;
		      threads[nt].num_pixels = num_pixels;
		      threads[nt].p_rgb = p_out;
		      base += pixels_per_thread;
		      if (num_pixels > 0)
			{
#ifdef _WIN32
			    thread_handles[nt] =
				CreateThread (NULL, 0, grid_render,
					      &(threads[nt]), 0,
					      &dwThreadIdArray[nt]);
#else
			    pthread_create (&(thread_ids[nt]), NULL,
					    grid_render, &(threads[nt]));
#endif
			}
		  }
		/* waiting until any concurrent thread terminates */
#ifdef _WIN32
		WaitForMultipleObjects (num_threads, thread_handles, TRUE,
					INFINITE);
#else
		for (nt = 0; nt < num_threads; nt++)
		    pthread_join (thread_ids[nt], NULL);
#endif
	    }
      }
    img_out->current_available_rows = img_in->current_available_rows;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphGetStripImageMinMaxValue (const void *in_ptr, double *min_value,
				double *max_value, double no_data_value)
{
/* determining MinMax values */
    int x;
    int y;
    short *p_int16;
    unsigned short *p_uint16;
    int *p_int32;
    unsigned int *p_uint32;
    float *p_float;
    double *p_double;
    double value;
    double min = DBL_MAX;
    double max = 0.0 - DBL_MAX;
    int save_row;
    gGraphStripImagePtr img_in = (gGraphStripImagePtr) in_ptr;

    *min_value = DBL_MAX;
    *max_value = 0.0 - DBL_MAX;
    if (img_in == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->pixel_format != GG_PIXEL_GRID)
	return GGRAPH_INVALID_IMAGE;

    save_row = img_in->next_row;
    img_in->next_row = 0;
    while (1)
      {
	  /* reading strips from input */
	  if (gGraphStripImageEOF (img_in) == GGRAPH_OK)
	      break;
	  if (gGraphReadNextStrip (img_in, NULL) != GGRAPH_OK)
	      goto error;
	  for (y = 0; y < img_in->current_available_rows; y++)
	    {
		if (img_in->sample_format == GGRAPH_SAMPLE_FLOAT)
		  {
		      if (img_in->bits_per_sample == 32)
			{
			    p_float = (float *) (img_in->pixels);
			    p_float += y * img_in->width;
			}
		      else
			{
			    p_double = (double *) (img_in->pixels);
			    p_double += y * img_in->width;
			}
		  }
		if (img_in->sample_format == GGRAPH_SAMPLE_INT)
		  {
		      if (img_in->bits_per_sample == 32)
			{
			    p_int32 = (int *) (img_in->pixels);
			    p_int32 += y * img_in->width;
			}
		      else
			{
			    p_int16 = (short *) (img_in->pixels);
			    p_int16 += y * img_in->width;
			}
		  }
		if (img_in->sample_format == GGRAPH_SAMPLE_UINT)
		  {
		      if (img_in->bits_per_sample == 32)
			{
			    p_uint32 = (unsigned int *) (img_in->pixels);
			    p_uint32 += y * img_in->width;
			}
		      else
			{
			    p_uint16 = (unsigned short *) (img_in->pixels);
			    p_uint16 += y * img_in->width;
			}
		  }
		for (x = 0; x < img_in->width; x++)
		  {
		      if (img_in->sample_format == GGRAPH_SAMPLE_FLOAT)
			{
			    if (img_in->bits_per_sample == 32)
				value = *p_float++;
			    else
				value = *p_double++;
			}
		      if (img_in->sample_format == GGRAPH_SAMPLE_INT)
			{
			    if (img_in->bits_per_sample == 32)
				value = *p_int32++;
			    else
				value = *p_int16++;
			}
		      if (img_in->sample_format == GGRAPH_SAMPLE_UINT)
			{
			    if (img_in->bits_per_sample == 32)
				value = *p_uint32++;
			    else
				value = *p_uint16++;
			}
		      if (value != no_data_value)
			{
			    if (value < min)
				min = value;
			    if (value > max)
				max = value;
			}
		  }
	    }
      }
    img_in->next_row = save_row;
    *min_value = min;
    *max_value = max;
    return GGRAPH_OK;

  error:
    img_in->next_row = save_row;
    return GGRAPH_ERROR;
}

GGRAPH_PRIVATE gGraphShadedReliefTripleRowPtr
gg_shaded_relief_triple_row_create (void)
{
/* allocating an empty Shaded Relief Triple Row object */
    gGraphShadedReliefTripleRowPtr ptr =
	malloc (sizeof (gGraphShadedReliefTripleRow));
    if (!ptr)
	return NULL;
    ptr->signature = GG_SHADED_RELIEF_3ROWS_MAGIC_SIGNATURE;
    ptr->width = -1;
    ptr->in_row1 = NULL;
    ptr->in_row2 = NULL;
    ptr->in_row3 = NULL;
    ptr->current_row = NULL;
    ptr->out_rgb = NULL;
    ptr->color_map = NULL;
    ptr->mono_red = 0;
    ptr->mono_green = 0;
    ptr->mono_blue = 0;
    ptr->z_factor = DBL_MAX;
    ptr->scale_factor = DBL_MAX;
    ptr->azimuth = DBL_MAX;
    ptr->altitude = DBL_MAX;
    ptr->no_data_value = DBL_MAX;
    ptr->no_red = 0;
    ptr->no_green = 0;
    ptr->no_blue = 0;
    return ptr;
}

GGRAPH_PRIVATE void
gg_shaded_relief_triple_row_destroy (gGraphShadedReliefTripleRowPtr ptr)
{
/* destroying a Shaded Relief Triple Row object */
    if (!ptr)
	return;
    if (ptr->in_row1)
	free (ptr->in_row1);
    if (ptr->in_row2)
	free (ptr->in_row2);
    if (ptr->in_row3)
	free (ptr->in_row3);
    if (ptr->out_rgb)
	free (ptr->out_rgb);
    free (ptr);
}

GGRAPH_DECLARE int
gGraphCreateShadedReliefTripleRow (int width, unsigned char background_red,
				   unsigned char background_green,
				   unsigned char background_blue,
				   double no_data, const void *color_map_handle,
				   unsigned char mono_red,
				   unsigned char mono_green,
				   unsigned char mono_blue, double z_factor,
				   double scale_factor, double azimuth,
				   double altitude,
				   const void **triple_row_handle)
{
/* creating a Shaded Relief Triple Row object */
    gGraphColorMapPtr map = (gGraphColorMapPtr) color_map_handle;
    gGraphShadedReliefTripleRowPtr triple_row =
	gg_shaded_relief_triple_row_create ();
    *triple_row_handle = NULL;
    if (!triple_row)
	return GGRAPH_INSUFFICIENT_MEMORY;

    if (map != NULL)
      {
	  if (map->signature != GG_COLOR_MAP_MAGIC_SIGNATURE)
	    {
		gg_shaded_relief_triple_row_destroy (triple_row);
		return GGRAPH_INVALID_COLOR_MAP;
	    }
      }

/* allocating the INPUT triple row buffer */
    triple_row->in_row1 = malloc (sizeof (float) * width);
    if (triple_row->in_row1 == NULL)
	goto error;
    triple_row->in_row2 = malloc (sizeof (float) * width);
    if (triple_row->in_row2 == NULL)
	goto error;
    triple_row->in_row3 = malloc (sizeof (float) * width);
    if (triple_row->in_row3 == NULL)
	goto error;
/* allocating the OUTPUT RGB buffer */
    triple_row->out_rgb = malloc (3 * (width - 2));
    if (triple_row->out_rgb == NULL)
	goto error;

    triple_row->width = width;
    triple_row->color_map = map;
    triple_row->no_red = background_red;
    triple_row->no_green = background_green;
    triple_row->no_blue = background_blue;
    triple_row->mono_red = mono_red;
    triple_row->mono_green = mono_green;
    triple_row->mono_blue = mono_blue;
    triple_row->z_factor = z_factor;
    triple_row->scale_factor = scale_factor;
    triple_row->azimuth = azimuth;
    triple_row->altitude = altitude;
    triple_row->no_data_value = no_data;
    *triple_row_handle = triple_row;
    return GGRAPH_OK;

  error:
    gg_shaded_relief_triple_row_destroy (triple_row);
    return GGRAPH_INSUFFICIENT_MEMORY;
}

GGRAPH_DECLARE void
gGraphDestroyShadedReliefTripleRow (const void *ptr)
{
/* destroyng a Shaded Relief Triple Row object */
    gGraphShadedReliefTripleRowPtr triple_row =
	(gGraphShadedReliefTripleRowPtr) ptr;

    if (triple_row == NULL)
	return;
    if (triple_row->signature == GG_SHADED_RELIEF_3ROWS_MAGIC_SIGNATURE)
	gg_shaded_relief_triple_row_destroy (triple_row);
}

static void
shaded_relief_triple_rotate (gGraphShadedReliefTripleRowPtr p)
{
/* rotating the Shaded Relief Triple Row scanlines */
    int x;
    float *p_out;
    if (p->current_row == NULL)
      {
	  p->current_row = p->in_row1;
	  goto no_data_fill;
      }
    if (p->current_row == p->in_row1)
      {
	  p->current_row = p->in_row2;
	  goto no_data_fill;
      }
    if (p->current_row == p->in_row2)
      {
	  p->current_row = p->in_row3;
	  goto no_data_fill;
      }
    p->current_row = p->in_row1;
    p->in_row1 = p->in_row2;
    p->in_row2 = p->in_row3;
    p->in_row3 = p->current_row;
  no_data_fill:
    p_out = p->current_row;
    for (x = 0; x < p->width; x++)
	*p_out++ = p->no_data_value;
}

GGRAPH_DECLARE int
gGraphStripImageGetShadedReliefScanline (const void *in_img_handle,
					 int row_index,
					 const void *triple_row_handle)
{
/* inserting a new scanline into the Shaded Relief Triple Row object */
    int x;
    short *p_int16;
    unsigned short *p_uint16;
    int *p_int32;
    unsigned int *p_uint32;
    float *p_float;
    double *p_double;
    float pixel;
    float *p_out;
    gGraphShadedReliefTripleRowPtr triple_row =
	(gGraphShadedReliefTripleRowPtr) triple_row_handle;
    gGraphStripImagePtr img = (gGraphStripImagePtr) in_img_handle;

    if (triple_row == NULL)
	return GGRAPH_INVALID_SHADED_RELIEF_3ROWS;
    if (triple_row->signature != GG_SHADED_RELIEF_3ROWS_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_SHADED_RELIEF_3ROWS;
    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img->pixel_format != GG_PIXEL_GRID)
	return GGRAPH_INVALID_IMAGE;
    if (img->width != triple_row->width)
	return GGRAPH_INVALID_IMAGE;
    if (row_index >= 0 && row_index < img->rows_per_block)
	;
    else
	return GGRAPH_ERROR;

    shaded_relief_triple_rotate (triple_row);

    if (img->sample_format == GGRAPH_SAMPLE_FLOAT)
      {
	  if (img->bits_per_sample == 32)
	    {
		p_float = (float *) (img->pixels);
		p_float += row_index * img->width;
	    }
	  else
	    {
		p_double = (double *) (img->pixels);
		p_double += row_index * img->width;
	    }
      }
    if (img->sample_format == GGRAPH_SAMPLE_INT)
      {
	  if (img->bits_per_sample == 32)
	    {
		p_int32 = (int *) (img->pixels);
		p_int32 += row_index * img->width;
	    }
	  else
	    {
		p_int16 = (short *) (img->pixels);
		p_int16 += row_index * img->width;
	    }
      }
    if (img->sample_format == GGRAPH_SAMPLE_UINT)
      {
	  if (img->bits_per_sample == 32)
	    {
		p_uint32 = (unsigned int *) (img->pixels);
		p_uint32 += row_index * img->width;
	    }
	  else
	    {
		p_uint16 = (unsigned short *) (img->pixels);
		p_uint16 += row_index * img->width;
	    }
      }
    p_out = triple_row->current_row;
    for (x = 0; x < img->width; x++)
      {
	  if (img->sample_format == GGRAPH_SAMPLE_FLOAT)
	    {
		if (img->bits_per_sample == 32)
		    pixel = *p_float++;
		else
		    pixel = *p_double++;
	    }
	  if (img->sample_format == GGRAPH_SAMPLE_INT)
	    {
		if (img->bits_per_sample == 32)
		    pixel = *p_int32++;
		else
		    pixel = *p_int16++;
	    }
	  if (img->sample_format == GGRAPH_SAMPLE_UINT)
	    {
		if (img->bits_per_sample == 32)
		    pixel = *p_uint32++;
		else
		    pixel = *p_uint16++;
	    }
	  *p_out++ = pixel;
      }
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphStripImageSetShadedReliefScanline (const void *triple_row_handle,
					 const void *out_img_handle,
					 int row_index)
{
/* copying out a Shaded Relief processed scanline */
    int x;
    unsigned char *p_in;
    unsigned char *p_out;
    gGraphShadedReliefTripleRowPtr triple_row =
	(gGraphShadedReliefTripleRowPtr) triple_row_handle;
    gGraphStripImagePtr img = (gGraphStripImagePtr) out_img_handle;

    if (triple_row == NULL)
	return GGRAPH_INVALID_SHADED_RELIEF_3ROWS;
    if (triple_row->signature != GG_SHADED_RELIEF_3ROWS_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_SHADED_RELIEF_3ROWS;
    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img->pixel_format != GG_PIXEL_RGB)
	return GGRAPH_INVALID_IMAGE;
    if (img->width != triple_row->width - 2)
	return GGRAPH_INVALID_IMAGE;
    if (row_index >= 0 && row_index < img->rows_per_block)
	;
    else
	return GGRAPH_ERROR;

    p_in = triple_row->out_rgb;
    p_out = img->pixels + (row_index * img->scanline_width);
    for (x = 0; x < img->width; x++)
      {
	  *p_out++ = *p_in++;	/* RED */
	  *p_out++ = *p_in++;	/* GREEN */
	  *p_out++ = *p_in++;	/* BLUE */
      }
    img->current_available_rows = row_index + 1;
    return GGRAPH_OK;
}

static void
do_shaded_relief_render (struct thread_shaded_relief_render *params)
{
/* performing actual rendering (1 single pixel) */
    int j;
    int n;
    double x;
    double y;
    double aspect;
    double slope;
    double cang;
    int gray;
    float afWin[9];
    double red;
    double green;
    double blue;
    double alpha;
    int bContainsNull;
    unsigned char *p_out;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    gGraphShadedReliefTripleRowPtr triple_row;

    p_out = params->p_rgb + (params->base * 3);
    triple_row = params->triple_row;

/* 
* Shaded Relief rendering
*
*****************************************************************
*
* DISCLAIMER: the following code is largely based upon Even Rouault's 
* gdaldem.c 
* wich was originally released on public domain for GRASS, and is
* now included into GDAL (X/MIT license)
*
*/


/*
/ Move a 3x3 pafWindow over each cell 
/ (where the cell in question is #4)
/ 
/      0 1 2
/      3 4 5
/      6 7 8
*/
    for (j = 0; j < params->num_pixels; j++)
      {
	  /* Exclude the edges */
	  int k = j + params->base;
	  if (j == 0 || j == params->num_pixels - 1)
	      continue;

	  afWin[0] = triple_row->in_row1[k - 1];
	  afWin[1] = triple_row->in_row1[k];
	  afWin[2] = triple_row->in_row1[k + 1];
	  afWin[3] = triple_row->in_row2[k - 1];
	  afWin[4] = triple_row->in_row2[k];
	  afWin[5] = triple_row->in_row2[k + 1];
	  afWin[6] = triple_row->in_row3[k - 1];
	  afWin[7] = triple_row->in_row3[k];
	  afWin[8] = triple_row->in_row3[k + 1];

	  bContainsNull = 0;
	  for (n = 0; n <= 8; n++)
	    {
		if (afWin[n] == triple_row->no_data_value)
		  {
		      bContainsNull = 1;
		      break;
		  }
	    }

	  if (bContainsNull)
	    {
		/* We have nulls so write nullValue and move on */
		r = triple_row->no_red;
		g = triple_row->no_green;
		b = triple_row->no_blue;
	    }
	  else
	    {
		/* We have a valid 3x3 window. */

		/* ---------------------------------------
		 * Compute Hillshade
		 */

		/* First Slope ... */
		x = triple_row->z_factor *
		    ((afWin[0] + afWin[3] + afWin[3] + afWin[6]) -
		     (afWin[2] + afWin[5] + afWin[5] +
		      afWin[8])) / (8.0 * triple_row->scale_factor);

		y = triple_row->z_factor *
		    ((afWin[6] + afWin[7] + afWin[7] + afWin[8]) -
		     (afWin[0] + afWin[1] + afWin[1] +
		      afWin[2])) / (8.0 * triple_row->scale_factor);

		slope = M_PI / 2 - atan (sqrt (x * x + y * y));

		/* ... then aspect... */
		aspect = atan2 (x, y);

		/* ... then the shade value */
		cang = sin (params->altRadians) * sin (slope) +
		    cos (params->altRadians) * cos (slope) *
		    cos (params->azRadians - M_PI / 2 - aspect);

		if (cang <= 0.0)
		    cang = 1.0;
		else
		    cang = 1.0 + (254.0 * cang);

		if (triple_row->color_map != NULL)
		  {
		      /* Color + Shaded Relief rendering */
		      match_color (triple_row->color_map, afWin[4],
				   triple_row->no_data_value, &r, &g, &b);
		      alpha = cang / 255.0;
		      red = (double) r *alpha;
		      green = (double) g *alpha;
		      blue = (double) b *alpha;
		      if (red < 0.0)
			  red = 0.0;
		      if (green < 0.0)
			  green = 0.0;
		      if (blue < 0.0)
			  blue = 0.0;
		      if (red > 255.0)
			  red = 255.0;
		      if (green > 255.0)
			  green = 255.0;
		      if (blue > 255.0)
			  blue = 255.0;
		      r = (unsigned char) red;
		      g = (unsigned char) green;
		      b = (unsigned) blue;
		  }
		else
		  {
		      if ((triple_row->mono_red == 0
			   && triple_row->mono_green == 0
			   && triple_row->mono_blue == 0)
			  || (triple_row->mono_red == 255
			      && triple_row->mono_green == 255
			      && triple_row->mono_blue == 255))
			{
			    /* plain gray-scale */
			    gray = (int) cang;
			    r = (unsigned char) gray;
			    g = (unsigned char) gray;
			    b = (unsigned) gray;
			}
		      else
			{
			    /* using the monochrome base color + ALPHA */
			    alpha = cang / 255.0;
			    red = (double) (triple_row->mono_red) * alpha;
			    green = (double) (triple_row->mono_green) * alpha;
			    blue = (double) (triple_row->mono_blue) * alpha;
			    if (red < 0.0)
				red = 0.0;
			    if (green < 0.0)
				green = 0.0;
			    if (blue < 0.0)
				blue = 0.0;
			    if (red > 255.0)
				red = 255.0;
			    if (green > 255.0)
				green = 255.0;
			    if (blue > 255.0)
				blue = 255.0;
			    r = (unsigned char) red;
			    g = (unsigned char) green;
			    b = (unsigned) blue;
			}
		  }
	    }
	  *p_out++ = r;
	  *p_out++ = g;
	  *p_out++ = b;
      }
}

#ifdef _WIN32
static DWORD WINAPI
#else
static void *
#endif
shaded_relief_render (void *arg)
{
/* threaded function: rendering a Shaded Relief pixel */
    struct thread_shaded_relief_render *params =
	(struct thread_shaded_relief_render *) arg;
    do_shaded_relief_render (params);
#ifdef _WIN32
    return 0;
#else
    pthread_exit (NULL);
#endif
}

GGRAPH_DECLARE int
gGraphShadedReliefRenderPixels (const void *triple_row_handle, int num_threads,
				int *out_row_ready)
{
/* rendering a Shaded Relief scanline */
    double degreesToRadians;
    unsigned char *p_out;
    int nt;
    struct thread_shaded_relief_render threads[GG_MAX_THREADS];
#ifdef _WIN32
    HANDLE thread_handles[GG_MAX_THREADS];
    DWORD dwThreadIdArray[GG_MAX_THREADS];
#else
    pthread_t thread_ids[GG_MAX_THREADS];
#endif
    gGraphShadedReliefTripleRowPtr triple_row =
	(gGraphShadedReliefTripleRowPtr) triple_row_handle;

    if (triple_row == NULL)
	return GGRAPH_INVALID_SHADED_RELIEF_3ROWS;
    if (triple_row->signature != GG_SHADED_RELIEF_3ROWS_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_SHADED_RELIEF_3ROWS;

    if (triple_row->current_row != triple_row->in_row3)
      {
	  /* not yet ready ( < 3 rows into the buffer) */
	  *out_row_ready = GGRAPH_FALSE;
	  return GGRAPH_OK;
      }

    if (num_threads > GG_MAX_THREADS)
	num_threads = GG_MAX_THREADS;
    if (num_threads < 1)
	num_threads = 1;
    degreesToRadians = M_PI / 180.0;

    p_out = triple_row->out_rgb;
    if (num_threads < 2)
      {
	  /* not using multithreading */
	  threads[0].triple_row = triple_row;
	  threads[0].altRadians = triple_row->altitude * degreesToRadians;
	  threads[0].azRadians = triple_row->azimuth * degreesToRadians;
	  threads[0].base = 0;
	  threads[0].num_pixels = triple_row->width;
	  threads[0].p_rgb = p_out;
	  do_shaded_relief_render (&(threads[0]));
      }
    else
      {
	  /* using concurrent threads */
	  int base = 0;
	  int pixels_per_thread = triple_row->width / num_threads;
	  if ((pixels_per_thread * num_threads) < triple_row->width)
	      pixels_per_thread++;
	  for (nt = 0; nt < num_threads; nt++)
	    {
		int num_pixels;
		threads[nt].triple_row = triple_row;
		threads[nt].altRadians =
		    triple_row->altitude * degreesToRadians;
		threads[nt].azRadians = triple_row->azimuth * degreesToRadians;
		if (nt == 0)
		    threads[nt].base = base;
		else
		    threads[nt].base = base - 1;
		num_pixels = pixels_per_thread + 1;
		if (base + pixels_per_thread > triple_row->width)
		    num_pixels = triple_row->width - base;
		threads[nt].num_pixels = num_pixels;
		threads[nt].p_rgb = p_out;
		base += pixels_per_thread;
		if (num_pixels > 0)
		  {
#ifdef _WIN32
		      thread_handles[nt] =
			  CreateThread (NULL, 0, shaded_relief_render,
					&(threads[nt]), 0,
					&dwThreadIdArray[nt]);
#else
		      pthread_create (&(thread_ids[nt]), NULL,
				      shaded_relief_render, &(threads[nt]));
#endif
		  }
	    }
	  /* waiting until any concurrent thread terminates */
#ifdef _WIN32
	  WaitForMultipleObjects (num_threads, thread_handles, TRUE, INFINITE);
#else
	  for (nt = 0; nt < num_threads; nt++)
	      pthread_join (thread_ids[nt], NULL);
#endif
      }

    *out_row_ready = GGRAPH_TRUE;
    return GGRAPH_OK;
}

static void
landsat_recalibrate (struct thread_landsat_recalibrate *p)
{
/* recalibrating color */
    double lmin;
    double lmax;
    double qcalmin;
    double qcalmax;
    double radiance;
    double albedo;
    double spectral_irradiance;
    double rads;
    double recalibrate;
    double recalibrate_min;
    double recalibrate_max;
    double gain;
    double sample = p->value;

    switch (p->band)
      {
      case LANDSAT_RED:
	  lmin = p->lmin_red;
	  lmax = p->lmax_red;
	  qcalmin = p->qcalmin_red;
	  qcalmax = p->qcalmax_red;
	  spectral_irradiance = p->spectral_irradiance_red;
	  if (p->gain_low_red)
	      gain = p->low_gain_factor_red;
	  else
	      gain = p->high_gain_factor_red;
	  recalibrate_min = p->recalibration_min_red;
	  recalibrate_max = p->recalibration_max_red;
	  break;
      case LANDSAT_GREEN:
	  lmin = p->lmin_green;
	  lmax = p->lmax_green;
	  qcalmin = p->qcalmin_green;
	  qcalmax = p->qcalmax_green;
	  spectral_irradiance = p->spectral_irradiance_green;
	  if (p->gain_low_green)
	      gain = p->low_gain_factor_green;
	  else
	      gain = p->high_gain_factor_green;
	  recalibrate_min = p->recalibration_min_green;
	  recalibrate_max = p->recalibration_max_green;
	  break;
      case LANDSAT_BLUE:
	  lmin = p->lmin_blue;
	  lmax = p->lmax_blue;
	  qcalmin = p->qcalmin_blue;
	  qcalmax = p->qcalmax_blue;
	  spectral_irradiance = p->spectral_irradiance_blue;
	  if (p->gain_low_blue)
	      gain = p->low_gain_factor_blue;
	  else
	      gain = p->high_gain_factor_blue;
	  recalibrate_min = p->recalibration_min_blue;
	  recalibrate_max = p->recalibration_max_blue;
	  break;
      case LANDSAT_PANCHRO:
	  lmin = p->lmin_panchro;
	  lmax = p->lmax_panchro;
	  qcalmin = p->qcalmin_panchro;
	  qcalmax = p->qcalmax_panchro;
	  spectral_irradiance = p->spectral_irradiance_panchro;
	  if (p->gain_low_panchro)
	      gain = p->low_gain_factor_panchro;
	  else
	      gain = p->high_gain_factor_panchro;
	  recalibrate_min = p->recalibration_min_panchro;
	  recalibrate_max = p->recalibration_max_panchro;
      };

/* converting Solar Zenith angle into Radians */
    rads = (90.0 - p->sun_elevation) * (M_PI / 180.0);
/* computing RADIANCE value */
    radiance =
	((lmax - lmin) / (qcalmax - qcalmin)) * (sample - qcalmin) + lmin;
/* computing REFLECTANCE aka ALBEDO value */
    albedo = M_PI * radiance * (p->sun_distance * p->sun_distance);
    albedo /= spectral_irradiance * cos (rads);
/* normalizing range [1-255] */
    sample = (qcalmax - qcalmin) * albedo * gain;
/* recalibrating (discarding both range extremities) */
    recalibrate =
	((sample - recalibrate_min) * (qcalmax - qcalmin)) / (recalibrate_max -
							      recalibrate_min);
    sample = recalibrate;
    if (sample > qcalmax)
	sample = qcalmax;
    if (sample < qcalmin)
	sample = qcalmin;
    p->value = (unsigned char) sample;
}

static void
landsat_rgb (struct thread_landsat_recalibrate *ptr)
{
/* precessing an RGB Landsat sub-strip */
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char *p_red;
    unsigned char *p_green;
    unsigned char *p_blue;
    unsigned char *p_rgb;
    struct thread_landsat_recalibrate p;

/* setting up the recalibrate struct */
    p.lmin_red = ptr->lmin_red;
    p.lmax_red = ptr->lmax_red;
    p.qcalmin_red = ptr->qcalmin_red;
    p.qcalmax_red = ptr->qcalmax_red;
    p.gain_low_red = ptr->gain_low_red;
    p.spectral_irradiance_red = ptr->spectral_irradiance_red;
    p.low_gain_factor_red = ptr->low_gain_factor_red;
    p.high_gain_factor_red = ptr->high_gain_factor_red;
    p.recalibration_min_red = ptr->recalibration_min_red;
    p.recalibration_max_red = ptr->recalibration_max_red;
    p.lmin_green = ptr->lmin_green;
    p.lmax_green = ptr->lmax_green;
    p.qcalmin_green = ptr->qcalmin_green;
    p.qcalmax_green = ptr->qcalmax_green;
    p.gain_low_green = ptr->gain_low_green;
    p.spectral_irradiance_green = ptr->spectral_irradiance_green;
    p.low_gain_factor_green = ptr->low_gain_factor_green;
    p.high_gain_factor_green = ptr->high_gain_factor_green;
    p.recalibration_min_green = ptr->recalibration_min_green;
    p.recalibration_max_green = ptr->recalibration_max_green;
    p.lmin_blue = ptr->lmin_blue;
    p.lmax_blue = ptr->lmax_blue;
    p.qcalmin_blue = ptr->qcalmin_blue;
    p.qcalmax_blue = ptr->qcalmax_blue;
    p.gain_low_blue = ptr->gain_low_blue;
    p.spectral_irradiance_blue = ptr->spectral_irradiance_blue;
    p.low_gain_factor_blue = ptr->low_gain_factor_blue;
    p.high_gain_factor_blue = ptr->high_gain_factor_blue;
    p.recalibration_min_blue = ptr->recalibration_min_blue;
    p.recalibration_max_blue = ptr->recalibration_max_blue;
    p.sun_distance = ptr->sun_distance;
    p.sun_elevation = ptr->sun_elevation;

    for (y = ptr->min_row; y < ptr->max_row; y++)
      {
	  p_red = ptr->img_red->pixels + (y * ptr->img_red->scanline_width);
	  p_green =
	      ptr->img_green->pixels + (y * ptr->img_green->scanline_width);
	  p_blue = ptr->img_blue->pixels + (y * ptr->img_blue->scanline_width);
	  p_rgb = ptr->img_out->pixels + (y * ptr->img_out->scanline_width);
	  for (x = 0; x < ptr->width; x++)
	    {
		red = *p_red++;
		green = *p_green++;
		blue = *p_blue++;
		if (red == 0 || green == 0 || blue == 0)
		  {
		      red = 0;
		      green = 0;
		      blue = 0;
		  }
		else
		  {
		      p.value = red;
		      p.band = LANDSAT_RED;
		      landsat_recalibrate (&p);
		      red = p.value;
		      p.value = green;
		      p.band = LANDSAT_GREEN;
		      landsat_recalibrate (&p);
		      green = p.value;
		      p.value = blue;
		      p.band = LANDSAT_BLUE;
		      landsat_recalibrate (&p);
		      blue = p.value;
		  }
		*p_rgb++ = red;
		*p_rgb++ = green;
		*p_rgb++ = blue;
	    }
      }
}

#ifdef _WIN32
static DWORD WINAPI
#else
static void *
#endif
landsat_rgb_recalibrate (void *arg)
{
/* threaded function: rendering an RGB Landsat sub-strip */
    struct thread_landsat_recalibrate *params =
	(struct thread_landsat_recalibrate *) arg;
    landsat_rgb (params);
#ifdef _WIN32
    return 0;
#else
    pthread_exit (NULL);
#endif
}

GGRAPH_DECLARE int
gGraphLandsatRGB (const void *red_ptr, const void *green_ptr,
		  const void *blue_ptr, const void *rgb_ptr, int width,
		  int num_rows, gGraphLandsatRecalibrationPtr params,
		  int num_threads)
{
/* processing a whole RGB Landsat strip (may be, in a multithreaded way) */

    gGraphStripImagePtr img_red = (gGraphStripImagePtr) red_ptr;
    gGraphStripImagePtr img_green = (gGraphStripImagePtr) green_ptr;
    gGraphStripImagePtr img_blue = (gGraphStripImagePtr) blue_ptr;
    gGraphStripImagePtr img_rgb = (gGraphStripImagePtr) rgb_ptr;
    int nt;
    struct thread_landsat_recalibrate threads[GG_MAX_THREADS];
#ifdef _WIN32
    HANDLE thread_handles[GG_MAX_THREADS];
    DWORD dwThreadIdArray[GG_MAX_THREADS];
#else
    pthread_t thread_ids[GG_MAX_THREADS];
#endif

    if (img_red == NULL || img_green == NULL || img_blue == NULL
	|| img_rgb == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_red->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_green->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_blue->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_rgb->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_red->pixel_format == GG_PIXEL_GRAYSCALE
	&& img_green->pixel_format == GG_PIXEL_GRAYSCALE
	&& img_blue->pixel_format == GG_PIXEL_GRAYSCALE
	&& img_rgb->pixel_format == GG_PIXEL_RGB)
	;
    else
	return GGRAPH_INVALID_IMAGE;
    if (img_red->width == width && img_green->width == width
	&& img_blue->width == width && img_rgb->width == width)
	;
    else
	return GGRAPH_INVALID_IMAGE;
    if (img_red->current_available_rows == num_rows
	&& img_green->current_available_rows == num_rows
	&& img_blue->current_available_rows == num_rows
	&& img_rgb->rows_per_block >= num_rows)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (num_threads > GG_MAX_THREADS)
	num_threads = GG_MAX_THREADS;
    if (num_threads < 1)
	num_threads = 1;

    for (nt = 0; nt < num_threads; nt++)
      {
	  /* setting up the recalibrate struct */
	  threads[nt].img_red = img_red;
	  threads[nt].img_green = img_green;
	  threads[nt].img_blue = img_blue;
	  threads[nt].img_out = img_rgb;
	  threads[nt].width = width;
	  threads[nt].lmin_red = params->lmin_red;
	  threads[nt].lmax_red = params->lmax_red;
	  threads[nt].qcalmin_red = params->qcalmin_red;
	  threads[nt].qcalmax_red = params->qcalmax_red;
	  threads[nt].gain_low_red = params->is_gain_low_red;
	  threads[nt].spectral_irradiance_red = params->spectral_irradiance_red;
	  threads[nt].low_gain_factor_red = params->low_gain_factor_red;
	  threads[nt].high_gain_factor_red = params->high_gain_factor_red;
	  threads[nt].recalibration_min_red = params->recalibration_min_red;
	  threads[nt].recalibration_max_red = params->recalibration_max_red;
	  threads[nt].lmin_green = params->lmin_green;
	  threads[nt].lmax_green = params->lmax_green;
	  threads[nt].qcalmin_green = params->qcalmin_green;
	  threads[nt].qcalmax_green = params->qcalmax_green;
	  threads[nt].gain_low_green = params->is_gain_low_green;
	  threads[nt].spectral_irradiance_green =
	      params->spectral_irradiance_green;
	  threads[nt].low_gain_factor_green = params->low_gain_factor_green;
	  threads[nt].high_gain_factor_green = params->high_gain_factor_green;
	  threads[nt].recalibration_min_green = params->recalibration_min_green;
	  threads[nt].recalibration_max_green = params->recalibration_max_green;
	  threads[nt].lmin_blue = params->lmin_blue;
	  threads[nt].lmax_blue = params->lmax_blue;
	  threads[nt].qcalmin_blue = params->qcalmin_blue;
	  threads[nt].qcalmax_blue = params->qcalmax_blue;
	  threads[nt].gain_low_blue = params->is_gain_low_blue;
	  threads[nt].spectral_irradiance_blue =
	      params->spectral_irradiance_blue;
	  threads[nt].low_gain_factor_blue = params->low_gain_factor_blue;
	  threads[nt].high_gain_factor_blue = params->high_gain_factor_blue;
	  threads[nt].recalibration_min_blue = params->recalibration_min_blue;
	  threads[nt].recalibration_max_blue = params->recalibration_max_blue;
	  threads[nt].sun_distance = params->sun_distance;
	  threads[nt].sun_elevation = params->sun_elevation;
      }

    if (num_threads == 1)
      {
	  /* not using concurrent multithreading */
	  threads[0].min_row = 0;
	  threads[0].max_row = num_rows;
	  landsat_rgb (&(threads[0]));
      }
    else
      {
	  /* using concurrent multithreading */
	  int base_row = 0;
	  int rows_per_thread = num_rows / num_threads;
	  if ((rows_per_thread * num_threads) < num_rows)
	      rows_per_thread++;
	  for (nt = 0; nt < num_threads; nt++)
	    {
		int max_row = base_row + rows_per_thread;
		if (max_row >= num_rows)
		    max_row = num_rows;
		threads[nt].min_row = base_row;
		threads[nt].max_row = max_row;
		base_row += rows_per_thread;
#ifdef _WIN32
		thread_handles[nt] =
		    CreateThread (NULL, 0, landsat_rgb_recalibrate,
				  &(threads[nt]), 0, &dwThreadIdArray[nt]);
#else
		pthread_create (&(thread_ids[nt]), NULL,
				landsat_rgb_recalibrate, &(threads[nt]));
#endif
	    }
	  /* waiting until any concurrent thread terminates */
#ifdef _WIN32
	  WaitForMultipleObjects (num_threads, thread_handles, TRUE, INFINITE);
#else
	  for (nt = 0; nt < num_threads; nt++)
	      pthread_join (thread_ids[nt], NULL);
#endif
      }

    return GGRAPH_OK;
}

static void
landsat_bw (struct thread_landsat_recalibrate *ptr)
{
/* precessing a B&W Landsat sub-strip */
    int x;
    int y;
    unsigned char gray;
    unsigned char *p_in;
    unsigned char *p_out;
    struct thread_landsat_recalibrate p;

/* setting up the recalibrate struct */
    p.lmin_panchro = ptr->lmin_panchro;
    p.lmax_panchro = ptr->lmax_panchro;
    p.qcalmin_panchro = ptr->qcalmin_panchro;
    p.qcalmax_panchro = ptr->qcalmax_panchro;
    p.gain_low_panchro = ptr->gain_low_panchro;
    p.spectral_irradiance_panchro = ptr->spectral_irradiance_panchro;
    p.low_gain_factor_panchro = ptr->low_gain_factor_panchro;
    p.high_gain_factor_panchro = ptr->high_gain_factor_panchro;
    p.recalibration_min_panchro = ptr->recalibration_min_panchro;
    p.recalibration_max_panchro = ptr->recalibration_max_panchro;
    p.sun_distance = ptr->sun_distance;
    p.sun_elevation = ptr->sun_elevation;

    for (y = ptr->min_row; y < ptr->max_row; y++)
      {
	  p_in = ptr->img_red->pixels + (y * ptr->img_red->scanline_width);
	  p_out = ptr->img_out->pixels + (y * ptr->img_out->scanline_width);
	  for (x = 0; x < ptr->width; x++)
	    {
		gray = *p_in++;
		if (gray == 0)
		    gray = 0;
		else
		  {
		      p.value = gray;
		      p.band = LANDSAT_PANCHRO;
		      landsat_recalibrate (&p);
		      gray = p.value;
		  }
		*p_out++ = gray;
	    }
      }
}

#ifdef _WIN32
static DWORD WINAPI
#else
static void *
#endif
landsat_bw_recalibrate (void *arg)
{
/* threaded function: rendering a B&W Landsat sub-strip */
    struct thread_landsat_recalibrate *params =
	(struct thread_landsat_recalibrate *) arg;
    landsat_bw (params);
#ifdef _WIN32
    return 0;
#else
    pthread_exit (NULL);
#endif
}

GGRAPH_DECLARE int
gGraphLandsatBW (const void *in_ptr, const void *out_ptr, int width,
		 int num_rows, gGraphLandsatRecalibrationPtr params,
		 int num_threads)
{
/* processing a whole B&W Landsat strip (may be, in a multithreaded way) */
    gGraphStripImagePtr img_in = (gGraphStripImagePtr) in_ptr;
    gGraphStripImagePtr img_out = (gGraphStripImagePtr) out_ptr;
    int nt;
    struct thread_landsat_recalibrate threads[GG_MAX_THREADS];
#ifdef _WIN32
    HANDLE thread_handles[GG_MAX_THREADS];
    DWORD dwThreadIdArray[GG_MAX_THREADS];
#else
    pthread_t thread_ids[GG_MAX_THREADS];
#endif

    if (img_in == NULL || img_out == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_out->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->pixel_format == GG_PIXEL_GRAYSCALE
	&& img_out->pixel_format == GG_PIXEL_GRAYSCALE)
	;
    else
	return GGRAPH_INVALID_IMAGE;
    if (img_in->width == width && img_out->width == width)
	;
    else
	return GGRAPH_INVALID_IMAGE;
    if (img_in->current_available_rows == num_rows
	&& img_out->rows_per_block >= num_rows)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    if (num_threads > GG_MAX_THREADS)
	num_threads = GG_MAX_THREADS;
    if (num_threads < 1)
	num_threads = 1;

    for (nt = 0; nt < num_threads; nt++)
      {
	  /* setting up the recalibrate struct */
	  threads[nt].img_red = img_in;
	  threads[nt].img_out = img_out;
	  threads[nt].width = width;
	  threads[nt].lmin_panchro = params->lmin_panchro;
	  threads[nt].lmax_panchro = params->lmax_panchro;
	  threads[nt].qcalmin_panchro = params->qcalmin_panchro;
	  threads[nt].qcalmax_panchro = params->qcalmax_panchro;
	  threads[nt].gain_low_panchro = params->is_gain_low_panchro;
	  threads[nt].spectral_irradiance_panchro =
	      params->spectral_irradiance_panchro;
	  threads[nt].low_gain_factor_panchro = params->low_gain_factor_panchro;
	  threads[nt].high_gain_factor_panchro =
	      params->high_gain_factor_panchro;
	  threads[nt].recalibration_min_panchro =
	      params->recalibration_min_panchro;
	  threads[nt].recalibration_max_panchro =
	      params->recalibration_max_panchro;
	  threads[nt].sun_distance = params->sun_distance;
	  threads[nt].sun_elevation = params->sun_elevation;
      }

    if (num_threads == 1)
      {
	  /* not using concurrent multithreading */
	  threads[0].min_row = 0;
	  threads[0].max_row = num_rows;
	  landsat_bw (&(threads[0]));
      }
    else
      {
	  /* using concurrent multithreading */
	  int base_row = 0;
	  int rows_per_thread = num_rows / num_threads;
	  if ((rows_per_thread * num_threads) < num_rows)
	      rows_per_thread++;
	  for (nt = 0; nt < num_threads; nt++)
	    {
		int max_row = base_row + rows_per_thread;
		if (max_row >= num_rows)
		    max_row = num_rows;
		threads[nt].min_row = base_row;
		threads[nt].max_row = max_row;
		base_row += rows_per_thread;
#ifdef _WIN32
		thread_handles[nt] =
		    CreateThread (NULL, 0, landsat_bw_recalibrate,
				  &(threads[nt]), 0, &dwThreadIdArray[nt]);
#else
		pthread_create (&(thread_ids[nt]), NULL,
				landsat_bw_recalibrate, &(threads[nt]));
#endif
	    }
	  /* waiting until any concurrent thread terminates */
#ifdef _WIN32
	  WaitForMultipleObjects (num_threads, thread_handles, TRUE, INFINITE);
#else
	  for (nt = 0; nt < num_threads; nt++)
	      pthread_join (thread_ids[nt], NULL);
#endif
      }

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphGetLandsatSceneExtent (const void *ptr, int base_row, double *top_x,
			     double *top_y, double *bottom_x, double *bottom_y,
			     double *left_x, double *left_y, double *right_x,
			     double *right_y)
{
/* determing the actual Landsat scene extent (by strips) */
    int x;
    int y;
    unsigned char *p_in;
    unsigned char gray;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    double px;
    double py;
    gGraphStripImagePtr img = (gGraphStripImagePtr) ptr;

    if (img == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img->signature != GG_STRIP_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img->pixel_format == GG_PIXEL_GRAYSCALE
	|| img->pixel_format == GG_PIXEL_RGB)
	;
    else
	return GGRAPH_INVALID_IMAGE;

    *top_x = 0.0 - DBL_MAX;
    *top_y = 0.0 - DBL_MAX;
    *bottom_x = DBL_MAX;
    *bottom_y = DBL_MAX;
    *left_x = DBL_MAX;
    *left_y = DBL_MAX;
    *right_x = 0.0 - DBL_MAX;
    *right_y = 0.0 - DBL_MAX;

    for (y = 0; y < img->current_available_rows; y++)
      {
	  p_in = img->pixels + (y * img->scanline_width);
	  for (x = 0; x < img->width; x++)
	    {
		if (img->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      gray = *p_in++;
		      if (gray == 0)
			  continue;
		  }
		else
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      if (red == 0 || green == 0 || blue == 0)
			  continue;
		  }
		/* ok, this one is a valid scene's pixel */
		px = img->upper_left_x + ((double) x * img->pixel_x_size);
		py = img->upper_left_y -
		    ((double) (base_row + y) * img->pixel_y_size);
		if (py > *top_y)
		  {
		      *top_y = py;
		      *top_x = px;
		  }
		if (py < *bottom_y)
		  {
		      *bottom_y = py;
		      *bottom_x = px;
		  }
		if (px < *left_x)
		  {
		      *left_x = px;
		      *left_y = py;
		  }
		if (px > *right_x)
		  {
		      *right_x = px;
		      *right_y = py;
		  }
	    }
      }
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphLandsatMergePixels (const void *ptr_in, int base_row, const void *ptr_out)
{
/* merging pixels between two images by Geographic position */
    int x;
    int y;
    unsigned char *p_in;
    unsigned char *p_out;
    unsigned char gray;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    double geo_x;
    double geo_y;
    int out_x;
    int out_y;
    double dout_x;
    double dout_y;
    double diff_x;
    double diff_y;
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
    if (img_in->pixel_format == GG_PIXEL_GRAYSCALE
	|| img_in->pixel_format == GG_PIXEL_RGB)
	;
    else
	return GGRAPH_INVALID_IMAGE;
    if (img_in->pixel_format != img_out->pixel_format)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->srid != img_out->srid)
	return GGRAPH_INVALID_IMAGE;

    for (y = 0; y < img_in->current_available_rows; y++)
      {
	  p_in = img_in->pixels + (y * img_in->scanline_width);
	  for (x = 0; x < img_in->width; x++)
	    {
		if (img_in->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      gray = *p_in++;
		      if (gray == 0)
			  continue;
		  }
		else
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      if (red == 0 || green == 0 || blue == 0)
			  continue;
		  }
		/* ok, this one is a valid scene's pixel */
		geo_x =
		    img_in->upper_left_x + ((double) x * img_in->pixel_x_size);
		geo_y =
		    img_in->upper_left_y -
		    ((double) (base_row + y) * img_in->pixel_y_size);
		dout_x =
		    ((geo_x - img_out->upper_left_x) / img_out->pixel_x_size);
		dout_y =
		    ((img_out->upper_left_y - geo_y) / img_out->pixel_y_size);
		out_x = (int) dout_x;
		out_y = (int) dout_y;
		diff_x = dout_x - (double) out_x;
		diff_y = dout_y - (double) out_y;
		if (diff_x >= 0.5)
		    out_x++;
		if (diff_y >= 0.5)
		    out_y++;
		if (out_x < 0 || out_x >= img_out->width)
		    continue;
		if (out_y < 0 || out_y >= img_out->height)
		    continue;
		/* ok, setting this pixel into the output image */
		p_out =
		    img_out->pixels + (out_y * img_out->scanline_width) +
		    (out_x * img_out->pixel_size);
		if (img_out->pixel_format == GG_PIXEL_GRAYSCALE)
		    *p_out++ = gray;
		else
		  {
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
	    }
      }

    return GGRAPH_OK;
}

static unsigned char
to_grayscale3 (unsigned char r, unsigned char g, unsigned char b)
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

static int
merge_grid (gGraphImagePtr img_in, gGraphImagePtr img_out)
{
/* mergine pixels between two GRIDS by Geographics position */
    int x;
    int y;
    unsigned char *p_in;
    unsigned char *p_out;
    double value;
    double geo_x;
    double geo_y;
    int out_x;
    int out_y;
    double dout_x;
    double dout_y;
    double diff_x;
    double diff_y;
    for (y = 0; y < img_in->height; y++)
      {
	  p_in = img_in->pixels + (y * img_in->scanline_width);
	  for (x = 0; x < img_in->width; x++)
	    {
		switch (img_in->sample_format)
		  {
		  case GGRAPH_SAMPLE_INT:
		      if (img_in->bits_per_sample == 16)
			{
			    short *p = (short *) p_in;
			    value = *p;
			    p_in += sizeof (short);
			}
		      else
			{
			    int *p = (int *) p_in;
			    value = *p;
			    p_in += sizeof (int);
			}
		      break;
		  case GGRAPH_SAMPLE_UINT:
		      if (img_in->bits_per_sample == 16)
			{
			    unsigned short *p = (unsigned short *) p_in;
			    value = *p;
			    p_in += sizeof (unsigned short);
			}
		      else
			{
			    unsigned int *p = (unsigned int *) p_in;
			    value = *p;
			    p_in += sizeof (unsigned int);
			}
		      break;
		  case GGRAPH_SAMPLE_FLOAT:
		      if (img_in->bits_per_sample == 32)
			{
			    float *p = (float *) p_in;
			    value = *p;
			    p_in += sizeof (float);
			}
		      else
			{
			    double *p = (double *) p_in;
			    value = *p;
			    p_in += sizeof (double);
			}
		      break;
		  };
		if (value == img_in->no_data_value)
		  {
		      /* NoData pixel: skipping */
		      continue;
		  }
		/* ok, this one is a valid scene's pixel */
		geo_x =
		    img_in->upper_left_x + ((double) x * img_in->pixel_x_size);
		geo_y =
		    img_in->upper_left_y - ((double) y * img_in->pixel_y_size);
		dout_x =
		    ((geo_x - img_out->upper_left_x) / img_out->pixel_x_size);
		dout_y =
		    ((img_out->upper_left_y - geo_y) / img_out->pixel_y_size);
		out_x = (int) dout_x;
		out_y = (int) dout_y;
		diff_x = dout_x - (double) out_x;
		diff_y = dout_y - (double) out_y;
		if (diff_x >= 0.5)
		    out_x++;
		if (diff_y >= 0.5)
		    out_y++;
		if (out_x < 0 || out_x >= img_out->width)
		    continue;
		if (out_y < 0 || out_y >= img_out->height)
		    continue;
		/* ok, setting this pixel into the output image */
		p_out =
		    img_out->pixels + (out_y * img_out->scanline_width) +
		    (out_x * img_out->pixel_size);
		switch (img_out->sample_format)
		  {
		  case GGRAPH_SAMPLE_INT:
		      if (img_out->bits_per_sample == 16)
			{
			    short *p = (short *) p_out;
			    *p = (short) value;
			    p_out += sizeof (short);
			}
		      else
			{
			    int *p = (int *) p_out;
			    *p = (int) value;
			    p_out += sizeof (int);
			}
		      break;
		  case GGRAPH_SAMPLE_UINT:
		      if (img_out->bits_per_sample == 16)
			{
			    unsigned short *p = (unsigned short *) p_out;
			    *p = (unsigned short) value;
			    p_out += sizeof (unsigned short);
			}
		      else
			{
			    unsigned int *p = (unsigned int *) p_out;
			    *p = (unsigned int) value;
			    p_out += sizeof (unsigned int);
			}
		      break;
		  case GGRAPH_SAMPLE_FLOAT:
		      if (img_out->bits_per_sample == 32)
			{
			    float *p = (float *) p_out;
			    *p = (float) value;
			    p_out += sizeof (float);
			}
		      else
			{
			    double *p = (double *) p_out;
			    *p = value;
			    p_out += sizeof (double);
			}
		      break;
		  };
	    }
      }
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphGeoMergePixels (const void *ptr_in, const void *ptr_out)
{
/* merging pixels between two images by Geographic position */
    int x;
    int y;
    unsigned char *p_in;
    unsigned char *p_out;
    unsigned char gray;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    double geo_x;
    double geo_y;
    int out_x;
    int out_y;
    double dout_x;
    double dout_y;
    double diff_x;
    double diff_y;
    gGraphImagePtr img_in = (gGraphImagePtr) ptr_in;
    gGraphImagePtr img_out = (gGraphImagePtr) ptr_out;

    if (img_in == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_out == NULL)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_out->signature != GG_IMAGE_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_IMAGE;
    if (img_in->srid != img_out->srid)
	return GGRAPH_INVALID_IMAGE;

    if (img_in->pixel_format == GG_PIXEL_GRID)
      {
	  if (img_out->pixel_format == GG_PIXEL_GRID)
	      return merge_grid (img_in, img_out);
	  return GGRAPH_INVALID_IMAGE;
      }

    for (y = 0; y < img_in->height; y++)
      {
	  p_in = img_in->pixels + (y * img_in->scanline_width);
	  for (x = 0; x < img_in->width; x++)
	    {
		if (img_in->pixel_format == GG_PIXEL_GRAYSCALE)
		  {
		      gray = *p_in++;
		      red = gray;
		      green = gray;
		      blue = gray;
		  }
		else if (img_in->pixel_format == GG_PIXEL_PALETTE)
		  {
		      int index = *p_in++;
		      red = img_in->palette_red[index];
		      green = img_in->palette_green[index];
		      blue = img_in->palette_blue[index];
		      if (red == green && red == blue)
			  gray = red;
		      else
			  gray = to_grayscale3 (red, green, blue);
		  }
		else
		  {
		      red = *p_in++;
		      green = *p_in++;
		      blue = *p_in++;
		      if (red == green && red == blue)
			  gray = red;
		      else
			  gray = to_grayscale3 (red, green, blue);
		  }
		/* ok, this one is a valid scene's pixel */
		geo_x =
		    img_in->upper_left_x + ((double) x * img_in->pixel_x_size);
		geo_y =
		    img_in->upper_left_y - ((double) y * img_in->pixel_y_size);
		dout_x =
		    ((geo_x - img_out->upper_left_x) / img_out->pixel_x_size);
		dout_y =
		    ((img_out->upper_left_y - geo_y) / img_out->pixel_y_size);
		out_x = (int) dout_x;
		out_y = (int) dout_y;
		diff_x = dout_x - (double) out_x;
		diff_y = dout_y - (double) out_y;
		if (diff_x >= 0.5)
		    out_x++;
		if (diff_y >= 0.5)
		    out_y++;
		if (out_x < 0 || out_x >= img_out->width)
		    continue;
		if (out_y < 0 || out_y >= img_out->height)
		    continue;
		/* ok, setting this pixel into the output image */
		p_out =
		    img_out->pixels + (out_y * img_out->scanline_width) +
		    (out_x * img_out->pixel_size);
		if (img_out->pixel_format == GG_PIXEL_GRAYSCALE)
		    *p_out++ = gray;
		else if (img_out->pixel_format == GG_PIXEL_RGBA)
		  {
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		      *p_out++ = 255;
		  }
		else
		  {
		      *p_out++ = red;
		      *p_out++ = green;
		      *p_out++ = blue;
		  }
	    }
      }

    return GGRAPH_OK;
}
