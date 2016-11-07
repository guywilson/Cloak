#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "secure_func.h"
#include "image.h"
#include "exception.h"
#include "errorcodes.h"

extern "C" {
	#include "pngreadwrite.h"
	#include "memdebug.h"
}

Image::Image(char *pszFilename)
{
	strcpy_s(this->szFilename, FILENAME_BUFFER_LENGTH, pszFilename);
}

Image::Image(byte *pData)
{
	this->pchData = pData;
}

Image::Image(Image *sourceImage, bool deep)
{
	this->setImageDataLength(sourceImage->getImageDataLength());
	this->setBitsPerPixel(sourceImage->getBitsPerPixel());
	this->setWidth(sourceImage->getWidth());
	this->setHeight(sourceImage->getHeight());
	
	if (deep) {
		this->pchData = (byte *)malloc_d((size_t)sourceImage->getImageDataLength(), "Image.Image():pchData");
		
		if (this->pchData == NULL) {
			throw new Exception(ERR_MALLOC, "Failed to allocate memory for cloning image data", __FILE__, "Image", "Image()", __LINE__);
		}
		
		memcpy(this->pchData, sourceImage->getData(), sourceImage->getImageDataLength());
	}
}

Image::~Image()
{
	free_d(pchData, "Image.~Image():pchData");
}


char * Image::getFilename()
{
	return this->szFilename;
}

void Image::setFilename(char *pszFilename)
{
	if (pszFilename != NULL) {
		strcpy_s(this->szFilename, FILENAME_BUFFER_LENGTH, pszFilename);
	}
}

ImageType Image::getImageType()
{
	return this->type;
}

void Image::setImageType(ImageType type)
{
	this->type = type;
}

void Image::transform(ImageType sourceType, ImageType targetType)
{
	long		i;
	long		x;
	long		y;
	long		rowBytes;
	byte *		data;
	byte *		sourceRow;
	byte *		targetRow;
	byte **		rows;
	
	if (sourceType != targetType) {
		/*
		** Bitmap data is encoded in rows left->right with the rows
		** encoded from bottom to top. Each pixel is encoded with 
		** 3 channels (RGB) but in BGR order.
		**
		** PNG data is encoded in rows left->right with the rows
		** encoded from top to bottom. Each pixel is encoded with 
		** 3 channels (RGB), in RGB order.
		*/
		rowBytes = getWidth() * 3;
		
		i = 0L;

		data = pchData;
		
		rows = (byte **)malloc_d(getHeight() * sizeof(byte *), "Image.transform():rows");
		
		for (y = getHeight()-1;y >= 0L;--y) {
			sourceRow = (byte *)malloc_d(rowBytes, "Image.transform():sourceRow");
			
			rows[y] = sourceRow;
			
			for (x = 0L;x < rowBytes;x += 3) {
				sourceRow[x+2] = *data++; //Blue/Red
				sourceRow[x+1] = *data++; //Green
				sourceRow[x]   = *data++; //Red/Blue

				i += 3;
			}
		}
		
		if (pchData) {
			free_d(pchData, "Image.transform():pchData");
		}

		pchData = (byte *)malloc_d(getImageDataLength(), "Image.transform():pchData");

		i = 0L;

		for (y = 0L;y < getHeight();y++) {
			targetRow = rows[y];

			for (x = 0L;x < rowBytes;x++) {
				pchData[i++] = *targetRow++;
			}

			free_d(rows[y], "Image.transform():rows[]");
		}
		
		free_d(rows, "Image.transform():rows");
	}
}

byte * Image::getData()
{
	return this->pchData;
}

void Image::setData(byte *pData)
{
	if (pData != NULL) {
		this->pchData = pData;
	}
}

word Image::getBitsPerPixel()
{
	return this->usBitsPerPixel;
}

long Image::getWidth()
{
	return this->lWidth;
}

long Image::getHeight()
{
	return this->lHeight;
}

void Image::setBitsPerPixel(word usValue)
{
	this->usBitsPerPixel = usValue;
}

void Image::setWidth(long lValue)
{
	this->lWidth = lValue;
}

void Image::setHeight(long lValue)
{
	this->lHeight = lValue;
}
 
dword Image::getImageDataLength()
{
	return this->ulDataLength;
}

void Image::setImageDataLength(dword ulValue)
{
	this->ulDataLength = ulValue;
}

ByteStreamIterator *Image::iterator()
{
	ByteStreamIterator		*iterator;
	
	iterator = new ByteStreamIterator(getData(), getImageDataLength());
	
	return iterator;
}

dword Image::getCapacity(word bitsPerByte)
{
	return ((getImageDataLength() / 8) * bitsPerByte) - 32;
}


PNG::PNG(char *pszFilename) : Image(pszFilename)
{
	initialise();
}

PNG::PNG(byte *pData) : Image(pchData)
{
	initialise();
}

PNG::PNG(Image *sourceImage) : Image(sourceImage, true)
{
	initialise();
	
	if (sourceImage->getImageType() == rgb_bitmap) {
		transform(rgb_bitmap, rgb_png);
	}
}

PNG::PNG() : Image()
{
	initialise();
}

PNG::~PNG()
{
}
 
 
void PNG::initialise()
{
	this->setImageType(rgb_png);
	this->compressionLevel = 9;
}

int PNG::getCompressionLevel()
{
	return this->compressionLevel;
}

void PNG::setCompressionLevel(int level)
{
	this->compressionLevel = level;
}

int	PNG::getFormat()
{
	return this->format;
}

void PNG::setFormat(int fmt)
{
	this->format = fmt;
}

int PNG::mapFormat(int colourType)
{
	return colourType;
}

void PNG::read()
{
	char *		filename;
	PNGDETAILS	details;

	filename = getFilename();

	setData(readPngImage(filename, &details));

	setBitsPerPixel(details.usBitsPerPixel);
	setWidth(details.lWidth);
	setHeight(details.lHeight);
	setFormat(mapFormat(details.colourType));
	
	setImageDataLength(getWidth() * getHeight() * (getBitsPerPixel() / 8));
}
 
void PNG::write()
{
	PNGDETAILS		details;
	
	details.compressionLevel = getCompressionLevel();
	details.lWidth = getWidth();
	details.lHeight = getHeight();
	details.usBitsPerPixel = getBitsPerPixel();
	
	writePngImage(getFilename(), getData(), &details);
}


Bitmap::Bitmap(char *pszFilename) : Image(pszFilename)
{
	initialise();
}

Bitmap::Bitmap(byte *pData) : Image(pchData)
{
	initialise();
}

Bitmap::Bitmap(Image *sourceImage) : Image(sourceImage, true)
{
	initialise();
	
	if (sourceImage->getImageType() == rgb_bitmap) {
		this->setMagicNumber(((Bitmap *)sourceImage)->getMagicNumber());
		this->setFileSize(((Bitmap *)sourceImage)->getFileSize());
		this->setReserved(((Bitmap *)sourceImage)->getReserved());
		this->setStartOffset(((Bitmap *)sourceImage)->getStartOffset());
		this->setHeaderSize(((Bitmap *)sourceImage)->getHeaderSize());
		this->setColourPlanes(((Bitmap *)sourceImage)->getColourPlanes());
		this->setCompressionMethod(((Bitmap *)sourceImage)->getCompressionMethod());
		this->setHorizontalResolution(((Bitmap *)sourceImage)->getHorizontalResolution());
		this->setVerticalResolution(((Bitmap *)sourceImage)->getVerticalResolution());
		this->setNumColours(((Bitmap *)sourceImage)->getNumColours());
		this->setNumImportantColours(((Bitmap *)sourceImage)->getNumImportantColours());
		this->setType();
	}
	else {
		char bm[2] = {'B', 'M'};
		byte reserved[4] = {0, 0, 0, 0};

		this->setMagicNumber(bm);
		this->setHeaderSize(WINV3_HEADER_SIZE);
		this->setFileSize(sourceImage->getImageDataLength() + WINV3_HEADER_SIZE + BMP_HEADER_SIZE);
		this->setReserved(reserved);
		this->setStartOffset(WINV3_HEADER_SIZE + BMP_HEADER_SIZE);
		this->setColourPlanes(0);
		this->setCompressionMethod(COMPRESSION_BI_RGB);
		this->setHorizontalResolution(300L);
		this->setVerticalResolution(300L);
		this->setNumColours(0);
		this->setNumImportantColours(0);
		this->setType();
		
		transform(rgb_png, rgb_bitmap);
	}
}

Bitmap::Bitmap() : Image()
{
	initialise();
}

Bitmap::~Bitmap()
{
	//
}

 
void Bitmap::initialise()
{
	this->setImageType(rgb_bitmap);
}

char *Bitmap::getMagicNumber()
{
	return this->szMagicNumber;
}

void Bitmap::setMagicNumber(char *pszValue)
{
	if (pszValue != NULL) {
		this->szMagicNumber[0] = pszValue[0];
		this->szMagicNumber[1] = pszValue[1];
	}
}


dword Bitmap::getFileSize()
{
	return ulFileSize;
}

void Bitmap::setFileSize(dword ulValue)
{
	this->ulFileSize = ulValue;
}


byte *Bitmap::getReserved()
{
	return this->uchReserved;
}

void Bitmap::setReserved(byte *pchValue)
{
	if (pchValue != NULL) {
		memcpy_d(this->uchReserved, pchValue, 4, "Bitmap.setReserved():pchValue->uchReserved");
	}
}


dword Bitmap::getStartOffset()
{
	return this->ulStartOffset;
}

void Bitmap::setStartOffset(dword ulValue)
{
	this->ulStartOffset = ulValue;
}


dword Bitmap::getHeaderSize()
{
	return ulHeaderSize;
}

void Bitmap::setHeaderSize(dword ulValue)
{
	this->ulHeaderSize = ulValue;
}


word Bitmap::getColourPlanes()
{
	return this->usColourPlanes;
}

void Bitmap::setColourPlanes(word usValue)
{
	this->usColourPlanes = usValue;
}


dword Bitmap::getCompressionMethod()
{
	return this->ulCompressionMethod;
}

void Bitmap::setCompressionMethod(dword ulValue)
{
	this->ulCompressionMethod = ulValue;
}


long Bitmap::getHorizontalResolution()
{
	return this->lHRes;
}

void Bitmap::setHorizontalResolution(long lValue)
{
	this->lHRes = lValue;
}


long Bitmap::getVerticalResolution()
{
	return this->lVRes;
}

void Bitmap::setVerticalResolution(long lValue)
{
	this->lVRes = lValue;
}


dword Bitmap::getNumColours()
{
	return this->ulNumColours;
}

void Bitmap::setNumColours(dword ulValue)
{
	this->ulNumColours = ulValue;
}


dword Bitmap::getNumImportantColours()
{
	return this->ulNumImportantColours;
}

void Bitmap::setNumImportantColours(dword ulValue)
{
	this->ulNumImportantColours = ulValue;
}


BitmapType Bitmap::getType()
{
	return this->type;
}

void Bitmap::setType()
{
	if (ulHeaderSize == WINV3_HEADER_SIZE) {
		type = WindowsV3;
	}
	else if (ulHeaderSize == OS2V1_HEADER_SIZE) {
		type = OS2V1;
	}
	else {
		type = UnknownType;
	}
}

void Bitmap::read()
{
	FILE *		fptr;
	byte *		data;
	char		szHeaderBuffer[BMP_HEADER_SIZE];
	char		szDIBHeaderBuffer[WINV3_HEADER_SIZE];
	
	fptr = fopen(getFilename(), "rb");
	
	if (fptr == NULL) {
		throw new Exception(ERR_OPEN_BITMAP, "Failed to open bitmap", __FILE__, "Bitmap", "read()", __LINE__);
	}
	
	/*
	** Read bitmap header...
	*/
	fread_d(szHeaderBuffer, 1, BMP_HEADER_SIZE, fptr, "Bitmap.read():szHeaderBuffer");

	memcpy(szMagicNumber, &szHeaderBuffer[0], 2);
	memcpy(&ulFileSize, &szHeaderBuffer[2], 4);
	memcpy(uchReserved, &szHeaderBuffer[6], 4);
	memcpy(&ulStartOffset, &szHeaderBuffer[10], 4);
	
	fread_d(szHeaderBuffer, 1, 4, fptr, "Bitmap.read():szHeaderBuffer");
	memcpy(&ulHeaderSize, &szHeaderBuffer[0], 4);
	
	fread_d(szDIBHeaderBuffer, 1, ulHeaderSize - 4, fptr, "Bitmap.read():szDIBHeaderBuffer");
	
	if (ulHeaderSize == WINV3_HEADER_SIZE) {
		type = WindowsV3;
		
        memcpy(&lWidth, &szDIBHeaderBuffer[0], 4);
        memcpy(&lHeight, &szDIBHeaderBuffer[4], 4);
        memcpy(&usColourPlanes, &szDIBHeaderBuffer[8], 2);
        memcpy(&usBitsPerPixel, &szDIBHeaderBuffer[10], 2);
        memcpy(&ulCompressionMethod, &szDIBHeaderBuffer[12], 4);
        memcpy(&ulDataLength, &szDIBHeaderBuffer[16], 4);
        memcpy(&lHRes, &szDIBHeaderBuffer[20], 4);
        memcpy(&lVRes, &szDIBHeaderBuffer[24], 4);
        memcpy(&ulNumColours, &szDIBHeaderBuffer[28], 4);
        memcpy(&ulNumImportantColours, &szDIBHeaderBuffer[32], 4);
	}
	else if (ulHeaderSize == OS2V1_HEADER_SIZE) {
		type = OS2V1;

        memcpy(&lWidth, &szDIBHeaderBuffer[0], 2);
        memcpy(&lHeight, &szDIBHeaderBuffer[2], 2);
        memcpy(&usColourPlanes, &szDIBHeaderBuffer[4], 2);
        memcpy(&usBitsPerPixel, &szDIBHeaderBuffer[6], 2);
		
		setImageDataLength(ulFileSize - ulStartOffset);
	}
	else {
		type = UnknownType;

		setImageDataLength(ulFileSize - ulStartOffset);
	}

	/*
	** Seek to beginning of bitmap data...
	*/
    if (fseek(fptr, ulStartOffset, SEEK_SET)) {
		throw new Exception(ERR_FSEEK, "Failed to seek to start of bitmap data", __FILE__, "Bitmap", "read()", __LINE__);
	}

	pchData = (byte *)malloc_d((size_t)getImageDataLength(), "Bitmap.read():pchData");
	
	if (pchData == NULL) {
		fclose(fptr);
		throw new Exception(ERR_MALLOC, "Failed to allocate memory for bitmap data", __FILE__, "Bitmap", "read()", __LINE__);
	}
	
	data = pchData;
	
	fread_d(data, 1, getImageDataLength(), fptr, "Bitmap.read():data");
	
	fclose(fptr);
}

void Bitmap::write()
{
	FILE		*fptr;
	char		szHeaderBuffer[BMP_HEADER_SIZE];
	char		szDIBHeaderBuffer[WINV3_HEADER_SIZE];
	
	fptr = fopen(getFilename(), "wb");
	
	if (fptr == NULL) {
		throw new Exception(ERR_OPEN_BITMAP, "Failed to open bitmap", __FILE__, "Bitmap", "read()", __LINE__);
	}
	
	memcpy(&szHeaderBuffer[0], szMagicNumber, 2);
	memcpy(&szHeaderBuffer[2], &ulFileSize, 4);
	memcpy(&szHeaderBuffer[6], uchReserved, 4);
	memcpy(&szHeaderBuffer[10], &ulStartOffset, 4);
	
	fwrite_d(szHeaderBuffer, 1, BMP_HEADER_SIZE, fptr, "Bitmap.write():szHeaderBuffer");
	
	if (type == WindowsV3) {
		memcpy(&szDIBHeaderBuffer[0], &ulHeaderSize, 4);
        memcpy(&szDIBHeaderBuffer[4], &lWidth, 4);
        memcpy(&szDIBHeaderBuffer[8], &lHeight, 4);
        memcpy(&szDIBHeaderBuffer[12], &usColourPlanes, 2);
        memcpy(&szDIBHeaderBuffer[14], &usBitsPerPixel, 2);
        memcpy(&szDIBHeaderBuffer[16], &ulCompressionMethod, 4);
        memcpy(&szDIBHeaderBuffer[20], &ulDataLength, 4);
        memcpy(&szDIBHeaderBuffer[24], &lHRes, 4);
        memcpy(&szDIBHeaderBuffer[28], &lVRes, 4);
        memcpy(&szDIBHeaderBuffer[32], &ulNumColours, 4);
        memcpy(&szDIBHeaderBuffer[36], &ulNumImportantColours, 4);
		
		fwrite(szDIBHeaderBuffer, 1, WINV3_HEADER_SIZE, fptr);
	}
	else if (type == OS2V1) {
		memcpy(&szDIBHeaderBuffer[0], &ulHeaderSize, 4);
        memcpy(&szDIBHeaderBuffer[4], &lWidth, 2);
        memcpy(&szDIBHeaderBuffer[6], &lHeight, 2);
        memcpy(&szDIBHeaderBuffer[8], &usColourPlanes, 2);
        memcpy(&szDIBHeaderBuffer[10], &usBitsPerPixel, 2);
		
		fwrite(szDIBHeaderBuffer, 1, OS2V1_HEADER_SIZE, fptr);
	}
	
	fwrite_d(pchData, 1, getImageDataLength(), fptr, "Bitmap.write():pchData");
	
	fclose(fptr);
}

