/* 
/ gaiagraphics_svg.c
/
/ SVG renderering
/
/ version 1.0, 2013 February 10
/
/ Author: Sandro Furieri a.furieri@lqt.it
/
/ Copyright (C) 2013  Alessandro Furieri
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
#include <float.h>

#include "gaiagraphics.h"
#include "gaiagraphics_internals.h"

#define GG_SVG_PI	3.141592653589793

struct gg_svg_parent_ref
{
/* a parent reference (inheritance chain) */
    struct gg_svg_group *parent;
    struct gg_svg_parent_ref *next;
};

struct gg_svg_parents
{
/* inheritance chain */
    struct gg_svg_parent_ref *first;
    struct gg_svg_parent_ref *last;
};

static void
gg_svg_free_parents (struct gg_svg_parents *chain)
{
/* freeing an inheritance chain */
    struct gg_svg_parent_ref *pp;
    struct gg_svg_parent_ref *ppn;
    pp = chain->first;
    while (pp)
      {
	  ppn = pp->next;
	  free (pp);
	  pp = ppn;
      }
}

static void
gg_svg_add_parent (struct gg_svg_parents *chain, struct gg_svg_group *parent)
{
/* adding an inheritance item into the chain */
    struct gg_svg_parent_ref *p = malloc (sizeof (struct gg_svg_parent_ref));
    p->parent = parent;
    p->next = chain->first;
/* updating a reverse-ordered linked list */
    if (chain->last == NULL)
	chain->last = p;
    chain->first = p;
}

static void
gg_svg_gradient_transformation (cairo_pattern_t * pattern,
				struct gg_svg_transform *trans)
{
/* applying a single transformation */
    double angle;
    double tangent;
    struct gg_svg_matrix *mtrx;
    struct gg_svg_translate *translate;
    struct gg_svg_scale *scale;
    struct gg_svg_rotate *rotate;
    struct gg_svg_skew *skew;
    cairo_matrix_t matrix;
    cairo_matrix_t matrix_in;

    if (trans->data == NULL)
	return;
    switch (trans->type)
      {
      case GG_SVG_MATRIX:
	  mtrx = trans->data;
	  cairo_pattern_get_matrix (pattern, &matrix);
	  matrix_in.xx = mtrx->a;
	  matrix_in.yx = mtrx->b;
	  matrix_in.xy = mtrx->c;
	  matrix_in.yy = mtrx->d;
	  matrix_in.x0 = mtrx->e;
	  matrix_in.y0 = mtrx->f;
	  cairo_matrix_multiply (&matrix, &matrix, &matrix_in);
	  cairo_matrix_invert (&matrix);
	  cairo_pattern_set_matrix (pattern, &matrix);
	  break;
      case GG_SVG_TRANSLATE:
	  translate = trans->data;
	  cairo_pattern_get_matrix (pattern, &matrix);
	  cairo_matrix_translate (&matrix, translate->tx, translate->ty);
	  cairo_matrix_invert (&matrix);
	  cairo_pattern_set_matrix (pattern, &matrix);
	  break;
      case GG_SVG_SCALE:
	  scale = trans->data;
	  cairo_pattern_get_matrix (pattern, &matrix);
	  cairo_matrix_scale (&matrix, scale->sx, scale->sy);
	  cairo_matrix_invert (&matrix);
	  cairo_pattern_set_matrix (pattern, &matrix);
	  break;
      case GG_SVG_ROTATE:
	  rotate = trans->data;
	  cairo_pattern_get_matrix (pattern, &matrix);
	  angle = rotate->angle * (GG_SVG_PI / 180.0);
	  cairo_matrix_translate (&matrix, rotate->cx, rotate->cy);
	  cairo_matrix_rotate (&matrix, angle);
	  cairo_matrix_translate (&matrix, -1.0 * rotate->cx,
				  -1.0 * rotate->cy);
	  cairo_matrix_invert (&matrix);
	  cairo_pattern_set_matrix (pattern, &matrix);
	  break;
      case GG_SVG_SKEW_X:
	  skew = trans->data;
	  cairo_pattern_get_matrix (pattern, &matrix);
	  angle = skew->angle * (GG_SVG_PI / 180.0);
	  tangent = tan (angle);
	  matrix_in.xx = 1.0;
	  matrix_in.yx = 0.0;
	  matrix_in.xy = tangent;
	  matrix_in.yy = 1.0;
	  matrix_in.x0 = 0.0;
	  matrix_in.y0 = 0.0;
	  cairo_matrix_multiply (&matrix, &matrix_in, &matrix);
	  cairo_matrix_invert (&matrix);
	  cairo_pattern_set_matrix (pattern, &matrix);
	  break;
      case GG_SVG_SKEW_Y:
	  skew = trans->data;
	  cairo_pattern_get_matrix (pattern, &matrix);
	  angle = skew->angle * (GG_SVG_PI / 180.0);
	  tangent = tan (angle);
	  matrix_in.xx = 1.0;
	  matrix_in.yx = tangent;
	  matrix_in.xy = 0.0;
	  matrix_in.yy = 1.0;
	  matrix_in.x0 = 0.0;
	  matrix_in.y0 = 0.0;
	  cairo_matrix_multiply (&matrix, &matrix_in, &matrix);
	  cairo_matrix_invert (&matrix);
	  cairo_pattern_set_matrix (pattern, &matrix);
	  break;
      };
}

static void
gg_svg_apply_gradient_transformations (cairo_pattern_t * pattern,
				       struct gg_svg_gradient *grad)
{
/* applying all Gradient-related transformations */
    struct gg_svg_transform *trans = grad->first_trans;
    while (trans)
      {
	  gg_svg_gradient_transformation (pattern, trans);
	  trans = trans->next;
      }
}

static void
gg_svg_set_pen (cairo_t * cairo, struct gg_svg_style *style)
{
/* setting up a Pen for Cairo */
    cairo_pattern_t *pattern;
    double lengths[4];
    lengths[0] = 1.0;
    cairo_set_line_width (cairo, style->stroke_width);
    if (style->stroke_url != NULL && style->stroke_pointer != NULL)
      {
	  struct gg_svg_gradient *grad = style->stroke_pointer;
	  struct gg_svg_gradient_stop *stop;
	  if (grad->type == GG_SVG_LINEAR_GRADIENT)
	    {
		pattern =
		    cairo_pattern_create_linear (grad->x1, grad->y1, grad->x2,
						 grad->y2);
		gg_svg_apply_gradient_transformations (pattern, grad);
		stop = grad->first_stop;
		while (stop)
		  {
		      cairo_pattern_add_color_stop_rgba (pattern, stop->offset,
							 stop->red, stop->green,
							 stop->blue,
							 stop->opacity *
							 style->opacity);
		      stop = stop->next;
		  }
		cairo_set_source (cairo, pattern);
		cairo_set_line_cap (cairo, style->stroke_linecap);
		cairo_set_line_join (cairo, style->stroke_linejoin);
		cairo_set_miter_limit (cairo, style->stroke_miterlimit);
		if (style->stroke_dashitems == 0
		    || style->stroke_dasharray == NULL)
		    cairo_set_dash (cairo, lengths, 0, 0.0);
		else
		    cairo_set_dash (cairo, style->stroke_dasharray,
				    style->stroke_dashitems,
				    style->stroke_dashoffset);
		cairo_pattern_destroy (pattern);
		return;
	    }
	  else if (grad->type == GG_SVG_RADIAL_GRADIENT)
	    {
		pattern =
		    cairo_pattern_create_radial (grad->cx, grad->cy, 0.0,
						 grad->fx, grad->fy, grad->r);
		gg_svg_apply_gradient_transformations (pattern, grad);
		stop = grad->first_stop;
		while (stop)
		  {
		      cairo_pattern_add_color_stop_rgba (pattern, stop->offset,
							 stop->red, stop->green,
							 stop->blue,
							 stop->opacity *
							 style->opacity);
		      stop = stop->next;
		  }
		cairo_set_source (cairo, pattern);
		cairo_set_line_cap (cairo, style->stroke_linecap);
		cairo_set_line_join (cairo, style->stroke_linejoin);
		cairo_set_miter_limit (cairo, style->stroke_miterlimit);
		if (style->stroke_dashitems == 0
		    || style->stroke_dasharray == NULL)
		    cairo_set_dash (cairo, lengths, 0, 0.0);
		else
		    cairo_set_dash (cairo, style->stroke_dasharray,
				    style->stroke_dashitems,
				    style->stroke_dashoffset);
		cairo_pattern_destroy (pattern);
		return;
	    }
      }
    cairo_set_source_rgba (cairo, style->stroke_red, style->stroke_green,
			   style->stroke_blue,
			   style->stroke_opacity * style->opacity);
    cairo_set_line_cap (cairo, style->stroke_linecap);
    cairo_set_line_join (cairo, style->stroke_linejoin);
    cairo_set_miter_limit (cairo, style->stroke_miterlimit);
    if (style->stroke_dashitems == 0 || style->stroke_dasharray == NULL)
	cairo_set_dash (cairo, lengths, 0, 0.0);
    else
	cairo_set_dash (cairo, style->stroke_dasharray, style->stroke_dashitems,
			style->stroke_dashoffset);
}

static void
gg_svg_set_brush (cairo_t * cairo, struct gg_svg_style *style)
{
/* setting up a Brush for Cairo */
    cairo_pattern_t *pattern;
    if (style->fill_url != NULL && style->fill_pointer != NULL)
      {
	  struct gg_svg_gradient *grad = style->fill_pointer;
	  struct gg_svg_gradient_stop *stop;
	  if (grad->type == GG_SVG_LINEAR_GRADIENT)
	    {
		pattern =
		    cairo_pattern_create_linear (grad->x1, grad->y1, grad->x2,
						 grad->y2);
		gg_svg_apply_gradient_transformations (pattern, grad);
		stop = grad->first_stop;
		while (stop)
		  {
		      cairo_pattern_add_color_stop_rgba (pattern, stop->offset,
							 stop->red, stop->green,
							 stop->blue,
							 stop->opacity *
							 style->opacity);
		      stop = stop->next;
		  }
		cairo_set_source (cairo, pattern);
		cairo_pattern_destroy (pattern);
		return;
	    }
	  else if (grad->type == GG_SVG_RADIAL_GRADIENT)
	    {
		pattern =
		    cairo_pattern_create_radial (grad->cx, grad->cy, 0.0,
						 grad->fx, grad->fy, grad->r);
		gg_svg_apply_gradient_transformations (pattern, grad);
		stop = grad->first_stop;
		while (stop)
		  {
		      cairo_pattern_add_color_stop_rgba (pattern, stop->offset,
							 stop->red, stop->green,
							 stop->blue,
							 stop->opacity *
							 style->opacity);
		      stop = stop->next;
		  }
		cairo_set_source (cairo, pattern);
		cairo_pattern_destroy (pattern);
		return;
	    }
      }

    cairo_set_source_rgba (cairo, style->fill_red, style->fill_green,
			   style->fill_blue,
			   style->fill_opacity * style->opacity);
    cairo_set_fill_rule (cairo, style->fill_rule);
}

static void
gg_svg_draw_rect (cairo_t * cairo, struct gg_svg_shape *shape,
		  struct gg_svg_style *style)
{
/* drawing an SVG Rect */
    struct gg_svg_rect *rect = shape->data;
    if (rect->width == 0 || rect->height == 0)
	return;
    if (style->visibility == 0)
	return;
    if (style->fill == 0 && style->stroke == 0)
	return;

    if (rect->rx <= 0 || rect->ry <= 0)
      {
	  /* normal Rect */
	  cairo_rectangle (cairo, rect->x, rect->y, rect->width, rect->height);
      }
    else
      {
	  /* rounded Rect */
	  cairo_new_sub_path (cairo);
	  cairo_save (cairo);
	  cairo_translate (cairo, rect->x + rect->rx, rect->y + rect->ry);
	  cairo_scale (cairo, rect->rx, rect->ry);
	  cairo_arc (cairo, 0.0, 0.0, 1.0, GG_SVG_PI, -1 * GG_SVG_PI / 2.0);
	  cairo_restore (cairo);
	  cairo_save (cairo);
	  cairo_translate (cairo, rect->x + rect->width - rect->rx,
			   rect->y + rect->ry);
	  cairo_scale (cairo, rect->rx, rect->ry);
	  cairo_arc (cairo, 0.0, 0.0, 1.0, -1 * GG_SVG_PI / 2.0, 0.0);
	  cairo_restore (cairo);
	  cairo_save (cairo);
	  cairo_translate (cairo, rect->x + rect->width - rect->rx,
			   rect->y + rect->height - rect->ry);
	  cairo_scale (cairo, rect->rx, rect->ry);
	  cairo_arc (cairo, 0.0, 0.0, 1.0, 0.0, GG_SVG_PI / 2.0);
	  cairo_restore (cairo);
	  cairo_save (cairo);
	  cairo_translate (cairo, rect->x + rect->rx,
			   rect->y + rect->height - rect->ry);
	  cairo_scale (cairo, rect->rx, rect->ry);
	  cairo_arc (cairo, 0.0, 0.0, 1.0, GG_SVG_PI / 2.0, GG_SVG_PI);
	  cairo_restore (cairo);
	  cairo_close_path (cairo);
      }

/* drawing the Rect */
    if (style->fill)
      {
	  /* filling */
	  gg_svg_set_brush (cairo, style);
	  if (style->stroke)
	      cairo_fill_preserve (cairo);
	  else
	      cairo_fill (cairo);
      }
    if (style->stroke)
      {
	  /* stroking */
	  gg_svg_set_pen (cairo, style);
	  cairo_stroke (cairo);
      }
}

static void
gg_svg_clip_rect (cairo_t * cairo, struct gg_svg_shape *shape)
{
/* clipping an SVG Rect */
    struct gg_svg_rect *rect = shape->data;
    if (rect->width == 0 || rect->height == 0)
	return;

    if (rect->rx <= 0 || rect->ry <= 0)
      {
	  /* normal Rect */
	  cairo_rectangle (cairo, rect->x, rect->y, rect->width, rect->height);
      }
    else
      {
	  /* rounded Rect */
	  cairo_new_sub_path (cairo);
	  cairo_save (cairo);
	  cairo_translate (cairo, rect->x + rect->rx, rect->y + rect->ry);
	  cairo_scale (cairo, rect->rx, rect->ry);
	  cairo_arc (cairo, 0.0, 0.0, 1.0, GG_SVG_PI, -1 * GG_SVG_PI / 2.0);
	  cairo_restore (cairo);
	  cairo_save (cairo);
	  cairo_translate (cairo, rect->x + rect->width - rect->rx,
			   rect->y + rect->ry);
	  cairo_scale (cairo, rect->rx, rect->ry);
	  cairo_arc (cairo, 0.0, 0.0, 1.0, -1 * GG_SVG_PI / 2.0, 0.0);
	  cairo_restore (cairo);
	  cairo_save (cairo);
	  cairo_translate (cairo, rect->x + rect->width - rect->rx,
			   rect->y + rect->height - rect->ry);
	  cairo_scale (cairo, rect->rx, rect->ry);
	  cairo_arc (cairo, 0.0, 0.0, 1.0, 0.0, GG_SVG_PI / 2.0);
	  cairo_restore (cairo);
	  cairo_save (cairo);
	  cairo_translate (cairo, rect->x + rect->rx,
			   rect->y + rect->height - rect->ry);
	  cairo_scale (cairo, rect->rx, rect->ry);
	  cairo_arc (cairo, 0.0, 0.0, 1.0, GG_SVG_PI / 2.0, GG_SVG_PI);
	  cairo_restore (cairo);
	  cairo_close_path (cairo);
      }
    cairo_clip (cairo);
}

static void
gg_svg_draw_circle (cairo_t * cairo, struct gg_svg_shape *shape,
		    struct gg_svg_style *style)
{
/* drawing an SVG Circle */
    struct gg_svg_circle *circle = shape->data;
    if (circle->r <= 0)
	return;
    if (style->visibility == 0)
	return;
    if (style->fill == 0 && style->stroke == 0)
	return;

/* drawing the Arc */
    cairo_arc (cairo, circle->cx, circle->cy, circle->r, 0.0, 2.0 * GG_SVG_PI);
    if (style->fill)
      {
	  /* filling */
	  gg_svg_set_brush (cairo, style);
	  if (style->stroke)
	      cairo_fill_preserve (cairo);
	  else
	      cairo_fill (cairo);
      }
    if (style->stroke)
      {
	  /* stroking */
	  gg_svg_set_pen (cairo, style);
	  cairo_stroke (cairo);
      }
}

static void
gg_svg_clip_circle (cairo_t * cairo, struct gg_svg_shape *shape)
{
/* clipping an SVG Circle */
    struct gg_svg_circle *circle = shape->data;
    if (circle->r <= 0)
	return;

/* clipping the Arc */
    cairo_arc (cairo, circle->cx, circle->cy, circle->r, 0.0, 2.0 * GG_SVG_PI);
    cairo_clip (cairo);
}

static void
gg_svg_draw_ellipse (cairo_t * cairo, struct gg_svg_shape *shape,
		     struct gg_svg_style *style)
{
/* drawing an SVG Ellipse */
    struct gg_svg_ellipse *ellipse = shape->data;
    if (ellipse->rx <= 0 || ellipse->ry <= 0)
	return;
    if (style->visibility == 0)
	return;
    if (style->fill == 0 && style->stroke == 0)
	return;

/* drawing the Ellipse */
    cairo_save (cairo);

    cairo_translate (cairo, ellipse->cx + ellipse->rx / 2.0,
		     ellipse->cy + ellipse->ry / 2.0);
    cairo_scale (cairo, ellipse->rx / 2.0, ellipse->ry / 2.0);
    cairo_arc (cairo, 0.0, 0.0, 2.0, 0.0, 2.0 * GG_SVG_PI);
    cairo_restore (cairo);
    if (style->fill)
      {
	  /* filling */
	  gg_svg_set_brush (cairo, style);
	  if (style->stroke)
	      cairo_fill_preserve (cairo);
	  else
	      cairo_fill (cairo);
      }
    if (style->stroke)
      {
	  /* stroking */
	  gg_svg_set_pen (cairo, style);
	  cairo_stroke (cairo);
      }
}

static void
gg_svg_clip_ellipse (cairo_t * cairo, struct gg_svg_shape *shape)
{
/* clipping an SVG Ellipse */
    struct gg_svg_ellipse *ellipse = shape->data;
    if (ellipse->rx <= 0 || ellipse->ry <= 0)
	return;

/* clipping the Ellipse */
    cairo_save (cairo);

    cairo_translate (cairo, ellipse->cx + ellipse->rx / 2.0,
		     ellipse->cy + ellipse->ry / 2.0);
    cairo_scale (cairo, ellipse->rx / 2.0, ellipse->ry / 2.0);
    cairo_arc (cairo, 0.0, 0.0, 2.0, 0.0, 2.0 * GG_SVG_PI);
    cairo_restore (cairo);
    cairo_clip (cairo);
}

static void
gg_svg_draw_line (cairo_t * cairo, struct gg_svg_shape *shape,
		  struct gg_svg_style *style)
{
/* drawing an SVG Line */
    struct gg_svg_line *line = shape->data;
    if (style->visibility == 0)
	return;
    if (style->fill == 0 && style->stroke == 0)
	return;

/* drawing the Line */
    cairo_move_to (cairo, line->x1, line->y1);
    cairo_line_to (cairo, line->x2, line->y2);
    if (style->fill)
      {
	  /* filling */
	  gg_svg_set_brush (cairo, style);
	  cairo_fill (cairo);
      }
    if (style->stroke)
      {
	  /* stroking */
	  gg_svg_set_pen (cairo, style);
	  cairo_stroke (cairo);
      }
}

static void
gg_svg_clip_line (cairo_t * cairo, struct gg_svg_shape *shape)
{
/* clipping an SVG Line */
    struct gg_svg_line *line = shape->data;

/* clipping the Line */
    cairo_move_to (cairo, line->x1, line->y1);
    cairo_line_to (cairo, line->x2, line->y2);
    cairo_clip (cairo);
}

static void
gg_svg_draw_polyline (cairo_t * cairo, struct gg_svg_shape *shape,
		      struct gg_svg_style *style)
{
/* drawing an SVG Polyline */
    int iv;
    struct gg_svg_polyline *poly = shape->data;
    if (poly->points <= 0 || poly->x == NULL || poly->y == NULL)
	return;
    if (style->visibility == 0)
	return;
    if (style->fill == 0 && style->stroke == 0)
	return;

/* drawing the Polyline */
    for (iv = 0; iv < poly->points; iv++)
      {
	  if (iv == 0)
	      cairo_move_to (cairo, *(poly->x + iv), *(poly->y + iv));
	  else
	      cairo_line_to (cairo, *(poly->x + iv), *(poly->y + iv));
      }
    if (style->fill)
      {
	  /* filling */
	  gg_svg_set_brush (cairo, style);
	  if (style->stroke)
	      cairo_fill_preserve (cairo);
	  else
	      cairo_fill (cairo);
      }
    if (style->stroke)
      {
	  /* stroking */
	  gg_svg_set_pen (cairo, style);
	  cairo_stroke (cairo);
      }
}

static void
gg_svg_clip_polyline (cairo_t * cairo, struct gg_svg_shape *shape)
{
/* clipping an SVG Polyline */
    int iv;
    struct gg_svg_polyline *poly = shape->data;
    if (poly->points <= 0 || poly->x == NULL || poly->y == NULL)
	return;

/* clipping the Polyline */
    for (iv = 0; iv < poly->points; iv++)
      {
	  if (iv == 0)
	      cairo_move_to (cairo, *(poly->x + iv), *(poly->y + iv));
	  else
	      cairo_line_to (cairo, *(poly->x + iv), *(poly->y + iv));
      }
    cairo_clip (cairo);
}

static void
gg_svg_draw_polygon (cairo_t * cairo, struct gg_svg_shape *shape,
		     struct gg_svg_style *style)
{
/* drawing an SVG Polygon */
    int iv;
    struct gg_svg_polygon *poly = shape->data;
    if (poly->points <= 0 || poly->x == NULL || poly->y == NULL)
	return;
    if (style->visibility == 0)
	return;
    if (style->fill == 0 && style->stroke == 0)
	return;

/* drawing the Polygon */
    for (iv = 0; iv < poly->points; iv++)
      {
	  if (iv == 0)
	      cairo_move_to (cairo, *(poly->x + iv), *(poly->y + iv));
	  else
	      cairo_line_to (cairo, *(poly->x + iv), *(poly->y + iv));
      }
    cairo_close_path (cairo);
    if (style->fill)
      {
	  /* filling */
	  gg_svg_set_brush (cairo, style);
	  if (style->stroke)
	      cairo_fill_preserve (cairo);
	  else
	      cairo_fill (cairo);
      }
    if (style->stroke)
      {
	  /* stroking */
	  gg_svg_set_pen (cairo, style);
	  cairo_stroke (cairo);
      }
}

static void
gg_svg_clip_polygon (cairo_t * cairo, struct gg_svg_shape *shape)
{
/* clipping an SVG Polygon */
    int iv;
    struct gg_svg_polygon *poly = shape->data;
    if (poly->points <= 0 || poly->x == NULL || poly->y == NULL)
	return;

/* clipping the Polygon */
    for (iv = 0; iv < poly->points; iv++)
      {
	  if (iv == 0)
	      cairo_move_to (cairo, *(poly->x + iv), *(poly->y + iv));
	  else
	      cairo_line_to (cairo, *(poly->x + iv), *(poly->y + iv));
      }
    cairo_close_path (cairo);
    cairo_clip (cairo);
}

static void
svg_rotate (double x, double y, double angle, double *rx, double *ry)
{
/* rotate a point of an angle around the origin point */
    *rx = x * cos (angle) - y * sin (angle);
    *ry = y * cos (angle) + x * sin (angle);
}

static double
gg_svg_point_angle (double cx, double cy, double px, double py)
{
/* return angle between x axis and point knowing given center */
    return atan2 (py - cy, px - cx);
}

static void
gg_svg_arc_to_cairo (struct gg_svg_path_ellipt_arc *arc, double x1, double y1,
		     double *xc, double *yc, double *rx, double *rotation,
		     double *radii_ratio, double *angle1, double *angle2)
{
/*
/ computing the arguments for CairoArc starting from an SVG Elliptic Arc 
/ (simply a C translation of pyCairo Python code) 
*/
    double x3;
    double y3;
    double ry;
    double xe;
    double ye;
    double angle;
/* converting absolute x3 and y3 to relative */
    x3 = arc->x - x1;
    y3 = arc->y - y1;
    *rx = arc->rx;
    ry = arc->ry;
    *radii_ratio = ry / *rx;
    *rotation = arc->rotation * (GG_SVG_PI / 180.0);
/* cancel the rotation of the second point */
    svg_rotate (x3, y3, *rotation * -1, &xe, &ye);
    ye /= *radii_ratio;
/* find the angle between the second point and the x axis */
    angle = gg_svg_point_angle (0.0, 0.0, xe, ye);
/* put the second point onto the x axis */
    xe = sqrt ((xe * xe) + (ye * ye));
    ye = 0.0;
/* update the x radius if it is too small */
    if (*rx < xe / 2.0)
	*rx = xe / 2.0;
/* find one circle centre */
    *xc = xe / 2.0;
    *yc = sqrt ((*rx * *rx) - (*xc * *xc));
/* choose between the two circles according to flags */
    if ((arc->large_arc ^ arc->sweep) == 0)
	*yc = *yc * -1.0;
/* put the second point and the center back to their positions */
    svg_rotate (xe, ye, angle, &xe, &ye);
    svg_rotate (*xc, *yc, angle, xc, yc);
/* find the drawing angles */
    *angle1 = gg_svg_point_angle (*xc, *yc, 0.0, 0.0);
    *angle2 = gg_svg_point_angle (*xc, *yc, xe, ye);
}

static void
gg_svg_draw_path (cairo_t * cairo, struct gg_svg_shape *shape,
		  struct gg_svg_style *style)
{
/* drawing an SVG Path */
    struct gg_svg_path_item *item;
    struct gg_svg_path *path = shape->data;
    struct gg_svg_path_move *move;
    struct gg_svg_path_bezier *bezier;
    struct gg_svg_path_ellipt_arc *arc;
    double x;
    double y;
    double x0;
    double y0;
    double radii_ratio;
    double xc;
    double yc;
    double rx;
    double rotation;
    double angle1;
    double angle2;
    int is_new_path = 0;
    if (path->error)
	return;
    if (style->visibility == 0)
	return;
    if (style->fill == 0 && style->stroke == 0)
	return;

/* drawing the Path */
    item = path->first;
    while (item)
      {
	  if (is_new_path && item->type != GG_SVG_MOVE_TO)
	    {
		/* implicit MoveTO */
		cairo_get_current_point (cairo, &x, &y);
		cairo_move_to (cairo, x, y);
	    }
	  is_new_path = 0;
	  switch (item->type)
	    {
	    case GG_SVG_CLOSE_PATH:
		cairo_close_path (cairo);
		is_new_path = 1;
		break;
	    case GG_SVG_MOVE_TO:
		move = item->data;
		cairo_move_to (cairo, move->x, move->y);
		break;
	    case GG_SVG_LINE_TO:
		move = item->data;
		cairo_line_to (cairo, move->x, move->y);
		break;
	    case GG_SVG_CURVE_3:
		bezier = item->data;
		cairo_curve_to (cairo, bezier->x1, bezier->y1, bezier->x2,
				bezier->y2, bezier->x, bezier->y);
		break;
	    case GG_SVG_CURVE_4:
		bezier = item->data;
		cairo_get_current_point (cairo, &x0, &y0);
		cairo_curve_to (cairo, 2.0 / 3.0 * bezier->x1 + 1.0 / 3.0 * x0,
				2.0 / 3.0 * bezier->y1 + 1.0 / 3.0 * y0,
				2.0 / 3.0 * bezier->x1 + 1.0 / 3.0 * bezier->x2,
				2.0 / 3.0 * bezier->y1 + 1.0 / 3.0 * bezier->y2,
				bezier->y1, bezier->y2);
		break;
	    case GG_SVG_ELLIPT_ARC:
		arc = item->data;
		cairo_get_current_point (cairo, &x0, &y0);
		gg_svg_arc_to_cairo (arc, x0, y0, &xc, &yc, &rx, &rotation,
				     &radii_ratio, &angle1, &angle2);
		cairo_save (cairo);
		cairo_translate (cairo, x0, y0);
		cairo_rotate (cairo, rotation);
		cairo_scale (cairo, 1.0, radii_ratio);
		if (arc->sweep == 0)
		    cairo_arc_negative (cairo, xc, yc, rx, angle1, angle2);
		else
		    cairo_arc (cairo, xc, yc, rx, angle1, angle2);
		cairo_restore (cairo);
		break;
	    };
	  item = item->next;
      }
    if (style->fill)
      {
	  /* filling */
	  gg_svg_set_brush (cairo, style);
	  if (style->stroke)
	      cairo_fill_preserve (cairo);
	  else
	      cairo_fill (cairo);
      }
    if (style->stroke)
      {
	  /* stroking */
	  gg_svg_set_pen (cairo, style);
	  cairo_stroke (cairo);
      }
}

static void
gg_svg_clip_path (cairo_t * cairo, struct gg_svg_shape *shape)
{
/* clipping an SVG Path */
    struct gg_svg_path_item *item;
    struct gg_svg_path *path = shape->data;
    struct gg_svg_path_move *move;
    struct gg_svg_path_bezier *bezier;
    struct gg_svg_path_ellipt_arc *arc;
    double x;
    double y;
    double x0;
    double y0;
    double radii_ratio;
    double xc;
    double yc;
    double rx;
    double rotation;
    double angle1;
    double angle2;
    int is_new_path = 0;
    if (path->error)
	return;
/* clipping the Path */
    item = path->first;
    while (item)
      {
	  if (is_new_path && item->type != GG_SVG_MOVE_TO)
	    {
		/* implicit MoveTO */
		cairo_get_current_point (cairo, &x, &y);
		cairo_move_to (cairo, x, y);
	    }
	  is_new_path = 0;
	  switch (item->type)
	    {
	    case GG_SVG_CLOSE_PATH:
		cairo_close_path (cairo);
		is_new_path = 1;
		break;
	    case GG_SVG_MOVE_TO:
		move = item->data;
		cairo_move_to (cairo, move->x, move->y);
		break;
	    case GG_SVG_LINE_TO:
		move = item->data;
		cairo_line_to (cairo, move->x, move->y);
		break;
	    case GG_SVG_CURVE_3:
		bezier = item->data;
		cairo_curve_to (cairo, bezier->x1, bezier->y1, bezier->x2,
				bezier->y2, bezier->x, bezier->y);
		break;
	    case GG_SVG_CURVE_4:
		bezier = item->data;
		cairo_get_current_point (cairo, &x0, &y0);
		cairo_curve_to (cairo, 2.0 / 3.0 * bezier->x1 + 1.0 / 3.0 * x0,
				2.0 / 3.0 * bezier->y1 + 1.0 / 3.0 * y0,
				2.0 / 3.0 * bezier->x1 + 1.0 / 3.0 * bezier->x2,
				2.0 / 3.0 * bezier->y1 + 1.0 / 3.0 * bezier->y2,
				bezier->y1, bezier->y2);
		break;
	    case GG_SVG_ELLIPT_ARC:
		arc = item->data;
		cairo_get_current_point (cairo, &x0, &y0);
		gg_svg_arc_to_cairo (arc, x0, y0, &xc, &yc, &rx, &rotation,
				     &radii_ratio, &angle1, &angle2);
		cairo_save (cairo);
		cairo_translate (cairo, x0, y0);
		cairo_rotate (cairo, rotation);
		cairo_scale (cairo, 1.0, radii_ratio);
		if (arc->sweep == 0)
		    cairo_arc_negative (cairo, xc, yc, rx, angle1, angle2);
		else
		    cairo_arc (cairo, xc, yc, rx, angle1, angle2);
		cairo_restore (cairo);
		break;
	    };
	  item = item->next;
      }
    cairo_clip (cairo);
}

static void
gg_svg_draw_shape (cairo_t * cairo, struct gg_svg_shape *shape,
		   struct gg_svg_style *style)
{
/* drawing an SVG Shape */
    if (shape->style.visibility == 0)
	return;
    switch (shape->type)
      {
      case GG_SVG_RECT:
	  gg_svg_draw_rect (cairo, shape, style);
	  break;
      case GG_SVG_CIRCLE:
	  gg_svg_draw_circle (cairo, shape, style);
	  break;
      case GG_SVG_ELLIPSE:
	  gg_svg_draw_ellipse (cairo, shape, style);
	  break;
      case GG_SVG_LINE:
	  gg_svg_draw_line (cairo, shape, style);
	  break;
      case GG_SVG_POLYLINE:
	  gg_svg_draw_polyline (cairo, shape, style);
	  break;
      case GG_SVG_POLYGON:
	  gg_svg_draw_polygon (cairo, shape, style);
	  break;
      case GG_SVG_PATH:
	  gg_svg_draw_path (cairo, shape, style);
	  break;
      };
}

static void
gg_svg_transformation (cairo_t * cairo, struct gg_svg_transform *trans)
{
/* applying a single transformation */
    double angle;
    double tangent;
    struct gg_svg_matrix *mtrx;
    struct gg_svg_translate *translate;
    struct gg_svg_scale *scale;
    struct gg_svg_rotate *rotate;
    struct gg_svg_skew *skew;
    cairo_matrix_t matrix;
    cairo_matrix_t matrix_in;

    if (trans->data == NULL)
	return;
    switch (trans->type)
      {
      case GG_SVG_MATRIX:
	  mtrx = trans->data;
	  cairo_get_matrix (cairo, &matrix);
	  matrix_in.xx = mtrx->a;
	  matrix_in.yx = mtrx->b;
	  matrix_in.xy = mtrx->c;
	  matrix_in.yy = mtrx->d;
	  matrix_in.x0 = mtrx->e;
	  matrix_in.y0 = mtrx->f;
	  cairo_matrix_multiply (&matrix, &matrix_in, &matrix);
	  cairo_set_matrix (cairo, &matrix);
	  break;
      case GG_SVG_TRANSLATE:
	  translate = trans->data;
	  cairo_get_matrix (cairo, &matrix);
	  cairo_matrix_translate (&matrix, translate->tx, translate->ty);
	  cairo_set_matrix (cairo, &matrix);
	  break;
      case GG_SVG_SCALE:
	  scale = trans->data;
	  cairo_get_matrix (cairo, &matrix);
	  cairo_matrix_scale (&matrix, scale->sx, scale->sy);
	  cairo_set_matrix (cairo, &matrix);
	  break;
      case GG_SVG_ROTATE:
	  rotate = trans->data;
	  cairo_get_matrix (cairo, &matrix);
	  angle = rotate->angle * (GG_SVG_PI / 180.0);
	  cairo_matrix_translate (&matrix, rotate->cx, rotate->cy);
	  cairo_matrix_rotate (&matrix, angle);
	  cairo_matrix_translate (&matrix, -1.0 * rotate->cx,
				  -1.0 * rotate->cy);
	  cairo_set_matrix (cairo, &matrix);
	  break;
      case GG_SVG_SKEW_X:
	  skew = trans->data;
	  cairo_get_matrix (cairo, &matrix);
	  angle = skew->angle * (GG_SVG_PI / 180.0);
	  tangent = tan (angle);
	  matrix_in.xx = 1.0;
	  matrix_in.yx = 0.0;
	  matrix_in.xy = tangent;
	  matrix_in.yy = 1.0;
	  matrix_in.x0 = 0.0;
	  matrix_in.y0 = 0.0;
	  cairo_matrix_multiply (&matrix, &matrix_in, &matrix);
	  cairo_set_matrix (cairo, &matrix);
	  break;
      case GG_SVG_SKEW_Y:
	  skew = trans->data;
	  cairo_get_matrix (cairo, &matrix);
	  angle = skew->angle * (GG_SVG_PI / 180.0);
	  tangent = tan (angle);
	  matrix_in.xx = 1.0;
	  matrix_in.yx = tangent;
	  matrix_in.xy = 0.0;
	  matrix_in.yy = 1.0;
	  matrix_in.x0 = 0.0;
	  matrix_in.y0 = 0.0;
	  cairo_matrix_multiply (&matrix, &matrix_in, &matrix);
	  cairo_set_matrix (cairo, &matrix);
	  break;
      };
}

static void
gg_svg_apply_transformations (cairo_t * cairo, struct gg_svg_document *svg_doc,
			      struct gg_svg_shape *shape)
{
/* applying the whole transformations chain (supporting inheritance) */
    struct gg_svg_group *parent;
    struct gg_svg_parents chain;
    struct gg_svg_parent_ref *ref;
    struct gg_svg_group *group;
    struct gg_svg_transform *trans;
    cairo_matrix_t matrix;

/* initializing the chain as empty */
    chain.first = NULL;
    chain.last = NULL;

    parent = shape->parent;
    if (parent != NULL)
      {
	  /* identifying all direct parents in reverse order */
	  while (parent)
	    {
		gg_svg_add_parent (&chain, parent);
		parent = parent->parent;
	    }
      }

/* starting from the SVG Document basic transformations */
    matrix.xx = svg_doc->matrix.xx;
    matrix.yx = svg_doc->matrix.yx;
    matrix.xy = svg_doc->matrix.xy;
    matrix.yy = svg_doc->matrix.yy;
    matrix.x0 = svg_doc->matrix.x0;
    matrix.y0 = svg_doc->matrix.y0;
    cairo_set_matrix (cairo, &matrix);

    ref = chain.first;
    while (ref)
      {
	  /* chaining all transformations inherited by ancestors */
	  group = ref->parent;
	  if (group != NULL)
	    {
		trans = group->first_trans;
		while (trans)
		  {
		      gg_svg_transformation (cairo, trans);
		      trans = trans->next;
		  }
	    }
	  ref = ref->next;
      }

/* applying Shape specific transformations */
    trans = shape->first_trans;
    while (trans)
      {
	  gg_svg_transformation (cairo, trans);
	  trans = trans->next;
      }

    gg_svg_free_parents (&chain);
}

static void
gg_svg_apply_style (struct gg_svg_shape *shape, struct gg_svg_style *style)
{
/* applying the final style (supporting inheritance) */
    struct gg_svg_group *parent;
    struct gg_svg_parents chain;
    struct gg_svg_parent_ref *ref;
    struct gg_svg_group *group;

/* initializing the chain as empty */
    chain.first = NULL;
    chain.last = NULL;

    parent = shape->parent;
    if (parent != NULL)
      {
	  /* identifying all parents in reverse order */
	  while (parent)
	    {
		gg_svg_add_parent (&chain, parent);
		parent = parent->parent;
	    }
      }

/* initializing an Undefined Style */
    style->visibility = 1;
    style->opacity = 1.0;
    style->fill = 1;
    style->no_fill = 0;
    style->fill_url = NULL;
    style->fill_pointer = NULL;
    style->fill_rule = -1;
    style->fill_red = 0.0;
    style->fill_green = 0.0;
    style->fill_blue = 0.0;
    style->fill_opacity = 1.0;
    style->stroke = -1;
    style->no_stroke = 0;
    style->stroke_url = NULL;
    style->stroke_pointer = NULL;
    style->stroke_width = -1.0;
    style->stroke_linecap = -1;
    style->stroke_linejoin = -1;
    style->stroke_miterlimit = -1.0;
    style->stroke_dashitems = 0;
    style->stroke_dasharray = NULL;
    style->stroke_dashoffset = 0.0;
    style->stroke_red = -1.0;
    style->stroke_green = -1.0;
    style->stroke_blue = -1.0;
    style->stroke_opacity = -1.0;
    style->clip_url = NULL;
    style->clip_pointer = NULL;

    ref = chain.first;
    while (ref)
      {
	  /* chaining all style definitions inherited by ancestors */
	  group = ref->parent;
	  if (group != NULL)
	    {
		if (group->style.visibility >= 0)
		    style->visibility = group->style.visibility;
		style->opacity = group->style.opacity;
		if (group->style.fill >= 0)
		    style->fill = group->style.fill;
		if (group->style.no_fill)
		    style->no_fill = group->style.no_fill;
		if (group->style.fill_rule >= 0)
		    style->fill_rule = group->style.fill_rule;
		if (group->style.fill_url != NULL)
		    gg_svg_add_fill_gradient_url (style, group->style.fill_url);
		if (group->style.fill_red >= 0.0)
		    style->fill_red = group->style.fill_red;
		if (group->style.fill_green >= 0.0)
		    style->fill_green = group->style.fill_green;
		if (group->style.fill_blue >= 0.0)
		    style->fill_blue = group->style.fill_blue;
		if (group->style.fill_opacity >= 0.0)
		    style->fill_opacity = group->style.fill_opacity;
		if (group->style.stroke >= 0)
		    style->stroke = group->style.stroke;
		if (group->style.no_stroke >= 0)
		    style->no_stroke = group->style.no_stroke;
		if (group->style.stroke_width >= 0.0)
		    style->stroke_width = group->style.stroke_width;
		if (group->style.stroke_linecap >= 0)
		    style->stroke_linecap = group->style.stroke_linecap;
		if (group->style.stroke_linejoin >= 0)
		    style->stroke_linejoin = group->style.stroke_linejoin;
		if (group->style.stroke_miterlimit >= 0.0)
		    style->stroke_miterlimit = group->style.stroke_miterlimit;
		if (group->style.stroke_dashitems > 0)
		  {
		      style->stroke_dashitems = group->style.stroke_dashitems;
		      if (style->stroke_dasharray != NULL)
			  free (style->stroke_dasharray);
		      style->stroke_dasharray = NULL;
		      if (group->style.stroke_dashitems > 0)
			{
			    int i;
			    style->stroke_dasharray =
				malloc (sizeof (double) *
					group->style.stroke_dashitems);
			    for (i = 0; i < group->style.stroke_dashitems; i++)
				style->stroke_dasharray[i] =
				    group->style.stroke_dasharray[i];
			}
		      style->stroke_dashoffset = group->style.stroke_dashoffset;
		  }
		if (group->style.stroke_url != NULL)
		    gg_svg_add_stroke_gradient_url (style,
						    group->style.stroke_url);
		if (group->style.stroke_red >= 0.0)
		    style->stroke_red = group->style.stroke_red;
		if (group->style.stroke_green >= 0.0)
		    style->stroke_green = group->style.stroke_green;
		if (group->style.stroke_blue >= 0.0)
		    style->stroke_blue = group->style.stroke_blue;
		if (group->style.stroke_opacity >= 0.0)
		    style->stroke_opacity = group->style.stroke_opacity;
		if (group->style.clip_url != NULL)
		  {
		      gg_svg_add_clip_url (style, group->style.clip_url);
		      style->clip_pointer = group->style.clip_pointer;
		  }
	    }
	  ref = ref->next;
      }

/* applying Shape specific style definitions */
    if (shape->style.visibility >= 0)
	style->visibility = shape->style.visibility;
    style->opacity = shape->style.opacity;
    if (shape->style.fill >= 0)
	style->fill = shape->style.fill;
    if (shape->style.no_fill >= 0)
	style->no_fill = shape->style.no_fill;
    if (shape->style.fill_rule >= 0)
	style->fill_rule = shape->style.fill_rule;
    if (shape->style.fill_url != NULL)
	gg_svg_add_fill_gradient_url (style, shape->style.fill_url);
    if (shape->style.fill_red >= 0.0)
	style->fill_red = shape->style.fill_red;
    if (shape->style.fill_green >= 0.0)
	style->fill_green = shape->style.fill_green;
    if (shape->style.fill_blue >= 0.0)
	style->fill_blue = shape->style.fill_blue;
    if (shape->style.fill_opacity >= 0.0)
	style->fill_opacity = shape->style.fill_opacity;
    if (shape->style.stroke >= 0)
	style->stroke = shape->style.stroke;
    if (shape->style.no_stroke >= 0)
	style->no_stroke = shape->style.no_stroke;
    if (shape->style.stroke_width >= 0.0)
	style->stroke_width = shape->style.stroke_width;
    if (shape->style.stroke_linecap >= 0)
	style->stroke_linecap = shape->style.stroke_linecap;
    if (shape->style.stroke_linejoin >= 0)
	style->stroke_linejoin = shape->style.stroke_linejoin;
    if (shape->style.stroke_miterlimit >= 0.0)
	style->stroke_miterlimit = shape->style.stroke_miterlimit;
    if (shape->style.stroke_dashitems > 0)
      {
	  style->stroke_dashitems = shape->style.stroke_dashitems;
	  if (style->stroke_dasharray != NULL)
	      free (style->stroke_dasharray);
	  style->stroke_dasharray = NULL;
	  if (shape->style.stroke_dashitems > 0)
	    {
		int i;
		style->stroke_dasharray =
		    malloc (sizeof (double) * shape->style.stroke_dashitems);
		for (i = 0; i < shape->style.stroke_dashitems; i++)
		    style->stroke_dasharray[i] =
			shape->style.stroke_dasharray[i];
	    }
	  style->stroke_dashoffset = shape->style.stroke_dashoffset;
      }
    if (shape->style.stroke_url != NULL)
	gg_svg_add_stroke_gradient_url (style, shape->style.stroke_url);
    if (shape->style.stroke_red >= 0.0)
	style->stroke_red = shape->style.stroke_red;
    if (shape->style.stroke_green >= 0.0)
	style->stroke_green = shape->style.stroke_green;
    if (shape->style.stroke_blue >= 0.0)
	style->stroke_blue = shape->style.stroke_blue;
    if (shape->style.stroke_opacity >= 0.0)
	style->stroke_opacity = shape->style.stroke_opacity;
    if (shape->style.clip_url != NULL)
      {
	  gg_svg_add_clip_url (style, shape->style.clip_url);
	  style->clip_pointer = shape->style.clip_pointer;
      }
/* final adjustements */
    if (style->fill < 0)
	style->fill = 1;
    if (style->stroke < 0)
	style->stroke = 1;
    if (style->no_fill < 0)
	style->no_fill = 0;
    if (style->no_stroke < 0)
	style->no_stroke = 0;
    if (style->fill_red < 0.0 && style->fill_green < 0.0
	&& style->fill_blue < 0.0 && style->fill_url == NULL)
	style->no_fill = 1;
    if (style->stroke_red < 0.0 && style->stroke_green < 0.0
	&& style->stroke_blue < 0.0 && style->stroke_url == NULL)
	style->no_stroke = 1;
    if (style->no_fill)
	style->fill = 0;
    if (style->no_stroke)
	style->stroke = 0;
    if (style->fill)
      {
	  if (style->fill_rule < 0)
	      style->fill_rule = CAIRO_FILL_RULE_WINDING;
	  if (style->fill_red < 0.0 || style->fill_red > 1.0)
	      style->fill_red = 0.0;
	  if (style->fill_green < 0.0 || style->fill_green > 1.0)
	      style->fill_green = 0.0;
	  if (style->fill_blue < 0.0 || style->fill_blue > 1.0)
	      style->fill_blue = 0.0;
	  if (style->fill_opacity < 0.0 || style->fill_opacity > 1.0)
	      style->fill_opacity = 1.0;
      }
    if (style->stroke)
      {
	  if (style->stroke_width <= 0.0)
	      style->stroke_width = 1.0;
	  if (style->stroke_linecap < 0)
	      style->stroke_linecap = CAIRO_LINE_CAP_BUTT;
	  if (style->stroke_linejoin < 0)
	      style->stroke_linejoin = CAIRO_LINE_JOIN_MITER;
	  if (style->stroke_miterlimit < 0)
	      style->stroke_miterlimit = 4.0;
	  if (style->stroke_red < 0.0 || style->stroke_red > 1.0)
	      style->stroke_red = 0.0;
	  if (style->stroke_green < 0.0 || style->stroke_green > 1.0)
	      style->stroke_green = 0.0;
	  if (style->stroke_blue < 0.0 || style->stroke_blue > 1.0)
	      style->stroke_blue = 0.0;
	  if (style->stroke_opacity < 0.0 || style->stroke_opacity > 1.0)
	      style->stroke_opacity = 1.0;
      }
    gg_svg_free_parents (&chain);
}

static void
gg_svg_resolve_fill_url (struct gg_svg_document *svg_doc,
			 struct gg_svg_style *style)
{
/* attempting to resolve a FillGradient by URL */
    struct gg_svg_gradient *gradient = svg_doc->first_grad;
    while (gradient)
      {
	  if (gradient->id != NULL)
	    {
		if (strcmp (style->fill_url, gradient->id) == 0)
		  {
		      style->fill_pointer = gradient;
		      return;
		  }
	    }
	  gradient = gradient->next;
      }
    style->fill_pointer = NULL;
    style->fill = 0;
}

static void
gg_svg_resolve_stroke_url (struct gg_svg_document *svg_doc,
			   struct gg_svg_style *style)
{
/* attempting to resolve a StrokeGradient by URL */
    struct gg_svg_gradient *gradient = svg_doc->first_grad;
    while (gradient)
      {
	  if (gradient->id != NULL)
	    {
		if (strcmp (style->stroke_url, gradient->id) == 0)
		  {
		      style->stroke_pointer = gradient;
		      return;
		  }
	    }
	  gradient = gradient->next;
      }
    style->stroke_pointer = NULL;
    style->stroke = 0;
}

static void
gg_svg_apply_clip2 (cairo_t * cairo, struct gg_svg_item *clip_path)
{
/* attempting to apply a ClipPath - actuation */
    struct gg_svg_group *group;
    struct gg_svg_shape *shape;
    struct gg_svg_clip *clip;
    struct gg_svg_item *item = clip_path;
    while (item)
      {
	  /* looping on Items */
	  if (item->type == GG_SVG_ITEM_SHAPE && item->pointer != NULL)
	    {
		shape = item->pointer;
		switch (shape->type)
		  {
		  case GG_SVG_RECT:
		      gg_svg_clip_rect (cairo, shape);
		      break;
		  case GG_SVG_CIRCLE:
		      gg_svg_clip_circle (cairo, shape);
		      break;
		  case GG_SVG_ELLIPSE:
		      gg_svg_clip_ellipse (cairo, shape);
		      break;
		  case GG_SVG_LINE:
		      gg_svg_clip_line (cairo, shape);
		      break;
		  case GG_SVG_POLYLINE:
		      gg_svg_clip_polyline (cairo, shape);
		      break;
		  case GG_SVG_POLYGON:
		      gg_svg_clip_polygon (cairo, shape);
		      break;
		  case GG_SVG_PATH:
		      gg_svg_clip_path (cairo, shape);
		      break;
		  };
	    }
	  if (item->type == GG_SVG_ITEM_GROUP && item->pointer != NULL)
	    {
		group = item->pointer;
		gg_svg_apply_clip2 (cairo, group->first);
	    }
	  if (item->type == GG_SVG_ITEM_CLIP && item->pointer != NULL)
	    {
		clip = item->pointer;
		gg_svg_apply_clip2 (cairo, clip->first);
	    }
	  item = item->next;
      }
}

static void
gg_svg_apply_clip (cairo_t * cairo, struct gg_svg_item *item)
{
/* attempting to apply a ClipPath */
    if (item->type == GG_SVG_ITEM_CLIP && item->pointer != NULL)
      {
	  struct gg_svg_clip *clip = item->pointer;
	  gg_svg_apply_clip2 (cairo, clip->first);
      }
}

static void
gg_svg_render_item (cairo_t * cairo, struct gg_svg_document *svg_doc,
		    struct gg_svg_item *item)
{
/* rendering all SVG Items */
    struct gg_svg_group *group;
    struct gg_svg_shape *shape;
    struct gg_svg_style style;

    while (item)
      {
	  /* looping on Items */
	  if (item->type == GG_SVG_ITEM_SHAPE && item->pointer != NULL)
	    {
		shape = item->pointer;
		if (shape->is_defs || shape->is_flow_root)
		    ;
		else
		  {
		      gg_svg_apply_transformations (cairo, svg_doc, shape);
		      gg_svg_apply_style (shape, &style);
		      if (style.visibility)
			{
			    if (style.fill_url != NULL)
				gg_svg_resolve_fill_url (svg_doc, &style);
			    if (style.stroke_url != NULL)
				gg_svg_resolve_stroke_url (svg_doc, &style);
			    if (style.clip_url != NULL
				&& style.clip_pointer != NULL)
				gg_svg_apply_clip (cairo, style.clip_pointer);
			    gg_svg_draw_shape (cairo, shape, &style);
			    cairo_reset_clip (cairo);
			}
		      if (style.fill_url != NULL)
			  free (style.fill_url);
		      if (style.stroke_url != NULL)
			  free (style.stroke_url);
		      if (style.clip_url != NULL)
			  free (style.clip_url);
		  }
	    }
	  if (item->type == GG_SVG_ITEM_GROUP && item->pointer != NULL)
	    {
		group = item->pointer;
		if (group->is_defs || group->is_flow_root)
		    ;
		else
		    gg_svg_render_item (cairo, svg_doc, group->first);
	    }
	  item = item->next;
      }
}

static void
gg_svg_find_href (struct gg_svg_document *svg_doc, struct gg_svg_item *item,
		  const char *href, struct gg_svg_item **pointer)
{
/* attempting to recursively resolve an xlink:href  reference */
    struct gg_svg_group *group;
    struct gg_svg_shape *shape;

    while (item)
      {
	  /* looping on Items */
	  if (item->type == GG_SVG_ITEM_SHAPE && item->pointer != NULL)
	    {
		shape = item->pointer;
		if (shape->id != NULL)
		  {
		      if (strcmp (shape->id, href + 1) == 0)
			{
			    *pointer = item;
			    return;
			}
		  }
	    }
	  if (item->type == GG_SVG_ITEM_GROUP && item->pointer != NULL)
	    {
		group = item->pointer;
		if (group->id != NULL)
		  {
		      if (strcmp (group->id, href + 1) == 0)
			{
			    *pointer = item;
			    return;
			}
		  }
		gg_svg_find_href (svg_doc, group->first, href, pointer);
	    }
	  item = item->next;
      }
}

static void
gg_svg_replace_use (struct gg_svg_item *item, struct gg_svg_item *repl)
{
/* replacing an Use with the corrensponding resolved item */
    struct gg_svg_use *use = item->pointer;

    if (repl->type == GG_SVG_ITEM_SHAPE && repl->pointer)
      {
	  struct gg_svg_shape *shape = repl->pointer;
	  item->pointer = gg_svg_clone_shape (shape, use);
	  item->type = GG_SVG_ITEM_SHAPE;
      }
    if (repl->type == GG_SVG_ITEM_GROUP && repl->pointer)
      {
	  struct gg_svg_group *group = repl->pointer;
	  item->pointer = gg_svg_clone_group (group, use);
	  item->type = GG_SVG_ITEM_GROUP;
      }
    gg_svg_free_use (use);
}

static void
gg_svg_find_gradient_href (struct gg_svg_document *svg_doc, const char *href,
			   struct gg_svg_gradient **pointer)
{
/* attempting to recursively resolve an xlink:href  reference (Gradients) */
    struct gg_svg_gradient *grad = svg_doc->first_grad;

    while (grad)
      {
	  /* looping on Gradients */
	  if (strcmp (grad->id, href + 1) == 0)
	    {
		*pointer = grad;
		return;
	    }
	  grad = grad->next;
      }
}

static struct gg_svg_gradient *
gg_svg_replace_gradient (struct gg_svg_document *svg_doc,
			 struct gg_svg_gradient *gradient,
			 struct gg_svg_gradient *repl)
{
/* replacing a Gradient with the corrensponding resolved item */
    struct gg_svg_gradient *new_grad = gg_svg_clone_gradient (repl, gradient);
    new_grad->prev = gradient->prev;
    new_grad->next = gradient->next;
    if (gradient->prev != NULL)
	gradient->prev->next = new_grad;
    if (gradient->next != NULL)
	gradient->next->prev = new_grad;
    if (svg_doc->first_grad == gradient)
	svg_doc->first_grad = new_grad;
    if (svg_doc->last_grad == gradient)
	svg_doc->last_grad = new_grad;
    gg_svg_free_gradient (gradient);
    return new_grad;
}

static void
gg_svg_resolve_gradients_xlink_href (struct gg_svg_document *svg_doc)
{
/* resolving any indirect reference: Gradient xlink:href */
    struct gg_svg_gradient *grad = svg_doc->first_grad;
    struct gg_svg_gradient *ret;

    while (grad)
      {
	  /* looping on Gradients */
	  if (grad->xlink_href != NULL)
	    {
		gg_svg_find_gradient_href (svg_doc, grad->xlink_href, &ret);
		if (ret != NULL)
		    grad = gg_svg_replace_gradient (svg_doc, grad, ret);
	    }
	  grad = grad->next;
      }
}

static void
gg_svg_resolve_xlink_href (struct gg_svg_document *svg_doc,
			   struct gg_svg_item *item)
{
/* recursively resolving any indirect reference: xlink:href */
    struct gg_svg_group *group;
    struct gg_svg_use *use;
    struct gg_svg_clip *clip;
    struct gg_svg_item *ret;

    while (item)
      {
	  /* looping on Items */
	  if (item->type == GG_SVG_ITEM_USE && item->pointer != NULL)
	    {
		use = item->pointer;
		gg_svg_find_href (svg_doc, svg_doc->first, use->xlink_href,
				  &ret);
		if (ret != NULL)
		    gg_svg_replace_use (item, ret);
	    }
	  if (item->type == GG_SVG_ITEM_GROUP && item->pointer != NULL)
	    {
		group = item->pointer;
		gg_svg_resolve_xlink_href (svg_doc, group->first);
	    }
	  if (item->type == GG_SVG_ITEM_CLIP && item->pointer != NULL)
	    {
		clip = item->pointer;
		gg_svg_resolve_xlink_href (svg_doc, clip->first);
	    }
	  item = item->next;
      }
}

static void
gg_svg_find_clip_href (struct gg_svg_document *svg_doc,
		       struct gg_svg_item *item, const char *href,
		       struct gg_svg_item **pointer)
{
/* attempting to recursively resolve an xlink:href reference (ClipPath) */

    struct gg_svg_group *group;
    struct gg_svg_clip *clip;

    while (item)
      {
	  /* looping on Items */
	  if (item->type == GG_SVG_ITEM_CLIP && item->pointer != NULL)
	    {
		clip = item->pointer;
		if (clip->id != NULL)
		  {
		      if (strcmp (clip->id, href) == 0)
			{
			    *pointer = item;
			    return;
			}
		  }
	    }
	  if (item->type == GG_SVG_ITEM_GROUP && item->pointer != NULL)
	    {
		group = item->pointer;
		if (group->id != NULL)
		  {
		      if (strcmp (group->id, href + 1) == 0)
			{
			    *pointer = item;
			    return;
			}
		  }
		gg_svg_find_clip_href (svg_doc, group->first, href, pointer);
	    }
	  item = item->next;
      }
}

static void
gg_svg_resolve_clip_xlink_href (struct gg_svg_document *svg_doc,
				struct gg_svg_item *item)
{
/* recursively resolving any indirect reference: ClipPath url */
    struct gg_svg_group *group;
    struct gg_svg_shape *shape;
    struct gg_svg_use *use;
    struct gg_svg_item *ret = NULL;

    while (item)
      {
	  /* looping on Items */
	  if (item->type == GG_SVG_ITEM_USE && item->pointer != NULL)
	    {
		use = item->pointer;
		if (use->style.clip_url != NULL)
		  {
		      gg_svg_find_clip_href (svg_doc, svg_doc->first,
					     use->style.clip_url, &ret);
		      if (ret != NULL)
			  use->style.clip_pointer = ret;
		  }
	    }
	  if (item->type == GG_SVG_ITEM_SHAPE && item->pointer != NULL)
	    {
		shape = item->pointer;
		if (shape->style.clip_url != NULL)
		  {
		      gg_svg_find_clip_href (svg_doc, svg_doc->first,
					     shape->style.clip_url, &ret);
		      if (ret != NULL)
			  shape->style.clip_pointer = ret;
		  }
	    }
	  if (item->type == GG_SVG_ITEM_GROUP && item->pointer != NULL)
	    {
		group = item->pointer;
		if (group->style.clip_url != NULL)
		  {
		      gg_svg_find_clip_href (svg_doc, svg_doc->first,
					     group->style.clip_url, &ret);
		      if (ret != NULL)
			  group->style.clip_pointer = ret;
		  }
		gg_svg_resolve_clip_xlink_href (svg_doc, group->first);
	    }
	  item = item->next;
      }
}

static int
gg_svg_to_img (const void **img_out, int size, struct gg_svg_document *svg_doc)
{
/* rendering an SVG document as an RGBA image */
    cairo_surface_t *surface;
    cairo_t *cairo;
    double ratio_x;
    double ratio_y;
    double width;
    double height;
    double shift_x;
    double shift_y;
    int ret = 1;
    gGraphImagePtr img = NULL;
    int w;
    int h;
    int x;
    int y;
    const unsigned char *in_buf;
    unsigned char *out_buf = NULL;

    if (svg_doc->viewbox_x != DBL_MIN && svg_doc->viewbox_y != DBL_MIN
	&& svg_doc->viewbox_width != DBL_MIN
	&& svg_doc->viewbox_height != DBL_MIN)
      {
	  /* setting the SVG dimensions from the ViewBox */
	  if (svg_doc->width <= 0)
	      svg_doc->width = svg_doc->viewbox_width;
	  if (svg_doc->height <= 0)
	      svg_doc->height = svg_doc->viewbox_height;
      }
    else
      {
	  /* setting the ViewBox from the SVG dimensions */
	  svg_doc->viewbox_x = 0.0;
	  svg_doc->viewbox_y = 0.0;
	  svg_doc->viewbox_width = svg_doc->width;
	  svg_doc->viewbox_height = svg_doc->height;
      }
    if (svg_doc->width <= 0.0 || svg_doc->height <= 0.0)
	return 0;

/* setting the image dimensions */
    ratio_x = svg_doc->width / (double) size;
    ratio_y = svg_doc->height / (double) size;
    if (ratio_x > ratio_y)
      {
	  width = svg_doc->width / ratio_x;
	  height = svg_doc->height / ratio_x;
      }
    else
      {
	  width = svg_doc->width / ratio_y;
	  height = svg_doc->height / ratio_y;
      }

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    if (cairo_surface_status (surface) == CAIRO_STATUS_SUCCESS)
	;
    else
	return 0;
    cairo = cairo_create (surface);
    if (cairo_status (cairo) == CAIRO_STATUS_NO_MEMORY)
      {
	  fprintf (stderr, "CAIRO reports: Insufficient Memory\n");
	  ret = 0;
	  goto stop;
      }

/* priming a transparent background */
    cairo_rectangle (cairo, 0, 0, width, height);
    cairo_set_source_rgba (cairo, 0.0, 0.0, 0.0, 0.0);
    cairo_fill (cairo);
/* setting the basic document matrix */
    svg_doc->matrix.xx = 1.0;
    svg_doc->matrix.yx = 0.0;
    svg_doc->matrix.xy = 0.0;
    svg_doc->matrix.yy = 1.0;
    svg_doc->matrix.x0 = 0.0;
    svg_doc->matrix.y0 = 0.0;
    ratio_x = width / svg_doc->viewbox_width;
    ratio_y = height / svg_doc->viewbox_height;
    cairo_matrix_scale (&(svg_doc->matrix), ratio_x, ratio_y);
    shift_x = -1 * svg_doc->viewbox_x;
    shift_y = -1 * svg_doc->viewbox_y;
    cairo_matrix_translate (&(svg_doc->matrix), shift_x, shift_y);

/* resolving any xlink:href */
    gg_svg_resolve_gradients_xlink_href (svg_doc);
    gg_svg_resolve_clip_xlink_href (svg_doc, svg_doc->first);
    gg_svg_resolve_xlink_href (svg_doc, svg_doc->first);

/* recursively rendering all SVG Items */
    gg_svg_render_item (cairo, svg_doc, svg_doc->first);

/* accessing the CairoSurface buffer */
    w = cairo_image_surface_get_width (surface);
    h = cairo_image_surface_get_height (surface);
    cairo_surface_flush (surface);
    in_buf = cairo_image_surface_get_data (surface);
    if (in_buf == NULL)
      {
	  ret = 0;
	  goto stop;
      }

/* creating the Image buffer */
    out_buf = malloc (h * w * 4);
    if (out_buf == NULL)
      {
	  ret = 0;
	  goto stop;
      }
    for (y = 0; y < h; y++)
      {
	  /* looping on lines */ double multiplier;
	  double r;
	  double g;
	  double b;
	  const unsigned char *p_in = in_buf + (y * w * 4);
	  unsigned char *p_out = out_buf + (y * w * 4);
	  for (x = 0; x < w; x++)
	    {
		/* looping on columns */
		unsigned char alpha;
		unsigned char red;
		unsigned char green;
		unsigned char blue;
		if (gg_endian_arch ())
		  {
		      /* little endian byte order */
		      alpha = *(p_in + 3);
		      red = *(p_in + 2);
		      green = *(p_in + 1);
		      blue = *(p_in + 0);
		  }
		else
		  {
		      /* big endian byte order */
		      alpha = *(p_in + 0);
		      red = *(p_in + 1);
		      green = *(p_in + 2);
		      blue = *(p_in + 3);
		  }
		/* Cairo colors are pre-multiplied; normalizing */
		multiplier = 255.0 / (double) alpha;
		r = (double) red *multiplier;
		g = (double) green *multiplier;
		b = (double) blue *multiplier;
		if (r < 0.0)
		    red = 0;
		else if (r > 255.0)
		    red = 255;
		else
		    red = r;
		if (g < 0.0)
		    green = 0;
		else if (g > 255.0)
		    green = 255;
		else
		    green = g;
		if (b < 0.0)
		    blue = 0;
		else if (b > 255.0)
		    blue = 255;
		else
		    blue = b;
		*(p_out + 0) = red;
		*(p_out + 1) = green;
		*(p_out + 2) = blue;
		*(p_out + 3) = alpha;
		p_in += 4;
		p_out += 4;
	    }
      }

/* creating the output image */
    img =
	gg_image_create_from_bitmap (out_buf, GG_PIXEL_RGBA, w, h, 8, 4,
				     GGRAPH_SAMPLE_UINT, NULL, NULL);
/* releasing ownership on output buffer */
    out_buf = NULL;

  stop:
    if (out_buf != NULL)
	free (out_buf);
    cairo_surface_destroy (surface);
    cairo_destroy (cairo);
    *img_out = img;
    return ret;
}

GGRAPH_DECLARE int
gGraphCreateSVG (const unsigned char *svg, int svg_bytes, void **svg_handle)
{
/* parsing the SVG Document and returning an opaque handle */
    struct gg_svg_document *svg_doc;

    *svg_handle = NULL;
    svg_doc = gg_svg_parse_doc (svg, svg_bytes);
    if (svg_doc == NULL)
      {
	  /* unable to parse the SVG Document */
	  return GGRAPH_INVALID_SVG;
      }
    *svg_handle = svg_doc;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphGetSVGDims (void *svg_handle, double *width, double *height)
{
/* destroying the SVG document */
    struct gg_svg_document *svg_doc = (struct gg_svg_document *) svg_handle;
    if (svg_doc == NULL)
	return GGRAPH_INVALID_SVG;
    if (svg_doc->signature != GG_GRAPHICS_SVG_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_SVG;
    *width = svg_doc->width;
    *height = svg_doc->height;
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphFreeSVG (void *svg_handle)
{
/* parsing the SVG Document */
    struct gg_svg_document *svg_doc = (struct gg_svg_document *) svg_handle;
    if (svg_doc == NULL)
	return GGRAPH_INVALID_SVG;
    if (svg_doc->signature != GG_GRAPHICS_SVG_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_SVG;
    gg_svg_free_document (svg_doc);
    return GGRAPH_OK;
}

GGRAPH_DECLARE int
gGraphImageFromSVG (void *svg_handle, double size, const void **img_out)
{
/* rendering the SVG Document into a raster image */
    struct gg_svg_document *svg_doc = (struct gg_svg_document *) svg_handle;
    if (svg_doc == NULL)
	return GGRAPH_INVALID_SVG;
    if (svg_doc->signature != GG_GRAPHICS_SVG_MAGIC_SIGNATURE)
	return GGRAPH_INVALID_SVG;
/* SVG image rendering */
    if (!gg_svg_to_img (img_out, size, svg_doc))
      {
	  gg_svg_free_document (svg_doc);
	  return GGRAPH_INVALID_SVG;
      }
    return GGRAPH_OK;
}
