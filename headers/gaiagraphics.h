/* 
/ gGraph.h
/
/ public gGraph declarations
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

#ifdef _WIN32
#ifdef DLL_EXPORT
#define GGRAPH_DECLARE __declspec(dllexport)
#else
#define GGRAPH_DECLARE extern
#endif
#else
#define GGRAPH_DECLARE __attribute__ ((visibility("default")))
#endif

#ifndef _GGRAPH_H
#define _GGRAPH_H

#ifdef __cplusplus
extern "C"
{
#endif

#define GGRAPH_COLORSPACE_UNKNOWN		-1000
#define GGRAPH_COLORSPACE_MONOCHROME		1001
#define GGRAPH_COLORSPACE_PALETTE		1002
#define GGRAPH_COLORSPACE_GRAYSCALE		1003
#define GGRAPH_COLORSPACE_TRUECOLOR		1004
#define GGRAPH_COLORSPACE_TRUECOLOR_ALPHA	1005
#define GGRAPH_COLORSPACE_GRID			1006

#define GGRAPH_SAMPLE_UNKNOWN	-1500
#define GGRAPH_SAMPLE_UINT	1501
#define GGRAPH_SAMPLE_INT	1502
#define GGRAPH_SAMPLE_FLOAT	1503

#define GGRAPH_TIFF_DEFAULT	-1001

#define GGRAPH_TIFF_LAYOUT_STRIPS	2001
#define GGRAPH_TIFF_LAYOUT_TILES	2002

#define GGRAPH_TIFF_COMPRESSION_UNKNOWN		-3000
#define GGRAPH_TIFF_COMPRESSION_NONE		3001
#define GGRAPH_TIFF_COMPRESSION_CCITTFAX3	3002
#define GGRAPH_TIFF_COMPRESSION_CCITTFAX4	3003
#define GGRAPH_TIFF_COMPRESSION_LZW		3004
#define GGRAPH_TIFF_COMPRESSION_DEFLATE		3005
#define GGRAPH_TIFF_COMPRESSION_JPEG		3006

#define GGRAPH_IMAGE_UNKNOWN	-4000
#define GGRAPH_IMAGE_GIF	4001
#define GGRAPH_IMAGE_PNG	4002
#define GGRAPH_IMAGE_JPEG	4003
#define GGRAPH_IMAGE_TIFF	4004
#define GGRAPH_IMAGE_GEOTIFF	4005
#define GGRAPH_IMAGE_HGT	4006
#define GGRAPH_IMAGE_BIN_HDR	4007
#define GGRAPH_IMAGE_FLT_HDR	4008
#define GGRAPH_IMAGE_DEM_HDR	4009
#define GGRAPH_IMAGE_ASCII_GRID	4010

#define GGRAPH_PENSTYLE_SOLID		5001
#define GGRAPH_PENSTYLE_DOT		5002
#define GGRAPH_PENSTYLE_LONG_DASH 	5003
#define GGRAPH_PENSTYLE_SHORT_DASH	5004
#define GGRAPH_PENSTYLE_DOT_DASH	5005

#define GGRAPH_FONTSTYLE_NORMAL		5101
#define GGRAPH_FONTSTYLE_ITALIC		5102

#define GGRAPH_FONTWEIGHT_NORMAL	5201
#define GGRAPH_FONTWEIGHT_BOLD		5202

#define GGRAPH_CLEAR_PATH	5100
#define GGRAPH_PRESERVE_PATH	5101


#define GGRAPH_OK				0
#define GGRAPH_ERROR				-1
#define GGRAPH_INVALID_IMAGE			-2
#define GGRAPH_INSUFFICIENT_MEMORY		-3
#define GGRAPH_FILE_OPEN_ERROR			-4
#define GGRAPH_FILE_READ_ERROR			-5
#define GGRAPH_FILE_WRITE_ERROR			-6
#define GGRAPH_GIF_CODEC_ERROR			-7
#define GGRAPH_PNG_CODEC_ERROR			-8
#define GGRAPH_JPEG_CODEC_ERROR			-9
#define GGRAPH_TIFF_CODEC_ERROR			-10
#define GGRAPH_GEOTIFF_CODEC_ERROR		-11
#define GGRAPH_HGT_CODEC_ERROR			-12
#define GGRAPH_BIN_CODEC_ERROR			-13
#define GGRAPH_FLT_CODEC_ERROR			-14
#define GGRAPH_DEM_CODEC_ERROR			-15
#define GGRAPH_ASCII_CODEC_ERROR		-16
#define GGRAPH_UNSUPPORTED_TIFF_LAYOUT		-17
#define GGRAPH_MISSING_GEO_DEFS			-18
#define GGRAPH_INVALID_COLOR_RULE		-19
#define GGRAPH_INVALID_COLOR_MAP		-20
#define GGRAPH_INVALID_SHADED_RELIEF_3ROWS	-21
#define GGRAPH_INVALID_PAINT_CONTEXT		-22
#define GGRAPH_INVALID_PAINT_BITMAP		-23
#define GGRAPH_INVALID_PAINT_BRUSH		-24
#define GGRAPH_INVALID_PAINT_FONT		-25
#define GGRAPH_INVALID_SVG			-26

#define GGRAPH_TRUE	-1
#define GGRAPH_FALSE	-2

    typedef struct gaia_graphics_landsat_recalibration
    {
/* a struct used for Landsat Radiometric Recalibration */
	double sun_distance;	/* Earth-Sun distance (in AU) */
	double sun_elevation;	/* Sun elevation (in degrees) */
/* RED band params (#3) */
	double lmin_red;	/* LMIN (from metafile) */
	double lmax_red;	/* LMAX (from metafile) */
	double qcalmin_red;	/* QCALMIN (from metafile) */
	double qcalmax_red;	/* QCALMAX (from metafile) */
	int is_gain_low_red;	/* GAIN LOW (1=yes 0=no) (from metafile) */
	double spectral_irradiance_red;	/* SPECTRAL IRRADIANCE constant */
	double low_gain_factor_red;	/* LOW GAIN multiplicator */
	double high_gain_factor_red;	/* HIGH GAIN multiplicator */
	unsigned char recalibration_min_red;	/* color recalibration MIN DN [0-255] */
	unsigned char recalibration_max_red;	/* color recalibration MAX DN [0-255] */
/* GREEN band params (#2) */
	double lmin_green;	/* same as RED band */
	double lmax_green;
	double qcalmin_green;
	double qcalmax_green;
	int is_gain_low_green;
	double spectral_irradiance_green;
	double low_gain_factor_green;
	double high_gain_factor_green;
	unsigned char recalibration_min_green;
	unsigned char recalibration_max_green;
/* BLUE band params (#1) */
	double lmin_blue;	/* same as RED band */
	double lmax_blue;
	double qcalmin_blue;
	double qcalmax_blue;
	int is_gain_low_blue;
	double spectral_irradiance_blue;
	double low_gain_factor_blue;
	double high_gain_factor_blue;
	unsigned char recalibration_min_blue;
	unsigned char recalibration_max_blue;
/* PANCHRO band params (#8) */
	double lmin_panchro;	/* same as RED band */
	double lmax_panchro;
	double qcalmin_panchro;
	double qcalmax_panchro;
	int is_gain_low_panchro;
	double spectral_irradiance_panchro;
	double low_gain_factor_panchro;
	double high_gain_factor_panchro;
	unsigned char recalibration_min_panchro;
	unsigned char recalibration_max_panchro;
    } gGraphLandsatRecalibration;
    typedef gGraphLandsatRecalibration *gGraphLandsatRecalibrationPtr;

    GGRAPH_DECLARE int gGraphCreateContext (int width, int height,
					    const void **context);
    GGRAPH_DECLARE int gGraphDestroyContext (const void *context);
    GGRAPH_DECLARE int gGraphCreateSvgContext (const char *path, int width,
					       int height,
					       const void **context);
    GGRAPH_DECLARE int gGraphDestroySvgContext (const void *context);
    GGRAPH_DECLARE int gGraphCreatePdfContext (const char *path, int page_width,
					       int page_height, int width,
					       int height,
					       const void **context);
    GGRAPH_DECLARE int gGraphDestroyPdfContext (const void *context);
    GGRAPH_DECLARE int gGraphSetPen (const void *context, unsigned char red,
				     unsigned char green, unsigned char blue,
				     unsigned char alpha, double width,
				     int style);
    GGRAPH_DECLARE int gGraphSetBrush (const void *context, unsigned char red,
				       unsigned char green, unsigned char blue,
				       unsigned char alpha);
    GGRAPH_DECLARE int gGraphSetLinearGradientBrush (const void *context,
						     double x, double y,
						     double width,
						     double height,
						     unsigned char red1,
						     unsigned char green1,
						     unsigned char blue1,
						     unsigned char alpha1,
						     unsigned char red2,
						     unsigned char green2,
						     unsigned char blue2,
						     unsigned char alpha2);
    GGRAPH_DECLARE int gGraphSetPatternBrush (const void *context,
					      const void *brush);
    GGRAPH_DECLARE int gGraphSetFont (const void *context, const void *font);
    GGRAPH_DECLARE int gGraphFillPath (const void *context, int preserve);
    GGRAPH_DECLARE int gGraphStrokePath (const void *context, int preserve);
    GGRAPH_DECLARE int gGraphMoveToPoint (const void *context, double x,
					  double y);
    GGRAPH_DECLARE int gGraphAddLineToPath (const void *context, double x,
					    double y);
    GGRAPH_DECLARE int gGraphCloseSubpath (const void *context);
    GGRAPH_DECLARE int gGraphStrokeLine (const void *context, double x0,
					 double y0, double x1, double y1);
    GGRAPH_DECLARE int gGraphDrawEllipse (const void *context, double x,
					  double y, double width,
					  double height);
    GGRAPH_DECLARE int gGraphDrawRectangle (const void *context, double x,
					    double y, double width,
					    double height);
    GGRAPH_DECLARE int gGraphDrawRoundedRectangle (const void *context,
						   double x, double y,
						   double width, double height,
						   double radius);
    GGRAPH_DECLARE int gGraphDrawCircleSector (const void *context,
					       double center_x, double center_y,
					       double radius, double from_angle,
					       double to_angle);
    GGRAPH_DECLARE int gGraphGetTextExtent (const void *context,
					    const char *text, double *pre_x,
					    double *pre_y, double *width,
					    double *height, double *post_x,
					    double *post_y);
    GGRAPH_DECLARE int gGraphDrawText (const void *context, const char *text,
				       double x, double y, double angle);
    GGRAPH_DECLARE int gGraphGetContextRgbArray (const void *context,
						 unsigned char **rgbArray);
    GGRAPH_DECLARE int gGraphGetContextAlphaArray (const void *context,
						   unsigned char **alphaArray);
    GGRAPH_DECLARE int gGraphDrawBitmap (const void *context,
					 const void *bitmap, int x, int y);
    GGRAPH_DECLARE int gGraphCreateBitmap (unsigned char *rgbaArray, int width,
					   int height, const void **bitmap);
    GGRAPH_DECLARE int gGraphDestroyBitmap (const void *bitmap);
    GGRAPH_DECLARE int gGraphCreateBrush (unsigned char *rgbaArray, int width,
					  int height, const void **brush);
    GGRAPH_DECLARE int gGraphDestroyBrush (const void *brush);
    GGRAPH_DECLARE int gGraphCreateFont (double size, int style, int weight,
					 const void **font);
    GGRAPH_DECLARE int gGraphDestroyFont (const void *font);
    GGRAPH_DECLARE int gGraphFontSetColor (const void *font, unsigned char red,
					   unsigned char green,
					   unsigned char blue,
					   unsigned char alpha);
    GGRAPH_DECLARE int gGraphFontSetOutline (const void *font, double width);

/*
/ image access methods
*/
    GGRAPH_DECLARE const void *gGraphCreateRgbImage (int width, int height);
    GGRAPH_DECLARE const void *gGraphCreateRgbImageFromBitmap (unsigned char
							       *bitmap,
							       int width,
							       int height);
    GGRAPH_DECLARE const void *gGraphCreateRgbaImage (int width, int height);
    GGRAPH_DECLARE const void *gGraphCreateGrayscaleImage (int width,
							   int height);
    GGRAPH_DECLARE const void *gGraphCreatePaletteImage (int width, int height);
    GGRAPH_DECLARE const void *gGraphCreateMonochromeImage (int width,
							    int height);
    GGRAPH_DECLARE const void *gGraphCreateGridInt16Image (int width,
							   int height);
    GGRAPH_DECLARE const void *gGraphCreateGridUInt16Image (int width,
							    int height);
    GGRAPH_DECLARE const void *gGraphCreateGridInt32Image (int width,
							   int height);
    GGRAPH_DECLARE const void *gGraphCreateGridUInt32Image (int width,
							    int height);
    GGRAPH_DECLARE const void *gGraphCreateGridFloatImage (int width,
							   int height);
    GGRAPH_DECLARE const void *gGraphCreateGridDoubleImage (int width,
							    int height);
    GGRAPH_DECLARE int gGraphImageBackgroundFill (const void *img,
						  unsigned char red,
						  unsigned char green,
						  unsigned char blue,
						  unsigned char alpha);
    GGRAPH_DECLARE int gGraphImageSetTransparentColor (const void *img,
						       unsigned char red,
						       unsigned char green,
						       unsigned char blue);
    GGRAPH_DECLARE int gGraphImageGetTransparentColor (const void *img,
						       unsigned char *red,
						       unsigned char *green,
						       unsigned char *blue);
    GGRAPH_DECLARE int gGraphGridBackgroundFill (const void *img,
						 double no_data_value);
    GGRAPH_DECLARE int gGraphImageSetNoDataValue (const void *img,
						  double no_data_vale);
    GGRAPH_DECLARE int gGraphImageGetNoDataValue (const void *img,
						  double *no_data_value);
    GGRAPH_DECLARE void gGraphDestroyImage (const void *img);
    GGRAPH_DECLARE void gGraphDestroyImageInfos (const void *img);
    GGRAPH_DECLARE int gGraphGetImageDims (const void *img, int *width,
					   int *height);
    GGRAPH_DECLARE int gGraphGetImageInfos (const void *img, int *width,
					    int *height, int *colorspace,
					    int *max_palette,
					    int *bits_per_sample,
					    int *samples_per_pixel,
					    int *sample_format, int *tile_width,
					    int *tile_height,
					    int *rows_per_strip,
					    int *compression,
					    double *no_data_value,
					    double *min_value,
					    double *max_value, int *scale_1_2,
					    int *scale_1_4, int *scale_1_8);
    GGRAPH_DECLARE int gGraphGetImageSize (const void *img, int *size);
    GGRAPH_DECLARE int gGraphImageColorSpaceOptimize (const void *img);
    GGRAPH_DECLARE int gGraphImageResampleAsRgb (const void *img);
    GGRAPH_DECLARE int gGraphImageResampleAsMonochrome (const void *img);
    GGRAPH_DECLARE int gGraphImageResampleAsPalette (const void *img,
						     int num_colors);
    GGRAPH_DECLARE int gGraphImageResampleAsGrayscale (const void *img);
    GGRAPH_DECLARE int gGraphImageResampleAsPhotographic (const void *img);
    GGRAPH_DECLARE int gGraphImageResampleAsGridInt16 (const void *img);
    GGRAPH_DECLARE int gGraphImageResampleAsGridUInt16 (const void *img);
    GGRAPH_DECLARE int gGraphImageResampleAsGridInt32 (const void *img);
    GGRAPH_DECLARE int gGraphImageResampleAsGridUInt32 (const void *img);
    GGRAPH_DECLARE int gGraphImageResampleAsGridFloat (const void *img);
    GGRAPH_DECLARE int gGraphImageResampleAsGridDouble (const void *img);
    GGRAPH_DECLARE int gGraphImageTransparentResample (const void *img);
    GGRAPH_DECLARE int gGraphImageGuessFormat (const void *mem_buf,
					       int mem_buf_size);
    GGRAPH_DECLARE int gGraphFileImageGuessFormat (const char *path, int *type);

/*
/ the following methods return a copy of the internal image buffer
/ PLEASE NOTE: you are responsible to free() the returned memory block
*/
    GGRAPH_DECLARE int gGraphImageBufferReferenceRGB (const void *img,
						      unsigned char **buffer);
    GGRAPH_DECLARE int gGraphImageBufferReferenceRGBA (const void *img,
						       unsigned char **buffer);
    GGRAPH_DECLARE int gGraphImageBufferReferenceARGB (const void *img,
						       unsigned char **buffer);
    GGRAPH_DECLARE int gGraphImageBufferReferenceBGR (const void *img,
						      unsigned char **buffer);
    GGRAPH_DECLARE int gGraphImageBufferReferenceBGRA (const void *img,
						       unsigned char **buffer);

/*
/ image resizing / image subsetting
*/
    GGRAPH_DECLARE int gGraphImageResizeNormal (const void *orig,
						const void **dest, int width,
						int height);
    GGRAPH_DECLARE int gGraphImageResizeHighQuality (const void *orig,
						     const void **dest,
						     int width, int height);
    GGRAPH_DECLARE int gGraphImageResizeToResolution (const void *orig,
						      const void **dest,
						      double pixel_x_size,
						      double pixel_y_size,
						      int *width, int *height);
    GGRAPH_DECLARE int gGraphImageSubSet (const void *orig, const void **dest,
					  int upper_left_x, int upper_left_y,
					  int width, int height);

/*
/ utility functions handling image georeferencing
*/
    GGRAPH_DECLARE int gGraphGetWorldFilePath (const char *main_path,
					       char *world_file_path);

    GGRAPH_DECLARE int gGraphCheckHgtPath (const char *path, int *lat,
					   int *lon);
    GGRAPH_DECLARE int gGraphCheckBinPath (const char *path, char *hdr_path,
					   int dont_test);
    GGRAPH_DECLARE int gGraphCheckFltPath (const char *path, char *hdr_path,
					   int dont_test);
    GGRAPH_DECLARE int gGraphCheckDemPath (const char *path, char *hdr_path,
					   int dont_test);
    GGRAPH_DECLARE int gGraphCheckAscPath (const char *path);

    GGRAPH_DECLARE int gGraphImageIsGeoRef (const void *img);
    GGRAPH_DECLARE int gGraphImageSetGeoRef (const void *img, int srid,
					     const char *srs_name,
					     const char *proj4text,
					     double upper_left_x,
					     double upper_left_y,
					     double pixel_x_size,
					     double pixel_y_size);
    GGRAPH_DECLARE int gGraphImageGetGeoRef (const void *img, int *srid,
					     char *srs_name,
					     char *proj4text,
					     double *upper_left_x,
					     double *upper_left_y,
					     double *pixel_x_size,
					     double *pixel_y_size);
    GGRAPH_DECLARE int gGraphImageInfosGetGeoRef (const void *img,
						  int *srid,
						  char *srs_name,
						  char *proj4text,
						  double *upper_left_x,
						  double *upper_left_y,
						  double *pixel_x_size,
						  double *pixel_y_size);
    GGRAPH_DECLARE int gGraphReadWorldFile (const char *path,
					    double *upper_left_x,
					    double *upper_left_y,
					    double *pixel_x_size,
					    double *pixel_y_size);

/*
/ utility functions returning an image
*/
    GGRAPH_DECLARE int gGraphImageFromFile (const char *path, int image_type,
					    const void **image_handle,
					    int scale);
    GGRAPH_DECLARE int gGraphImageFromMemBuf (const void *mem_buf,
					      int mem_buf_size, int image_type,
					      const void **image_handle,
					      int scale);

/*
/ utility functions returning image infos
*/
    GGRAPH_DECLARE int gGraphImageInfosFromHgtFile (const char *path,
						    int lat, int lon,
						    const void **infos_handle);
    GGRAPH_DECLARE int gGraphImageInfosFromBinFile (const char *path,
						    const char *hdr_path,
						    const void **infos_handle);
    GGRAPH_DECLARE int gGraphImageInfosFromFltFile (const char *path,
						    const char *hdr_path,
						    const void **infos_handle);
    GGRAPH_DECLARE int gGraphImageInfosFromDemFile (const char *path,
						    const char *hdr_path,
						    const void **infos_handle);
    GGRAPH_DECLARE int gGraphImageInfosFromAscFile (const char *path,
						    const void **infos_handle);
    GGRAPH_DECLARE int gGraphImageInfosFromFile (const char *path,
						 int image_type,
						 const void **infos_handle);
    GGRAPH_DECLARE int gGraphImageInfosFromMemBuf (const void *mem_buf,
						   int mem_buf_size,
						   int image_type,
						   const void **infos_handle);

/*
/ utility functions generating a file from an image
*/
    GGRAPH_DECLARE int gGraphImageToJpegFile (const void *img, const char *path,
					      int quality);
    GGRAPH_DECLARE int gGraphImageToPngFile (const void *img, const char *path,
					     int compression_level,
					     int quantization_factor,
					     int interlaced);
    GGRAPH_DECLARE int gGraphImageToGifFile (const void *img, const char *path);

/*
/ utility functions generating a memory buffer from an image
*/
    GGRAPH_DECLARE int gGraphImageToJpegMemBuf (const void *img, void **mem_buf,
						int *mem_buf_size,
						int jpeg_quality);
    GGRAPH_DECLARE int gGraphImageToPngMemBuf (const void *img, void **mem_buf,
					       int *mem_buf_size,
					       int compression_level,
					       int quantization_factor,
					       int interlaced,
					       int is_transparent);
    GGRAPH_DECLARE int gGraphImageToGifMemBuf (const void *img, void **mem_buf,
					       int *mem_buf_size,
					       int is_transparent);

/*
/ utility functions handling Adam7 and Monochrome encoding
*/
    GGRAPH_DECLARE int gGraphImageToAdam7 (const void *img, void *mem_bufs[7],
					   int mem_buf_sizes[7], void **palette,
					   int *palette_size);
    GGRAPH_DECLARE int gGraphImageFromAdam7 (void *mem_bufs[7],
					     int mem_buf_sizes[7],
					     void *palette, int palette_size,
					     const void **image_handle,
					     int scale);
    GGRAPH_DECLARE int gGraphImageToMonochrome (const void *img, void **mem_buf,
						int *mem_buf_size);
    GGRAPH_DECLARE int gGraphImageFromMonochrome (const void *mem_buf,
						  int mem_buf_size,
						  const void **image_handle);

/*
/ functions for RAW image handling
*/
    GGRAPH_DECLARE int gGraphImageFromRawMemBuf (const void *mem_buf,
						 int mem_buf_size,
						 const void **image_handle);
    GGRAPH_DECLARE int gGraphIsRawImage (const void *mem_buf, int mem_buf_size);

/*
/ methods accessing a file-based Image by Strips
*/
    GGRAPH_DECLARE int gGraphImageFromHgtFileByStrips (const char *path,
						       int lat, int lon,
						       const void
						       **strip_handle);
    GGRAPH_DECLARE int gGraphImageFromBinFileByStrips (const char *path,
						       const char *hdr_path,
						       const void
						       **strip_handle);
    GGRAPH_DECLARE int gGraphImageFromFltFileByStrips (const char *path,
						       const char *hdr_path,
						       const void
						       **strip_handle);
    GGRAPH_DECLARE int gGraphImageFromDemFileByStrips (const char *path,
						       const char *hdr_path,
						       const void
						       **strip_handle);
    GGRAPH_DECLARE int gGraphImageFromAscFileByStrips (const char *path,
						       const void
						       **strip_handle);
    GGRAPH_DECLARE int gGraphImageFromFileByStrips (const char *path,
						    int image_type,
						    const void **strip_handle);
    GGRAPH_DECLARE int gGraphReadNextStrip (const void *strip_handle,
					    int *progress);

    GGRAPH_DECLARE int gGraphImageToJpegFileByStrips (const void **strip_handle,
						      const char *path,
						      int width, int height,
						      int color_model,
						      int quality);
    GGRAPH_DECLARE int gGraphImageToPngFileByStrips (const void **strip_handle,
						     const char *path,
						     int width, int height,
						     int color_model,
						     int bits_per_sample,
						     int num_palette,
						     unsigned char *red,
						     unsigned char *green,
						     unsigned char *blue,
						     int compression_level,
						     int quantization_factor);
    GGRAPH_DECLARE int gGraphImageToTiffFileByStrips (const void **strip_handle,
						      const char *path,
						      int width, int height,
						      int color_model,
						      int is_tiled,
						      int tile_width,
						      int tile_height,
						      int rows_per_strip,
						      int bits_per_sample,
						      int sample_format,
						      int num_palette,
						      unsigned char *red,
						      unsigned char *green,
						      unsigned char *blue,
						      int compression);
    GGRAPH_DECLARE int gGraphImageToGeoTiffFileByStrips (const void
							 **strip_handle,
							 const char *path,
							 int width, int height,
							 int color_model,
							 int is_tiled,
							 int tile_width,
							 int tile_height,
							 int rows_per_strip,
							 int bits_per_sample,
							 int sample_format,
							 int num_palette,
							 unsigned char *red,
							 unsigned char *green,
							 unsigned char *blue,
							 int compression,
							 int srid,
							 const char *srs_name,
							 const char *proj4text,
							 double upper_left_x,
							 double upper_left_y,
							 double pixel_x_size,
							 double pixel_y_size);
    GGRAPH_DECLARE int gGraphImageToBinHdrFileByStrips (const void
							**strip_handle,
							const char *path,
							int width, int height,
							int bits_per_sample,
							double upper_left_x,
							double upper_left_y,
							double pixel_x_size,
							double pixel_y_size,
							double no_data_value);
    GGRAPH_DECLARE int gGraphImageToFltHdrFileByStrips (const void
							**strip_handle,
							const char *path,
							int width, int height,
							int bits_per_sample,
							double upper_left_x,
							double upper_left_y,
							double pixel_x_size,
							double pixel_y_size,
							double no_data_value);
    GGRAPH_DECLARE int gGraphImageToAscFileByStrips (const void **strip_handle,
						     const char *path,
						     int width, int height,
						     int sample,
						     int bits_per_sample,
						     double upper_left_x,
						     double upper_left_y,
						     double pixel_x_size,
						     double pixel_y_size,
						     double no_data_value);
    GGRAPH_DECLARE int gGraphWriteNextStrip (const void *strip_handle,
					     int *progress);
    GGRAPH_DECLARE int gGraphWriteBinHeader (const char *hdr_path,
					     const void *strip_handle);
    GGRAPH_DECLARE int gGraphWriteFltHeader (const char *hdr_path,
					     const void *strip_handle);

    GGRAPH_DECLARE int gGraphStripImageClonePalette (const void *strip_handle,
						     int *color_model,
						     int *num_palette,
						     unsigned char *red,
						     unsigned char *green,
						     unsigned char *blue);
    GGRAPH_DECLARE int gGraphStripImageAllocPixels (const void *strip_handle,
						    int rows_per_block);
    GGRAPH_DECLARE int gGraphStripImageEOF (const void *strip_handle);
    GGRAPH_DECLARE int gGraphStripImageCopyPixels (const void *in_strip_handle,
						   const void
						   *out_strip_handle);
    GGRAPH_DECLARE int gGraphStripImageRenderGridPixels (const void
							 *in_strip_handle,
							 const void
							 *out_strip_handle,
							 const void
							 *color_map_handle,
							 int num_threads);
    GGRAPH_DECLARE int gGraphStripImageSubSetPixels (const void
						     *in_strip_handle,
						     const void
						     *out_strip_handle,
						     int start_from, int row);
    GGRAPH_DECLARE int gGraphStripImageGetNextRow (const void *in_strip_handle,
						   int *next_row);
    GGRAPH_DECLARE int gGraphStripIsFull (const void *in_strip_handle);
    GGRAPH_DECLARE int gGraphGetStripImageMinMaxValue (const void
						       *in_strip_handle,
						       double *min_value,
						       double *max_value,
						       double no_data_value);

    GGRAPH_DECLARE int gGraphStripImageRewind (const void *in_strip_handle);
    GGRAPH_DECLARE int gGraphStripImageGetCurrentRows (const void
						       *in_strip_handle,
						       int *rows);
    GGRAPH_DECLARE int gGraphStripImageSetCurrentRows (const void
						       *in_strip_handle,
						       int rows);
    GGRAPH_DECLARE int gGraphStripImageGetPixelRGB (const void *in_strip_handle,
						    int col, int row,
						    unsigned char *red,
						    unsigned char *green,
						    unsigned char *blue);
    GGRAPH_DECLARE int gGraphStripImageSetPixelRGB (const void *in_strip_handle,
						    int col, int row,
						    unsigned char red,
						    unsigned char green,
						    unsigned char blue);

    GGRAPH_DECLARE int gGraphCountColors (const char *path, int image_type,
					  int rows_per_block);
    GGRAPH_DECLARE void gGraphSmartPrintf (double value, char *buf);

    GGRAPH_DECLARE int gGraphColorRuleFromFile (const char *path,
						const void **color_rule);
    GGRAPH_DECLARE int gGraphColorRuleFromMemBuf (char *buf,
						  const void **color_rule);
    GGRAPH_DECLARE void gGraphDestroyColorRule (const void *color_rule);
    GGRAPH_DECLARE int gGraphIsColorRuleRelative (const void *color_rule,
						  int *relative);
    GGRAPH_DECLARE int gGraphCreateColorMapAbsolute (const void *color_rule,
						     unsigned char
						     background_red,
						     unsigned char
						     background_green,
						     unsigned char
						     background_blue,
						     const void **color_map);
    GGRAPH_DECLARE int gGraphCreateColorMapRelative (const void *color_rule,
						     double min, double max,
						     unsigned char
						     background_red,
						     unsigned char
						     background_green,
						     unsigned char
						     background_blue,
						     const void **color_map);
    GGRAPH_DECLARE void gGraphDestroyColorMap (const void *color_map);

    GGRAPH_DECLARE int gGraphCreateShadedReliefTripleRow (int width,
							  unsigned char
							  background_red,
							  unsigned char
							  background_green,
							  unsigned char
							  background_blue,
							  double no_data,
							  const void
							  *color_map_handle,
							  unsigned char
							  mono_red,
							  unsigned char
							  mono_green,
							  unsigned char
							  mono_blue,
							  double z_factor,
							  double scale_factor,
							  double azimuth,
							  double altitude,
							  const void
							  **triple_row_handle);
    GGRAPH_DECLARE void gGraphDestroyShadedReliefTripleRow (const void
							    *triple_row_handle);

    GGRAPH_DECLARE int gGraphShadedReliefRenderPixels (const void
						       *triple_row_handle,
						       int num_threads,
						       int *out_row_ready);
    GGRAPH_DECLARE int gGraphStripImageGetShadedReliefScanline (const void
								*in_img_handle,
								int row_index,
								const void
								*triple_row_handle);
    GGRAPH_DECLARE int gGraphStripImageSetShadedReliefScanline (const void
								*triple_row_handle,
								const void
								*out_img_handle,
								int row_index);
    GGRAPH_DECLARE int gGraphLandsatRGB (const void *img_red,
					 const void *img_green,
					 const void *img_blue,
					 const void *img_rgb, int width,
					 int num_rows,
					 gGraphLandsatRecalibrationPtr params,
					 int num_threads);
    GGRAPH_DECLARE int gGraphLandsatBW (const void *img_in, const void *img_out,
					int width, int num_rows,
					gGraphLandsatRecalibrationPtr params,
					int num_threads);
    GGRAPH_DECLARE int gGraphGetLandsatSceneExtent (const void *img_in,
						    int base_row, double *top_x,
						    double *top_y,
						    double *bottom_x,
						    double *bottom_y,
						    double *left_x,
						    double *left_y,
						    double *right_x,
						    double *right_y);
    GGRAPH_DECLARE int gGraphLandsatMergePixels (const void *img_in,
						 int base_row,
						 const void *img_out);
    GGRAPH_DECLARE int gGraphOutputPixelsToStripImage (const void *img_in,
						       const void *img_out,
						       int in_row, int out_row);
    GGRAPH_DECLARE int gGraphInputPixelsFromStripImage (const void *img_in,
							const void *img_out,
							int in_col);
    GGRAPH_DECLARE int gGraphGeoMergePixels (const void *img_in,
					     const void *img_out);
    GGRAPH_DECLARE int gGraphImageFromStripImage (const void *img_in,
						  int color_space,
						  int sample_format,
						  int bits_per_sample,
						  int samples_per_pixel,
						  int start_line,
						  const void **img_out);

/* SVG images */
    GGRAPH_DECLARE int gGraphCreateSVG (const unsigned char *svg_document,
					int svg_bytes, void **svg_handle);
    GGRAPH_DECLARE int gGraphGetSVGDims (void *svg_handle, double *width,
					 double *height);
    GGRAPH_DECLARE int gGraphImageFromSVG (void *handle, double size,
					   const void **img_out);
    GGRAPH_DECLARE int gGraphFreeSVG (void *sgv_handle);

#ifdef __cplusplus
}
#endif

#endif				/* _GGRAPH_H */
