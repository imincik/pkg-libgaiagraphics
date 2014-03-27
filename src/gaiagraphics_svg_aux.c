/* 
/ gaiagraphics_svg_aux.c
/
/ SVG auxiliary methods
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

GGRAPH_PRIVATE void
gg_svg_free_transform (struct gg_svg_transform *p)
{
/* freeing an SVG Transform */
    if (p == NULL)
	return;
    if (p->data != NULL)
	free (p->data);
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_polyline (struct gg_svg_polyline *p)
{
/* freeing an SVG Polyline */
    if (p == NULL)
	return;
    if (p->x != NULL)
	free (p->x);
    if (p->y != NULL)
	free (p->y);
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_polygon (struct gg_svg_polygon *p)
{
/* freeing an SVG Polygon */
    if (p == NULL)
	return;
    if (p->x != NULL)
	free (p->x);
    if (p->y != NULL)
	free (p->y);
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_path_item (struct gg_svg_path_item *p)
{
/* freeing an SVG Path item */
    if (p == NULL)
	return;
    if (p->data != NULL)
	free (p->data);
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_path (struct gg_svg_path *p)
{
/* freeing an SVG Path */
    struct gg_svg_path_item *pp;
    struct gg_svg_path_item *ppn;
    if (p == NULL)
	return;
    pp = p->first;
    while (pp != NULL)
      {
	  ppn = pp->next;
	  gg_svg_free_path_item (pp);
	  pp = ppn;
      }
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_shape (struct gg_svg_shape *p)
{
/* freeing an SVG Shape */
    struct gg_svg_transform *ptn;
    struct gg_svg_transform *pt = p->first_trans;
    if (p->id != NULL)
	free (p->id);
    while (pt)
      {
	  ptn = pt->next;
	  gg_svg_free_transform (pt);
	  pt = ptn;
      }
    switch (p->type)
      {
      case GG_SVG_POLYLINE:
	  gg_svg_free_polyline (p->data);
	  break;
      case GG_SVG_POLYGON:
	  gg_svg_free_polygon (p->data);
	  break;
      case GG_SVG_PATH:
	  gg_svg_free_path (p->data);
	  break;
      default:
	  if (p->data)
	      free (p->data);
	  break;
      };
    if (p->style.stroke_dasharray != NULL)
	free (p->style.stroke_dasharray);
    if (p->style.fill_url != NULL)
	free (p->style.fill_url);
    if (p->style.stroke_url != NULL)
	free (p->style.stroke_url);
    if (p->style.clip_url != NULL)
	free (p->style.clip_url);
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_use (struct gg_svg_use *p)
{
/* freeing an SVG Use (xlink:href) */
    struct gg_svg_transform *pt;
    struct gg_svg_transform *ptn;
    if (p->xlink_href != NULL)
	free (p->xlink_href);
    pt = p->first_trans;
    while (pt)
      {
	  ptn = pt->next;
	  gg_svg_free_transform (pt);
	  pt = ptn;
      }
    if (p->style.stroke_dasharray != NULL)
	free (p->style.stroke_dasharray);
    if (p->style.fill_url != NULL)
	free (p->style.fill_url);
    if (p->style.stroke_url != NULL)
	free (p->style.stroke_url);
    if (p->style.clip_url != NULL)
	free (p->style.clip_url);
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_item (struct gg_svg_item *p)
{
/* freeing an SVG Item */
    if (p->type == GG_SVG_ITEM_GROUP)
	gg_svg_free_group ((struct gg_svg_group *) (p->pointer));
    if (p->type == GG_SVG_ITEM_SHAPE)
	gg_svg_free_shape ((struct gg_svg_shape *) (p->pointer));
    if (p->type == GG_SVG_ITEM_CLIP)
	gg_svg_free_clip ((struct gg_svg_clip *) (p->pointer));
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_clip (struct gg_svg_clip *p)
{
/* freeing an SVG Clip */
    struct gg_svg_item *pi;
    struct gg_svg_item *pin;
    if (p->id != NULL)
	free (p->id);
    pi = p->first;
    while (pi)
      {
	  pin = pi->next;
	  gg_svg_free_item (pi);
	  pi = pin;
      }
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_group (struct gg_svg_group *p)
{
/* freeing an SVG Group <g> */
    struct gg_svg_item *pi;
    struct gg_svg_item *pin;
    struct gg_svg_transform *pt;
    struct gg_svg_transform *ptn;
    if (p->id != NULL)
	free (p->id);
    pi = p->first;
    while (pi)
      {
	  pin = pi->next;
	  gg_svg_free_item (pi);
	  pi = pin;
      }
    pt = p->first_trans;
    while (pt)
      {
	  ptn = pt->next;
	  gg_svg_free_transform (pt);
	  pt = ptn;
      }
    if (p->style.stroke_dasharray != NULL)
	free (p->style.stroke_dasharray);
    if (p->style.fill_url != NULL)
	free (p->style.fill_url);
    if (p->style.stroke_url != NULL)
	free (p->style.stroke_url);
    if (p->style.clip_url != NULL)
	free (p->style.clip_url);
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_gradient_stop (struct gg_svg_gradient_stop *p)
{
/* freeing an SVG GradientStop */
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_gradient (struct gg_svg_gradient *p)
{
/* freeing an SVG Gradient */
    struct gg_svg_transform *pt;
    struct gg_svg_transform *ptn;
    struct gg_svg_gradient_stop *ps;
    struct gg_svg_gradient_stop *psn;
    if (p->id != NULL)
	free (p->id);
    if (p->xlink_href != NULL)
	free (p->xlink_href);
    pt = p->first_trans;
    while (pt)
      {
	  ptn = pt->next;
	  gg_svg_free_transform (pt);
	  pt = ptn;
      }
    ps = p->first_stop;
    while (ps)
      {
	  psn = ps->next;
	  gg_svg_free_gradient_stop (ps);
	  ps = psn;
      }
    free (p);
}

GGRAPH_PRIVATE void
gg_svg_free_document (struct gg_svg_document *p)
{
/* freeing an SVG Document */
    struct gg_svg_item *pi;
    struct gg_svg_item *pin;
    struct gg_svg_gradient *pg;
    struct gg_svg_gradient *pgn;
    pi = p->first;
    while (pi)
      {
	  pin = pi->next;
	  if (pi->type == GG_SVG_ITEM_GROUP)
	      gg_svg_free_group ((struct gg_svg_group *) (pi->pointer));
	  if (pi->type == GG_SVG_ITEM_SHAPE)
	      gg_svg_free_shape ((struct gg_svg_shape *) (pi->pointer));
	  if (pi->type == GG_SVG_ITEM_USE)
	      gg_svg_free_use ((struct gg_svg_use *) (pi->pointer));
	  if (pi->type == GG_SVG_ITEM_CLIP)
	      gg_svg_free_clip ((struct gg_svg_clip *) (pi->pointer));
	  free (pi);
	  pi = pin;
      }
    pg = p->first_grad;
    while (pg)
      {
	  pgn = pg->next;
	  gg_svg_free_gradient (pg);
	  pg = pgn;
      }
    free (p);
}

GGRAPH_PRIVATE struct gg_svg_matrix *
gg_svg_alloc_matrix (double a, double b, double c, double d, double e, double f)
{
/* allocating and initializing an SVG Matrix */
    struct gg_svg_matrix *p = malloc (sizeof (struct gg_svg_matrix));
    p->a = a;
    p->b = b;
    p->c = c;
    p->d = d;
    p->e = e;
    p->f = f;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_matrix *
gg_svg_clone_matrix (struct gg_svg_matrix *in)
{
/* cloning an SVG Matrix */
    struct gg_svg_matrix *p = malloc (sizeof (struct gg_svg_matrix));
    p->a = in->a;
    p->b = in->b;
    p->c = in->c;
    p->d = in->d;
    p->e = in->e;
    p->f = in->f;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_translate *
gg_svg_alloc_translate (double tx, double ty)
{
/* allocating and initializing an SVG Translate */
    struct gg_svg_translate *p = malloc (sizeof (struct gg_svg_translate));
    p->tx = tx;
    p->ty = ty;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_translate *
gg_svg_clone_translate (struct gg_svg_translate *in)
{
/* cloning an SVG Translate */
    struct gg_svg_translate *p = malloc (sizeof (struct gg_svg_translate));
    p->tx = in->tx;
    p->ty = in->ty;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_scale *
gg_svg_alloc_scale (double sx, double sy)
{
/* allocating and initializing an SVG Scale */
    struct gg_svg_scale *p = malloc (sizeof (struct gg_svg_scale));
    p->sx = sx;
    p->sy = sy;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_scale *
gg_svg_clone_scale (struct gg_svg_scale *in)
{
/* cloning an SVG Scale */
    struct gg_svg_scale *p = malloc (sizeof (struct gg_svg_scale));
    p->sx = in->sx;
    p->sy = in->sy;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_rotate *
gg_svg_alloc_rotate (double angle, double cx, double cy)
{
/* allocating and initializing an SVG Scale */
    struct gg_svg_rotate *p = malloc (sizeof (struct gg_svg_rotate));
    p->angle = angle;
    p->cx = cx;
    p->cy = cy;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_rotate *
gg_svg_clone_rotate (struct gg_svg_rotate *in)
{
/* cloning an SVG Scale */
    struct gg_svg_rotate *p = malloc (sizeof (struct gg_svg_rotate));
    p->angle = in->angle;
    p->cx = in->cx;
    p->cy = in->cy;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_skew *
gg_svg_alloc_skew (double angle)
{
/* allocating and initializing an SVG Skew */
    struct gg_svg_skew *p = malloc (sizeof (struct gg_svg_skew));
    p->angle = angle;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_skew *
gg_svg_clone_skew (struct gg_svg_skew *in)
{
/* cloning an SVG Skew */
    struct gg_svg_skew *p = malloc (sizeof (struct gg_svg_skew));
    p->angle = in->angle;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_transform *
gg_svg_alloc_transform (int type, void *data)
{
/* allocating and initializing an empty SVG Transform */
    struct gg_svg_transform *p = malloc (sizeof (struct gg_svg_transform));
    p->type = type;
    p->data = data;
    p->next = NULL;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_transform *
gg_svg_clone_transform (struct gg_svg_transform *in)
{
/* cloning SVG Transform */
    struct gg_svg_matrix *matrix;
    struct gg_svg_translate *translate;
    struct gg_svg_scale *scale;
    struct gg_svg_rotate *rotate;
    struct gg_svg_skew *skew;
    struct gg_svg_transform *p = malloc (sizeof (struct gg_svg_transform));
    p->type = in->type;
    switch (in->type)
      {
      case GG_SVG_MATRIX:
	  matrix = in->data;
	  p->data = gg_svg_clone_matrix (matrix);
	  break;
      case GG_SVG_TRANSLATE:
	  translate = in->data;
	  p->data = gg_svg_clone_translate (translate);
	  break;
      case GG_SVG_SCALE:
	  scale = in->data;
	  p->data = gg_svg_clone_scale (scale);
	  break;
      case GG_SVG_ROTATE:
	  rotate = in->data;
	  p->data = gg_svg_clone_rotate (rotate);
	  break;
      case GG_SVG_SKEW_X:
      case GG_SVG_SKEW_Y:
	  skew = in->data;
	  p->data = gg_svg_clone_skew (skew);
	  break;
      };
    p->next = NULL;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_rect *
gg_svg_alloc_rect (double x, double y, double width, double height, double rx,
		   double ry)
{
/* allocating and initializing an SVG Rect */
    struct gg_svg_rect *p = malloc (sizeof (struct gg_svg_rect));
    p->x = x;
    p->y = y;
    p->width = width;
    p->height = height;
    p->rx = rx;
    p->ry = ry;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_rect *
gg_svg_clone_rect (struct gg_svg_rect *in)
{
/* cloning an SVG Rect */
    struct gg_svg_rect *p = malloc (sizeof (struct gg_svg_rect));
    p->x = in->x;
    p->y = in->y;
    p->width = in->width;
    p->height = in->height;
    p->rx = in->rx;
    p->ry = in->ry;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_circle *
gg_svg_alloc_circle (double cx, double cy, double r)
{
/* allocating and initializing an SVG Circle */
    struct gg_svg_circle *p = malloc (sizeof (struct gg_svg_circle));
    p->cx = cx;
    p->cy = cy;
    p->r = r;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_circle *
gg_svg_clone_circle (struct gg_svg_circle *in)
{
/* cloning an SVG Circle */
    struct gg_svg_circle *p = malloc (sizeof (struct gg_svg_circle));
    p->cx = in->cx;
    p->cy = in->cy;
    p->r = in->r;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_ellipse *
gg_svg_alloc_ellipse (double cx, double cy, double rx, double ry)
{
/* allocating and initializing an SVG Ellipse */
    struct gg_svg_ellipse *p = malloc (sizeof (struct gg_svg_ellipse));
    p->cx = cx;
    p->cy = cy;
    p->rx = rx;
    p->ry = ry;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_ellipse *
gg_svg_clone_ellipse (struct gg_svg_ellipse *in)
{
/* cloning an SVG Ellipse */
    struct gg_svg_ellipse *p = malloc (sizeof (struct gg_svg_ellipse));
    p->cx = in->cx;
    p->cy = in->cy;
    p->rx = in->rx;
    p->ry = in->ry;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_line *
gg_svg_alloc_line (double x1, double y1, double x2, double y2)
{
/* allocating and initializing an SVG Line */
    struct gg_svg_line *p = malloc (sizeof (struct gg_svg_line));
    p->x1 = x1;
    p->y1 = y1;
    p->x2 = x2;
    p->y2 = y2;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_line *
gg_svg_clone_line (struct gg_svg_line *in)
{
/* cloning an SVG Line */
    struct gg_svg_line *p = malloc (sizeof (struct gg_svg_line));
    p->x1 = in->x1;
    p->y1 = in->y1;
    p->x2 = in->x2;
    p->y2 = in->y2;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_polyline *
gg_svg_alloc_polyline (int points, double *x, double *y)
{
/* allocating and initializing an SVG Polyline */
    struct gg_svg_polyline *p = malloc (sizeof (struct gg_svg_polyline));
    p->points = points;
    p->x = x;
    p->y = y;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_polyline *
gg_svg_clone_polyline (struct gg_svg_polyline *in)
{
/* cloning an SVG Polyline */
    int iv;
    struct gg_svg_polyline *p = malloc (sizeof (struct gg_svg_polyline));
    p->points = in->points;
    p->x = malloc (sizeof (double) * in->points);
    p->y = malloc (sizeof (double) * in->points);
    for (iv = 0; iv < in->points; iv++)
      {
	  *(p->x + iv) = *(in->x + iv);
	  *(p->y + iv) = *(in->y + iv);
      }
    return p;
}

GGRAPH_PRIVATE struct gg_svg_polygon *
gg_svg_alloc_polygon (int points, double *x, double *y)
{
/* allocating and initializing an SVG Polygon */
    struct gg_svg_polygon *p = malloc (sizeof (struct gg_svg_polygon));
    p->points = points;
    p->x = x;
    p->y = y;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_polygon *
gg_svg_clone_polygon (struct gg_svg_polygon *in)
{
/* cloning an SVG Polygon */
    int iv;
    struct gg_svg_polygon *p = malloc (sizeof (struct gg_svg_polygon));
    p->points = in->points;
    p->x = malloc (sizeof (double) * in->points);
    p->y = malloc (sizeof (double) * in->points);
    for (iv = 0; iv < in->points; iv++)
      {
	  *(p->x + iv) = *(in->x + iv);
	  *(p->y + iv) = *(in->y + iv);
      }
    return p;
}

GGRAPH_PRIVATE struct gg_svg_path_move *
gg_svg_alloc_path_move (double x, double y)
{
/* allocating and initializing an SVG Path MoveTo or LineTo */
    struct gg_svg_path_move *p = malloc (sizeof (struct gg_svg_path_move));
    p->x = x;
    p->y = y;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_path_move *
gg_svg_clone_path_move (struct gg_svg_path_move *in)
{
/* cloning an SVG Path MoveTo or LineTo */
    struct gg_svg_path_move *p = malloc (sizeof (struct gg_svg_path_move));
    p->x = in->x;
    p->y = in->y;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_path_bezier *
gg_svg_alloc_path_bezier (double x1, double y1, double x2, double y2, double x,
			  double y)
{
/* allocating and initializing an SVG Bezier Curve */
    struct gg_svg_path_bezier *p = malloc (sizeof (struct gg_svg_path_bezier));
    p->x1 = x1;
    p->y1 = y1;
    p->x2 = x2;
    p->y2 = y2;
    p->x = x;
    p->y = y;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_path_bezier *
gg_svg_clone_path_bezier (struct gg_svg_path_bezier *in)
{
/* cloning an SVG Bezier Curve */
    struct gg_svg_path_bezier *p = malloc (sizeof (struct gg_svg_path_bezier));
    p->x1 = in->x1;
    p->y1 = in->y1;
    p->x2 = in->x2;
    p->y2 = in->y2;
    p->x = in->x;
    p->y = in->y;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_path_ellipt_arc *
gg_svg_alloc_path_ellipt_arc (double rx, double ry, double rotation,
			      int large_arc, int sweep, double x, double y)
{
/* allocating and initializing an SVG Elliptic Arc */
    struct gg_svg_path_ellipt_arc *p =
	malloc (sizeof (struct gg_svg_path_ellipt_arc));
    p->rx = rx;
    p->ry = ry;
    p->rotation = rotation;
    if (large_arc == 0)
	p->large_arc = 0;
    else
	p->large_arc = 1;
    if (sweep == 0)
	p->sweep = 0;
    else
	p->sweep = 1;
    p->x = x;
    p->y = y;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_path_ellipt_arc *
gg_svg_clone_path_ellipt_arc (struct gg_svg_path_ellipt_arc *in)
{
/* cloning an SVG Elliptic Arc */
    struct gg_svg_path_ellipt_arc *p =
	malloc (sizeof (struct gg_svg_path_ellipt_arc));
    p->rx = in->rx;
    p->ry = in->ry;
    p->rotation = in->rotation;
    p->large_arc = in->large_arc;
    p->sweep = in->sweep;
    p->x = in->x;
    p->y = in->y;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_path_item *
gg_svg_alloc_path_item (int type, void *data)
{
/* allocating and initializing an empty SVG Path item */
    struct gg_svg_path_item *p = malloc (sizeof (struct gg_svg_path_item));
    p->type = type;
    p->data = data;
    p->next = NULL;
    return p;
}

GGRAPH_PRIVATE void
gg_svg_add_path_item (struct gg_svg_path *path, int type, void *data)
{
/* add a Command to an SVG Path */
    struct gg_svg_path_item *p = gg_svg_alloc_path_item (type, data);
    if (path->first == NULL)
	path->first = p;
    if (path->last != NULL)
	path->last->next = p;
    path->last = p;
}

GGRAPH_PRIVATE struct gg_svg_path *
gg_svg_alloc_path (void)
{
/* allocating and initializing an empty SVG Path */
    struct gg_svg_path *p = malloc (sizeof (struct gg_svg_path));
    p->first = NULL;
    p->last = NULL;
    p->error = 0;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_path *
gg_svg_clone_path (struct gg_svg_path *in)
{
/* cloning an empty SVG Path */
    struct gg_svg_path_move *move;
    struct gg_svg_path_bezier *bezier;
    struct gg_svg_path_ellipt_arc *arc;
    struct gg_svg_path_item *pp;
    struct gg_svg_path *p = malloc (sizeof (struct gg_svg_path));
    p->first = NULL;
    p->last = NULL;
    pp = in->first;
    while (pp != NULL)
      {
	  switch (pp->type)
	    {
	    case GG_SVG_MOVE_TO:
	    case GG_SVG_LINE_TO:
		move = gg_svg_clone_path_move (pp->data);
		gg_svg_add_path_item (p, pp->type, move);
		break;
	    case GG_SVG_CURVE_3:
	    case GG_SVG_CURVE_4:
		bezier = gg_svg_clone_path_bezier (pp->data);
		gg_svg_add_path_item (p, pp->type, bezier);
		break;
	    case GG_SVG_ELLIPT_ARC:
		arc = gg_svg_clone_path_ellipt_arc (pp->data);
		gg_svg_add_path_item (p, pp->type, arc);
		break;
	    case GG_SVG_CLOSE_PATH:
		gg_svg_add_path_item (p, pp->type, NULL);
		break;
	    };
	  pp = pp->next;
      }
    p->error = in->error;
    return p;
}

GGRAPH_PRIVATE void
gg_svg_add_clip_url (struct gg_svg_style *style, const char *url)
{
/* adding an URL (ClipPath ref) to some Style */
    int len;
    if (style->clip_url != NULL)
	free (style->clip_url);
    if (url == NULL)
      {
	  style->clip_url = NULL;
	  return;
      }
    len = strlen (url);
    style->clip_url = malloc (len + 1);
    strcpy (style->clip_url, url);
}

GGRAPH_PRIVATE void
gg_svg_add_fill_gradient_url (struct gg_svg_style *style, const char *url)
{
/* adding an URL (Fill Gradient ref) to some Style */
    int len;
    if (style->fill_url != NULL)
	free (style->fill_url);
    if (url == NULL)
      {
	  style->fill_url = NULL;
	  return;
      }
    len = strlen (url);
    style->fill_url = malloc (len + 1);
    strcpy (style->fill_url, url);
}

GGRAPH_PRIVATE void
gg_svg_add_stroke_gradient_url (struct gg_svg_style *style, const char *url)
{
/* adding an URL (Stroke Gradient ref) to some Style */
    int len;
    if (style->stroke_url != NULL)
	free (style->stroke_url);
    if (url == NULL)
      {
	  style->stroke_url = NULL;
	  return;
      }
    len = strlen (url);
    style->stroke_url = malloc (len + 1);
    strcpy (style->stroke_url, url);
}

GGRAPH_PRIVATE struct gg_svg_shape *
gg_svg_alloc_shape (int type, void *data, struct gg_svg_group *parent)
{
/* allocating and initializing an empty SVG Shape */
    struct gg_svg_shape *p = malloc (sizeof (struct gg_svg_shape));
    p->id = NULL;
    p->type = type;
    p->data = data;
    p->parent = parent;
    p->style.visibility = -1;
    p->style.opacity = 1.0;
    p->style.fill = -1;
    p->style.no_fill = -1;
    p->style.fill_rule = -1;
    p->style.fill_url = NULL;
    p->style.fill_pointer = NULL;
    p->style.fill_red = -1.0;
    p->style.fill_green = -1.0;
    p->style.fill_blue = -1.0;
    p->style.fill_opacity = -1.0;
    p->style.stroke = -1;
    p->style.no_stroke = -1;
    p->style.stroke_width = -1.0;
    p->style.stroke_linecap = -1;
    p->style.stroke_linejoin = -1;
    p->style.stroke_miterlimit = -1.0;
    p->style.stroke_dashitems = 0;
    p->style.stroke_dasharray = NULL;
    p->style.stroke_dashoffset = 0.0;
    p->style.stroke_url = NULL;
    p->style.stroke_pointer = NULL;
    p->style.stroke_red = -1.0;
    p->style.stroke_green = -1.0;
    p->style.stroke_blue = -1.0;
    p->style.stroke_opacity = -1.0;
    p->style.clip_url = NULL;
    p->style.clip_pointer = NULL;
    p->first_trans = NULL;
    p->last_trans = NULL;
    p->is_defs = 0;
    p->is_flow_root = 0;
    p->next = NULL;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_shape *
gg_svg_clone_shape (struct gg_svg_shape *in, struct gg_svg_use *use)
{
/* cloning an SVG Shape */
    struct gg_svg_rect *rect;
    struct gg_svg_circle *circle;
    struct gg_svg_ellipse *ellipse;
    struct gg_svg_line *line;
    struct gg_svg_polyline *polyline;
    struct gg_svg_polygon *polygon;
    struct gg_svg_path *path;
    struct gg_svg_transform *pt;
    struct gg_svg_transform *trans;
    struct gg_svg_shape *out = malloc (sizeof (struct gg_svg_shape));
    out->id = NULL;
    out->type = in->type;
    switch (in->type)
      {
      case GG_SVG_RECT:
	  rect = in->data;
	  out->data = gg_svg_clone_rect (rect);
	  break;
      case GG_SVG_CIRCLE:
	  circle = in->data;
	  out->data = gg_svg_clone_circle (circle);
	  break;
      case GG_SVG_ELLIPSE:
	  ellipse = in->data;
	  out->data = gg_svg_clone_ellipse (ellipse);
	  break;
      case GG_SVG_LINE:
	  line = in->data;
	  out->data = gg_svg_clone_line (line);
	  break;
      case GG_SVG_POLYLINE:
	  polyline = in->data;
	  out->data = gg_svg_clone_polyline (polyline);
	  break;
      case GG_SVG_POLYGON:
	  polygon = in->data;
	  out->data = gg_svg_clone_polygon (polygon);
	  break;
      case GG_SVG_PATH:
	  path = in->data;
	  out->data = gg_svg_clone_path (path);
	  break;
      }
    if (use != NULL)
	out->parent = use->parent;
    else
	out->parent = in->parent;
    out->style.visibility = in->style.visibility;
    out->style.opacity = in->style.opacity;
    out->style.fill = in->style.fill;
    out->style.no_fill = in->style.no_fill;
    out->style.fill_rule = in->style.fill_rule;
    out->style.fill_url = NULL;
    out->style.fill_pointer = NULL;
    if (in->style.fill_url != NULL)
	gg_svg_add_fill_gradient_url (&(out->style), in->style.fill_url);
    out->style.fill_red = in->style.fill_red;
    out->style.fill_green = in->style.fill_green;
    out->style.fill_blue = in->style.fill_blue;
    out->style.fill_opacity = in->style.fill_opacity;
    out->style.stroke = in->style.stroke;
    out->style.no_stroke = in->style.no_stroke;
    out->style.stroke_width = in->style.stroke_width;
    out->style.stroke_linecap = in->style.stroke_linecap;
    out->style.stroke_linejoin = in->style.stroke_linejoin;
    out->style.stroke_miterlimit = in->style.stroke_miterlimit;
    out->style.stroke_dashitems = 0;
    out->style.stroke_dasharray = NULL;
    if (in->style.stroke_dashitems > 0)
      {
	  out->style.stroke_dashitems = in->style.stroke_dashitems;
	  if (out->style.stroke_dasharray != NULL)
	      free (out->style.stroke_dasharray);
	  out->style.stroke_dasharray = NULL;
	  if (in->style.stroke_dashitems > 0)
	    {
		int i;
		out->style.stroke_dasharray =
		    malloc (sizeof (double) * in->style.stroke_dashitems);
		for (i = 0; i < in->style.stroke_dashitems; i++)
		    out->style.stroke_dasharray[i] =
			in->style.stroke_dasharray[i];
	    }
	  out->style.stroke_dashoffset = in->style.stroke_dashoffset;
      }
    out->style.stroke_url = NULL;
    out->style.stroke_pointer = NULL;
    if (in->style.stroke_url != NULL)
	gg_svg_add_stroke_gradient_url (&(out->style), in->style.stroke_url);
    out->style.stroke_red = in->style.stroke_red;
    out->style.stroke_green = in->style.stroke_green;
    out->style.stroke_blue = in->style.stroke_blue;
    out->style.stroke_opacity = in->style.stroke_opacity;
    out->style.clip_url = NULL;
    out->style.clip_pointer = NULL;
    if (in->style.clip_url != NULL)
	gg_svg_add_clip_url (&(out->style), in->style.clip_url);
    out->first_trans = NULL;
    out->last_trans = NULL;
    pt = in->first_trans;
    while (pt)
      {
	  /* clonig all transformations */
	  trans = gg_svg_clone_transform (pt);
	  if (out->first_trans == NULL)
	      out->first_trans = trans;
	  if (out->last_trans != NULL)
	      out->last_trans->next = trans;
	  out->last_trans = trans;
	  pt = pt->next;
      }
    out->is_defs = 0;
    out->is_flow_root = 0;
    out->next = NULL;
    if (use != NULL)
      {
	  /* adding the Use-level defs */
	  pt = use->first_trans;
	  while (pt)
	    {
		/* clonig all Use transformations */
		trans = gg_svg_clone_transform (pt);
		if (out->first_trans == NULL)
		    out->first_trans = trans;
		if (out->last_trans != NULL)
		    out->last_trans->next = trans;
		out->last_trans = trans;
		pt = pt->next;
	    }
	  if (use->x != DBL_MAX || use->y != DBL_MAX)
	    {
		/* adding the implict Use Translate transformation */
		struct gg_svg_translate *translate = NULL;
		if (use->x != DBL_MAX && use->y != DBL_MAX)
		    translate = gg_svg_alloc_translate (use->x, use->y);
		else if (use->x != DBL_MAX)
		    translate = gg_svg_alloc_translate (use->x, 0.0);
		else if (use->y != DBL_MAX)
		    translate = gg_svg_alloc_translate (0.0, use->y);
		trans = gg_svg_alloc_transform (GG_SVG_TRANSLATE, translate);
		if (out->first_trans == NULL)
		    out->first_trans = trans;
		if (out->last_trans != NULL)
		    out->last_trans->next = trans;
		out->last_trans = trans;
	    }
	  /* Use-level styles */
	  if (use->style.visibility >= 0)
	      out->style.visibility = use->style.visibility;
	  out->style.opacity = use->style.opacity;
	  if (use->style.fill >= 0)
	      out->style.fill = use->style.fill;
	  if (use->style.no_fill >= 0)
	      out->style.no_fill = use->style.no_fill;
	  if (use->style.fill_rule >= 0)
	      out->style.fill_rule = use->style.fill_rule;
	  if (use->style.fill_url != NULL)
	      gg_svg_add_fill_gradient_url (&(out->style), use->style.fill_url);
	  if (use->style.fill_red >= 0.0)
	      out->style.fill_red = use->style.fill_red;
	  if (use->style.fill_green >= 0.0)
	      out->style.fill_green = use->style.fill_green;
	  if (use->style.fill_blue >= 0.0)
	      out->style.fill_blue = use->style.fill_blue;
	  if (use->style.fill_opacity >= 0.0)
	      out->style.fill_opacity = use->style.fill_opacity;
	  if (use->style.stroke >= 0)
	      out->style.stroke = use->style.stroke;
	  if (use->style.no_stroke >= 0)
	      out->style.no_stroke = use->style.no_stroke;
	  if (use->style.stroke_width >= 0.0)
	      out->style.stroke_width = use->style.stroke_width;
	  if (use->style.stroke_linecap >= 0)
	      out->style.stroke_linecap = use->style.stroke_linecap;
	  if (use->style.stroke_linejoin >= 0)
	      out->style.stroke_linejoin = use->style.stroke_linejoin;
	  if (use->style.stroke_miterlimit >= 0.0)
	      out->style.stroke_miterlimit = use->style.stroke_miterlimit;
	  if (use->style.stroke_dashitems > 0)
	    {
		out->style.stroke_dashitems = use->style.stroke_dashitems;
		if (out->style.stroke_dasharray != NULL)
		    free (out->style.stroke_dasharray);
		out->style.stroke_dasharray = NULL;
		if (use->style.stroke_dashitems > 0)
		  {
		      int i;
		      out->style.stroke_dasharray =
			  malloc (sizeof (double) *
				  use->style.stroke_dashitems);
		      for (i = 0; i < use->style.stroke_dashitems; i++)
			  out->style.stroke_dasharray[i] =
			      use->style.stroke_dasharray[i];
		  }
		out->style.stroke_dashoffset = use->style.stroke_dashoffset;
	    }
	  if (use->style.stroke_url != NULL)
	      gg_svg_add_stroke_gradient_url (&(out->style),
					      use->style.stroke_url);
	  if (use->style.stroke_red >= 0.0)
	      out->style.stroke_red = use->style.stroke_red;
	  if (use->style.stroke_green >= 0.0)
	      out->style.stroke_green = use->style.stroke_green;
	  if (use->style.stroke_blue >= 0.0)
	      out->style.stroke_blue = use->style.stroke_blue;
	  if (use->style.stroke_opacity >= 0.0)
	      out->style.stroke_opacity = use->style.stroke_opacity;
	  if (use->style.clip_url != NULL)
	      gg_svg_add_clip_url (&(out->style), use->style.clip_url);
      }
    return out;
}

GGRAPH_PRIVATE void
gg_svg_add_shape_id (struct gg_svg_shape *shape, const char *id)
{
/* setting the ID for some Shape */
    int len = strlen (id);
    if (shape->id != NULL)
	free (shape->id);
    shape->id = malloc (len + 1);
    strcpy (shape->id, id);
}

GGRAPH_PRIVATE struct gg_svg_use *
gg_svg_alloc_use (void *parent, const char *xlink_href, double x, double y,
		  double width, double height)
{
/* allocating and initializing an empty SVG Use (xlink:href) */
    int len = strlen (xlink_href);
    struct gg_svg_use *p = malloc (sizeof (struct gg_svg_use));
    p->xlink_href = malloc (len + 1);
    strcpy (p->xlink_href, xlink_href);
    p->x = x;
    p->y = y;
    p->width = width;
    p->height = height;
    p->parent = parent;
    p->style.visibility = -1;
    p->style.opacity = 1.0;
    p->style.fill = -1;
    p->style.no_fill = -1;
    p->style.fill_rule = -1;
    p->style.fill_url = NULL;
    p->style.fill_pointer = NULL;
    p->style.fill_red = -1.0;
    p->style.fill_green = -1.0;
    p->style.fill_blue = -1.0;
    p->style.fill_opacity = -1.0;
    p->style.stroke = -1;
    p->style.no_stroke = -1;
    p->style.stroke_width = -1.0;
    p->style.stroke_linecap = -1;
    p->style.stroke_linejoin = -1;
    p->style.stroke_miterlimit = -1.0;
    p->style.stroke_dashitems = 0;
    p->style.stroke_dasharray = NULL;
    p->style.stroke_dashoffset = 0.0;
    p->style.stroke_url = NULL;
    p->style.stroke_pointer = NULL;
    p->style.stroke_red = -1.0;
    p->style.stroke_green = -1.0;
    p->style.stroke_blue = -1.0;
    p->style.stroke_opacity = -1.0;
    p->style.clip_url = NULL;
    p->style.clip_pointer = NULL;
    p->first_trans = NULL;
    p->last_trans = NULL;
    p->next = NULL;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_use *
gg_svg_clone_use (struct gg_svg_use *in)
{
/* cloning an SVG Use (xlink:href) */
    struct gg_svg_transform *pt;
    struct gg_svg_transform *trans;
    int len = strlen (in->xlink_href);
    struct gg_svg_use *out = malloc (sizeof (struct gg_svg_use));
    out->xlink_href = malloc (len + 1);
    strcpy (out->xlink_href, in->xlink_href);
    out->x = in->x;
    out->y = in->y;
    out->width = in->width;
    out->height = in->height;
    out->parent = in->parent;
    out->style.visibility = in->style.visibility;
    out->style.opacity = in->style.opacity;
    out->style.fill = in->style.fill;
    out->style.no_fill = in->style.no_fill;
    out->style.fill_rule = in->style.fill_rule;
    out->style.fill_url = NULL;
    out->style.fill_pointer = NULL;
    if (in->style.fill_url != NULL)
	gg_svg_add_fill_gradient_url (&(out->style), in->style.fill_url);
    out->style.fill_red = in->style.fill_red;
    out->style.fill_green = in->style.fill_green;
    out->style.fill_blue = in->style.fill_blue;
    out->style.fill_opacity = in->style.fill_opacity;
    out->style.stroke = in->style.stroke;
    out->style.no_stroke = in->style.no_stroke;
    out->style.stroke_width = in->style.stroke_width;
    out->style.stroke_linecap = in->style.stroke_linecap;
    out->style.stroke_linejoin = in->style.stroke_linejoin;
    out->style.stroke_miterlimit = in->style.stroke_miterlimit;
    out->style.stroke_dashitems = 0;
    out->style.stroke_dasharray = NULL;
    if (in->style.stroke_dashitems > 0)
      {
	  out->style.stroke_dashitems = in->style.stroke_dashitems;
	  if (out->style.stroke_dasharray != NULL)
	      free (out->style.stroke_dasharray);
	  out->style.stroke_dasharray = NULL;
	  if (in->style.stroke_dashitems > 0)
	    {
		int i;
		out->style.stroke_dasharray =
		    malloc (sizeof (double) * in->style.stroke_dashitems);
		for (i = 0; i < in->style.stroke_dashitems; i++)
		    out->style.stroke_dasharray[i] =
			in->style.stroke_dasharray[i];
	    }
	  out->style.stroke_dashoffset = in->style.stroke_dashoffset;
      }
    out->style.stroke_url = NULL;
    out->style.stroke_pointer = NULL;
    if (in->style.stroke_url != NULL)
	gg_svg_add_stroke_gradient_url (&(out->style), in->style.stroke_url);
    out->style.stroke_red = in->style.stroke_red;
    out->style.stroke_green = in->style.stroke_green;
    out->style.stroke_blue = in->style.stroke_blue;
    out->style.stroke_opacity = in->style.stroke_opacity;
    out->style.clip_url = NULL;
    out->style.clip_pointer = NULL;
    if (in->style.clip_url != NULL)
	gg_svg_add_clip_url (&(out->style), in->style.clip_url);
    out->first_trans = NULL;
    out->last_trans = NULL;
    pt = in->first_trans;
    while (pt)
      {
	  /* clonig all transformations */
	  trans = gg_svg_clone_transform (pt);
	  if (out->first_trans == NULL)
	      out->first_trans = trans;
	  if (out->last_trans != NULL)
	      out->last_trans->next = trans;
	  out->last_trans = trans;
	  pt = pt->next;
      }
    out->next = NULL;
    return out;
}

GGRAPH_PRIVATE struct gg_svg_group *
gg_svg_alloc_group (void)
{
/* allocating and initializing an empty SVG Group <g> */
    struct gg_svg_group *p = malloc (sizeof (struct gg_svg_group));
    p->id = NULL;
    p->style.visibility = -1;
    p->style.opacity = 1.0;
    p->style.fill = -1;
    p->style.no_fill = -1;
    p->style.fill_rule = -1;
    p->style.fill_url = NULL;
    p->style.fill_pointer = NULL;
    p->style.fill_red = -1.0;
    p->style.fill_green = -1.0;
    p->style.fill_blue = -1.0;
    p->style.fill_opacity = -1.0;
    p->style.stroke = -1;
    p->style.no_stroke = -1;
    p->style.stroke_width = -1.0;
    p->style.stroke_linecap = -1;
    p->style.stroke_linejoin = -1;
    p->style.stroke_miterlimit = -1.0;
    p->style.stroke_dashitems = 0;
    p->style.stroke_dasharray = NULL;
    p->style.stroke_dashoffset = 0.0;
    p->style.stroke_url = NULL;
    p->style.stroke_pointer = NULL;
    p->style.stroke_red = -1.0;
    p->style.stroke_green = -1.0;
    p->style.stroke_blue = -1.0;
    p->style.stroke_opacity = -1.0;
    p->style.clip_url = NULL;
    p->style.clip_pointer = NULL;
    p->parent = NULL;
    p->first = NULL;
    p->last = NULL;
    p->first_trans = NULL;
    p->last_trans = NULL;
    p->is_defs = 0;
    p->is_flow_root = 0;
    p->next = NULL;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_group *
gg_svg_clone_group (struct gg_svg_group *in, struct gg_svg_use *use)
{
/* cloning an SVG Group */
    struct gg_svg_transform *pt;
    struct gg_svg_transform *trans;
    struct gg_svg_item *item;
    struct gg_svg_item *itm;
    struct gg_svg_group *out = malloc (sizeof (struct gg_svg_group));
    out->id = NULL;
    out->style.visibility = in->style.visibility;
    out->style.opacity = in->style.opacity;
    out->style.fill = in->style.fill;
    out->style.no_fill = in->style.no_fill;
    out->style.fill_rule = in->style.fill_rule;
    out->style.fill_url = NULL;
    out->style.fill_pointer = NULL;
    if (in->style.fill_url != NULL)
	gg_svg_add_fill_gradient_url (&(out->style), in->style.fill_url);
    out->style.fill_red = in->style.fill_red;
    out->style.fill_green = in->style.fill_green;
    out->style.fill_blue = in->style.fill_blue;
    out->style.fill_opacity = in->style.fill_opacity;
    out->style.stroke = in->style.stroke;
    out->style.no_stroke = in->style.no_stroke;
    out->style.stroke_width = in->style.stroke_width;
    out->style.stroke_linecap = in->style.stroke_linecap;
    out->style.stroke_linejoin = in->style.stroke_linejoin;
    out->style.stroke_miterlimit = in->style.stroke_miterlimit;
    out->style.stroke_dashitems = 0;
    out->style.stroke_dasharray = NULL;
    if (in->style.stroke_dashitems > 0)
      {
	  out->style.stroke_dashitems = in->style.stroke_dashitems;
	  if (out->style.stroke_dasharray != NULL)
	      free (out->style.stroke_dasharray);
	  out->style.stroke_dasharray = NULL;
	  if (in->style.stroke_dashitems > 0)
	    {
		int i;
		out->style.stroke_dasharray =
		    malloc (sizeof (double) * in->style.stroke_dashitems);
		for (i = 0; i < in->style.stroke_dashitems; i++)
		    out->style.stroke_dasharray[i] =
			in->style.stroke_dasharray[i];
	    }
	  out->style.stroke_dashoffset = in->style.stroke_dashoffset;
      }
    out->style.stroke_url = NULL;
    out->style.stroke_pointer = NULL;
    if (in->style.stroke_url != NULL)
	gg_svg_add_stroke_gradient_url (&(out->style), in->style.stroke_url);
    out->style.stroke_red = in->style.stroke_red;
    out->style.stroke_green = in->style.stroke_green;
    out->style.stroke_blue = in->style.stroke_blue;
    out->style.stroke_opacity = in->style.stroke_opacity;
    out->style.clip_url = NULL;
    out->style.clip_pointer = NULL;
    if (in->style.clip_url != NULL)
	gg_svg_add_clip_url (&(out->style), in->style.clip_url);
    if (use != NULL)
	out->parent = use->parent;
    else
	out->parent = in->parent;
    out->first = NULL;
    out->last = NULL;
    out->is_defs = 0;
    out->is_flow_root = 0;
    item = in->first;
    while (item)
      {
	  /* looping on Group Items */
	  itm = gg_svg_clone_item (item);
	  gg_svg_set_group_parent (itm, out);
	  if (out->first == NULL)
	      out->first = itm;
	  if (out->last != NULL)
	      out->last->next = itm;
	  out->last = itm;
	  item = item->next;
      }
    out->first_trans = NULL;
    out->last_trans = NULL;
    out->next = NULL;
    if (use != NULL)
      {
	  /* adding the Use-level defs */
	  pt = use->first_trans;
	  while (pt)
	    {
		/* clonig all Use transformations */
		trans = gg_svg_clone_transform (pt);
		if (out->first_trans == NULL)
		    out->first_trans = trans;
		if (out->last_trans != NULL)
		    out->last_trans->next = trans;
		out->last_trans = trans;
		pt = pt->next;
	    }
      }
    pt = in->first_trans;
    while (pt)
      {
	  /* clonig all transformations */
	  trans = gg_svg_clone_transform (pt);
	  if (out->first_trans == NULL)
	      out->first_trans = trans;
	  if (out->last_trans != NULL)
	      out->last_trans->next = trans;
	  out->last_trans = trans;
	  pt = pt->next;
      }
    if (use != NULL)
      {
	  /* adding the Use-level defs */
	  if (use->x != DBL_MAX || use->y != DBL_MAX)
	    {
		/* adding the implict Use Translate transformation */
		struct gg_svg_translate *translate = NULL;
		if (use->x != DBL_MAX && use->y != DBL_MAX)
		    translate = gg_svg_alloc_translate (use->x, use->y);
		else if (use->x != DBL_MAX)
		    translate = gg_svg_alloc_translate (use->x, 0.0);
		else if (use->y != DBL_MAX)
		    translate = gg_svg_alloc_translate (0.0, use->y);
		trans = gg_svg_alloc_transform (GG_SVG_TRANSLATE, translate);
		if (out->first_trans == NULL)
		    out->first_trans = trans;
		if (out->last_trans != NULL)
		    out->last_trans->next = trans;
		out->last_trans = trans;
	    }
	  /* Use-level styles */
	  if (use->style.visibility >= 0)
	      out->style.visibility = use->style.visibility;
	  out->style.opacity = use->style.opacity;
	  if (use->style.fill >= 0)
	      out->style.fill = use->style.fill;
	  if (use->style.no_fill >= 0)
	      out->style.no_fill = use->style.no_fill;
	  if (use->style.fill_rule >= 0)
	      out->style.fill_rule = use->style.fill_rule;
	  if (use->style.fill_url != NULL)
	      gg_svg_add_fill_gradient_url (&(out->style), use->style.fill_url);
	  if (use->style.fill_red >= 0.0)
	      out->style.fill_red = use->style.fill_red;
	  if (use->style.fill_green >= 0.0)
	      out->style.fill_green = use->style.fill_green;
	  if (use->style.fill_blue >= 0.0)
	      out->style.fill_blue = use->style.fill_blue;
	  if (use->style.fill_opacity >= 0.0)
	      out->style.fill_opacity = use->style.fill_opacity;
	  if (use->style.stroke >= 0)
	      out->style.stroke = use->style.stroke;
	  if (use->style.no_stroke >= 0)
	      out->style.no_stroke = use->style.no_stroke;
	  if (use->style.stroke_width >= 0.0)
	      out->style.stroke_width = use->style.stroke_width;
	  if (use->style.stroke_linecap >= 0)
	      out->style.stroke_linecap = use->style.stroke_linecap;
	  if (use->style.stroke_linejoin >= 0)
	      out->style.stroke_linejoin = use->style.stroke_linejoin;
	  if (use->style.stroke_miterlimit >= 0.0)
	      out->style.stroke_miterlimit = use->style.stroke_miterlimit;
	  if (in->style.stroke_dashitems > 0)
	    {
		out->style.stroke_dashitems = use->style.stroke_dashitems;
		if (out->style.stroke_dasharray != NULL)
		    free (out->style.stroke_dasharray);
		out->style.stroke_dasharray = NULL;
		if (use->style.stroke_dashitems > 0)
		  {
		      int i;
		      out->style.stroke_dasharray =
			  malloc (sizeof (double) *
				  use->style.stroke_dashitems);
		      for (i = 0; i < use->style.stroke_dashitems; i++)
			  out->style.stroke_dasharray[i] =
			      use->style.stroke_dasharray[i];
		  }
		out->style.stroke_dashoffset = use->style.stroke_dashoffset;
	    }
	  if (use->style.stroke_url != NULL)
	      gg_svg_add_stroke_gradient_url (&(out->style),
					      use->style.stroke_url);
	  if (use->style.stroke_red >= 0.0)
	      out->style.stroke_red = use->style.stroke_red;
	  if (use->style.stroke_green >= 0.0)
	      out->style.stroke_green = use->style.stroke_green;
	  if (use->style.stroke_blue >= 0.0)
	      out->style.stroke_blue = use->style.stroke_blue;
	  if (use->style.stroke_opacity >= 0.0)
	      out->style.stroke_opacity = use->style.stroke_opacity;
	  if (use->style.clip_url != NULL)
	      gg_svg_add_clip_url (&(out->style), use->style.clip_url);
      }
    out->next = NULL;
    return out;
}

GGRAPH_PRIVATE struct gg_svg_clip *
gg_svg_alloc_clip (void)
{
/* allocating and initializing an empty SVG ClipPath */
    struct gg_svg_clip *p = malloc (sizeof (struct gg_svg_clip));
    p->id = NULL;
    p->first = NULL;
    p->last = NULL;
    p->next = NULL;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_clip *
gg_svg_clone_clip (struct gg_svg_clip *in)
{
/* cloning an SVG ClipPath */
    struct gg_svg_item *item;
    struct gg_svg_item *itm;
    struct gg_svg_clip *out = malloc (sizeof (struct gg_svg_clip));
    out->id = NULL;
    out->first = NULL;
    out->last = NULL;
    item = in->first;
    while (item)
      {
	  /* looping on Group Items */
	  itm = gg_svg_clone_item (item);
	  if (out->first == NULL)
	      out->first = itm;
	  if (out->last != NULL)
	      out->last->next = itm;
	  out->last = itm;
	  item = item->next;
      }
    out->next = NULL;
    return out;
}

GGRAPH_PRIVATE void
gg_svg_add_group_id (struct gg_svg_group *group, const char *id)
{
/* setting the ID for some Group */
    int len = strlen (id);
    if (group->id != NULL)
	free (group->id);
    group->id = malloc (len + 1);
    strcpy (group->id, id);
}

GGRAPH_PRIVATE void
gg_svg_add_clip_id (struct gg_svg_clip *clip, const char *id)
{
/* setting the ID for some ClipPath */
    int len = strlen (id);
    if (clip->id != NULL)
	free (clip->id);
    clip->id = malloc (len + 1);
    strcpy (clip->id, id);
}

GGRAPH_PRIVATE struct gg_svg_item *
gg_svg_alloc_item (int type, void *pointer)
{
/* allocating and initializing an empty SVG item */
    struct gg_svg_item *p = malloc (sizeof (struct gg_svg_item));
    p->type = type;
    p->pointer = pointer;
    p->next = NULL;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_item *
gg_svg_clone_item (struct gg_svg_item *in)
{
/* cloning an SVG item */
    struct gg_svg_group *group;
    struct gg_svg_shape *shape;
    struct gg_svg_use *use;
    struct gg_svg_clip *clip;
    struct gg_svg_item *p = malloc (sizeof (struct gg_svg_item));
    p->type = in->type;
    switch (in->type)
      {
      case GG_SVG_ITEM_GROUP:
	  group = in->pointer;
	  p->pointer = gg_svg_clone_group (group, NULL);
	  break;
      case GG_SVG_ITEM_SHAPE:
	  shape = in->pointer;
	  p->pointer = gg_svg_clone_shape (shape, NULL);
	  break;
      case GG_SVG_ITEM_USE:
	  use = in->pointer;
	  p->pointer = gg_svg_clone_use (use);
      case GG_SVG_ITEM_CLIP:
	  clip = in->pointer;
	  p->pointer = gg_svg_clone_clip (clip);
	  break;
      };
    p->next = NULL;
    return p;
}

GGRAPH_PRIVATE void
gg_svg_set_group_parent (struct gg_svg_item *item, struct gg_svg_group *grp)
{
/* cloning an SVG item */
    struct gg_svg_group *group;
    struct gg_svg_shape *shape;
    struct gg_svg_use *use;
    switch (item->type)
      {
      case GG_SVG_ITEM_GROUP:
	  group = item->pointer;
	  group->parent = grp;
	  break;
      case GG_SVG_ITEM_SHAPE:
	  shape = item->pointer;
	  shape->parent = grp;
	  break;
      case GG_SVG_ITEM_USE:
	  use = item->pointer;
	  use->parent = grp;
	  break;
      };
}

GGRAPH_PRIVATE struct gg_svg_gradient_stop *
gg_svg_alloc_gradient_stop (double offset, double red, double green,
			    double blue, double opacity)
{
/* allocating and initializing an SVG GradientStop */
    struct gg_svg_gradient_stop *p =
	malloc (sizeof (struct gg_svg_gradient_stop));
    p->offset = offset;
    p->red = red;
    p->green = green;
    p->blue = blue;
    p->opacity = opacity;
    p->next = NULL;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_gradient_stop *
gg_svg_clone_gradient_stop (struct gg_svg_gradient_stop *in)
{
/* cloning an SVG GradientStop */
    struct gg_svg_gradient_stop *out =
	malloc (sizeof (struct gg_svg_gradient_stop));
    out->offset = in->offset;
    out->red = in->red;
    out->green = in->green;
    out->blue = in->blue;
    out->opacity = in->opacity;
    out->next = NULL;
    return out;
}

GGRAPH_PRIVATE struct gg_svg_gradient *
gg_svg_alloc_gradient (void)
{
/* allocating and initializing an empty SVG Gradient */
    struct gg_svg_gradient *p = malloc (sizeof (struct gg_svg_gradient));
    p->type = -1;
    p->id = NULL;
    p->xlink_href = NULL;
    p->gradient_units = -1;
    p->x1 = DBL_MAX;
    p->y1 = DBL_MAX;
    p->x2 = DBL_MAX;
    p->y2 = DBL_MAX;
    p->cx = DBL_MAX;
    p->cy = DBL_MAX;
    p->fx = DBL_MAX;
    p->fy = DBL_MAX;
    p->r = DBL_MAX;
    p->first_trans = NULL;
    p->last_trans = NULL;
    p->first_stop = NULL;
    p->last_stop = NULL;
    p->next = NULL;
    p->prev = NULL;
    return p;
}

GGRAPH_PRIVATE struct gg_svg_gradient *
gg_svg_clone_gradient (struct gg_svg_gradient *in, struct gg_svg_gradient *old)
{
/* cloning an SVG Gradient */
    struct gg_svg_transform *pt;
    struct gg_svg_transform *trans;
    struct gg_svg_gradient_stop *ps;
    struct gg_svg_gradient_stop *stop;
    int len;
    struct gg_svg_gradient *out = malloc (sizeof (struct gg_svg_gradient));
    out->type = old->type;
    out->id = NULL;
    out->xlink_href = NULL;
    if (old->id != NULL)
      {
	  len = strlen (old->id);
	  out->id = malloc (len + 1);
	  strcpy (out->id, old->id);
      }
    if (old->xlink_href != NULL)
      {
	  len = strlen (old->xlink_href);
	  out->xlink_href = malloc (len + 1);
	  strcpy (out->xlink_href, old->xlink_href);
      }
    out->gradient_units = in->gradient_units;
    if (old->gradient_units >= 0)
	out->gradient_units = old->gradient_units;
    out->x1 = in->x1;
    if (old->x1 != DBL_MAX)
	out->x1 = old->x1;
    out->y1 = in->y1;
    if (old->y1 != DBL_MAX)
	out->y1 = old->y1;
    out->x2 = in->x2;
    if (old->x2 != DBL_MAX)
	out->x2 = old->x2;
    out->y2 = in->y2;
    if (old->y2 != DBL_MAX)
	out->y2 = old->y2;
    out->cx = in->cx;
    if (old->cx != DBL_MAX)
	out->cx = old->cx;
    out->cy = in->cy;
    if (old->cy != DBL_MAX)
	out->cy = old->cy;
    out->fx = in->fx;
    if (old->fx != DBL_MAX)
	out->fx = old->fx;
    out->fy = in->fy;
    if (old->fy != DBL_MAX)
	out->fy = old->fy;
    out->r = in->r;
    if (old->r != DBL_MAX)
	out->r = old->r;
    out->first_trans = NULL;
    out->last_trans = NULL;
    out->first_stop = NULL;
    out->last_stop = NULL;
    pt = in->first_trans;
    while (pt)
      {
	  /* clonig all inherited transformations */
	  trans = gg_svg_clone_transform (pt);
	  if (out->first_trans == NULL)
	      out->first_trans = trans;
	  if (out->last_trans != NULL)
	      out->last_trans->next = trans;
	  out->last_trans = trans;
	  pt = pt->next;
      }
    pt = old->first_trans;
    while (pt)
      {
	  /* clonig all direct transformations */
	  trans = gg_svg_clone_transform (pt);
	  if (out->first_trans == NULL)
	      out->first_trans = trans;
	  if (out->last_trans != NULL)
	      out->last_trans->next = trans;
	  out->last_trans = trans;
	  pt = pt->next;
      }
    ps = in->first_stop;
    while (ps)
      {
	  /* clonig all inherited Stops */
	  stop = gg_svg_clone_gradient_stop (ps);
	  if (out->first_stop == NULL)
	      out->first_stop = stop;
	  if (out->last_stop != NULL)
	      out->last_stop->next = stop;
	  out->last_stop = stop;
	  ps = ps->next;
      }
    ps = old->first_stop;
    while (ps)
      {
	  /* clonig all inherited Stops */
	  stop = gg_svg_clone_gradient_stop (ps);
	  if (out->first_stop == NULL)
	      out->first_stop = stop;
	  if (out->last_stop != NULL)
	      out->last_stop->next = stop;
	  out->last_stop = stop;
	  ps = ps->next;
      }
    out->next = NULL;
    out->prev = NULL;
    return out;
}

GGRAPH_PRIVATE struct gg_svg_document *
gg_svg_alloc_document (void)
{
/* allocating and initializing an empty SVG Document */
    struct gg_svg_document *p = malloc (sizeof (struct gg_svg_document));
    p->signature = GG_GRAPHICS_SVG_MAGIC_SIGNATURE;
    p->width = 0.0;
    p->height = 0.0;
    p->viewbox_x = DBL_MIN;
    p->viewbox_y = DBL_MIN;
    p->viewbox_width = DBL_MIN;
    p->viewbox_height = DBL_MIN;
    p->first = NULL;
    p->last = NULL;
    p->first_grad = NULL;
    p->last_grad = NULL;
    p->current_group = NULL;
    p->current_shape = NULL;
    p->current_clip = NULL;
    p->defs_count = 0;
    p->flow_root_count = 0;
    return p;
}

GGRAPH_PRIVATE void
gg_svg_close_group (struct gg_svg_document *gg_svg_doc)
{
/* closing the current SVG Group */
    struct gg_svg_group *group = gg_svg_doc->current_group;
    gg_svg_doc->current_group = group->parent;
}

GGRAPH_PRIVATE void
gg_svg_insert_group (struct gg_svg_document *gg_svg_doc)
{
/* inserting a new Group into the SVG Document */
    struct gg_svg_item *item;
    struct gg_svg_group *parent;
    struct gg_svg_group *group = gg_svg_alloc_group ();
    if (gg_svg_doc->current_group != NULL)
      {
	  /* nested group */
	  parent = gg_svg_doc->current_group;
	  group->parent = parent;
	  if (gg_svg_doc->defs_count > 0)
	      group->is_defs = 1;
	  if (gg_svg_doc->flow_root_count > 0)
	      group->is_flow_root = 1;
	  item = gg_svg_alloc_item (GG_SVG_ITEM_GROUP, group);
	  if (parent->first == NULL)
	      parent->first = item;
	  if (parent->last != NULL)
	      parent->last->next = item;
	  parent->last = item;
	  gg_svg_doc->current_group = group;
	  return;
      }
    if (gg_svg_doc->current_clip != NULL)
      {
	  /* ClipPath group */
	  if (gg_svg_doc->defs_count > 0)
	      group->is_defs = 1;
	  if (gg_svg_doc->flow_root_count > 0)
	      group->is_flow_root = 1;
	  item = gg_svg_alloc_item (GG_SVG_ITEM_GROUP, group);
	  if (gg_svg_doc->current_clip->first == NULL)
	      gg_svg_doc->current_clip->first = item;
	  if (gg_svg_doc->current_clip->last != NULL)
	      gg_svg_doc->current_clip->last->next = item;
	  gg_svg_doc->current_clip->last = item;
	  gg_svg_doc->current_group = group;
	  return;
      }
/* first level Group */
    parent = gg_svg_doc->current_group;
    group->parent = parent;
    if (gg_svg_doc->defs_count > 0)
	group->is_defs = 1;
    if (gg_svg_doc->flow_root_count > 0)
	group->is_flow_root = 1;
    item = gg_svg_alloc_item (GG_SVG_ITEM_GROUP, group);
    if (gg_svg_doc->first == NULL)
	gg_svg_doc->first = item;
    if (gg_svg_doc->last != NULL)
	gg_svg_doc->last->next = item;
    gg_svg_doc->last = item;
    gg_svg_doc->current_group = group;
}

GGRAPH_PRIVATE void
gg_svg_close_clip (struct gg_svg_document *gg_svg_doc)
{
/* closing the current SVG ClipPath */
    gg_svg_doc->current_clip = NULL;
}

GGRAPH_PRIVATE void
gg_svg_insert_clip (struct gg_svg_document *gg_svg_doc)
{
/* inserting a new ClipPath into the SVG Document */
    struct gg_svg_item *item;
    struct gg_svg_clip *clip = gg_svg_alloc_clip ();
    item = gg_svg_alloc_item (GG_SVG_ITEM_CLIP, clip);
    if (gg_svg_doc->first == NULL)
	gg_svg_doc->first = item;
    if (gg_svg_doc->last != NULL)
	gg_svg_doc->last->next = item;
    gg_svg_doc->last = item;
    gg_svg_doc->current_clip = clip;
}

GGRAPH_PRIVATE struct gg_svg_use *
gg_svg_insert_use (struct gg_svg_document *gg_svg_doc, const char *xlink_href,
		   double x, double y, double width, double height)
{
/* inserting a new Use (xlink:href) into the SVG Document */
    struct gg_svg_use *use;
    struct gg_svg_group *group;
    struct gg_svg_clip *clip;
    struct gg_svg_item *item;

    if (gg_svg_doc->current_group == NULL && gg_svg_doc->current_clip == NULL)
      {
	  /* inserting directly into the Document */
	  use = gg_svg_alloc_use (NULL, xlink_href, x, y, width, height);
	  item = gg_svg_alloc_item (GG_SVG_ITEM_USE, use);
	  if (gg_svg_doc->first == NULL)
	      gg_svg_doc->first = item;
	  if (gg_svg_doc->last != NULL)
	      gg_svg_doc->last->next = item;
	  gg_svg_doc->last = item;
      }
    else if (gg_svg_doc->current_clip != NULL)
      {
	  /* inserting into the current ClipPath */
	  clip = gg_svg_doc->current_clip;
	  use = gg_svg_alloc_use (clip, xlink_href, x, y, width, height);
	  item = gg_svg_alloc_item (GG_SVG_ITEM_USE, use);
	  if (clip->first == NULL)
	      clip->first = item;
	  if (clip->last != NULL)
	      clip->last->next = item;
	  clip->last = item;
      }
    else if (gg_svg_doc->current_group != NULL)
      {
	  /* inserting into the current Group */
	  group = gg_svg_doc->current_group;
	  use = gg_svg_alloc_use (group, xlink_href, x, y, width, height);
	  item = gg_svg_alloc_item (GG_SVG_ITEM_USE, use);
	  if (group->first == NULL)
	      group->first = item;
	  if (group->last != NULL)
	      group->last->next = item;
	  group->last = item;
      }
    return use;
}

GGRAPH_PRIVATE void
gg_svg_insert_shape (struct gg_svg_document *gg_svg_doc, int type, void *data)
{
/* inserting a new Shape into the SVG Document */
    struct gg_svg_group *group;
    struct gg_svg_shape *shape;
    struct gg_svg_clip *clip;
    struct gg_svg_item *item;

    if (gg_svg_doc->current_group == NULL && gg_svg_doc->current_clip == NULL)
      {
	  /* inserting directly into the Document */
	  shape = gg_svg_alloc_shape (type, data, NULL);
	  if (gg_svg_doc->defs_count > 0)
	      shape->is_defs = 1;
	  if (gg_svg_doc->flow_root_count > 0)
	      shape->is_flow_root = 1;
	  item = gg_svg_alloc_item (GG_SVG_ITEM_SHAPE, shape);
	  if (gg_svg_doc->first == NULL)
	      gg_svg_doc->first = item;
	  if (gg_svg_doc->last != NULL)
	      gg_svg_doc->last->next = item;
	  gg_svg_doc->last = item;
      }
    else if (gg_svg_doc->current_group != NULL)
      {
	  /* inserting into the current Group */
	  group = gg_svg_doc->current_group;
	  shape = gg_svg_alloc_shape (type, data, group);
	  if (gg_svg_doc->defs_count > 0)
	      shape->is_defs = 1;
	  if (gg_svg_doc->flow_root_count > 0)
	      shape->is_flow_root = 1;
	  item = gg_svg_alloc_item (GG_SVG_ITEM_SHAPE, shape);
	  if (group->first == NULL)
	      group->first = item;
	  if (group->last != NULL)
	      group->last->next = item;
	  group->last = item;
      }
    else if (gg_svg_doc->current_clip != NULL)
      {
	  /* inserting into the current ClipPath */
	  clip = gg_svg_doc->current_clip;
	  shape = gg_svg_alloc_shape (type, data, NULL);
	  if (gg_svg_doc->defs_count > 0)
	      shape->is_defs = 1;
	  if (gg_svg_doc->flow_root_count > 0)
	      shape->is_flow_root = 1;
	  item = gg_svg_alloc_item (GG_SVG_ITEM_SHAPE, shape);
	  if (clip->first == NULL)
	      clip->first = item;
	  if (clip->last != NULL)
	      clip->last->next = item;
	  clip->last = item;
      }
    gg_svg_doc->current_shape = shape;
}

GGRAPH_PRIVATE void
gg_svg_insert_gradient_stop (struct gg_svg_gradient *gradient,
			     double offset, double red, double green,
			     double blue, double opacity)
{
/* inserting a Stop into a Gradient */
    struct gg_svg_gradient_stop *stop =
	gg_svg_alloc_gradient_stop (offset, red, green, blue, opacity);
    if (gradient->first_stop == NULL)
	gradient->first_stop = stop;
    if (gradient->last_stop != NULL)
	gradient->last_stop->next = stop;
    gradient->last_stop = stop;
}

GGRAPH_PRIVATE struct gg_svg_gradient *
gg_svg_insert_linear_gradient (struct gg_svg_document *gg_svg_doc,
			       const char *id, const char *xlink_href,
			       double x1, double y1, double x2, double y2,
			       int units)
{
/* inserting a new Linear Gradient into the SVG Document */
    int len;
    struct gg_svg_gradient *gradient = gg_svg_alloc_gradient ();
    gradient->type = GG_SVG_LINEAR_GRADIENT;
    gradient->id = NULL;
    if (id != NULL)
      {
	  len = strlen (id);
	  gradient->id = malloc (len + 1);
	  strcpy (gradient->id, id);
      }
    gradient->xlink_href = NULL;
    if (xlink_href != NULL)
      {
	  len = strlen (xlink_href);
	  gradient->xlink_href = malloc (len + 1);
	  strcpy (gradient->xlink_href, xlink_href);
      }
    gradient->gradient_units = units;
    gradient->x1 = x1;
    gradient->y1 = y1;
    gradient->x2 = x2;
    gradient->y2 = y2;
    gradient->prev = gg_svg_doc->last_grad;
    if (gg_svg_doc->first_grad == NULL)
	gg_svg_doc->first_grad = gradient;
    if (gg_svg_doc->last_grad != NULL)
	gg_svg_doc->last_grad->next = gradient;
    gg_svg_doc->last_grad = gradient;
    return gradient;
}

GGRAPH_PRIVATE struct gg_svg_gradient *
gg_svg_insert_radial_gradient (struct gg_svg_document *gg_svg_doc,
			       const char *id, const char *xlink_href,
			       double cx, double cy, double fx, double fy,
			       double r, int units)
{
/* inserting a new Linear Gradient into the SVG Document */
    int len;
    struct gg_svg_gradient *gradient = gg_svg_alloc_gradient ();
    gradient->type = GG_SVG_RADIAL_GRADIENT;
    gradient->id = NULL;
    if (id != NULL)
      {
	  len = strlen (id);
	  gradient->id = malloc (len + 1);
	  strcpy (gradient->id, id);
      }
    gradient->xlink_href = NULL;
    if (xlink_href != NULL)
      {
	  len = strlen (xlink_href);
	  gradient->xlink_href = malloc (len + 1);
	  strcpy (gradient->xlink_href, xlink_href);
      }
    gradient->gradient_units = units;
    gradient->cx = cx;
    gradient->cy = cy;
    gradient->fx = fx;
    gradient->fy = fy;
    gradient->r = r;
    gradient->prev = gg_svg_doc->last_grad;
    if (gg_svg_doc->first_grad == NULL)
	gg_svg_doc->first_grad = gradient;
    if (gg_svg_doc->last_grad != NULL)
	gg_svg_doc->last_grad->next = gradient;
    gg_svg_doc->last_grad = gradient;
    return gradient;
}
