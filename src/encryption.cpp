#include <stdio.h>
#include <string.h>
#include <iostream>

#include "encryption.h"
#include "errorcodes.h"
#include "exception.h"

extern "C" {
	#include <sph_sha2.h>
	#include "aes.h"
}

using namespace std;

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
	dword				i;
	dword				pwdLength;
	byte				pwd[64];
	sph_sha512_context	ctx;

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

	for (i = 0;i < 64;i++) {
		pwd[i] = (byte)pszPassword[i];
	}

	/*
	** Get the SHA-512 hash of the password...
	*/
	sph_sha512_init(&ctx);
	sph_sha512(&ctx, pwd, pwdLength);
	sph_sha512_close(&ctx, key);

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
