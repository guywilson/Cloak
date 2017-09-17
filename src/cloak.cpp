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
	sourceData = NULL;
	targetData = NULL;

	bitsPerByte = (word)0;
	compressionLevel = DEFAULT_COMPRESSION_LEVEL;
}

Cloak::Cloak(int compressionLevel) : Cloak()
{
	setCompressionLevel(compressionLevel);
}

Cloak::~Cloak()
{
	if (sourceImage != NULL) {
		delete sourceImage;
	}
	if (targetImage != NULL) {
		delete targetImage;
	}
	if (sourceData != NULL) {
		delete sourceData;
	}
	if (targetData != NULL) {
		delete targetData;
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
	if (sourceData != NULL) {
		delete sourceData;
	}

	DataFile sourceFile(pszFilename, FILE_OPEN_READ | FILE_MODE_BINARY);

	sourceData = sourceFile.read(sourceFile.getLength());
}

void Cloak::setCompressionLevel(int compressionLevel)
{
	this->compressionLevel = compressionLevel;
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
	return sourceData->getLength();
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

void Cloak::_printStats()
{
	cout << endl << "Stats:" << endl;
	cout << "\tFile size: " << sourceData->getOriginalLength() << endl;

	if (sourceData->isCompressed()) {
		cout << "\tCompressed: yes" << endl;
		cout << "\tCompression level: " << this->compressionLevel << endl;
		cout <<
			"\tCompressed size: " <<
			sourceData->getCompressedLength() <<
			" (" <<
			((double)sourceData->getCompressedLength() / (double)sourceData->getOriginalLength()) * 100.0 <<
			"%)" <<
			endl;
	}
	else {
		cout << "\tCompressed: no" << endl;
	}

	if (sourceData->isEncrypted()) {
		cout << "\tEncrypted: yes" << endl;
	}
	else {
		cout << "\tEncrypted: no" << endl;
	}

	cout << endl;
}

void Cloak::_merge(byte * pTargetBytes)
{
	byte		dataBits;
	byte		bitmapByte;
	byte		targetByte;
	byte		bitMask;
	int			i = 0;

	ByteStreamIterator *bitmapIterator = targetImage->iterator();
	BitStreamIterator *dataIterator = sourceData->iterator(bitsPerByte);

	bitMask = dataIterator->getBitMask();

	while (dataIterator->hasNext()) {
		bitmapByte = bitmapIterator->nextByte();

		dataBits = dataIterator->nextBits();

		/*
		** Merge data bits and bitmap data...
		** First, clear the first n bits and then bitwise OR with data...
		*/
		targetByte = (bitmapByte & ~bitMask) | dataBits;

		pTargetBytes[i++] = targetByte;
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
	byte				chHeaderBuffer[DATA_HEADER_SIZE];
	dword				ulSecretBytes = 0L;
	bool				readHeader = true;
    int             	bitCounter = 0;
    dword             	pos = 0L;

	bitMask = BitStreamIterator::getBitMask(bitsPerByte);

	bitmapIterator = sourceImage->iterator();

	while (bitmapIterator->hasNext()) {
		bitmapByte = bitmapIterator->nextByte();

		chSecretBits = bitmapByte & bitMask;
		chSecretByte += chSecretBits << bitCounter;

		bitCounter += bitsPerByte;

        if (bitCounter == 8) {
			if (readHeader) {
				chHeaderBuffer[pos++] = chSecretByte;

				if (pos == DATA_HEADER_SIZE) {
					ulSecretBytes = Data::getEncryptedLength(chHeaderBuffer);

					if (ulSecretBytes > getImageCapacity()) {
						throw new Exception(
									ERR_NO_HIDDEN_FILE,
									"Bitmap does not seem to contain a valid hidden file",
									__FILE__,
									"Cloak",
									"extract()",
									__LINE__);
					}

					pchSecretBuffer = (byte *)malloc(ulSecretBytes);

					if (pchSecretBuffer == NULL) {
						throw new Exception(
									ERR_MALLOC,
									"Failed to allocate memory for secret file",
									__FILE__,
									"Cloak",
									"extract()",
									__LINE__);
					}

					readHeader = false;
					pos = 0L;
				}
			}
			else {
				pchSecretBuffer[pos++] = chSecretByte;

				if (pos >= ulSecretBytes) {
					break;
				}
			}

            bitCounter = 0;
            chSecretByte = 0x00;
        }
	}

	delete bitmapIterator;

	targetData = new Data(chHeaderBuffer, pchSecretBuffer, ulSecretBytes);

	free(pchSecretBuffer);
}

void Cloak::merge(char *pszFilename, char *pszPassword)
{
	byte	*	pTargetBytes;

	sourceData->compress(this->compressionLevel);
	sourceData->encrypt(pszPassword);

	validate();

	_printStats();

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

	sourceData->compress(this->compressionLevel);
	sourceData->encrypt(pbKeystream, ulKeyLength);

	validate();

	_printStats();

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

	targetData->decrypt(pszPassword);
	targetData->decompress();

	DataFile outputFile(pszFilename, FILE_OPEN_WRITE | FILE_MODE_BINARY);

	outputFile.write(targetData, targetData->getLength());
}

void Cloak::extract(char *pszFilename, byte *pbKeystream, dword ulKeyLength)
{
	_extract();

	targetData->decrypt(pbKeystream, ulKeyLength);
	targetData->decompress();

	DataFile outputFile(pszFilename, FILE_OPEN_WRITE | FILE_MODE_BINARY);

	outputFile.write(targetData, targetData->getLength());
}

void Cloak::validate()
{
	dword		ulHiddenFileSize;

	ulHiddenFileSize = (sourceData->getTotalLength() * 8L) / (dword)bitsPerByte;

	if (!(bitsPerByte == 1 || bitsPerByte == 2 || bitsPerByte == 4)) {
		throw new Exception(
					ERR_VALIDATION,
					"Invalid bits per byte, must be 1, 2 or 4",
					__FILE__,
					"Cloak",
					"validate()",
					__LINE__);
	}
	if (sourceImage->getBitsPerPixel() != 24) {
		throw new Exception(
					ERR_VALIDATION,
					"Image must be 24-bit",
					__FILE__,
					"Cloak",
					"validate()",
					__LINE__);
	}
	if (sourceImage->getImageDataLength() < ulHiddenFileSize) {
		throw new Exception(
					ERR_VALIDATION,
					"Image not large enough to store file",
					__FILE__,
					"Cloak",
					"validate()",
					__LINE__);
	}
	if (sourceImage->getImageType() == rgb_png) {
		if (((PNG *)sourceImage)->getFormat() != PNG_FMT_RGB) {
			throw new Exception(
					ERR_VALIDATION,
					"Image must be RGB format",
					__FILE__,
					"Cloak",
					"validate()",
					__LINE__);
		}
	}
}

const char *Cloak::getTargetFileExtension()
{
	return "*";
}
