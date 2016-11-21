#include "iterator.h"
#include "types.h"

#ifndef _INCL_BITMAP
#define _INCL_BITMAP

#define FILENAME_BUFFER_LENGTH		512

/*
** Header sizes
*/
#define BMP_HEADER_SIZE		14
#define WINV3_HEADER_SIZE   40
#define OS2V1_HEADER_SIZE   12


/*
** Compression methods
*/
#define COMPRESSION_BI_RGB          0x00    // None - default
#define COMPRESSION_BI_RLE8         0X01    // RLE 8-bit/pixel
#define COMPRESSION_BI_RLE4         0x02    // RLE 4-bit/pixel
#define COMPRESSION_BI_BITFIELDS    0x03    // Bit field
#define COMPRESSION_BI_JPEG         0x04    // JPEG
#define COMPRESSION_BI_PNG          0x05    // PNG

/*
** PNG Format
*/
#define PNG_FMT_RGB					0x00
#define PNG_FMT_RGBA				0x01
#define PNG_FMT_GREY				0x02
#define PNG_FMT_GREYA				0x04
#define PNG_FMT_PALETTE				0x08
#define PNG_FMT_UNSUPPORTED			0xFF


enum ImageType {rgb_bitmap, rgb_png, image_type_unknown};

class Image
{
	private:
		char			szFilename[FILENAME_BUFFER_LENGTH];		// Bitmap filename
		ImageType		type;
	
	protected:
		byte			*pchData;				// Actual image data

		unsigned long	ulDataLength;
		word  			usBitsPerPixel;			// 1, 2, 4, 8, 16 or 24
		long            lWidth;					// Width of the image
		long            lHeight;				// Height of the image
		
		void			setImageType(ImageType type);
		
		virtual void	initialise() = 0;
		
		void			transform(ImageType sourceType, ImageType targetType);

	public:
						Image(char *pszFilename);
						Image(byte *pData);
						Image(Image *sourceImage, bool deep);
						Image() {}
						
						~Image();

		ImageType		getImageType();
		
		byte *			getData();
		void			setData(byte *pData);
		
		char		  * getFilename();
		void			setFilename(char *pszFilename);
		
		dword			getImageDataLength();
		void			setImageDataLength(dword ulValue);

		word			getBitsPerPixel();
		void			setBitsPerPixel(word usValue);

		long			getWidth();
		void			setWidth(long lValue);

		long			getHeight();
		void			setHeight(long lValue);

		dword			getCapacity(word bitsPerByte);
		
		ByteStreamIterator	*	iterator();
		
		virtual void	read();
		virtual void	write();
};

class PNG : public Image
{
    private:
		int				compressionLevel;
		int				format;
		
		int				mapFormat(int colourType);
     
	protected:
		virtual void	initialise();
		
    public:
                        PNG(char *pszFilename);
                        PNG(byte *pData);
						PNG(Image *sourceImage);
                        PNG();

		int				getCompressionLevel();
		void			setCompressionLevel(int level);
		
		int				getFormat();
		void			setFormat(int fmt);
 
        virtual void    read();
        virtual void    write();
};

enum BitmapType {WindowsV3, OS2V1, UnknownType};

class Bitmap : public Image
{
	private:
		char            szMagicNumber[2];		// Always ASCII 'BM'
		dword   		ulFileSize;				// Total length of the bitmap file
		byte            uchReserved[4];			// Reserved for application use
		dword   		ulStartOffset;			// Offset of actual bitmap data
		
		dword			ulHeaderSize;			// Size of the header
		word  			usColourPlanes;			// Always set to 0
		dword   		ulCompressionMethod;	// Compression for bitmap data
		long            lHRes;					// Horizontal resolution
		long            lVRes;					// Vertical resolution
		dword   		ulNumColours;			// Number of colours, 0 for RGB
		dword   		ulNumImportantColours;	// Usually 0.
		
		BitmapType		type;					// Bitmap type (Windows or OS/2)
		
        void            initialiseImage();
     
	protected:
		virtual void	initialise();
		
	public:
						Bitmap(char *pszFilename);
						Bitmap(byte *pData);
						Bitmap(Image *sourceImage);
						Bitmap();
						
		char		*	getMagicNumber();
		void			setMagicNumber(char *pszValue);
		
		dword			getFileSize();
		void			setFileSize(dword ulValue);
		
		byte		*	getReserved();
		void			setReserved(byte *pchValue);
		
		dword			getStartOffset();
		void			setStartOffset(dword ulValue);
		
		dword			getHeaderSize();
		void			setHeaderSize(dword ulValue);
		
		word			getColourPlanes();
		void			setColourPlanes(word usValue);
		
		dword			getCompressionMethod();
		void			setCompressionMethod(dword ulValue);
		
		long			getHorizontalResolution();
		void			setHorizontalResolution(long lValue);
		
		long			getVerticalResolution();
		void			setVerticalResolution(long lValue);
		
		dword			getNumColours();
		void			setNumColours(dword ulValue);
		
		dword			getNumImportantColours();
		void			setNumImportantColours(dword ulValue);
		
		BitmapType		getType();
		void			setType();
		
		virtual void	read();
		virtual void	write();
};

#endif
