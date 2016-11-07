
#define NUM_CHANNELS			3

/*
** PNG Colour Types
*/
#define PNG_COLOUR_TYPE_RGB				0x00
#define PNG_COLOUR_TYPE_RGBA			0x01
#define PNG_COLOUR_TYPE_GREY			0x02
#define PNG_COLOUR_TYPE_GREYA			0x04
#define PNG_COLOUR_TYPE_PALETTE			0x08
#define PNG_COLOUR_TYPE_UNSUPPORTED		0xFF

typedef struct _pngdetails
{
	unsigned short		usBitsPerPixel;
	long				lWidth;
	long				lHeight;
	int					colourType;
	int					compressionLevel;
}
PNGDETAILS;

typedef PNGDETAILS * PPNGDETAILS;

byte *readPngImage(char *pszFilename, PPNGDETAILS details);
int writePngImage(char *pszFilename, byte *pbData, PPNGDETAILS details);
int mapColourType(int colourType);
