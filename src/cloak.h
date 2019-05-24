#include "image.h"
#include "data.h"
#include "types.h"

#ifndef _INCL_CLOAK
#define _INCL_CLOAK

#define DEFAULT_COMPRESSION_LEVEL		5

class Cloak
{
	private:
		Image *				sourceImage;
		Image *				targetImage;

		Data *				sourceData;
		Data *				targetData;

		word				bitsPerByte;

		int					compressionLevel;

		ImageType			sourceImageType;
		ImageType			targetImageType;

		ImageType			_getImageType(char *pszFilename);

		void				_merge(byte * pTargetBytes);
		void				_extract();

		void				_populateSizeBuffer(dword size, byte *pBuffer);
		dword				_getSizeFromBuffer(byte *pBuffer);

		void				_printStats();

	public:
							Cloak();
							Cloak(int compressionLevel);
							~Cloak();

		word				getBitsPerByte();
		void				setBitsPerByte(word bitsPerByte);

		void				setCompressionLevel(int CompressionLevel);

		ImageType			getSourceImageType();
		ImageType			getTargetImageType();

		Image *				getSourceImage();

		void				getImageSize(long &width, long &height, word &bitsPerPixel);
		dword				getImageCapacity();

		dword				getDataFileSize();

		void				loadSourceImage(char *pszFilename);
		void				loadSourceDataFile(char *pszFilename);

		void				copy(char *pszFilename);

		void				merge(char *pszFilename);
		void				merge(char *pszFilename, byte *pbKeystream, dword ulKeyLength);

		void				extract(char *pszFilename);
		void				extract(char *pszFilename, byte *pbKeystream, dword ulKeyLength);

		void				validate();

		const char *		getTargetFileExtension();
};

#endif
