/* 
/ GGRAPH_internals.h
/
/ internal declarations
/
/ version  1.0, 2010 July 20
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

#include <cairo/cairo.h>
#include <cairo/cairo-svg.h>
#include <cairo/cairo-pdf.h>

#ifdef _WIN32
#ifdef DLL_EXPORT
#define GGRAPH_PRIVATE
#else
#define GGRAPH_PRIVATE
#endif
#else
#define GGRAPH_PRIVATE __attribute__ ((visibility("hidden")))
#endif

#define GG_IMAGE_JPEG_RGB	100
#define GG_IMAGE_JPEG_BW	101
#define GG_IMAGE_TIFF_FAX4	102
#define GG_IMAGE_TIFF_PALETTE	103
#define GG_IMAGE_TIFF_GRAYSCALE	104
#define GG_IMAGE_TIFF_RGB	105
#define GG_IMAGE_PNG_PALETTE	106
#define GG_IMAGE_PNG_GRAYSCALE	107
#define GG_IMAGE_PNG_RGB	108
#define GG_IMAGE_GIF_PALETTE	109

#define GG_PIXEL_UNKNOWN	-200
#define GG_PIXEL_RGB		201
#define GG_PIXEL_RGBA		202
#define GG_PIXEL_ARGB		203
#define GG_PIXEL_BGR		204
#define GG_PIXEL_BGRA		205
#define GG_PIXEL_GRAYSCALE	206
#define GG_PIXEL_PALETTE	207
#define GG_PIXEL_GRID		208

#define GG_TARGET_IS_MEMORY	2001
#define GG_TARGET_IS_FILE	2002

#define GG_IMAGE_MAGIC_SIGNATURE		65391
#define GG_IMAGE_INFOS_MAGIC_SIGNATURE		37183
#define GG_STRIP_IMAGE_MAGIC_SIGNATURE		17359
#define GG_COLOR_RULE_MAGIC_SIGNATURE		23713
#define GG_COLOR_MAP_MAGIC_SIGNATURE		27317
#define GG_SHADED_RELIEF_3ROWS_MAGIC_SIGNATURE	18573

#define GG_GRAPHICS_CONTEXT_MAGIC_SIGNATURE	1314
#define GG_GRAPHICS_SVG_CONTEXT_MAGIC_SIGNATURE	1334
#define GG_GRAPHICS_PDF_CONTEXT_MAGIC_SIGNATURE	1374
#define GG_GRAPHICS_BITMAP_MAGIC_SIGNATURE	5317
#define GG_GRAPHICS_BRUSH_MAGIC_SIGNATURE	2671
#define GG_GRAPHICS_FONT_MAGIC_SIGNATURE	7459
#define GG_GRAPHICS_SVG_MAGIC_SIGNATURE		3265

/* ADAM7/RAW markers */
#define GG_MONOCHROME_START		3301
#define GG_MONOCHROME_END		3311
#define GG_ADAM7_PALETTE_START		3901
#define GG_ADAM7_PALETTE_END		3911
#define GG_ADAM7_0_RGB_START		3001
#define GG_ADAM7_0_RGB_END		3011
#define GG_ADAM7_1_RGB_START		3002
#define GG_ADAM7_1_RGB_END		3012
#define GG_ADAM7_2_RGB_START		3003
#define GG_ADAM7_2_RGB_END		3013
#define GG_ADAM7_3_RGB_START		3004
#define GG_ADAM7_3_RGB_END		3014
#define GG_ADAM7_4_RGB_START		3005
#define GG_ADAM7_4_RGB_END		3015
#define GG_ADAM7_5_RGB_START		3006
#define GG_ADAM7_5_RGB_END		3016
#define GG_ADAM7_6_RGB_START		3007
#define GG_ADAM7_6_RGB_END		3017
#define GG_ADAM7_0_GRAYSCALE_START	3101
#define GG_ADAM7_0_GRAYSCALE_END	3111
#define GG_ADAM7_1_GRAYSCALE_START	3102
#define GG_ADAM7_1_GRAYSCALE_END	3112
#define GG_ADAM7_2_GRAYSCALE_START	3103
#define GG_ADAM7_2_GRAYSCALE_END	3113
#define GG_ADAM7_3_GRAYSCALE_START	3104
#define GG_ADAM7_3_GRAYSCALE_END	3114
#define GG_ADAM7_4_GRAYSCALE_START	3105
#define GG_ADAM7_4_GRAYSCALE_END	3115
#define GG_ADAM7_5_GRAYSCALE_START	3106
#define GG_ADAM7_5_GRAYSCALE_END	3116
#define GG_ADAM7_6_GRAYSCALE_START	3107
#define GG_ADAM7_6_GRAYSCALE_END	3117
#define GG_ADAM7_0_PALETTE_START	3201
#define GG_ADAM7_0_PALETTE_END		3211
#define GG_ADAM7_1_PALETTE_START	3202
#define GG_ADAM7_1_PALETTE_END		3212
#define GG_ADAM7_2_PALETTE_START	3203
#define GG_ADAM7_2_PALETTE_END		3213
#define GG_ADAM7_3_PALETTE_START	3204
#define GG_ADAM7_3_PALETTE_END		3214
#define GG_ADAM7_4_PALETTE_START	3205
#define GG_ADAM7_4_PALETTE_END		3215
#define GG_ADAM7_5_PALETTE_START	3206
#define GG_ADAM7_5_PALETTE_END		3216
#define GG_ADAM7_6_PALETTE_START	3207
#define GG_ADAM7_6_PALETTE_END		3217
#define GG_ADAM7_0_INT16_START		3401
#define GG_ADAM7_0_INT16_END		3411
#define GG_ADAM7_1_INT16_START		3402
#define GG_ADAM7_1_INT16_END		3412
#define GG_ADAM7_2_INT16_START		3403
#define GG_ADAM7_2_INT16_END		3413
#define GG_ADAM7_3_INT16_START		3404
#define GG_ADAM7_3_INT16_END		3414
#define GG_ADAM7_4_INT16_START		3405
#define GG_ADAM7_4_INT16_END		3415
#define GG_ADAM7_5_INT16_START		3406
#define GG_ADAM7_5_INT16_END		3416
#define GG_ADAM7_6_INT16_START		3407
#define GG_ADAM7_6_INT16_END		3417
#define GG_ADAM7_0_UINT16_START		3501
#define GG_ADAM7_0_UINT16_END		3511
#define GG_ADAM7_1_UINT16_START		3502
#define GG_ADAM7_1_UINT16_END		3512
#define GG_ADAM7_2_UINT16_START		3503
#define GG_ADAM7_2_UINT16_END		3513
#define GG_ADAM7_3_UINT16_START		3504
#define GG_ADAM7_3_UINT16_END		3514
#define GG_ADAM7_4_UINT16_START		3505
#define GG_ADAM7_4_UINT16_END		3515
#define GG_ADAM7_5_UINT16_START		3506
#define GG_ADAM7_5_UINT16_END		3516
#define GG_ADAM7_6_UINT16_START		3507
#define GG_ADAM7_6_UINT16_END		3517
#define GG_ADAM7_0_INT32_START		3501
#define GG_ADAM7_0_INT32_END		3511
#define GG_ADAM7_1_INT32_START		3502
#define GG_ADAM7_1_INT32_END		3512
#define GG_ADAM7_2_INT32_START		3503
#define GG_ADAM7_2_INT32_END		3513
#define GG_ADAM7_3_INT32_START		3504
#define GG_ADAM7_3_INT32_END		3514
#define GG_ADAM7_4_INT32_START		3505
#define GG_ADAM7_4_INT32_END		3515
#define GG_ADAM7_5_INT32_START		3506
#define GG_ADAM7_5_INT32_END		3516
#define GG_ADAM7_6_INT32_START		3507
#define GG_ADAM7_6_INT32_END		3517
#define GG_ADAM7_0_UINT32_START		3601
#define GG_ADAM7_0_UINT32_END		3611
#define GG_ADAM7_1_UINT32_START		3602
#define GG_ADAM7_1_UINT32_END		3612
#define GG_ADAM7_2_UINT32_START		3603
#define GG_ADAM7_2_UINT32_END		3613
#define GG_ADAM7_3_UINT32_START		3604
#define GG_ADAM7_3_UINT32_END		3614
#define GG_ADAM7_4_UINT32_START		3605
#define GG_ADAM7_4_UINT32_END		3615
#define GG_ADAM7_5_UINT32_START		3606
#define GG_ADAM7_5_UINT32_END		3616
#define GG_ADAM7_6_UINT32_START		3607
#define GG_ADAM7_6_UINT32_END		3617
#define GG_ADAM7_0_FLOAT_START		3701
#define GG_ADAM7_0_FLOAT_END		3711
#define GG_ADAM7_1_FLOAT_START		3702
#define GG_ADAM7_1_FLOAT_END		3712
#define GG_ADAM7_2_FLOAT_START		3703
#define GG_ADAM7_2_FLOAT_END		3713
#define GG_ADAM7_3_FLOAT_START		3704
#define GG_ADAM7_3_FLOAT_END		3714
#define GG_ADAM7_4_FLOAT_START		3705
#define GG_ADAM7_4_FLOAT_END		3715
#define GG_ADAM7_5_FLOAT_START		3706
#define GG_ADAM7_5_FLOAT_END		3716
#define GG_ADAM7_6_FLOAT_START		3707
#define GG_ADAM7_6_FLOAT_END		3717
#define GG_ADAM7_0_DOUBLE_START		3801
#define GG_ADAM7_0_DOUBLE_END		3811
#define GG_ADAM7_1_DOUBLE_START		3802
#define GG_ADAM7_1_DOUBLE_END		3812
#define GG_ADAM7_2_DOUBLE_START		3803
#define GG_ADAM7_2_DOUBLE_END		3813
#define GG_ADAM7_3_DOUBLE_START		3804
#define GG_ADAM7_3_DOUBLE_END		3814
#define GG_ADAM7_4_DOUBLE_START		3805
#define GG_ADAM7_4_DOUBLE_END		3815
#define GG_ADAM7_5_DOUBLE_START		3806
#define GG_ADAM7_5_DOUBLE_END		3816
#define GG_ADAM7_6_DOUBLE_START		3807
#define GG_ADAM7_6_DOUBLE_END		3817

/* SVG constants */
#define GG_SVG_UNKNOWN		0

/* SVG basic shapes */
#define GG_SVG_RECT		1
#define	GG_SVG_CIRCLE		2
#define GG_SVG_ELLIPSE		3
#define GG_SVG_LINE		4
#define GG_SVG_POLYLINE		5
#define GG_SVG_POLYGON		6
#define GG_SVG_PATH		7

/* SVG transformations */
#define GG_SVG_MATRIX		8
#define GG_SVG_TRANSLATE	9
#define GG_SVG_SCALE		10
#define GG_SVG_ROTATE		11
#define GG_SVG_SKEW_X		12
#define GG_SVG_SKEW_Y		13

/* SVG Path actions */
#define GG_SVG_MOVE_TO		14
#define GG_SVG_LINE_TO		15
#define GG_SVG_CURVE_3		16
#define GG_SVG_CURVE_4		17
#define GG_SVG_ELLIPT_ARC	18
#define GG_SVG_CLOSE_PATH	19

/* SGV Items */
#define GG_SVG_ITEM_GROUP	20
#define GG_SVG_ITEM_SHAPE	21
#define GG_SVG_ITEM_USE		22
#define GG_SVG_ITEM_CLIP	23

/* SVG Gradients */
#define GG_SVG_LINEAR_GRADIENT	24
#define GG_SVG_RADIAL_GRADIENT	25
#define GG_SVG_USER_SPACE	26
#define GG_SVG_BOUNDING_BOX	27

typedef struct gaia_graphics_image_infos
{
/* a generic image INFOS */
    int signature;
    int width;
    int height;
    int pixel_format;
    int bits_per_sample;
    int samples_per_pixel;
    int sample_format;
    int max_palette;
    unsigned char palette_red[256];
    unsigned char palette_green[256];
    unsigned char palette_blue[256];
    int is_transparent;
    int tile_width;
    int tile_height;
    int rows_per_strip;
    int compression;
    int scale_1_2;
    int scale_1_4;
    int scale_1_8;
    int is_georeferenced;
    int srid;
    char *srs_name;
    char *proj4text;
    double upper_left_x;
    double upper_left_y;
    double pixel_x_size;
    double pixel_y_size;
    double no_data_value;
    double min_value;
    double max_value;
} gGraphImageInfos;
typedef gGraphImageInfos *gGraphImageInfosPtr;

typedef struct gaia_graphics_image
{
/* a generic image  */
    int signature;
    unsigned char *pixels;
    int width;
    int height;
    int bits_per_sample;
    int samples_per_pixel;
    int sample_format;
    int scanline_width;
    int pixel_size;
    int pixel_format;
    int max_palette;
    unsigned char palette_red[256];
    unsigned char palette_green[256];
    unsigned char palette_blue[256];
    int is_transparent;
    unsigned char transparent_red;
    unsigned char transparent_green;
    unsigned char transparent_blue;
    int tile_width;
    int tile_height;
    int rows_per_strip;
    int compression;
    int is_georeferenced;
    int srid;
    char *srs_name;
    char *proj4text;
    double upper_left_x;
    double upper_left_y;
    double pixel_x_size;
    double pixel_y_size;
    double no_data_value;
    double min_value;
    double max_value;
} gGraphImage;
typedef gGraphImage *gGraphImagePtr;

typedef struct gaia_graphics_strip_image
{
/* a file-based image accessed by strips  */
    int signature;
    FILE *file_handle;
    int codec_id;
    int rows_per_block;
    int current_available_rows;
    int next_row;
    unsigned char *pixels;
    int width;
    int height;
    int bits_per_sample;
    int samples_per_pixel;
    int sample_format;
    int scanline_width;
    int pixel_size;
    int pixel_format;
    int max_palette;
    unsigned char palette_red[256];
    unsigned char palette_green[256];
    unsigned char palette_blue[256];
    int is_transparent;
    unsigned char transparent_red;
    unsigned char transparent_green;
    unsigned char transparent_blue;
    int tile_width;
    int tile_height;
    int rows_per_strip;
    int compression;
    int is_georeferenced;
    int srid;
    char *srs_name;
    char *proj4text;
    double upper_left_x;
    double upper_left_y;
    double pixel_x_size;
    double pixel_y_size;
    double no_data_value;
    double min_value;
    double max_value;
    void *codec_data;
} gGraphStripImage;
typedef gGraphStripImage *gGraphStripImagePtr;

typedef struct gaia_graphics_color_rule_item
{
/* a Color Rule Item */
    double value;
    double percent_value;
    unsigned char is_percent;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    struct gaia_graphics_color_rule_item *next;
} gGraphColorRuleItem;
typedef gGraphColorRuleItem *gGraphColorRuleItemPtr;

typedef struct gaia_graphics_color_rule
{
/* a Color Rule definition [GRASS-like] */
    int signature;
    struct gaia_graphics_color_rule_item *first;
    struct gaia_graphics_color_rule_item *last;
    unsigned char no_data_red;
    unsigned char no_data_green;
    unsigned char no_data_blue;
    unsigned char needs_range;
} gGraphColorRule;
typedef gGraphColorRule *gGraphColorRulePtr;

typedef struct gaia_graphics_color_map_entry
{
/* a Color Map Entry */
    double min;
    double max;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    struct gaia_graphics_color_map_entry *next;
} gGraphColorMapEntry;
typedef gGraphColorMapEntry *gGraphColorMapEntryPtr;

typedef struct gaia_graphics_color_map
{
/* a Color Map */
    int signature;
    unsigned char no_data_red;
    unsigned char no_data_green;
    unsigned char no_data_blue;
    unsigned char not_found_red;
    unsigned char not_found_green;
    unsigned char not_found_blue;
    struct gaia_graphics_color_map_entry *first;
    struct gaia_graphics_color_map_entry *last;
    int num_entries;
    struct gaia_graphics_color_map_entry **array;
} gGraphColorMap;
typedef gGraphColorMap *gGraphColorMapPtr;

typedef struct shaded_relief_triple_row
{
/* the 3-scanlines object for Shaded Relief */
    int signature;
    int width;
    float *in_row1;
    float *in_row2;
    float *in_row3;
    float *current_row;
    unsigned char *out_rgb;
    struct gaia_graphics_color_map *color_map;
    unsigned char mono_red;
    unsigned char mono_green;
    unsigned char mono_blue;
    double z_factor;
    double scale_factor;
    double azimuth;
    double altitude;
    float no_data_value;
    unsigned char no_red;
    unsigned char no_green;
    unsigned char no_blue;
} gGraphShadedReliefTripleRow;
typedef gGraphShadedReliefTripleRow *gGraphShadedReliefTripleRowPtr;

struct gaia_graphics_pen
{
/* a struct wrapping a Cairo Pen */
    double red;
    double green;
    double blue;
    double alpha;
    double width;
    double lengths[4];
    int lengths_count;
};

struct gaia_graphics_brush
{
/* a struct wrapping a Cairo Brush */
    int is_solid_color;
    int is_linear_gradient;
    int is_pattern;
    double red;
    double green;
    double blue;
    double alpha;
    double x0;
    double y0;
    double x1;
    double y1;
    double red2;
    double green2;
    double blue2;
    double alpha2;
    cairo_pattern_t *pattern;
};

typedef struct gaia_graphics_context
{
/* a Cairo based painting context */
    int signature;
    cairo_surface_t *surface;
    cairo_t *cairo;
    struct gaia_graphics_pen current_pen;
    struct gaia_graphics_brush current_brush;
    double font_red;
    double font_green;
    double font_blue;
    double font_alpha;
    int is_font_outlined;
    double font_outline_width;
} gGraphContext;
typedef gGraphContext *gGraphContextPtr;

typedef struct gaia_graphics_bitmap
{
/* a Cairo based symbol bitmap */
    int signature;
    int width;
    int height;
    cairo_surface_t *bitmap;
    cairo_pattern_t *pattern;
} gGraphBitmap;
typedef gGraphBitmap *gGraphBitmapPtr;

typedef struct gaia_graphics_pattern_brush
{
/* a Cairo based pattern brush */
    int signature;
    int width;
    int height;
    cairo_surface_t *bitmap;
    cairo_pattern_t *pattern;
} gGraphPatternBrush;
typedef gGraphPatternBrush *gGraphPatternBrushPtr;

typedef struct gaia_graphics_font
{
/* a struct wrapping a Cairo Font */
    int signature;
    double size;
    int is_outlined;
    double outline_width;
    int style;
    int weight;
    double red;
    double green;
    double blue;
    double alpha;
} gGraphFont;
typedef gGraphFont *gGraphFontPtr;

struct gg_svg_matrix
{
/* SVG Matrix data */
    double a;
    double b;
    double c;
    double d;
    double e;
    double f;
};

struct gg_svg_translate
{
/* SVG Translate data */
    double tx;
    double ty;
};

struct gg_svg_scale
{
/* SVG Scale data */
    double sx;
    double sy;
};

struct gg_svg_rotate
{
/* SVG Rotate data */
    double angle;
    double cx;
    double cy;
};

struct gg_svg_skew
{
/* SVG Skew data */
    double angle;
};

struct gg_svg_transform
{
/* SVG Transform (linked list) */
    int type;
    void *data;
    struct gg_svg_transform *next;
};

struct gg_svg_gradient_stop
{
/* SVG Gradient Stop */
    char *id;
    double offset;
    double red;
    double green;
    double blue;
    double opacity;
    struct gg_svg_gradient_stop *next;
};

struct gg_svg_gradient
{
/* SVG Gradient */
    int type;
    char *id;
    char *xlink_href;
    int gradient_units;
    double x1;
    double y1;
    double x2;
    double y2;
    double cx;
    double cy;
    double fx;
    double fy;
    double r;
    struct gg_svg_gradient_stop *first_stop;
    struct gg_svg_gradient_stop *last_stop;
    struct gg_svg_transform *first_trans;
    struct gg_svg_transform *last_trans;
    struct gg_svg_gradient *prev;
    struct gg_svg_gradient *next;
};

struct gg_svg_rect
{
/* SVG Rect data */
    double x;
    double y;
    double width;
    double height;
    double rx;
    double ry;
};

struct gg_svg_circle
{
/* SVG Circle data */
    double cx;
    double cy;
    double r;
};

struct gg_svg_ellipse
{
/* SVG Ellipse data */
    double cx;
    double cy;
    double rx;
    double ry;
};

struct gg_svg_line
{
/* SVG Line data */
    double x1;
    double y1;
    double x2;
    double y2;
};

struct gg_svg_polyline
{
/* SVG Polyline data */
    int points;
    double *x;
    double *y;
};

struct gg_svg_polygon
{
/* SVG Polygon data */
    int points;
    double *x;
    double *y;
};

struct gg_svg_path_move
{
/* SVG Path: MoveTo and LineTo data */
    double x;
    double y;
};

struct gg_svg_path_bezier
{
/* SVG Path: any Bezier Curve */
    double x1;
    double y1;
    double x2;
    double y2;
    double x;
    double y;
};

struct gg_svg_path_ellipt_arc
{
/* SVG Path: Elliptical Arc */
    double rx;
    double ry;
    double rotation;
    int large_arc;
    int sweep;
    double x;
    double y;
};

struct gg_svg_path_item
{
/* SVG Path item (linked list) */
    int type;
    void *data;
    struct gg_svg_path_item *next;
};

struct gg_svg_path
{
/* SVG Path */
    struct gg_svg_path_item *first;
    struct gg_svg_path_item *last;
    int error;
};

struct gg_svg_style
{
/* SVG Style-related definitions */
    char visibility;
    double opacity;
    char fill;
    char no_fill;
    int fill_rule;
    char *fill_url;
    struct gg_svg_gradient *fill_pointer;
    double fill_red;
    double fill_green;
    double fill_blue;
    double fill_opacity;
    char stroke;
    char no_stroke;
    double stroke_width;
    int stroke_linecap;
    int stroke_linejoin;
    double stroke_miterlimit;
    int stroke_dashitems;
    double *stroke_dasharray;
    double stroke_dashoffset;
    char *stroke_url;
    struct gg_svg_gradient *stroke_pointer;
    double stroke_red;
    double stroke_green;
    double stroke_blue;
    double stroke_opacity;
    char *clip_url;
    struct gg_svg_item *clip_pointer;
};

struct gg_svg_shape
{
/* generic SVG shape container */
    char *id;
    int type;
    void *data;
    struct gg_svg_group *parent;
    struct gg_svg_style style;
    struct gg_svg_transform *first_trans;
    struct gg_svg_transform *last_trans;
    int is_defs;
    int is_flow_root;
    struct gg_svg_shape *next;
};

struct gg_svg_use
{
/* SVG Use (xlink:href) */
    char *xlink_href;
    double x;
    double y;
    double width;
    double height;
    struct gg_svg_style style;
    struct gg_svg_group *parent;
    struct gg_svg_transform *first_trans;
    struct gg_svg_transform *last_trans;
    struct gg_svg_use *next;
};

struct gg_svg_item
{
/* SVG generic item */
    int type;
    void *pointer;
    struct gg_svg_item *next;
};

struct gg_svg_group
{
/* SVG group container: <g> */
    char *id;
    struct gg_svg_style style;
    struct gg_svg_group *parent;
    struct gg_svg_item *first;
    struct gg_svg_item *last;
    struct gg_svg_transform *first_trans;
    struct gg_svg_transform *last_trans;
    int is_defs;
    int is_flow_root;
    struct gg_svg_group *next;
};

struct gg_svg_clip
{
/* SVG group container: <clipPath> */
    char *id;
    struct gg_svg_item *first;
    struct gg_svg_item *last;
    struct gg_svg_clip *next;
};


struct gg_svg_document
{
/* SVG document main container */
    int signature;
    cairo_matrix_t matrix;
    double width;
    double height;
    double viewbox_x;
    double viewbox_y;
    double viewbox_width;
    double viewbox_height;
    struct gg_svg_item *first;
    struct gg_svg_item *last;
    struct gg_svg_gradient *first_grad;
    struct gg_svg_gradient *last_grad;
    struct gg_svg_group *current_group;
    struct gg_svg_shape *current_shape;
    struct gg_svg_clip *current_clip;
    int defs_count;
    int flow_root_count;
};

GGRAPH_PRIVATE gGraphColorRulePtr gg_color_rule_create (void);
GGRAPH_PRIVATE void gg_color_rule_destroy (gGraphColorRulePtr color_rule);
GGRAPH_PRIVATE gGraphColorMapPtr gg_color_map_create (void);
GGRAPH_PRIVATE void gg_color_map_destroy (gGraphColorMapPtr color_map);

GGRAPH_PRIVATE gGraphShadedReliefTripleRowPtr
gg_shaded_relief_triple_row_create (void);
GGRAPH_PRIVATE void
gg_shaded_relief_triple_row_destroy (gGraphShadedReliefTripleRowPtr triple_row);

GGRAPH_PRIVATE gGraphImageInfosPtr gg_image_infos_create (int pixel_format,
							  int width, int height,
							  int bits_per_sample,
							  int samples_per_pixel,
							  int sample_format,
							  const char *srs_name,
							  const char
							  *proj4text);
GGRAPH_PRIVATE void gg_image_infos_destroy (gGraphImageInfosPtr img);
GGRAPH_PRIVATE gGraphImagePtr gg_image_create (int pixel_format, int width,
					       int height, int bits_per_sample,
					       int samples_per_pixel,
					       int sample_format,
					       const char *srs_name,
					       const char *proj4text);
GGRAPH_PRIVATE gGraphImagePtr gg_image_create_from_bitmap (unsigned char
							   *bitmap,
							   int pixel_format,
							   int width,
							   int height,
							   int bits_per_sample,
							   int
							   samples_per_pixel,
							   int sample_format,
							   const char *srs_name,
							   const char
							   *proj4text);
GGRAPH_PRIVATE void gg_image_destroy (gGraphImagePtr img);
GGRAPH_PRIVATE gGraphStripImagePtr gg_strip_image_create (FILE * file_handle,
							  int codec_id,
							  int pixel_format,
							  int width, int height,
							  int bits_per_sample,
							  int samples_per_pixel,
							  int sample_format,
							  const char *srs_name,
							  const char
							  *proj4text);
GGRAPH_PRIVATE void gg_strip_image_destroy (gGraphStripImagePtr img);
GGRAPH_PRIVATE void gg_png_codec_destroy (void *p);
GGRAPH_PRIVATE void gg_jpeg_codec_destroy (void *p);
GGRAPH_PRIVATE void gg_tiff_codec_destroy (void *p);
GGRAPH_PRIVATE void gg_grid_codec_destroy (void *p);
GGRAPH_PRIVATE void gg_image_fill (const gGraphImagePtr img, unsigned char r,
				   unsigned char g, unsigned char b,
				   unsigned char alpha);
GGRAPH_PRIVATE unsigned char gg_match_palette (const gGraphImagePtr img,
					       unsigned char r, unsigned char g,
					       unsigned char b);
GGRAPH_PRIVATE int gg_set_image_transparent_color (unsigned char r,
						   unsigned char g,
						   unsigned char b);
GGRAPH_PRIVATE int gg_is_image_monochrome (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_is_image_monochrome_ready (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_is_image_grayscale (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_is_image_palette256 (const gGraphImagePtr img);

GGRAPH_PRIVATE void gg_make_thumbnail (const gGraphImagePtr thumbnail,
				       const gGraphImagePtr image);
GGRAPH_PRIVATE void gg_image_resize (const gGraphImagePtr dst,
				     const gGraphImagePtr src);
GGRAPH_PRIVATE void gg_make_grid_thumbnail (const gGraphImagePtr thumbnail,
					    const gGraphImagePtr image);
GGRAPH_PRIVATE void gg_grid_resize (const gGraphImagePtr dst,
				    const gGraphImagePtr src);
GGRAPH_PRIVATE void gg_image_clone_georeferencing (const gGraphImagePtr dst,
						   const gGraphImagePtr src);
GGRAPH_PRIVATE void gg_image_sub_set (const gGraphImagePtr dst,
				      const gGraphImagePtr src,
				      int upper_left_x, int upper_left_y);

GGRAPH_PRIVATE int gg_image_to_jpeg (const gGraphImagePtr img, void **mem_buf,
				     int *mem_buf_size, FILE * out,
				     int dest_type, int quality);
GGRAPH_PRIVATE int gg_image_prepare_to_jpeg_by_strip (const gGraphStripImagePtr
						      img, FILE * out,
						      int quality);
GGRAPH_PRIVATE int gg_image_write_to_jpeg_by_strip (const gGraphStripImagePtr
						    img, int *progress);
GGRAPH_PRIVATE int gg_image_to_png (const gGraphImagePtr img, void **mem_buf,
				    int *mem_buf_size, FILE * out,
				    int dest_type, int compression_level,
				    int quantization_factor, int interlaced,
				    int is_transparent);
GGRAPH_PRIVATE int gg_image_prepare_to_png_by_strip (const gGraphStripImagePtr
						     img, FILE * out,
						     int compression_level,
						     int quantization_factor);
GGRAPH_PRIVATE int gg_image_write_to_png_by_strip (const gGraphStripImagePtr
						   img, int *progress);
GGRAPH_PRIVATE int gg_image_to_gif (const gGraphImagePtr img, void **mem_buf,
				    int *mem_buf_size, FILE * out,
				    int dest_type, int is_transparent);
GGRAPH_PRIVATE int gg_image_write_to_bin_hdr_by_strip (const gGraphStripImagePtr
						       img, int *progress);
GGRAPH_PRIVATE int gg_image_write_to_flt_hdr_by_strip (const gGraphStripImagePtr
						       img, int *progress);
GGRAPH_PRIVATE int gg_image_write_to_ascii_grid_by_strip (const
							  gGraphStripImagePtr
							  img, int *progress);
GGRAPH_PRIVATE int gg_image_prepare_to_tiff_by_strip (const gGraphStripImagePtr
						      img, const char *path,
						      int layout,
						      int tile_width,
						      int tile_height,
						      int rows_per_strip,
						      int color_model,
						      int bits_per_sample,
						      int sample_format,
						      int num_palette,
						      unsigned char *red,
						      unsigned char *green,
						      unsigned char *blue,
						      int compression);
GGRAPH_PRIVATE int gg_image_prepare_to_geotiff_by_strip (const
							 gGraphStripImagePtr
							 img, const char *path,
							 int layout,
							 int tile_width,
							 int tile_height,
							 int rows_per_strip,
							 int color_model,
							 int bits_per_sample,
							 int sample_format,
							 int num_palette,
							 unsigned char *red,
							 unsigned char *green,
							 unsigned char *blue,
							 int compression);
GGRAPH_PRIVATE int gg_image_prepare_to_bin_hdr_by_strip (const
							 gGraphStripImagePtr
							 img);
GGRAPH_PRIVATE int gg_image_prepare_to_flt_hdr_by_strip (const
							 gGraphStripImagePtr
							 img);
GGRAPH_PRIVATE int gg_image_prepare_to_ascii_grid_by_strip (const
							    gGraphStripImagePtr
							    img, FILE * out);
GGRAPH_PRIVATE int gg_image_write_to_tiff_by_strip (const gGraphStripImagePtr
						    img, int *progress);

GGRAPH_PRIVATE int gg_convert_image_to_rgb (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_rgba (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_argb (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_bgr (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_bgra (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_grayscale (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_palette (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_image_resample_as_palette (const gGraphImagePtr img,
						 int num_colors);
GGRAPH_PRIVATE int gg_convert_image_to_monochrome (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_grid_int16 (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_grid_uint16 (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_grid_int32 (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_grid_uint32 (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_grid_float (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_convert_image_to_grid_double (const gGraphImagePtr img);

GGRAPH_PRIVATE int gg_resample_transparent_rgb (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_resample_transparent_rgba (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_resample_transparent_grayscale (const gGraphImagePtr img);
GGRAPH_PRIVATE int gg_resample_transparent_palette (const gGraphImagePtr img);

GGRAPH_PRIVATE int gg_image_from_raw (int size, const void *data,
				      gGraphImagePtr * image_handle);
GGRAPH_PRIVATE int gg_image_from_jpeg (int size, const void *data,
				       int source_type,
				       gGraphImagePtr * image_handle,
				       int scale);
GGRAPH_PRIVATE int gg_image_from_png (int size, const void *data,
				      int source_type,
				      gGraphImagePtr * image_handle, int scale);
GGRAPH_PRIVATE int gg_image_from_gif (int size, const void *data,
				      int source_type,
				      gGraphImagePtr * image_handle);
GGRAPH_PRIVATE int gg_image_from_mem_tiff (int size, const void *data,
					   gGraphImagePtr * image_handle);

GGRAPH_PRIVATE int gg_image_strip_prepare_from_png (FILE * in,
						    gGraphStripImagePtr *
						    image_handle);
GGRAPH_PRIVATE int gg_image_strip_prepare_from_jpeg (FILE * in,
						     gGraphStripImagePtr *
						     image_handle);
GGRAPH_PRIVATE int gg_image_strip_prepare_from_jpeg (FILE * in,
						     gGraphStripImagePtr *
						     image_handle);
GGRAPH_PRIVATE int gg_image_strip_prepare_from_tiff (const char *path,
						     gGraphStripImagePtr *
						     image_handle);
GGRAPH_PRIVATE int gg_image_strip_prepare_from_geotiff (const char *path,
							gGraphStripImagePtr *
							image_handle);
GGRAPH_PRIVATE int gg_image_strip_prepare_from_hgt (FILE * in, int lon, int lat,
						    gGraphStripImagePtr *
						    image_handle);
GGRAPH_PRIVATE int gg_image_strip_prepare_from_bin_hdr (FILE * in,
							const char *hdr_path,
							gGraphStripImagePtr *
							image_handle);
GGRAPH_PRIVATE int gg_image_strip_prepare_from_flt_hdr (FILE * in,
							const char *hdr_path,
							gGraphStripImagePtr *
							image_handle);
GGRAPH_PRIVATE int gg_image_strip_prepare_from_dem_hdr (FILE * in,
							const char *hdr_path,
							gGraphStripImagePtr *
							image_handle);
GGRAPH_PRIVATE int gg_image_strip_prepare_from_ascii_grid (FILE * in,
							   gGraphStripImagePtr *
							   image_handle);

GGRAPH_PRIVATE int gg_image_strip_read_from_png (gGraphStripImagePtr
						 image_handle, int *progress);
GGRAPH_PRIVATE int gg_image_strip_read_from_jpeg (gGraphStripImagePtr
						  image_handle, int *progress);
GGRAPH_PRIVATE int gg_image_strip_read_from_tiff (gGraphStripImagePtr
						  image_handle, int *progress);
GGRAPH_PRIVATE int gg_image_strip_read_from_hgt (gGraphStripImagePtr
						 image_handle, int *progress);
GGRAPH_PRIVATE int gg_image_strip_read_from_bin_grid (gGraphStripImagePtr
						      image_handle,
						      int *progress);
GGRAPH_PRIVATE int gg_image_strip_read_from_dem_grid (gGraphStripImagePtr
						      image_handle,
						      int *progress);
GGRAPH_PRIVATE int gg_image_strip_read_from_ascii_grid (gGraphStripImagePtr
							image_handle,
							int *progress);

GGRAPH_PRIVATE int gg_image_infos_from_jpeg (int size, const void *data,
					     int source_type,
					     gGraphImageInfosPtr *
					     infos_handle);
GGRAPH_PRIVATE int gg_image_infos_from_png (int size, const void *data,
					    int source_type,
					    gGraphImageInfosPtr * infos_handle);
GGRAPH_PRIVATE int gg_image_infos_from_gif (int size, const void *data,
					    int source_type,
					    gGraphImageInfosPtr * infos_handle);
GGRAPH_PRIVATE int gg_image_infos_from_mem_tiff (int size, const void *data,
						 gGraphImageInfosPtr *
						 infos_handle);
GGRAPH_PRIVATE int gg_image_infos_from_tiff (const char *path,
					     gGraphImageInfosPtr *
					     infos_handle);
GGRAPH_PRIVATE int gg_image_infos_from_geo_tiff (const char *path,
						 gGraphImageInfosPtr *
						 infos_handle);

GGRAPH_PRIVATE int gg_endian_arch (void);
GGRAPH_PRIVATE short gg_import_int16 (const unsigned char *p,
				      int little_endian,
				      int little_endian_arch);
GGRAPH_PRIVATE int gg_import_int32 (const unsigned char *p, int little_endian,
				    int little_endian_arch);
GGRAPH_PRIVATE unsigned short gg_import_uint16 (const unsigned char *p,
						int little_endian,
						int little_endian_arch);
GGRAPH_PRIVATE unsigned int gg_import_uint32 (const unsigned char *p,
					      int little_endian,
					      int little_endian_arch);
GGRAPH_PRIVATE float gg_import_float (const unsigned char *p, int little_endian,
				      int little_endian_arch);
GGRAPH_PRIVATE double gg_import_double (const unsigned char *p,
					int little_endian,
					int little_endian_arch);

GGRAPH_PRIVATE void gg_export_int16 (short value, unsigned char *p,
				     int little_endian, int little_endian_arch);
GGRAPH_PRIVATE void gg_export_int32 (int value, unsigned char *p,
				     int little_endian, int little_endian_arch);
GGRAPH_PRIVATE void gg_export_uint16 (unsigned short value, unsigned char *p,
				      int little_endian,
				      int little_endian_arch);
GGRAPH_PRIVATE void gg_export_uint32 (unsigned int value, unsigned char *p,
				      int little_endian,
				      int little_endian_arch);
GGRAPH_PRIVATE void gg_export_float (float value, unsigned char *p,
				     int little_endian, int little_endian_arch);
GGRAPH_PRIVATE void gg_export_double (double value, unsigned char *p,
				      int little_endian,
				      int little_endian_arch);


GGRAPH_PRIVATE struct gg_svg_transform *gg_svg_alloc_transform (int type,
								void *data);
GGRAPH_PRIVATE void gg_svg_free_transform (struct gg_svg_transform *p);
GGRAPH_PRIVATE struct gg_svg_polyline *gg_svg_alloc_polyline (int points,
							      double *x,
							      double *y);
GGRAPH_PRIVATE void gg_svg_free_polyline (struct gg_svg_polyline *p);
GGRAPH_PRIVATE struct gg_svg_polygon *gg_svg_alloc_polygon (int points,
							    double *x,
							    double *y);
GGRAPH_PRIVATE void gg_svg_free_polygon (struct gg_svg_polygon *p);
GGRAPH_PRIVATE struct gg_svg_path_item *gg_svg_alloc_path_item (int type,
								void *data);
GGRAPH_PRIVATE void gg_svg_free_path_item (struct gg_svg_path_item *p);
GGRAPH_PRIVATE struct gg_svg_path *gg_svg_alloc_path (void);
GGRAPH_PRIVATE void gg_svg_free_path (struct gg_svg_path *p);
GGRAPH_PRIVATE struct gg_svg_shape *gg_svg_alloc_shape (int type, void *data,
							struct gg_svg_group
							*parent);
GGRAPH_PRIVATE void gg_svg_free_shape (struct gg_svg_shape *p);
GGRAPH_PRIVATE void gg_svg_add_shape_id (struct gg_svg_shape *p,
					 const char *id);
GGRAPH_PRIVATE struct gg_svg_use *gg_svg_alloc_use (void *parent,
						    const char *xlink_href,
						    double x, double y,
						    double width,
						    double height);
GGRAPH_PRIVATE void gg_svg_free_use (struct gg_svg_use *p);
GGRAPH_PRIVATE struct gg_svg_group *gg_svg_alloc_group (void);
GGRAPH_PRIVATE void gg_svg_free_group (struct gg_svg_group *p);
GGRAPH_PRIVATE void gg_svg_add_group_id (struct gg_svg_group *p,
					 const char *id);
GGRAPH_PRIVATE struct gg_svg_clip *svg_alloc_clip (void);
GGRAPH_PRIVATE void gg_svg_free_clip (struct gg_svg_clip *p);
GGRAPH_PRIVATE void gg_svg_add_clip_id (struct gg_svg_clip *p, const char *id);
GGRAPH_PRIVATE struct gg_svg_gradient_stop *gg_svg_alloc_gradient_stop (double
									offset,
									double
									red,
									double
									green,
									double
									blue,
									double
									opacity);
GGRAPH_PRIVATE struct gg_svg_gradient_stop *gg_svg_clone_gradient_stop (struct
									gg_svg_gradient_stop
									*in);
GGRAPH_PRIVATE void gg_svg_free_gradient_stop (struct gg_svg_gradient_stop *p);
GGRAPH_PRIVATE struct gg_svg_gradient *gg_svg_clone_gradient (struct
							      gg_svg_gradient
							      *in,
							      struct
							      gg_svg_gradient
							      *old);
GGRAPH_PRIVATE void gg_svg_free_gradient (struct gg_svg_gradient *p);
GGRAPH_PRIVATE struct gg_svg_document *gg_svg_alloc_document (void);
GGRAPH_PRIVATE void gg_svg_free_document (struct gg_svg_document *p);
GGRAPH_PRIVATE struct gg_svg_item *gg_svg_alloc_item (int type, void *pointer);
GGRAPH_PRIVATE struct gg_svg_matrix *gg_svg_alloc_matrix (double a, double b,
							  double c, double d,
							  double e, double f);
GGRAPH_PRIVATE struct gg_svg_translate *gg_svg_alloc_translate (double tx,
								double ty);
GGRAPH_PRIVATE struct gg_svg_scale *gg_svg_alloc_scale (double sx, double sy);
GGRAPH_PRIVATE struct gg_svg_rotate *gg_svg_alloc_rotate (double angle,
							  double cx, double cy);
GGRAPH_PRIVATE struct gg_svg_skew *gg_svg_alloc_skew (double angle);
GGRAPH_PRIVATE struct gg_svg_rect *gg_svg_alloc_rect (double x, double y,
						      double width,
						      double height, double rx,
						      double ry);
GGRAPH_PRIVATE struct gg_svg_circle *gg_svg_alloc_circle (double cx, double cy,
							  double r);
GGRAPH_PRIVATE struct gg_svg_ellipse *gg_svg_alloc_ellipse (double cx,
							    double cy,
							    double rx,
							    double ry);
GGRAPH_PRIVATE struct gg_svg_line *gg_svg_alloc_line (double x1, double y1,
						      double x2, double y2);
GGRAPH_PRIVATE struct gg_svg_path_move *gg_svg_alloc_path_move (double x,
								double y);
GGRAPH_PRIVATE struct gg_svg_path_bezier *gg_svg_alloc_path_bezier (double x1,
								    double y1,
								    double x2,
								    double y2,
								    double x,
								    double y);
GGRAPH_PRIVATE struct gg_svg_path_ellipt_arc
    *gg_svg_alloc_path_ellipt_arc (double rx, double ry, double rotation,
				   int large_arc, int sweep, double x,
				   double y);

GGRAPH_PRIVATE void gg_svg_add_path_item (struct gg_svg_path *path, int type,
					  void *data);
GGRAPH_PRIVATE void gg_svg_insert_shape (struct gg_svg_document *svg_doc,
					 int type, void *data);
GGRAPH_PRIVATE struct gg_svg_use *gg_svg_insert_use (struct gg_svg_document
						     *svg_doc,
						     const char *xlink_href,
						     double x, double y,
						     double width,
						     double height);
GGRAPH_PRIVATE void gg_svg_insert_group (struct gg_svg_document *svg_doc);
GGRAPH_PRIVATE void gg_svg_close_group (struct gg_svg_document *svg_doc);
GGRAPH_PRIVATE void gg_svg_insert_clip (struct gg_svg_document *svg_doc);
GGRAPH_PRIVATE void gg_svg_close_clip (struct gg_svg_document *svg_doc);
GGRAPH_PRIVATE struct gg_svg_gradient *gg_svg_insert_linear_gradient (struct
								      gg_svg_document
								      *svg_doc,
								      const char
								      *id,
								      const char
								      *xlink_href,
								      double x1,
								      double y1,
								      double x2,
								      double y2,
								      int
								      units);
GGRAPH_PRIVATE void gg_svg_insert_gradient_stop (struct gg_svg_gradient
						 *gradient, double offset,
						 double red, double green,
						 double blue, double opacity);
GGRAPH_PRIVATE struct gg_svg_gradient *gg_svg_insert_radial_gradient (struct
								      gg_svg_document
								      *svg_doc,
								      const char
								      *id,
								      const char
								      *xlink_href,
								      double cx,
								      double cy,
								      double fx,
								      double fy,
								      double r,
								      int
								      units);

GGRAPH_PRIVATE struct gg_svg_document *gg_svg_parse_doc (const unsigned char
							 *svg, int svg_len);

GGRAPH_PRIVATE struct gg_svg_shape *gg_svg_clone_shape (struct gg_svg_shape *in,
							struct gg_svg_use *use);
GGRAPH_PRIVATE struct gg_svg_group *gg_svg_clone_group (struct gg_svg_group *in,
							struct gg_svg_use *use);
GGRAPH_PRIVATE struct gg_svg_clip *gg_svg_clone_clip (struct gg_svg_clip *in);
GGRAPH_PRIVATE void gg_svg_set_group_parent (struct gg_svg_item *item,
					     struct gg_svg_group *group);
GGRAPH_PRIVATE struct gg_svg_item *gg_svg_clone_item (struct gg_svg_item *in);
GGRAPH_PRIVATE void gg_svg_add_fill_gradient_url (struct gg_svg_style *style,
						  const char *url);
GGRAPH_PRIVATE void gg_svg_add_stroke_gradient_url (struct gg_svg_style *style,
						    const char *url);
GGRAPH_PRIVATE void gg_svg_add_clip_url (struct gg_svg_style *style,
					 const char *url);

/* 
/
/ DISCLAIMER:
/ all the following code merely represents an 'ad hoc' adaption
/ deriving from the original GD lib code [BSD-like licensed]
/
*/

#define XGD_CTX_FREE	300
#define XGD_CTX_DONT_FREE	400

typedef struct xgdIOCtx
{
    int (*getC) (struct xgdIOCtx *);
    int (*getBuf) (struct xgdIOCtx *, void *, int);
    void (*putC) (struct xgdIOCtx *, int);
    int (*putBuf) (struct xgdIOCtx *, const void *, int);
    int (*seek) (struct xgdIOCtx *, const int);
    long (*tell) (struct xgdIOCtx *);
    void (*xgd_free) (struct xgdIOCtx *);
}
xgdIOCtx;

typedef struct xgdIOCtx *xgdIOCtxPtr;

typedef struct dpStruct
{
    FILE *file;
    void *data;
    int logicalSize;
    int realSize;
    int dataGood;
    int pos;
    int freeOK;
}
dynamicPtr;

typedef struct dpIOCtx
{
    xgdIOCtx ctx;
    dynamicPtr *dp;
}
dpIOCtx;

typedef struct dpIOCtx *dpIOCtxPtr;

GGRAPH_PRIVATE int overflow2 (int a, int b);
GGRAPH_PRIVATE void *xgdDPExtractData (struct xgdIOCtx *ctx, int *size);
GGRAPH_PRIVATE xgdIOCtx *xgdNewDynamicCtx (int initialSize, const void *data,
					   int mem_or_file);
GGRAPH_PRIVATE xgdIOCtx *xgdNewDynamicCtxEx (int initialSize, const void *data,
					     int freeOKFlag, int mem_or_file);
GGRAPH_PRIVATE int xgdPutBuf (const void *buf, int size, xgdIOCtx * ctx);
GGRAPH_PRIVATE int xgdGetBuf (void *, int, xgdIOCtx *);
