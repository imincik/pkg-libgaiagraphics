/* 
/ gaiagraphics_paint.c
/
/ painting helpers
/
/ version 1.0, 2010 October 19
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
#include <math.h>

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

GGRAPH_DECLARE int
gGraphCreateContext (int width, int height, const void **context)
{
/* creating a Graphics Context */
    gGraphContextPtr ctx;

    *context = NULL;

    ctx = malloc (sizeof (gGraphContext));
    if (!ctx)
	return GGRAPH_INSUFFICIENT_MEMORY;
    ctx->signature = GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE;
    ctx->surface =
	cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status (ctx->surface) == CAIRO_STATUS_SUCCESS)
	;
    else
	goto error1;
    ctx->cairo = cairo_create (ctx->surface);
    if (cairo_status (ctx->cairo) == CAIRO_STATUS_NO_MEMORY)
	goto error2;

/* setting up a default Black Pen */
    ctx->current_pen.red = 0.0;
    ctx->current_pen.green = 0.0;
    ctx->current_pen.blue = 0.0;
    ctx->current_pen.alpha = 1.0;
    ctx->current_pen.width = 1.0;
    ctx->current_pen.lengths[0] = 1.0;
    ctx->current_pen.lengths_count = 1;

/* setting up a default Black Brush */
    ctx->current_brush.is_solid_color = 1;
    ctx->current_brush.is_linear_gradient = 0;
    ctx->current_brush.is_pattern = 0;
    ctx->current_brush.red = 0.0;
    ctx->current_brush.green = 0.0;
    ctx->current_brush.blue = 0.0;
    ctx->current_brush.alpha = 1.0;
    ctx->current_brush.pattern = NULL;

/* priming a transparent background */
    cairo_rectangle (ctx->cairo, 0, 0, width, height);
    cairo_set_source_rgba (ctx->cairo, 0.0, 0.0, 0.0, 0.0);
    cairo_fill (ctx->cairo);

/* setting up default Font options */
    ctx->font_red = 0.0;
    ctx->font_green = 0.0;
    ctx->font_blue = 0.0;
    ctx->font_alpha = 1.0;
    ctx->is_font_outlined = 0;
    ctx->font_outline_width = 0.0;

    *context = ctx;
    return GGRAPH_OK;
  error2:
    cairo_destroy (ctx->cairo);
    cairo_surface_destroy (ctx->surface);
    return GGRAPH_ERROR;
  error1:
    cairo_surface_destroy (ctx->surface);
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphDestroyContext (const void *context)
{
/* freeing a Graphics Context */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature != GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_destroy (ctx->cairo);
    cairo_surface_destroy (ctx->surface);
    free (ctx);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphCreateSvgContext (const char *path, int width, int height,
			const void **context)
{
/* creating an SVG Graphics Context */
    gGraphContextPtr ctx;

    *context = NULL;

    ctx = malloc (sizeof (gGraphContext));
    if (!ctx)
	return GGRAPH_INSUFFICIENT_MEMORY;
    ctx->signature = GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE;

    ctx->surface =
	cairo_svg_surface_create (path, (double) width, (double) height);

    if (cairo_surface_status (ctx->surface) == CAIRO_STATUS_SUCCESS)
	;
    else
	goto error1;
    ctx->cairo = cairo_create (ctx->surface);
    if (cairo_status (ctx->cairo) == CAIRO_STATUS_NO_MEMORY)
	goto error2;

/* setting up a default Black Pen */
    ctx->current_pen.red = 0.0;
    ctx->current_pen.green = 0.0;
    ctx->current_pen.blue = 0.0;
    ctx->current_pen.alpha = 1.0;
    ctx->current_pen.width = 1.0;
    ctx->current_pen.lengths[0] = 1.0;
    ctx->current_pen.lengths_count = 1;

/* setting up a default Black Brush */
    ctx->current_brush.is_solid_color = 1;
    ctx->current_brush.is_linear_gradient = 0;
    ctx->current_brush.is_pattern = 0;
    ctx->current_brush.red = 0.0;
    ctx->current_brush.green = 0.0;
    ctx->current_brush.blue = 0.0;
    ctx->current_brush.alpha = 1.0;
    ctx->current_brush.pattern = NULL;

/* priming a transparent background */
    cairo_rectangle (ctx->cairo, 0, 0, width, height);
    cairo_set_source_rgba (ctx->cairo, 0.0, 0.0, 0.0, 0.0);
    cairo_fill (ctx->cairo);

/* setting up default Font options */
    ctx->font_red = 0.0;
    ctx->font_green = 0.0;
    ctx->font_blue = 0.0;
    ctx->font_alpha = 1.0;
    ctx->is_font_outlined = 0;
    ctx->font_outline_width = 0.0;

    *context = ctx;
    return GGRAPH_OK;
  error2:
    cairo_destroy (ctx->cairo);
    cairo_surface_destroy (ctx->surface);
    return GGRAPH_ERROR;
  error1:
    cairo_surface_destroy (ctx->surface);
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphDestroySvgContext (const void *context)
{
/* freeing an SVG Graphics Context */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature != GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_surface_show_page (ctx->surface);
    cairo_destroy (ctx->cairo);
    cairo_surface_finish (ctx->surface);
    cairo_surface_destroy (ctx->surface);
    free (ctx);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphCreatePdfContext (const char *path, int page_width, int page_height,
			int width, int height, const void **context)
{
/* creating an PDF Graphics Context */
    int base_x = (page_width - width) / 2;
    int base_y = (page_height - height) / 2;
    gGraphContextPtr ctx;

    *context = NULL;

    ctx = malloc (sizeof (gGraphContext));
    if (!ctx)
	return GGRAPH_INSUFFICIENT_MEMORY;
    ctx->signature = GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE;
    ctx->surface =
	cairo_pdf_surface_create (path, (double) page_width,
				  (double) page_height);
    if (cairo_surface_status (ctx->surface) == CAIRO_STATUS_SUCCESS)
	;
    else
	goto error1;
    ctx->cairo = cairo_create (ctx->surface);
    if (cairo_status (ctx->cairo) == CAIRO_STATUS_NO_MEMORY)
	goto error2;

/* setting up a default Black Pen */
    ctx->current_pen.red = 0.0;
    ctx->current_pen.green = 0.0;
    ctx->current_pen.blue = 0.0;
    ctx->current_pen.alpha = 1.0;
    ctx->current_pen.width = 1.0;
    ctx->current_pen.lengths[0] = 1.0;
    ctx->current_pen.lengths_count = 1;

/* setting up a default Black Brush */
    ctx->current_brush.is_solid_color = 1;
    ctx->current_brush.is_linear_gradient = 0;
    ctx->current_brush.is_pattern = 0;
    ctx->current_brush.red = 0.0;
    ctx->current_brush.green = 0.0;
    ctx->current_brush.blue = 0.0;
    ctx->current_brush.alpha = 1.0;
    ctx->current_brush.pattern = NULL;

/* priming a transparent background */
    cairo_rectangle (ctx->cairo, 0, 0, width, height);
    cairo_set_source_rgba (ctx->cairo, 0.0, 0.0, 0.0, 0.0);
    cairo_fill (ctx->cairo);

/* setting up default Font options */
    ctx->font_red = 0.0;
    ctx->font_green = 0.0;
    ctx->font_blue = 0.0;
    ctx->font_alpha = 1.0;
    ctx->is_font_outlined = 0;
    ctx->font_outline_width = 0.0;

    cairo_translate (ctx->cairo, base_x, base_y);
    *context = ctx;
    return GGRAPH_OK;
  error2:
    cairo_destroy (ctx->cairo);
    cairo_surface_destroy (ctx->surface);
    return GGRAPH_ERROR;
  error1:
    cairo_surface_destroy (ctx->surface);
    return GGRAPH_ERROR;
}

GGRAPH_DECLARE int
gGraphDestroyPdfContext (const void *context)
{
/* freeing an PDF Graphics Context */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature != GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_surface_show_page (ctx->surface);
    cairo_destroy (ctx->cairo);
    cairo_surface_finish (ctx->surface);
    cairo_surface_destroy (ctx->surface);
    free (ctx);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphSetPen (const void *context, unsigned char red, unsigned char green,
	      unsigned char blue, unsigned char alpha, double width, int style)
{
/* creating a Color Pen */
    double d_red = (double) red / 255.0;
    double d_green = (double) green / 255.0;
    double d_blue = (double) blue / 255.0;
    double d_alpha = (double) alpha / 255.0;
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;

    ctx->current_pen.width = width;
    ctx->current_pen.red = d_red;
    ctx->current_pen.green = d_green;
    ctx->current_pen.blue = d_blue;
    ctx->current_pen.alpha = d_alpha;
    switch (style)
      {
      case GGRAPH_PENSTYLE_DOT:
	  ctx->current_pen.lengths[0] = 2;
	  ctx->current_pen.lengths[1] = 2;
	  ctx->current_pen.lengths_count = 2;
	  break;
      case GGRAPH_PENSTYLE_LONG_DASH:
	  ctx->current_pen.lengths[0] = 16;
	  ctx->current_pen.lengths[1] = 8;
	  ctx->current_pen.lengths_count = 2;
	  break;
      case GGRAPH_PENSTYLE_SHORT_DASH:
	  ctx->current_pen.lengths[0] = 8;
	  ctx->current_pen.lengths[1] = 4;
	  ctx->current_pen.lengths_count = 2;
	  break;
      case GGRAPH_PENSTYLE_DOT_DASH:
	  ctx->current_pen.lengths[0] = 8;
	  ctx->current_pen.lengths[1] = 4;
	  ctx->current_pen.lengths[2] = 2;
	  ctx->current_pen.lengths[3] = 4;
	  ctx->current_pen.lengths_count = 4;
	  break;
      default:
	  ctx->current_pen.lengths[0] = 1;
	  ctx->current_pen.lengths[1] = 0;
	  ctx->current_pen.lengths_count = 2;
      };
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphSetBrush (const void *context, unsigned char red, unsigned char green,
		unsigned char blue, unsigned char alpha)
{
/* setting up a Color Brush */
    double d_red = (double) red / 255.0;
    double d_green = (double) green / 255.0;
    double d_blue = (double) blue / 255.0;
    double d_alpha = (double) alpha / 255.0;
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;

    ctx->current_brush.is_solid_color = 1;
    ctx->current_brush.is_linear_gradient = 0;
    ctx->current_brush.is_pattern = 0;
    ctx->current_brush.red = d_red;
    ctx->current_brush.green = d_green;
    ctx->current_brush.blue = d_blue;
    ctx->current_brush.alpha = d_alpha;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphSetLinearGradientBrush (const void *context, double x, double y,
			      double width, double height, unsigned char red1,
			      unsigned char green1, unsigned char blue1,
			      unsigned char alpha1, unsigned char red2,
			      unsigned char green2, unsigned char blue2,
			      unsigned char alpha2)
{
/* setting up a Linear Gradient Brush */
    double d_red = (double) red1 / 255.0;
    double d_green = (double) green1 / 255.0;
    double d_blue = (double) blue1 / 255.0;
    double d_alpha = (double) alpha1 / 255.0;
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;

    ctx->current_brush.is_solid_color = 0;
    ctx->current_brush.is_linear_gradient = 1;
    ctx->current_brush.is_pattern = 0;
    ctx->current_brush.red = d_red;
    ctx->current_brush.green = d_green;
    ctx->current_brush.blue = d_blue;
    ctx->current_brush.alpha = d_alpha;
    ctx->current_brush.x0 = x;
    ctx->current_brush.y0 = y;
    ctx->current_brush.x1 = x + width;
    ctx->current_brush.y1 = y + height;
    d_red = (double) red2 / 255.0;
    d_green = (double) green2 / 255.0;
    d_blue = (double) blue2 / 255.0;
    d_alpha = (double) alpha2 / 255.0;
    ctx->current_brush.red2 = d_red;
    ctx->current_brush.green2 = d_green;
    ctx->current_brush.blue2 = d_blue;
    ctx->current_brush.alpha2 = d_alpha;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphSetPatternBrush (const void *context, const void *brush)
{
/* setting up a Pattern Brush */
    gGraphPatternBrushPtr pattern = (gGraphPatternBrushPtr) brush;
    gGraphContextPtr ctx = (gGraphContextPtr) context;

    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;

    if (!pattern)
	return GGRAPH_INVALID_PAINT_BRUSH;
    if (pattern->signature != GG_GRAPHICS_BRUSH_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_BRUSH;

    ctx->current_brush.is_solid_color = 0;
    ctx->current_brush.is_linear_gradient = 0;
    ctx->current_brush.is_pattern = 1;

    ctx->current_brush.pattern = pattern->pattern;
    return GGRAPH_OK;
}

static void
set_current_brush (gGraphContextPtr ctx)
{
/* setting up the current Brush */
    if (ctx->current_brush.is_solid_color)
      {
	  /* using a Solid Color Brush */
	  cairo_set_source_rgba (ctx->cairo, ctx->current_brush.red,
				 ctx->current_brush.green,
				 ctx->current_brush.blue,
				 ctx->current_brush.alpha);
      }
    else if (ctx->current_brush.is_linear_gradient)
      {
	  /* using a Linear Gradient Brush */
	  cairo_pattern_t *pattern =
	      cairo_pattern_create_linear (ctx->current_brush.x0,
					   ctx->current_brush.y0,
					   ctx->current_brush.x1,
					   ctx->current_brush.y1);
	  cairo_pattern_add_color_stop_rgba (pattern, 0.0,
					     ctx->current_brush.red,
					     ctx->current_brush.green,
					     ctx->current_brush.blue,
					     ctx->current_brush.alpha);
	  cairo_pattern_add_color_stop_rgba (pattern, 1.0,
					     ctx->current_brush.red2,
					     ctx->current_brush.green2,
					     ctx->current_brush.blue2,
					     ctx->current_brush.alpha2);
	  cairo_set_source (ctx->cairo, pattern);
	  cairo_pattern_destroy (pattern);
      }
    else if (ctx->current_brush.is_pattern)
      {
	  /* using a Pattern Brush */
	  cairo_set_source (ctx->cairo, ctx->current_brush.pattern);
      }
}

GGRAPH_DECLARE int
gGraphFillPath (const void *context, int preserve)
{
/* Filling a path */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;

    set_current_brush (ctx);
    if (preserve == GGRAPH_PRESERVE_PATH)
	cairo_fill_preserve (ctx->cairo);
    else
	cairo_fill (ctx->cairo);
    return GGRAPH_OK;
}

static void
set_current_pen (gGraphContextPtr ctx)
{
/* setting up the current Pen */
    cairo_set_line_width (ctx->cairo, ctx->current_pen.width);
    cairo_set_source_rgba (ctx->cairo, ctx->current_pen.red,
			   ctx->current_pen.green, ctx->current_pen.blue,
			   ctx->current_pen.alpha);
    cairo_set_line_cap (ctx->cairo, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join (ctx->cairo, CAIRO_LINE_JOIN_MITER);
    cairo_set_dash (ctx->cairo, ctx->current_pen.lengths,
		    ctx->current_pen.lengths_count, 0.0);
}

GGRAPH_DECLARE int
gGraphStrokePath (const void *context, int preserve)
{
/* Stroking a path */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;

    set_current_pen (ctx);
    if (preserve == GGRAPH_PRESERVE_PATH)
	cairo_stroke_preserve (ctx->cairo);
    else
	cairo_stroke (ctx->cairo);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphMoveToPoint (const void *context, double x, double y)
{
/* Moving to a Path Point */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_move_to (ctx->cairo, x, y);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphAddLineToPath (const void *context, double x, double y)
{
/* Adding a Lint to a Path */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_line_to (ctx->cairo, x, y);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphCloseSubpath (const void *context)
{
/* Closing a SubPath */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_close_path (ctx->cairo);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphStrokeLine (const void *context, double x0, double y0, double x1,
		  double y1)
{
/* Stroking a line */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_move_to (ctx->cairo, x0, y0);
    cairo_line_to (ctx->cairo, x1, y1);
    set_current_pen (ctx);
    cairo_stroke (ctx->cairo);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphDrawEllipse (const void *context, double x, double y, double width,
		   double height)
{
/* Drawing a filled ellipse */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_save (ctx->cairo);
    cairo_translate (ctx->cairo, x + (width / 2.0), y + (height / 2.0));
    cairo_scale (ctx->cairo, width / 2.0, height / 2.0);
    cairo_arc (ctx->cairo, 0.0, 0.0, 1.0, 0.0, 2.0 * M_PI);
    cairo_restore (ctx->cairo);
    set_current_brush (ctx);
    cairo_fill_preserve (ctx->cairo);
    set_current_pen (ctx);
    cairo_stroke (ctx->cairo);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphDrawCircleSector (const void *context, double center_x,
			double center_y, double radius, double from_angle,
			double to_angle)
{
/* drawing a filled circular sector */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_move_to (ctx->cairo, center_x, center_y);
    cairo_arc (ctx->cairo, center_x, center_y, radius, from_angle, to_angle);
    cairo_line_to (ctx->cairo, center_x, center_y);
    set_current_brush (ctx);
    cairo_fill_preserve (ctx->cairo);
    set_current_pen (ctx);
    cairo_stroke (ctx->cairo);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphDrawRectangle (const void *context, double x, double y, double width,
		     double height)
{
/* Drawing a filled rectangle */
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_rectangle (ctx->cairo, x, y, width, height);
    set_current_brush (ctx);
    cairo_fill_preserve (ctx->cairo);
    set_current_pen (ctx);
    cairo_stroke (ctx->cairo);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphDrawRoundedRectangle (const void *context, double x, double y,
			    double width, double height, double radius)
{
/* Drawing a filled rectangle with rounded corners */
    double degrees = M_PI / 180.0;
    gGraphContextPtr ctx = (gGraphContextPtr) context;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;
    cairo_new_sub_path (ctx->cairo);
    cairo_arc (ctx->cairo, x + width - radius, y + radius, radius,
	       -90 * degrees, 0 * degrees);
    cairo_arc (ctx->cairo, x + width - radius, y + height - radius, radius,
	       0 * degrees, 90 * degrees);
    cairo_arc (ctx->cairo, x + radius, y + height - radius, radius,
	       90 * degrees, 180 * degrees);
    cairo_arc (ctx->cairo, x + radius, y + radius, radius, 180 * degrees,
	       270 * degrees);
    cairo_close_path (ctx->cairo);
    set_current_brush (ctx);
    cairo_fill_preserve (ctx->cairo);
    set_current_pen (ctx);
    cairo_stroke (ctx->cairo);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphGetContextRgbArray (const void *context, unsigned char **rgbArray)
{
/* creating an RGB buffer from the given Context */
    int width;
    int height;
    int x;
    int y;
    unsigned char *p_in;
    unsigned char *p_out;
    unsigned char *rgb;
    int little_endian = gg_endian_arch ();
    gGraphContextPtr ctx = (gGraphContextPtr) context;

    *rgbArray = NULL;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature != GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_CONTEXT;

    width = cairo_image_surface_get_width (ctx->surface);
    height = cairo_image_surface_get_height (ctx->surface);
    rgb = malloc (width * height * 3);
    if (!rgb)
	return GGRAPH_INSUFFICIENT_MEMORY;

    p_in = cairo_image_surface_get_data (ctx->surface);
    p_out = rgb;
    for (y = 0; y < height; y++)
      {
	  for (x = 0; x < width; x++)
	    {
		unsigned char r;
		unsigned char g;
		unsigned char b;
		if (little_endian)
		  {
		      b = *p_in++;
		      g = *p_in++;
		      r = *p_in++;
		      p_in++;	/* skipping Alpha */
		  }
		else
		  {
		      p_in++;	/* skipping Alpha */
		      r = *p_in++;
		      g = *p_in++;
		      b = *p_in++;
		  }
		*p_out++ = r;
		*p_out++ = g;
		*p_out++ = b;
	    }
      }
    *rgbArray = rgb;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphGetContextAlphaArray (const void *context, unsigned char **alphaArray)
{
/* creating an Alpha buffer from the given Context */
    int width;
    int height;
    int x;
    int y;
    unsigned char *p_in;
    unsigned char *p_out;
    unsigned char *alpha;
    gGraphContextPtr ctx = (gGraphContextPtr) context;

    *alphaArray = NULL;
    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature != GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_CONTEXT;

    width = cairo_image_surface_get_width (ctx->surface);
    height = cairo_image_surface_get_height (ctx->surface);
    alpha = malloc (width * height);
    if (!alpha)
	return GGRAPH_INSUFFICIENT_MEMORY;

    p_in = cairo_image_surface_get_data (ctx->surface);
    p_out = alpha;
    for (y = 0; y < height; y++)
      {
	  for (x = 0; x < width; x++)
	    {
		p_in += 3;	/* skipping RGB */
		*p_out++ = *p_in++;
	    }
      }
    *alphaArray = alpha;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphDrawBitmap (const void *context, const void *bitmap, int x, int y)
{
/* drawing a symbol bitmap */
    gGraphBitmapPtr bmp = (gGraphBitmapPtr) bitmap;
    gGraphContextPtr ctx = (gGraphContextPtr) context;

    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;

    if (!bmp)
	return GGRAPH_INVALID_PAINT_BITMAP;
    if (bmp->signature != GG_GRAPHICS_BITMAP_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_BITMAP;

    cairo_save (ctx->cairo);
    cairo_scale (ctx->cairo, 1, 1);
    cairo_translate (ctx->cairo, x, y);
    cairo_set_source (ctx->cairo, bmp->pattern);
    cairo_rectangle (ctx->cairo, 0, 0, bmp->width, bmp->height);
    cairo_fill (ctx->cairo);
    cairo_restore (ctx->cairo);
    return GGRAPH_OK;
}

static void
adjust_for_endianness (unsigned char *rgbaArray, int width, int height)
{
/* Adjusting from RGBA to ARGB respecting platform endianness */
    int x;
    int y;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
    unsigned char *p_in = rgbaArray;
    unsigned char *p_out = rgbaArray;
    int little_endian = gg_endian_arch ();

    for (y = 0; y < height; y++)
      {
	  for (x = 0; x < width; x++)
	    {
		red = *p_in++;
		green = *p_in++;
		blue = *p_in++;
		alpha = *p_in++;
		if (alpha == 0)
		  {
		      *p_out++ = 0;
		      *p_out++ = 0;
		      *p_out++ = 0;
		      *p_out++ = 0;
		  }
		else
		  {
		      if (little_endian)
			{
			    *p_out++ = blue;
			    *p_out++ = green;
			    *p_out++ = red;
			    *p_out++ = alpha;
			}
		      else
			{
			    *p_out++ = alpha;
			    *p_out++ = red;
			    *p_out++ = green;
			    *p_out++ = blue;
			}
		  }
	    }
      }
}

GGRAPH_DECLARE int
gGraphCreateBitmap (unsigned char *rgbaArray, int width, int height,
		    const void **bitmap)
{
/* creating a symbol bitmap */
    gGraphBitmapPtr bmp;

    *bitmap = NULL;
    if (!rgbaArray)
	return GGRAPH_ERROR;

    adjust_for_endianness (rgbaArray, width, height);
    bmp = malloc (sizeof (gGraphBitmap));
    if (!bmp)
	return GGRAPH_INSUFFICIENT_MEMORY;
    bmp->signature = GG_GRAPHICS_BITMAP_MAGIC_SIGNATURE;
    bmp->width = width;
    bmp->height = height;
    bmp->bitmap =
	cairo_image_surface_create_for_data (rgbaArray, CAIRO_FORMAT_ARGB32,
					     width, height, width * 4);
    bmp->pattern = cairo_pattern_create_for_surface (bmp->bitmap);
    *bitmap = bmp;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphDestroyBitmap (const void *bitmap)
{
/* destroying a symbol bitmap */
    gGraphBitmapPtr bmp = (gGraphBitmapPtr) bitmap;

    if (!bmp)
	return GGRAPH_INVALID_PAINT_BITMAP;
    if (bmp->signature != GG_GRAPHICS_BITMAP_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_BITMAP;

    cairo_pattern_destroy (bmp->pattern);
    cairo_surface_destroy (bmp->bitmap);
    free (bmp);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphCreateBrush (unsigned char *rgbaArray, int width, int height,
		   const void **brush)
{
/* creating a pattern brush */
    gGraphPatternBrushPtr pattern;

    *brush = NULL;
    if (!rgbaArray)
	return GGRAPH_ERROR;

    adjust_for_endianness (rgbaArray, width, height);
    pattern = malloc (sizeof (gGraphPatternBrush));
    if (!pattern)
	return GGRAPH_INSUFFICIENT_MEMORY;
    pattern->signature = GG_GRAPHICS_BRUSH_MAGIC_SIGNATURE;
    pattern->width = width;
    pattern->height = height;
    pattern->bitmap =
	cairo_image_surface_create_for_data (rgbaArray, CAIRO_FORMAT_ARGB32,
					     width, height, width * 4);
    pattern->pattern = cairo_pattern_create_for_surface (pattern->bitmap);
    cairo_pattern_set_extend (pattern->pattern, CAIRO_EXTEND_REPEAT);
    *brush = pattern;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphDestroyBrush (const void *brush)
{
/* destroying a pattern brush */
    gGraphPatternBrushPtr pattern = (gGraphPatternBrushPtr) brush;

    if (!pattern)
	return GGRAPH_INVALID_PAINT_BRUSH;
    if (pattern->signature != GG_GRAPHICS_BRUSH_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_BRUSH;

    cairo_pattern_destroy (pattern->pattern);
    cairo_surface_destroy (pattern->bitmap);
    free (pattern);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphCreateFont (double size, int style, int weight, const void **font)
{
/* creating a font */
    gGraphFontPtr fnt;

    *font = NULL;
    fnt = malloc (sizeof (gGraphFont));
    if (!fnt)
	return GGRAPH_INSUFFICIENT_MEMORY;
    fnt->signature = GG_GRAPHICS_FONT_MAGIC_SIGNATURE;
    if (size < 1.0)
	fnt->size = 1.0;
    else if (size > 32.0)
	fnt->size = 32.0;
    else
	fnt->size = size;
    if (style == GGRAPH_FONTSTYLE_ITALIC)
	fnt->style = GGRAPH_FONTSTYLE_ITALIC;
    else
	fnt->style = GGRAPH_FONTSTYLE_NORMAL;
    if (weight == GGRAPH_FONTWEIGHT_BOLD)
	fnt->weight = GGRAPH_FONTWEIGHT_BOLD;
    else
	fnt->weight = GGRAPH_FONTWEIGHT_NORMAL;
    fnt->is_outlined = 0;
    fnt->outline_width = 0.0;
    fnt->red = 0.0;
    fnt->green = 0.0;
    fnt->blue = 0.0;
    fnt->alpha = 1.0;
    *font = fnt;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphDestroyFont (const void *font)
{
/* destroying a font */
    gGraphFontPtr fnt = (gGraphFontPtr) font;

    if (!fnt)
	return GGRAPH_INVALID_PAINT_FONT;
    if (fnt->signature != GG_GRAPHICS_FONT_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_FONT;

    free (fnt);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphFontSetColor (const void *font, unsigned char red, unsigned char green,
		    unsigned char blue, unsigned char alpha)
{
/* setting up the font color */
    gGraphFontPtr fnt = (gGraphFontPtr) font;

    if (!fnt)
	return GGRAPH_INVALID_PAINT_FONT;
    if (fnt->signature != GG_GRAPHICS_FONT_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_FONT;

    fnt->red = (double) red / 255.0;
    fnt->green = (double) green / 255.0;
    fnt->blue = (double) blue / 255.0;
    fnt->alpha = (double) alpha / 255.0;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphFontSetOutline (const void *font, double width)
{
/* setting up the font outline */
    gGraphFontPtr fnt = (gGraphFontPtr) font;

    if (!fnt)
	return GGRAPH_INVALID_PAINT_FONT;
    if (fnt->signature != GG_GRAPHICS_FONT_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_FONT;

    if (width <= 0.0)
      {
	  fnt->is_outlined = 0;
	  fnt->outline_width = 0.0;
      }
    else
      {
	  fnt->is_outlined = 1;
	  fnt->outline_width = width;
      }
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphSetFont (const void *context, const void *font)
{
/* setting up the current font */
    int style = CAIRO_FONT_SLANT_NORMAL;
    int weight = CAIRO_FONT_WEIGHT_NORMAL;
    double size;
    gGraphFontPtr fnt = (gGraphFontPtr) font;
    gGraphContextPtr ctx = (gGraphContextPtr) context;

    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;

    if (!fnt)
	return GGRAPH_INVALID_PAINT_FONT;
    if (fnt->signature != GG_GRAPHICS_FONT_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_PAINT_FONT;

    if (fnt->style == GGRAPH_FONTSTYLE_ITALIC)
	style = CAIRO_FONT_SLANT_ITALIC;
    if (fnt->weight == GGRAPH_FONTWEIGHT_BOLD)
	weight = CAIRO_FONT_WEIGHT_BOLD;
    cairo_select_font_face (ctx->cairo, "monospace", style, weight);
    size = fnt->size;
    if (fnt->is_outlined)
	size += fnt->outline_width;
    cairo_set_font_size (ctx->cairo, size);
    ctx->font_red = fnt->red;
    ctx->font_green = fnt->green;
    ctx->font_blue = fnt->blue;
    ctx->font_alpha = fnt->alpha;
    ctx->is_font_outlined = fnt->is_outlined;
    ctx->font_outline_width = fnt->outline_width;

    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphGetTextExtent (const void *context, const char *text, double *pre_x,
		     double *pre_y, double *width, double *height,
		     double *post_x, double *post_y)
{
/* measuring the text extent (using the current font) */
    cairo_text_extents_t extents;
    gGraphContextPtr ctx = (gGraphContextPtr) context;

    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;

    cairo_text_extents (ctx->cairo, text, &extents);
    *pre_x = extents.x_bearing;
    *pre_y = extents.y_bearing;
    *width = extents.width;
    *height = extents.height;
    *post_x = extents.x_advance;
    *post_y = extents.y_advance;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphDrawText (const void *context, const char *text, double x, double y,
		double angle)
{
/* drawing a text string (using the current font) */
    gGraphContextPtr ctx = (gGraphContextPtr) context;

    if (!ctx)
	return GGRAPH_INVALID_PAINT_CONTEXT;
    if (ctx->signature == GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE ||
	ctx->signature == GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE)
	;
    else
	return GGRAPH_INVALID_PAINT_CONTEXT;

    cairo_save (ctx->cairo);
    cairo_translate (ctx->cairo, x, y);
    cairo_rotate (ctx->cairo, angle);
    if (ctx->is_font_outlined)
      {
	  /* outlined font */
	  cairo_move_to (ctx->cairo, 0.0, 0.0);
	  cairo_text_path (ctx->cairo, text);
	  cairo_set_source_rgba (ctx->cairo, ctx->font_red, ctx->font_green,
				 ctx->font_blue, ctx->font_alpha);
	  cairo_fill_preserve (ctx->cairo);
	  cairo_set_source_rgba (ctx->cairo, 1.0, 1.0, 1.0, ctx->font_alpha);
	  cairo_set_line_width (ctx->cairo, ctx->font_outline_width);
	  cairo_stroke (ctx->cairo);
      }
    else
      {
	  /* no outline */
	  cairo_set_source_rgba (ctx->cairo, ctx->font_red, ctx->font_green,
				 ctx->font_blue, ctx->font_alpha);
	  cairo_move_to (ctx->cairo, 0.0, 0.0);
	  cairo_show_text (ctx->cairo, text);
      }
    cairo_restore (ctx->cairo);
    return GGRAPH_OK;
}
