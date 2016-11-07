#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "png.h"

#include "types.h"
#include "pngreadwrite.h"

typedef struct _png_struct {
    int				sample_depth;
	int				compression_level;
    unsigned long	width;
    unsigned long	height;
    FILE *			outfile;
    jmp_buf			jmpbuf;
}
PNG_INFO;

static void pngreadwrite_error_handler(png_structp png_ptr, png_const_charp msg);

int mapColourType(int colourType)
{
	int		returnType;
	
	switch (colourType) {
		case PNG_COLOR_TYPE_GRAY:
			returnType = PNG_COLOUR_TYPE_GREY;
			break;

		case PNG_COLOR_TYPE_GRAY_ALPHA:
			returnType = PNG_COLOUR_TYPE_GREYA;
			break;

		case PNG_COLOR_TYPE_RGB:
			returnType = PNG_COLOUR_TYPE_RGB;
			break;

		case PNG_COLOR_TYPE_RGB_ALPHA:
			returnType = PNG_COLOUR_TYPE_RGBA;
			break;
			
		case PNG_COLOR_TYPE_PALETTE:
			returnType = PNG_COLOUR_TYPE_PALETTE;
			break;
		
		default:
			returnType = PNG_COLOUR_TYPE_UNSUPPORTED;
			break;
	}
	
	return returnType;
}

byte *readPngImage(char *pszFilename, PPNGDETAILS details)
{
	unsigned int	x;
	unsigned int	y;
	png_structp		png_ptr;
	png_infop		info_ptr;
	FILE *			fp;
	byte *			row = NULL;
	byte *			rowStart = NULL;
	byte *			data = NULL;
	byte *			dataStart = NULL;

	PNG_INFO		pngInfo;

	if ((fp = fopen(pszFilename, "rb")) == NULL) {
	  return NULL;
	}

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also supply the
	* the compiler header file version, so that we know if the application
	* was compiled with a compatible version of the library.  REQUIRED
	*/
	png_ptr = png_create_read_struct(
					PNG_LIBPNG_VER_STRING,
					&pngInfo, 
					pngreadwrite_error_handler, 
					NULL);

	if (png_ptr == NULL)
	{
	  fclose(fp);
	  return NULL;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
	  fclose(fp);
	  png_destroy_read_struct(&png_ptr, NULL, NULL);
	  return NULL;
	}

	/* Set error handling if you are using the setjmp/longjmp method (this is
	* the normal method of doing things with libpng).  REQUIRED unless you
	* set up your own error handlers in the png_create_read_struct() earlier.
	*/

	if (setjmp(pngInfo.jmpbuf))
	{
	  /* Free all of the memory associated with the png_ptr and info_ptr */
	  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	  fclose(fp);
	  /* If we get here, we had a problem reading the file */
	  return NULL;
	}

	/* Set up the input control if you are using standard C streams */
	png_init_io(png_ptr, fp);
	
	png_read_info(png_ptr, info_ptr);
	
	details->lWidth = png_get_image_width(png_ptr, info_ptr);
	details->lHeight = png_get_image_height(png_ptr, info_ptr);
	details->usBitsPerPixel = png_get_bit_depth(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr);
	details->colourType = mapColourType(png_get_color_type(png_ptr, info_ptr));
	
	data = (byte *)malloc(png_get_image_width(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr) * png_get_image_height(png_ptr, info_ptr));
	
	if (data == NULL) {
	  fclose(fp);
	  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	  return NULL;
	}
	
	dataStart = data;
	
	row = (byte *)malloc(png_get_image_width(png_ptr, info_ptr) * png_get_channels(png_ptr, info_ptr));
	
	if (row == NULL) {
	  fclose(fp);
	  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	  return NULL;
	}
	
	rowStart = row;
	
	for (y = 0;y < png_get_image_height(png_ptr, info_ptr);++y) {
		row = rowStart;
		
		png_read_row(png_ptr, row, NULL);
		
		for (x = 0;x < png_get_image_width(png_ptr, info_ptr);++x) {
			*data++ = *row++; // Red
			*data++ = *row++; // Green
			*data++ = *row++; // Blue
		}
	}

	png_read_end(png_ptr, NULL);
	
	  /* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	/* close the file */
	fclose(fp);
	
	return dataStart;
}


int writePngImage(char *pszFilename, byte *pbData, PPNGDETAILS details)
{
	long			y;
	long			x;
	png_structp		png_ptr = NULL;
    png_infop		info_ptr = NULL;
	FILE *			pOutputFile;
	byte *			row = NULL;
	byte *			rowStart = NULL;

	PNG_INFO		pngInfo;
	
	pOutputFile = fopen(pszFilename, "wb");
	
	if (pOutputFile == NULL) {
		return -1;
	}

    /* could also replace libpng warning-handler (final NULL), but no need: */
    png_ptr = png_create_write_struct(
					PNG_LIBPNG_VER_STRING, 
					&pngInfo,
					pngreadwrite_error_handler, 
					NULL);
    
	if (!png_ptr)
        return 4;   /* out of memory */

    info_ptr = png_create_info_struct(png_ptr);
    
	if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        return 4;   /* out of memory */
    }

    /* make sure outfile is (re)opened in BINARY mode */

    png_init_io(png_ptr, pOutputFile);

    /* set the compression level*/

    png_set_compression_level(png_ptr, details->compressionLevel);

    /* set the image parameters appropriately */

    png_set_IHDR(
			png_ptr, 
			info_ptr, 
			details->lWidth, 
			details->lHeight,
			8, 
			PNG_COLOR_TYPE_RGB, 
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, 
			PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);
	
 	row = (byte *)malloc(sizeof(byte) * details->lWidth * NUM_CHANNELS);

	if (row == NULL) {
		fclose(pOutputFile);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		
		return -1;
	}

    if (setjmp(pngInfo.jmpbuf)) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(pOutputFile);
		free(row);
        return 2;
    }
	
	rowStart = row;

	for (y = 0L;y < details->lHeight;++y) {
		row = rowStart;
	
		for (x = 0L;x < details->lWidth;++x) {
			*row++ = *pbData++; // Red
			*row++ = *pbData++; // Green
			*row++ = *pbData++; // Blue
		}

		png_write_row(png_ptr, rowStart);
	}
	
    png_write_end(png_ptr, NULL);
	
	if (pOutputFile != NULL) {
		fclose(pOutputFile);
	}
	
	if (png_ptr != NULL && info_ptr != NULL) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}

	return 0;
}

static void pngreadwrite_error_handler(png_structp png_ptr, png_const_charp msg)
{
	PNG_INFO *		pngInfo;

    /* This function, aside from the extra step of retrieving the "error
     * pointer" (below) and the fact that it exists within the application
     * rather than within libpng, is essentially identical to libpng's
     * default error handler.  The second point is critical:  since both
     * setjmp() and longjmp() are called from the same code, they are
     * guaranteed to have compatible notions of how big a jmp_buf is,
     * regardless of whether _BSD_SOURCE or anything else has (or has not)
     * been defined. */

    fprintf(stderr, "writepng libpng error: %s\n", msg);
    fflush(stderr);

    pngInfo = png_get_error_ptr(png_ptr);
    if (pngInfo == NULL) {         /* we are completely hosed now */
        fprintf(stderr,
          "writepng severe error:  jmpbuf not recoverable; terminating.\n");
        fflush(stderr);
        exit(99);
    }

    longjmp(pngInfo->jmpbuf, 1);
}
