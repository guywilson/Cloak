#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern "C" {
	#include "memdebug.h"
}
#include "cloak.h"
#include "encryption.h"
#include "exception.h"
#include "errorcodes.h"

using namespace std;

Cloak::Cloak()
{
	sourceImage = NULL;
	targetImage = NULL;
	sourceFile = NULL;
	targetFile = NULL;

	bitsPerByte = (word)0;
}

Cloak::~Cloak()
{
	if (sourceImage != NULL) {
		delete sourceImage;
	}
	if (targetImage != NULL) {
		delete targetImage;
	}
	if (sourceFile != NULL) {
		delete sourceFile;
	}
	if (targetFile != NULL) {
		delete targetFile;
	}
}

Image *Cloak::getSourceImage()
{
	return this->sourceImage;
}

ImageType Cloak::_getImageType(char *pszFilename)
{
	dword		i;
	dword		fileNameLength;
	int			j;
	ImageType	type;
	char		ch;
	char		szExtension[4];

	fileNameLength = (dword)strlen(pszFilename);

	// Work backwords to pick out the file extension
	// either .png or .bmp
	for (i = fileNameLength - 1;i > 0;i--) {
		ch = pszFilename[i];

		if (ch == '.') {
			i++;

			for (j = 0;j < 4;j++) {
				if (pszFilename[i] != '\0') {
					szExtension[j] = tolower(pszFilename[i++]);
				}
				else {
					break;
				}
			}

			szExtension[j] = '\0';
			break;
		}
	}

	if (strncmp(szExtension, "bmp", 3) == 0) {
		type = rgb_bitmap;
	}
	else {
		type = rgb_png;
	}

	return type;
}

void Cloak::loadSourceImage(char *pszFilename)
{
	if (sourceImage != NULL) {
		delete sourceImage;
	}

	ImageType type = _getImageType(pszFilename);

	if (type == rgb_bitmap) {
		sourceImage = new Bitmap(pszFilename);
	}
	else if (type == rgb_png) {
		sourceImage = new PNG(pszFilename);
	}
	else {
		throw new Exception(
					ERR_UNDEFINED,
					"Invalid image type",
					__FILE__,
					"Cloak",
					"loadSourceImage()",
					__LINE__);
	}

	sourceImageType = type;

	sourceImage->read();
}

void Cloak::loadSourceDataFile(char *pszFilename)
{
	if (sourceFile != NULL) {
		delete sourceFile;
	}

	sourceFile = new EncryptedDataFile(pszFilename);
	sourceFile->read();
}

word Cloak::getBitsPerByte()
{
	return this->bitsPerByte;
}

void Cloak::setBitsPerByte(word bitsPerByte)
{
	this->bitsPerByte = bitsPerByte;
}

ImageType Cloak::getSourceImageType()
{
	return this->sourceImageType;
}

ImageType Cloak::getTargetImageType()
{
	return this->targetImageType;
}

void Cloak::getImageSize(long &width, long &height, word &bitsPerPixel)
{
	width = sourceImage->getWidth();
	height = sourceImage->getHeight();
	bitsPerPixel = sourceImage->getBitsPerPixel();
}

dword Cloak::getImageCapacity()
{
	return sourceImage->getCapacity(this->bitsPerByte);
}

dword Cloak::getDataFileSize()
{
	return sourceFile->getFileLength();
}

void Cloak::copy(char *pszFilename)
{
	targetImageType = _getImageType(pszFilename);

	if (targetImageType == rgb_bitmap) {
		targetImage = new Bitmap(sourceImage);
	}
	else if (targetImageType == rgb_png) {
		targetImage = new PNG(sourceImage);
	}

	targetImage->setFilename(pszFilename);

	targetImage->write();
}

/*
** Encode the file size by XORing with the image size. An attempt
** to disguise the size of the hidden file rather than encoding it
** as-is in the image.
*/
void Cloak::_populateSizeBuffer(dword size, byte *pBuffer)
{
	int		i;
	dword	imageSize;
	byte	imageSizeBuffer[4];

	imageSize = sourceImage->getImageDataLength();

	memcpy(pBuffer, &size, 4);
	memcpy(imageSizeBuffer, &imageSize, 4);

	for (i = 0;i < 4;i++) {
		pBuffer[i] = pBuffer[i] ^ imageSizeBuffer[i];
	}
}

/*
** Extract the file size by XORing with the image size. An attempt
** to disguise the size of the hidden file rather than encoding it
** as-is in the image.
*/
dword Cloak::_getSizeFromBuffer(byte *pBuffer)
{
	int		i;
	dword	imageSize;
	dword	ulSize = 0L;
	byte	imageSizeBuffer[4];

	imageSize = sourceImage->getImageDataLength();

	memcpy(imageSizeBuffer, &imageSize, 4);

	for (i = 0;i < 4;i++) {
		pBuffer[i] = pBuffer[i] ^ imageSizeBuffer[i];
	}

	memcpy(&ulSize, pBuffer, 4);

	return ulSize;
}

void Cloak::_merge(byte * pTargetBytes)
{
	byte		dataBits;
	byte		bitmapByte;
	byte		targetByte;
	byte		bitMask;
	byte		chSizeBuffer[4];
	dword		ulFileLength;
	int			i = 0;

	ByteStreamIterator *bitmapIterator = targetImage->iterator();
	BitStreamIterator *dataIterator = sourceFile->iterator(bitsPerByte);

	bitMask = dataIterator->getBitMask();

	ulFileLength = sourceFile->getFileLength();

	_populateSizeBuffer(ulFileLength, chSizeBuffer);

	BitStreamIterator sizeIterator(chSizeBuffer, 4, 1);

	while (dataIterator->hasNext()) {
		bitmapByte = bitmapIterator->nextByte();

		/*
		** Get bits for size of hidden file first, then the actual data bits...
		*/
		if (sizeIterator.hasNext()) {
			dataBits = sizeIterator.nextBits();

			/*
			** Merge size data bits and bitmap data...
			** First, clear the first bit and then bitwise OR with data...
			** Secret file size is always stored as 1 bit per byte
			*/
			targetByte = (bitmapByte & ~0x01) | dataBits;

			pTargetBytes[i++] = targetByte;
		}
		else {
			dataBits = dataIterator->nextBits();

			/*
			** Merge data bits and bitmap data...
			** First, clear the first n bits and then bitwise OR with data...
			*/
			targetByte = (bitmapByte & ~bitMask) | dataBits;

			pTargetBytes[i++] = targetByte;
		}
	}

	delete bitmapIterator;
	delete dataIterator;

	targetImage->write();
}

void Cloak::_extract()
{
	ByteStreamIterator	*bitmapIterator;
	byte				bitmapByte;
	byte				chSecretBits;
	byte				chSecretByte = 0x00;;
	byte				bitMask;
	byte	*			pchSecretBuffer;
	byte				chSizeBuffer[4];
	dword				ulSecretBytes = 0L;
	dword				ulHiddenFileLength = 0L;
	bool				readSize = true;
    int             	bitCounter = 0;
    dword             	pos = 0L;

	bitMask = BitStreamIterator::getBitMask(bitsPerByte);

	bitmapIterator = sourceImage->iterator();

	while (bitmapIterator->hasNext()) {
		bitmapByte = bitmapIterator->nextByte();

		/*
		** Size of secret file is always stored at 1 bit/byte...
		*/
		if (readSize) {
			chSecretBits = bitmapByte & 0x01;
			chSecretByte += chSecretBits << bitCounter;

			bitCounter++;
		}
		else {
			chSecretBits = bitmapByte & bitMask;
			chSecretByte += chSecretBits << bitCounter;

			bitCounter += bitsPerByte;
		}

        if (bitCounter == 8) {
			if (readSize) {
				chSizeBuffer[pos++] = chSecretByte;

				if (pos == 4L) {
					ulSecretBytes = _getSizeFromBuffer(chSizeBuffer);

					if (ulSecretBytes > getImageCapacity()) {
						throw new Exception(ERR_NO_HIDDEN_FILE, "Bitmap does not seem to contain a valid hidden file", __FILE__, "Cloak", "extract()", __LINE__);
					}

					ulHiddenFileLength = EncryptionAlgorithm::getEncryptedDataLength(ulSecretBytes);

					pchSecretBuffer = (byte *)malloc_d(ulHiddenFileLength, "Cloak._extract():pchSecretBuffer");

					if (pchSecretBuffer == NULL) {
						throw new Exception(ERR_MALLOC, "Failed to allocate memory for secret file", __FILE__, "Cloak", "extract()", __LINE__);
					}

					readSize = false;
					pos = 0L;
				}
			}
			else {
				pchSecretBuffer[pos++] = chSecretByte;

				if (pos >= ulHiddenFileLength) {
					break;
				}
			}

            bitCounter = 0;
            chSecretByte = 0x00;
        }
	}

	delete bitmapIterator;

	targetFile = new EncryptedDataFile(pchSecretBuffer, ulSecretBytes);
}

void Cloak::merge(char *pszFilename, char *pszPassword)
{
	byte	*	pTargetBytes;

	validate();

	sourceFile->encrypt(pszPassword);

	targetImageType = _getImageType(pszFilename);

	if (targetImageType == rgb_bitmap) {
		targetImage = new Bitmap(sourceImage);
	}
	else if (targetImageType == rgb_png) {
		targetImage = new PNG(sourceImage);
	}

	targetImage->setFilename(pszFilename);

	pTargetBytes = targetImage->getData();

	_merge(pTargetBytes);
}

void Cloak::merge(char *pszFilename, byte *pbKeystream, dword ulKeyLength)
{
	byte	*	pTargetBytes;

	validate();

	sourceFile->encrypt(pbKeystream, ulKeyLength);

	targetImageType = _getImageType(pszFilename);

	if (targetImageType == rgb_bitmap) {
		targetImage = new Bitmap(sourceImage);
	}
	else if (targetImageType == rgb_png) {
		targetImage = new PNG(sourceImage);
	}

	targetImage->setFilename(pszFilename);

	pTargetBytes = targetImage->getData();

	_merge(pTargetBytes);
}

void Cloak::extract(char *pszFilename, char *pszPassword)
{
	_extract();

	targetFile->setFilename(pszFilename);
	targetFile->decrypt(pszPassword);
	targetFile->write();
}

void Cloak::extract(char *pszFilename, byte *pbKeystream, dword ulKeyLength)
{
	_extract();

	targetFile->setFilename(pszFilename);
	targetFile->decrypt(pbKeystream, ulKeyLength);
	targetFile->write();
}

void Cloak::validate()
{
	dword		ulHiddenFileSize;

	ulHiddenFileSize = ((EncryptionAlgorithm::getEncryptedDataLength(sourceFile->getFileLength()) * 8L) / (dword)bitsPerByte);

	if (!(bitsPerByte == 1 || bitsPerByte == 2 || bitsPerByte == 4)) {
		throw new Exception(ERR_VALIDATION, "Invalid bits per byte, must be 1, 2 or 4", __FILE__, "Cloak", "validate()", __LINE__);
	}
	if (sourceImage->getBitsPerPixel() != 24) {
		throw new Exception(ERR_VALIDATION, "Image must be 24-bit", __FILE__, "Cloak", "validate()", __LINE__);
	}
	if (sourceImage->getImageDataLength() < ulHiddenFileSize) {
		throw new Exception(ERR_VALIDATION, "Image not large enough to store file", __FILE__, "Cloak", "validate()", __LINE__);
	}
	if (sourceImage->getImageType() == rgb_png) {
		if (((PNG *)sourceImage)->getFormat() != PNG_FMT_RGB) {
			throw new Exception(ERR_VALIDATION, "Image must be RGB format", __FILE__, "Cloak", "validate()", __LINE__);
		}
	}
}

const char *Cloak::getTargetFileExtension()
{
	return "*";
}
