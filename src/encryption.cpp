#include <stdio.h>
#include <string.h>
#include <iostream>

#include "encryption.h"
#include "errorcodes.h"
#include "exception.h"
#include "key.h"

using namespace std;

EncryptionAlgorithm::EncryptionAlgorithm(PBYTE pInputData, dword bufferLength)
{
	this->ulDataLength = bufferLength;
	this->data = pInputData;
	this->ulDataPtr = 0L;

	this->blockCount = 0L;

	this->numBlocks = bufferLength / BLOCK_SIZE;

	this->key = NULL;
	this->keyLength = 0;
}

EncryptionAlgorithm::~EncryptionAlgorithm()
{
	this->data = NULL;
}

dword EncryptionAlgorithm::getEncryptedDataLength()
{
	return EncryptionAlgorithm::getEncryptedDataLength(this->ulDataLength);
}

dword EncryptionAlgorithm::getEncryptedDataLength(dword ulDataLength)
{
	if (ulDataLength % BLOCK_SIZE == 0) {
		return ulDataLength;
	}
	else {
		return ulDataLength + (BLOCK_SIZE - (ulDataLength % BLOCK_SIZE));
	}
}

PBYTE EncryptionAlgorithm::generateKeyFromPassword(PSZ pszPassword, PBYTE key)
{
	dword				i;
	dword				pwdLength;
	byte				pwd[KEY_LENGTH];

	pwdLength = (dword)strlen(pszPassword);

	/*
	** Validate password length...
	*/
	if (pwdLength > KEY_LENGTH) {
			throw new Exception(
						ERR_INVALID_PWD_LEN,
						"Invalid password length, must be < 64 characters",
						__FILE__,
						"EncryptionAlgorithm",
						"generateKeyFromPassword()",
						__LINE__);
	}

	for (i = 0;i < KEY_LENGTH;i++) {
		pwd[i] = (byte)pszPassword[i];
	}

	/*
	** Get the SHA-256 hash of the password...
	*/
	gcry_md_hash_buffer(GCRY_MD_SHA3_256, key, pwd, pwdLength);

	/*
	** Debug block...

	int		j = 0;
	char	szKey[129];

	for (i = 0;i < KEY_LENGTH;i++) {
		sprintf(&szKey[j], "%02X", key[i]);
		j += 2;
	}

	cout << "Key for password '" << pszPassword << "' is '0x" << szKey << "'" << endl;
	*/

	return key;
}

void EncryptionAlgorithm::getSecondaryKey(PSZ pszPassword, PBYTE pSecondaryKey)
{
	int					i;
	int					bank = 0;
	int					keyIndex = 0;
	int					pwdLen;
	byte				b;
	byte				md5key[16];
	byte				blakekey[16];
	gcry_cipher_hd_t	aes_hd;

	/*
	** 1. Get a 128 bit (16 byte) hash of the password using MD5
	** 2. Substitute each byte using the key table
	** 3. Encrypt using AES-128, using the Blake-128 hash of the password as a key
	** 4. Use this as the secondary key
	*/

	pwdLen = strlen(pszPassword);

	gcry_md_hash_buffer(GCRY_MD_MD5, md5key, pszPassword, pwdLen);

	/*
	** Use each byte of the new key as a lookup to the staticKey table
	** defined in key.h
	*/
	for (i = 0;i < 16;i++) {
		b = md5key[i];

		keyIndex = (int)b + (256 * bank);

		bank++;

		if (bank == 4) {
			bank = 0;
		}

		pSecondaryKey[i] = keyTable[keyIndex];
	}

	gcry_md_hash_buffer(GCRY_MD_BLAKE2S_128, blakekey, pszPassword, pwdLen);

    int err = gcry_cipher_open(
    					&aes_hd,
    					GCRY_CIPHER_AES128,
                        GCRY_CIPHER_MODE_ECB,
                        0);
    if (err) {
		throw new Exception(
					ERR_INVALID_STATE,
					gcry_strerror(err),
					__FILE__,
					"EncryptionAlgorithm",
					"getSecondaryKey()",
					__LINE__);
    }

    err = gcry_cipher_setkey(aes_hd, (const void*)blakekey, 16);

    if (err) {
		throw new Exception(
					ERR_INVALID_STATE,
					gcry_strerror(err),
					__FILE__,
					"EncryptionAlgorithm",
					"getSecondaryKey()",
					__LINE__);
    }

    err = gcry_cipher_encrypt(
    					aes_hd,
    					pSecondaryKey,
    					16,
    					NULL,
    					0);

    if (err) {
		throw new Exception(
					ERR_INVALID_STATE,
					gcry_strerror(err),
					__FILE__,
					"EncryptionAlgorithm",
					"getSecondaryKey()",
					__LINE__);
    }

    gcry_cipher_close(aes_hd);

	/*
	** Debug block...

	int		j = 0;
	char	szKey[33];

	for (i = 0;i < 16;i++) {
		sprintf(&szKey[j], "%02X", pSecondaryKey[i]);
		j += 2;
	}

	cout << "Key for password '" << pszPassword << "' is '0x" << szKey << "'" << endl;
	*/
}

void EncryptionAlgorithm::getNextDataBlock(PBYTE block)
{
	dword		i;

	for (i = 0L;i < BLOCK_SIZE;i++) {
		if ((this->ulDataPtr + i) >= this->ulDataLength) {
			block[i] = 0;
		}
		else {
			block[i] = data[i + this->ulDataPtr];
		}
	}

#ifdef DEBUG_MEMORY
	printf("[AES.getNextDataBlock()]");
	for (i = 0;i < BLOCK_SIZE;i++) {printf("[0x%02X]%c;",block[i], (char)block[i]);} printf("\n");
#endif

	this->ulDataPtr += BLOCK_SIZE;
	this->blockCount++;
}

bool EncryptionAlgorithm::hasNextBlock()
{
	return (this->blockCount < this->numBlocks);
}

void EncryptionAlgorithm::setKey(PBYTE key, int keyLen)
{
	this->key = key;
	this->keyLength = keyLen;
}

PBYTE EncryptionAlgorithm::getKey()
{
	return this->key;
}

int EncryptionAlgorithm::getKeyLength()
{
	return this->keyLength;
}

AES256::AES256(PBYTE pInputData, dword ulDataLength) : EncryptionAlgorithm(pInputData, ulDataLength)
{
	byte	iv[16];

    int err = gcry_cipher_open(
    					&this->aes_hd,
    					GCRY_CIPHER_AES256,
                        GCRY_CIPHER_MODE_CBC,
                        0);

    if (err) {
		throw new Exception(
					ERR_INVALID_STATE,
					gcry_strerror(err),
					__FILE__,
					"AES",
					"encrypt()",
					__LINE__);
    }

	memset(iv, 0x00, 16);
	setIv(iv, 16);
}

AES256::~AES256()
{
    gcry_cipher_close(this->aes_hd);
}

void AES256::setKey(PBYTE key, int keyLen)
{
    int err = gcry_cipher_setkey(
    					this->aes_hd,
    					(const void*)key,
    					keyLen);

    if (err) {
		throw new Exception(
					ERR_INVALID_STATE,
					gcry_strerror(err),
					__FILE__,
					"AES",
					"encrypt()",
					__LINE__);
    }
}

void AES256::setIv(PBYTE iv, int ivLen)
{
    int err = gcry_cipher_setiv(
    					this->aes_hd,
    					(const void*)iv,
    					ivLen);

    if (err) {
		throw new Exception(
					ERR_INVALID_STATE,
					gcry_strerror(err),
					__FILE__,
					"AES",
					"encrypt()",
					__LINE__);
    }
}

dword AES256::encrypt(PBYTE pOutputData, dword bufferLength)
{
	int err = gcry_cipher_encrypt(
							this->aes_hd,
							pOutputData,
							bufferLength,
							this->data,
							this->ulDataLength);

	if (err) {
		throw new Exception(
					ERR_INVALID_STATE,
					gcry_strerror(err),
					__FILE__,
					"AES",
					"encrypt()",
					__LINE__);
	}

	return bufferLength;
}

dword AES256::decrypt(PBYTE pOutputData, dword bufferLength)
{
    int err = gcry_cipher_decrypt(
							this->aes_hd,
							pOutputData,
							bufferLength,
							this->data,
							this->ulDataLength);

    if (err) {
		throw new Exception(
					ERR_INVALID_STATE,
					gcry_strerror(err),
					__FILE__,
					"AES",
					"encrypt()",
					__LINE__);
    }

	return bufferLength;
}

XOR::XOR(PBYTE pInputData, dword ulDataLength) : EncryptionAlgorithm(pInputData, ulDataLength)
{
}

dword XOR::encrypt(PBYTE pOutputData, dword bufferLength)
{
	byte		keyIterator;
	int			i;
	byte		block[BLOCK_SIZE];
	dword		index = 0L;
	PBYTE		key;

	key = getKey();

	keyIterator = 0x00;

	while (hasNextBlock()) {
		getNextDataBlock(block);

		for (i = 0; i < BLOCK_SIZE; i++) {
			pOutputData[i + index] = block[i] ^ (key[i] + keyIterator);
		}

		keyIterator++;
		index += BLOCK_SIZE;
	}

	return this->ulDataPtr;
}

/*
** With XOR encryption, decryption is identical to encryption...
*/
dword XOR::decrypt(PBYTE pOutputData, dword bufferLength)
{
	return encrypt(pOutputData, bufferLength);
}

dword XOR::encrypt(PBYTE pKey, dword ulKeyLength, PBYTE pOutputData, dword bufferLength)
{
	dword 		ulCounter = 0L;
	dword 		ulKeyCounter = 0L;

	for (ulCounter = 0L;ulCounter < ulDataLength;ulCounter++) {
		pOutputData[ulCounter] = data[ulCounter] ^ pKey[ulKeyCounter++];

		if (ulKeyCounter == ulKeyLength) {
			ulKeyCounter = 0L;
		}
	}

	return ulCounter;
}

/*
** With XOR encryption, decryption is identical to encryption...
*/
dword XOR::decrypt(PBYTE pKey, dword ulKeyLength, PBYTE pOutputData, dword bufferLength)
{
	return encrypt(pKey, ulKeyLength, pOutputData, bufferLength);
}
