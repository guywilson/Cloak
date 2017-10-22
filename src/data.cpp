#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <iostream>

#include "secure_func.h"
#include "data.h"
#include "encryption.h"
#include "exception.h"
#include "errorcodes.h"
#include "key.h"

using namespace std;

/*******************************************************************************
*
* How this works:
*
* Scenario 1 - Merge
* 		1) Read in a data file of length l
*		2) Compress the data to yield length c (where typically c < l)
*		3) Encrypt the data to yield length e (where typically e >= c)
*		4) Encode the data (e bytes) into a PNG or BMP image
*
* Scenario 2 - Extract
*		1) Decode the data (e bytes) from a PNG or BMP image
*		2) Decrypt the data to yield length c (where typically c <= e)
*		3) Decompress the data to yield length l (where typically l > c)
*		4) Write a data file of length l
*
* We need to know:
*		1) Length l in orer to be able to malloc() enough space for the output
*		   file
*		2) Length e so we know how many bytes to decode from the image. This can
*		   be derived from c as we know the encryption block size.
*		3) Length c so that we know how many bytes to malloc from decryption
*
*******************************************************************************/
Data::Data()
{
	this->_data = NULL;
	this->_initialise();
}

Data::Data(dword size) : Data()
{
	/*
	** Allocate enough for the data + the header...
	*/
	this->_data = (byte *)malloc(size + DATA_HEADER_SIZE);

	if (this->_data == NULL) {
		throw new Exception(
					ERR_MALLOC,
					"Failed to allocate memory for data",
					__FILE__,
					"Data",
					"init()",
					__LINE__);
	}

	this->_length = size;
	this->setOriginalLength(size);
	this->setCompressedLength(size);
}

/*
** Copy constructor...
*/
Data::Data(Data & src) : Data(src.getLength())
{
	memcpy(this->_data, src.getData(), src.getLength());

	this->setIsCompressed(src.isCompressed());
	this->setIsEncrypted(src.isEncrypted());

	this->setOriginalLength(src.getOriginalLength());
	this->setCompressedLength(src.getCompressedLength());
}

Data::Data(byte * header, byte * data, dword length) : Data(length)
{
	if (!this->_isInitialised) {
		throw new Exception(
					ERR_ILLEGAL_CALL,
					"Illegal call",
					__FILE__,
					"Data",
					"init()",
					__LINE__);
	}

	if (header != NULL && data != NULL) {
		memcpy(&this->_data[0], header, DATA_HEADER_SIZE);
		memcpy(&this->_data[DATA_HEADER_SIZE], data, length);
	}
}

Data::~Data()
{
	this->_initialise();

	if (this->_data != NULL) {
		free(this->_data);
		this->_data = NULL;
	}
}

/*
** Wipe memory for security
*/
void Data::_initialise()
{
	if (this->_data != NULL) {
		memset(this->_data, 0x00, this->getLength() + DATA_HEADER_SIZE);
		memset(this->_data, 0xFF, this->getLength() + DATA_HEADER_SIZE);
		memset(this->_data, 0x00, this->getLength() + DATA_HEADER_SIZE);
	}

	this->_length		= 0L;
	this->_blockSize	= 0L;
	this->_blockPtr		= 0L;

	this->_isInitialised = true;
}

byte * Data::getData()
{
	if (!this->_isInitialised) {
		throw new Exception(
					ERR_ILLEGAL_CALL,
					"Illegal call",
					__FILE__,
					"Data",
					"getData()",
					__LINE__);
	}

	return &this->_data[DATA_HEADER_SIZE];
}

void Data::setOriginalLength(dword len)
{
	dword		length;

	length = len & 0x7FFFFFFF;
	length += ((this->_data[0] & 0x80) << 24);

	memcpy(&this->_data[0], &length, 4);
}

void Data::setCompressedLength(dword len)
{
	dword		length;

	length = len & 0x7FFFFFFF;

	length += ((this->_data[4] & 0x80) << 24);

	memcpy(&this->_data[4], &length, 4);
}

dword Data::getOriginalLength()
{
	return Data::getOriginalLength(this->_data);
}

dword Data::getOriginalLength(byte * header)
{
	dword		len = 0L;

	memcpy(&len, &header[0], 4);

	len &= 0x7FFFFFFF;

	return len;
}

dword Data::getCompressedLength()
{
	return Data::getCompressedLength(this->_data);
}

dword Data::getCompressedLength(byte * header)
{
	dword		len = 0L;

	memcpy(&len, &header[4], 4);

	len &= 0x7FFFFFFF;

	return len;
}

dword Data::getEncryptedLength()
{
	return Data::getEncryptedLength(this->_data);
}

dword Data::getEncryptedLength(byte * header)
{
	if (isEncrypted(header)) {
		return EncryptionAlgorithm::getEncryptedDataLength(getCompressedLength(header));
	}
	else {
		return getCompressedLength(header);
	}
}

void Data::setIsCompressed(bool yn)
{
	if (yn) {
		this->_data[7] |= 0x80;
	}
	else {
		this->_data[7] &= 0x7F;
	}
}

void Data::setIsEncrypted(bool yn)
{
	if (yn) {
		this->_data[3] |= 0x80;
	}
	else {
		this->_data[3] &= 0x7F;
	}
}

bool Data::isCompressed()
{
	return Data::isCompressed(this->_data);
}

bool Data::isCompressed(byte * header)
{
	return ((header[7] & 0x80) >> 7) == 1 ? true : false;
}

bool Data::isEncrypted()
{
	return Data::isEncrypted(this->_data);
}

bool Data::isEncrypted(byte * header)
{
	return ((header[3] & 0x80) >> 7) == 1 ? true : false;
}

dword Data::getLength()
{
	return this->_length;
}

dword Data::getTotalLength()
{
	return this->_length + DATA_HEADER_SIZE;
}

void Data::initiateBlockIterator(dword blockSize)
{
	this->_blockSize = blockSize;
	this->_blockPtr = 0L;
}

bool Data::hasNextBlock()
{
	return (this->_blockPtr < this->_length);
}

dword Data::nextBlock(byte * block)
{
	dword		blockLength = 0L;

	if (this->_blockPtr < this->_length) {
		blockLength = this->_length - this->_blockPtr;

		if (blockLength >= this->_blockSize) {
			blockLength = this->_blockSize;
		}

		memcpy(block, &this->_data[this->_blockPtr + DATA_HEADER_SIZE], blockLength);

		this->_blockPtr += blockLength;
	}

	return blockLength;
}


BitStreamIterator * Data::iterator(word usBitsPerByte)
{
	BitStreamIterator * iterator = new BitStreamIterator(this->_data, this->_length + DATA_HEADER_SIZE, usBitsPerByte);

	return iterator;
}

void Data::encrypt(char * pszPassword)
{
	byte 	key[16];
	byte	key2[16];
	byte *	data;

	/*
	** Do not encrypt if no password supplied...
	*/
	if (pszPassword != NULL && strlen(pszPassword) > 0) {
		EncryptionAlgorithm::generateKeyFromPassword(pszPassword, key);
		EncryptionAlgorithm::getSecondaryKey(key, key2);

		data = (byte *)malloc(EncryptionAlgorithm::getEncryptedDataLength(this->_length) + DATA_HEADER_SIZE);

		memcpy(data, this->_data, DATA_HEADER_SIZE);

		AES cipher(&this->_data[DATA_HEADER_SIZE], this->_length);
		cipher.encrypt(key, &data[DATA_HEADER_SIZE]);

		AES cipher2(&data[DATA_HEADER_SIZE], EncryptionAlgorithm::getEncryptedDataLength(this->_length));
		cipher2.encrypt(key2, &data[DATA_HEADER_SIZE]);

		XOR cipher3(&data[DATA_HEADER_SIZE], EncryptionAlgorithm::getEncryptedDataLength(this->_length));
		cipher3.encrypt(XORkey, STATIC_KEY_LENGTH, &data[DATA_HEADER_SIZE]);

		free(this->_data);

		this->_data = data;
		this->_length = EncryptionAlgorithm::getEncryptedDataLength(this->_length);

		this->setIsEncrypted(true);
	}
}

void Data::encrypt(byte * pbKey, dword ulKeyLength)
{
	if (ulKeyLength > 0L) {
		XOR cipher(&this->_data[DATA_HEADER_SIZE], this->_length);
		cipher.encrypt(pbKey, ulKeyLength, &this->_data[DATA_HEADER_SIZE]);
	}
}

void Data::decrypt(char * pszPassword)
{
	byte 	key[16];
	byte	key2[16];

	/*
	** Do not encrypt if no password supplied...
	*/
	if (pszPassword != NULL && strlen(pszPassword) > 0 && this->isEncrypted()) {
		EncryptionAlgorithm::generateKeyFromPassword(pszPassword, key);
		EncryptionAlgorithm::getSecondaryKey(key, key2);

		XOR cipher3(&this->_data[DATA_HEADER_SIZE], this->_length);
		cipher3.decrypt(XORkey, STATIC_KEY_LENGTH, &this->_data[DATA_HEADER_SIZE]);

		AES cipher2(&this->_data[DATA_HEADER_SIZE], this->_length);
		cipher2.decrypt(key2, &this->_data[DATA_HEADER_SIZE]);

		AES cipher(&this->_data[DATA_HEADER_SIZE], this->_length);
		cipher.decrypt(key, &this->_data[DATA_HEADER_SIZE]);

		this->setIsEncrypted(false);
		this->_length = this->getCompressedLength();
	}
}

void Data::decrypt(byte * pbKey, dword ulKeyLength)
{
	if (ulKeyLength > 0L) {
		XOR cipher(&this->_data[DATA_HEADER_SIZE], this->_length);
		cipher.decrypt(pbKey, ulKeyLength, &this->_data[DATA_HEADER_SIZE]);
	}
}

dword Data::compress(int level)
{
	dword			compressedSize = 0L;
	unsigned long	ulSize = 0L;
	byte *			compressed;
	int				rtn = 0;

	ulSize = compressBound(this->_length);

	compressed = (byte *)malloc(ulSize + DATA_HEADER_SIZE);

	if (compressed == NULL) {
		throw new Exception(
			ERR_MALLOC,
			"Failed to allocate memory for compressed data",
			__FILE__,
			"Data",
			"compress()",
			__LINE__);
	}

	memcpy(compressed, this->_data, DATA_HEADER_SIZE);

	rtn = compress2(
				&compressed[DATA_HEADER_SIZE],
				&ulSize,
				&this->_data[DATA_HEADER_SIZE],
				this->_length,
				level);

	if (rtn != Z_OK) {
		throw new Exception(
			ERR_MALLOC,
			"Failed to compress data",
			__FILE__,
			"Data",
			"compress()",
			__LINE__);
	}

	free(this->_data);

	this->_data = compressed;

	compressedSize = (dword)ulSize;

	this->setCompressedLength(compressedSize);
	this->setIsCompressed(true);

	this->_length = compressedSize;

	return compressedSize;
}

void Data::decompress()
{
	unsigned long	uncompressedSize = 0L;
	byte *			uncompressed;
	int				rtn = 0;

	uncompressedSize = this->getOriginalLength();

	uncompressed = (byte *)malloc(uncompressedSize + DATA_HEADER_SIZE);

	if (uncompressed == NULL) {
		throw new Exception(
			ERR_MALLOC,
			"Failed to allocate memory for compressed data",
			__FILE__,
			"Data",
			"decompress()",
			__LINE__);
	}

	memcpy(uncompressed, this->_data, DATA_HEADER_SIZE);

	rtn = uncompress(
				&uncompressed[DATA_HEADER_SIZE],
				&uncompressedSize,
				&this->_data[DATA_HEADER_SIZE],
				this->_length);

	if (rtn != Z_OK) {
		throw new Exception(
			ERR_MALLOC,
			"Failed to compress data",
			__FILE__,
			"Data",
			"decompress()",
			__LINE__);
	}

	free(this->_data);

	this->_data = uncompressed;

	this->_length = uncompressedSize;

	this->setIsCompressed(false);
}


DataFile::DataFile(char * pszFileName, int mode)
{
	char		szMode[4];
	int			i = 0;

	this->mode = mode;

	if (mode & FILE_OPEN_READ) {
		szMode[i++] = 'r';
	}
	else if (mode & FILE_OPEN_WRITE) {
		szMode[i++] = 'w';
	}
	else if (mode & FILE_OPEN_APPEND) {
		szMode[i++] = 'a';
	}

	if (mode & FILE_MODE_BINARY) {
		szMode[i++] = 'b';
	}
	else if (mode & FILE_MODE_TEXT) {
		szMode[i++] = 't';
	}

	if (mode & FILE_OPEN_UPDATE) {
		szMode[i++] = '+';
	}

	szMode[i] = '\0';

	this->fptr = fopen(pszFileName, szMode);

	if (this->fptr == NULL) {
		throw new Exception(
					ERR_OPEN_DATA_FILE,
					"Failed to open data file",
					__FILE__,
					"DataFile",
					"init()",
					__LINE__);
	}
}

DataFile::~DataFile()
{
	fclose(this->fptr);
}

dword DataFile::getLength()
{
    dword   ulCurrentPos;
    dword	ulFileLength;

    ulCurrentPos = ftell(this->fptr);

    if (!fseek(this->fptr, 0L, SEEK_END)) {
        ulFileLength = ftell(this->fptr);
        fseek(this->fptr, ulCurrentPos, SEEK_SET);
    }
	else {
		throw new Exception(
					ERR_FSEEK,
					"Failed to retrieve file length.",
					__FILE__,
					"DataFile",
					"getLength()",
					__LINE__);
	}

    return ulFileLength;
}

bool DataFile::isRead()
{
	return (this->mode & FILE_OPEN_READ) != 0 ? true : false;
}

bool DataFile::isWrite()
{
	return (this->mode & FILE_OPEN_WRITE) != 0 ? true : false;
}

bool DataFile::isAppend()
{
	return (this->mode & FILE_OPEN_APPEND) != 0 ? true : false;
}

bool DataFile::isUpdate()
{
	return (this->mode & FILE_OPEN_UPDATE) != 0 ? true : false;
}

bool DataFile::isBinary()
{
	return (this->mode & FILE_MODE_BINARY) != 0 ? true : false;
}

bool DataFile::isText()
{
	return (this->mode & FILE_MODE_BINARY) != 0 ? true : false;
}

Data * DataFile::read(dword numBytes)
{
	Data * data = new Data(numBytes);

	fread(data->getData(), 1, numBytes, this->fptr);

	return data;
}

void DataFile::write(Data * data, dword numBytes)
{
	fwrite(data->getData(), 1, numBytes, this->fptr);
}
