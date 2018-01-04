
extern "C" {
	#include <gcrypt.h>
}

#include "types.h"

#ifndef _INCL_ENCRYPTION
#define	_INCL_ENCRYPTION

#define BLOCK_SIZE			16
#define KEY_LENGTH			32
#define IV_LENGTH			BLOCK_SIZE

class EncryptionAlgorithm
{
	private:
		dword			numBlocks;
		dword			blockCount;
		PBYTE			key;
		int				keyLength;

	protected:
		dword			ulDataLength;
		dword			ulDataPtr;
		PBYTE			data;

		void			getNextDataBlock(PBYTE block);
		bool			hasNextBlock();

		PBYTE			getKey();
		int				getKeyLength();

	public:
						EncryptionAlgorithm(PBYTE pInputData, dword ulDataLength);
						~EncryptionAlgorithm();

		static PBYTE	generateKeyFromPassword(PSZ pszPassword, PBYTE key);
		static void 	getSecondaryKey(PSZ pszPassword, PBYTE pSecondaryKey);

		dword			getEncryptedDataLength();
		static dword	getEncryptedDataLength(dword ulDataLength);

		void			setKey(PBYTE key, int keyLen);


		/*
		** It is assumed that the key is 16 bytes (128 bit)
		*/
		virtual dword	encrypt(PBYTE pOutputData, dword bufferLength) = 0;
		virtual dword	decrypt(PBYTE pOutputData, dword bufferLength) = 0;
};


class AES256 : public EncryptionAlgorithm
{
	private:
		gcry_cipher_hd_t	aes_hd;

	protected:
		PBYTE			getKey();
		int				getIvLength();

	public:
						AES256(PBYTE pInputData, dword ulDataLength);
						~AES256();

		void			setKey(PBYTE key, int keyLen);
		void			setIv(PBYTE iv, int ivLen);

		dword			encrypt(PBYTE pOutputData, dword bufferLength);
		dword			decrypt(PBYTE pOutputData, dword bufferLength);
};


class XOR : public EncryptionAlgorithm
{
	public:
						XOR(PBYTE pInputData, dword ulDataLength);

		dword			encrypt(PBYTE pOutputData, dword bufferLength);
		dword			decrypt(PBYTE pOutputData, dword bufferLength);

		dword			encrypt(PBYTE pKey, dword ulKeyLength, PBYTE pOutputData, dword bufferLength);
		dword			decrypt(PBYTE pKey, dword ulKeyLength, PBYTE pOutputData, dword bufferLength);
};

#endif
