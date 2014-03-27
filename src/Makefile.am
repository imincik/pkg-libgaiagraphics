
AM_CFLAGS = -D_LARGE_FILE=1 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE=1 \
	@LIBPNG_CFLAGS@ @LIBCAIRO_CFLAGS@ @LIBXML2_CFLAGS@

INCLUDES = @CFLAGS@
INCLUDES += -I$(top_srcdir)/headers

lib_LTLIBRARIES = libgaiagraphics.la 

libgaiagraphics_la_SOURCES = \
	gaiagraphics_paint.c \
	gaiagraphics_io.c \
	gaiagraphics_image.c \
	gaiagraphics_aux.c \
	gaiagraphics_quantize.c \
	gaiagraphics_gif.c \
	gaiagraphics_png.c \
	gaiagraphics_jpeg.c \
	gaiagraphics_tiff.c \
	gaiagraphics_grids.c \
	gaiagraphics_adam7.c \
	gaiagraphics_color_rules.c \
	gaiagraphics_svg.c \
	gaiagraphics_svg_aux.c \
	gaiagraphics_svg_xml.c 

libgaiagraphics_la_LDFLAGS = -version-info 1:0:0 -no-undefined

if MINGW
libgaiagraphics_la_LIBADD = \
	@LIBXML2_LIBS@ @LIBCAIRO_LIBS@ -lgeotiff -ltiff -ljpeg -lproj @LIBPNG_LIBS@
else
libgaiagraphics_la_LIBADD = \
	@LIBXML2_LIBS@ @LIBCAIRO_LIBS@ -lgeotiff -ltiff -ljpeg -lproj @LIBPNG_LIBS@ -lpthread
endif

