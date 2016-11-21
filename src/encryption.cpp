#include <stdio.h>
#include <string.h>
#include <iostream>

#include "encryption.h"
#include "salt.h"
#include "errorcodes.h"
#include "exception.h"
#include "key.h"

extern "C" {
	#include "aes.h"
}

EncryptionAlgorithm::EncryptionAlgorithm(PBYTE pInputData, dword ulDataLength)
{
	this->ulDataLength = ulDataLength;
	this->data = pInputData;
	this->ulDataPtr = 0L;
	
	this->blockCount = 0L;
	
	this->numBlocks = getEncryptedDataLength() / BLOCK_SIZE;
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
	dword		i;
	dword		pwdLength;
	byte		pwd[64];
	
	pwdLength = (dword)strlen(pszPassword);
	
	/*
	** Validate password length...
	*/
	if (pwdLength > 64) {
			throw new Exception(
						ERR_INVALID_PWD_LEN, 
						"Invalid password length, must be < 64 characters", 
						__FILE__, 
						"SimpleXOR", 
						"encrypt()", 
						__LINE__);
	}

	/*
	** Copy the password + remainder of the salt into a byte array...
	*/
	for (i = 0;i < SALT_LENGTH;i++) {
		if (i < pwdLength) {
			pwd[i] = (byte)pszPassword[i];
		}
		else {
			pwd[i] = salt[i];
		}
	}
		
	/*
	** Get the MD5 hash of the password...
	*/
	MD5 hash;
	hash.initialise();
	hash.update(pwd, SALT_LENGTH);
	hash.finalise(key);
	hash.clear();
	
	return key;
}

void EncryptionAlgorithm::getSecondaryKey(PBYTE pInitialKey, PBYTE pSecondaryKey)
{
	int			i;
	int			j = 0;
	int			bank = 0;
	int			keyIndex = 0;
	byte		b;
	byte		key[16];
	char		szPasswordFromKey[33];
	static char hexChars[17] = "0123456789ABCDEF";
	
	/*
	** Build a 32 character hex string from the binary key...
	*/
	for (i = 0;i < 16;i++) {
		szPasswordFromKey[j++] = hexChars[(pInitialKey[i] >> 4) & 0x0F];
		szPasswordFromKey[j++] = hexChars[pInitialKey[i] & 0x0F];
	}
	szPasswordFromKey[j] = 0;
	
	/*
	** Generate a new key from the hex representation of the original key
	*/
	EncryptionAlgorithm::generateKeyFromPassword(szPasswordFromKey, key);
	
	/*
	** Use each byte of the new key as a lookup to the staticKey table
	** defined in key.h
	*/
	for (i = 0;i < 16;i++) {
		b = key[i];
		
		keyIndex = (int)b + (256 * bank);
		
		bank++;
		
		if (bank == 4) {
			bank = 0;
		}
		
		pSecondaryKey[i] = keyTable[keyIndex];
	}
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

AES::AES(PBYTE pInputData, dword ulDataLength) : EncryptionAlgorithm(pInputData, ulDataLength)
{
}

dword AES::encrypt(PBYTE pKey, PBYTE pOutputData)
{
	byte		block[BLOCK_SIZE];
	dword		index = 0L;

#ifdef DEBUG_MEMORY	
	int			i;
	
	printf("[AES.encrypt()] : Encrypting %ld bytes from source address range 0x%09X - 0x%09X to target address range 0x%09X - 0x%09X\n", ulDataLength, (dword)data, (dword)(data + ulDataLength), (dword)pOutputData, (dword)(pOutputData + ulDataLength));
#endif
	
	while (hasNextBlock()) {
		getNextDataBlock(block);
		
		AES128_ECB_encrypt(block, pKey, &pOutputData[index]);
#ifdef DEBUG_MEMORY	
		printf("[AES.encrypt()] : Writing data to 0x%09X\n", (dword)&pOutputData[index]);
		for (i = 0;i < BLOCK_SIZE;i++) {printf("[0x%02X];",pOutputData[index+i]);} printf("\n");
		fflush(stdout);
#endif
		
		index += BLOCK_SIZE;
	}
	
	return index;
}

dword AES::decrypt(PBYTE pKey, PBYTE pOutputData)
{
	byte		block[BLOCK_SIZE];
	dword		index = 0L;
	
#ifdef DEBUG_MEMORY	
	int			i;

	printf("[AES.decrypt()] : Decrypting %ld bytes from source address range 0x%09X - 0x%09X to target address range 0x%09X - 0x%09X\n", ulDataLength, (dword)data, (dword)(data + ulDataLength), (dword)pOutputData, (dword)(pOutputData + ulDataLength));
#endif
	
	while (hasNextBlock()) {
		getNextDataBlock(block);
		
		AES128_ECB_decrypt(block, pKey, &pOutputData[index]);
#ifdef DEBUG_MEMORY	
		printf("[AES.decrypt()] : Writing data to 0x%09X\n", (dword)&pOutputData[index]);
		for (i = 0;i < BLOCK_SIZE;i++) {printf("[0x%02X]%c;",pOutputData[index+i],(char)pOutputData[index+i]);} printf("\n");
		fflush(stdout);
#endif
		
		index += BLOCK_SIZE;
	}
	
	return index;
}

XOR::XOR(PBYTE pInputData, dword ulDataLength) : EncryptionAlgorithm(pInputData, ulDataLength)
{
}

dword XOR::encrypt(PBYTE pKey, PBYTE pOutputData)
{
	byte		keyIterator;
	int			i;
	byte		block[BLOCK_SIZE];
	dword		index = 0L;

	keyIterator = 0x00;
	
	while (hasNextBlock()) {
		getNextDataBlock(block);
		
		for (i = 0; i < BLOCK_SIZE; i++) {
			pOutputData[i + index] = block[i] ^ (pKey[i] + keyIterator);
		}
		
		keyIterator++;
		index += BLOCK_SIZE;
	}
	
	return this->ulDataPtr;
}

/*
** With XOR encryption, decryption is identical to encryption...
*/
dword XOR::decrypt(PBYTE pKey, PBYTE pOutputData)
{
	return encrypt(pKey, pOutputData);
}

dword XOR::encrypt(PBYTE pKey, dword ulKeyLength, PBYTE pOutputData)
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
dword XOR::decrypt(PBYTE pKey, dword ulKeyLength, PBYTE pOutputData)
{
	return encrypt(pKey, ulKeyLength, pOutputData);
}
