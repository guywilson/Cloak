#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

extern "C" {
	#include "memdebug.h"
}
#include "secure_func.h"
#include "data.h"
#include "encryption.h"
#include "exception.h"
#include "errorcodes.h"
#include "key.h"

DataFile::DataFile()
{
}

DataFile::DataFile(char *pszFilename)
{
	strcpy_s(this->szFilename, FILENAME_BUFFER_LENGTH, pszFilename);
}

DataFile::DataFile(byte *pchData, dword ulLength)
{
	this->data = pchData;
	this->ulFileLength = ulLength;
}

DataFile::~DataFile()
{
	free_d(data, "DataFile.~DataFile():data");
}

char *DataFile::getFilename()
{
	return this->szFilename;
}

void DataFile::setFilename(char *pszFilename)
{
	strcpy_s(this->szFilename, FILENAME_BUFFER_LENGTH, pszFilename);
}

dword DataFile::_getFileLength(FILE *fptr)
{
    dword   ulCurrentPos;

    ulCurrentPos = ftell(fptr);

    if (!fseek(fptr, 0L, SEEK_END)) {
        ulFileLength = ftell(fptr);
        fseek(fptr, ulCurrentPos, SEEK_SET);
    }
	else {
		throw new Exception(ERR_FSEEK, "Failed to retrieve file length.", __FILE__, "DataFile", "_getFileLength()", __LINE__);
	}

    return ulFileLength;
}

dword DataFile::getFileLength()
{
	return this->ulFileLength;
}

void DataFile::getExtension(char *pszExtension)
{
	dword	i;
	dword	fileNameLength;
	char	ch;

	fileNameLength = (dword)strlen(szFilename);

	for (i = (fileNameLength - 1);i > 0;i--) {
		ch = szFilename[i];

		if (ch == '.') {
		#ifdef _WIN32
			strncpy_s(pszExtension, EXTENSION_BUFFER_LENGTH, &szFilename[i + 1], 4);
		#else
			strncpy(pszExtension, &szFilename[i + 1], 4);
		#endif
			break;
		}
	}
}

byte * DataFile::getData()
{
	return this->data;
}

void DataFile::read()
{
	FILE		*fptr;

	fptr = fopen(this->szFilename, "rb");

	if (fptr == NULL) {
		throw new Exception(ERR_OPEN_DATA_FILE, "Failed to open data file", __FILE__, "DataFile", "read()", __LINE__);
	}

	_getFileLength(fptr);

	data = (byte *)malloc_d(ulFileLength, "DataFile.read():data");

	if (data == NULL) {
		throw new Exception(
			ERR_MALLOC,
			"Failed to allocate memory for data file",
			__FILE__,
			"DataFile",
			"read()",
			__LINE__);
	}

	fread_d(data, 1, this->ulFileLength, fptr, "DataFile.read():data");

	fclose(fptr);
}

void DataFile::write()
{
	FILE		*fptr;

	fptr = fopen(this->szFilename, "wb");

	if (fptr == NULL) {
		throw new Exception(ERR_OPEN_DATA_FILE, "Failed to open data file", __FILE__, "DataFile", "write()", __LINE__);
	}

	fwrite_d(data, 1, this->ulFileLength, fptr, "DataFile.write():data");

	fclose(fptr);
}


EncryptedDataFile::EncryptedDataFile(char *pszFilename) : DataFile(pszFilename)
{
	ulEncryptedDataLength = 0L;
}

EncryptedDataFile::EncryptedDataFile(byte *pchData, dword ulLength) : DataFile(pchData, ulLength)
{
	this->szDefaultExtension[0] = '\0';
}

EncryptedDataFile::EncryptedDataFile() : DataFile()
{
}

EncryptedDataFile::~EncryptedDataFile()
{

}

dword EncryptedDataFile::getEncryptedDataLength()
{
	return this->ulEncryptedDataLength;
}

BitStreamIterator * EncryptedDataFile::iterator(word usBitsPerByte)
{
	BitStreamIterator *iterator;

	iterator = new BitStreamIterator(data, ulEncryptedDataLength, usBitsPerByte);

	return iterator;
}

char * EncryptedDataFile::getDefaultExtension()
{
	return this->szDefaultExtension;
}

void EncryptedDataFile::setDefaultExtension(char *pszExtension)
{
	strcpy_s(this->szDefaultExtension, EXTENSION_BUFFER_LENGTH, pszExtension);
}

void EncryptedDataFile::encrypt(char *pszPassword)
{
	byte 	key[16];
	byte	key2[16];

	/*
	** Do not encrypt if no password supplied...
	*/
	if (pszPassword != NULL || strlen(pszPassword) > 0) {
		EncryptionAlgorithm::generateKeyFromPassword(pszPassword, key);
		EncryptionAlgorithm::getSecondaryKey(key, key2);

		AES cipher(data, ulFileLength);
		ulEncryptedDataLength = cipher.encrypt(key, data);

		AES cipher2(data, ulEncryptedDataLength);
		cipher2.encrypt(key2, data);

		XOR cipher3(data, ulEncryptedDataLength);
		cipher3.encrypt(XORkey, STATIC_KEY_LENGTH, data);
	}
}

void EncryptedDataFile::encrypt(byte * pbKey, dword ulKeyLength)
{
	if (ulKeyLength > 0L) {
		XOR cipher(data, ulFileLength);
		ulEncryptedDataLength = cipher.encrypt(pbKey, ulKeyLength, data);
	}
}

void EncryptedDataFile::decrypt(char *pszPassword)
{
	byte 	key[16];
	byte	key2[16];

	/*
	** Do not encrypt if no password supplied...
	*/
	if (pszPassword != NULL || strlen(pszPassword) > 0) {
		EncryptionAlgorithm::generateKeyFromPassword(pszPassword, key);
		EncryptionAlgorithm::getSecondaryKey(key, key2);

		XOR cipher3(data, EncryptionAlgorithm::getEncryptedDataLength(ulFileLength));
		cipher3.decrypt(XORkey, STATIC_KEY_LENGTH, data);

		AES cipher2(data, EncryptionAlgorithm::getEncryptedDataLength(ulFileLength));
		cipher2.decrypt(key2, data);

		AES cipher(data, EncryptionAlgorithm::getEncryptedDataLength(ulFileLength));
		ulEncryptedDataLength = cipher.decrypt(key, data);
	}
}

void EncryptedDataFile::decrypt(byte * pbKey, dword ulKeyLength)
{
	if (ulKeyLength > 0L) {
		XOR cipher(data, ulFileLength);
		ulEncryptedDataLength = cipher.decrypt(pbKey, ulKeyLength, data);
	}
}

void EncryptedDataFile::read()
{
	FILE		*fptr;

	fptr = fopen(this->szFilename, "rb");

	if (fptr == NULL) {
		throw new Exception(ERR_OPEN_DATA_FILE, "Failed to open data file", __FILE__, "DataFile", "read()", __LINE__);
	}

	_getFileLength(fptr);

	data = (byte *)malloc_d(EncryptionAlgorithm::getEncryptedDataLength(this->ulFileLength), "EncryptedDataFile.read():data");

	if (data == NULL) {
		throw new Exception(
			ERR_MALLOC,
			"Failed to allocate memory for data file",
			__FILE__,
			"DataFile",
			"read()",
			__LINE__);
	}

	fread_d(data, 1, this->ulFileLength, fptr, "EncryptedDataFile.read():data");

	fclose(fptr);
}

void EncryptedDataFile::write()
{
	FILE		*fptr;

	fptr = fopen(this->szFilename, "wb");

	if (fptr == NULL) {
		throw new Exception(ERR_OPEN_DATA_FILE, "Failed to open data file", __FILE__, "DataFile", "write()", __LINE__);
	}

	fwrite_d(data, 1, this->ulFileLength, fptr, "EncryptedDataFile.write():data");

	fclose(fptr);
}
