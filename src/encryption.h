#include "types.h"
#include "md5.h"

#ifndef _INCL_ENCRYPTION
#define	_INCL_ENCRYPTION

#define BLOCK_SIZE			16
#define KEY_LENGTH			16

class EncryptionAlgorithm
{
	private:
		dword			numBlocks;
		dword			blockCount;
	
	protected:
		dword			ulDataLength;
		dword			ulDataPtr;
		PBYTE			data;

		void			getNextDataBlock(PBYTE block);
		bool			hasNextBlock();
		
	public:
						EncryptionAlgorithm(PBYTE pInputData, dword ulDataLength);
						~EncryptionAlgorithm();
						
		static PBYTE	generateKeyFromPassword(PSZ pszPassword, PBYTE key);
		static void 	getSecondaryKey(PBYTE pInitialKey, PBYTE pSecondaryKey);
		
		dword			getEncryptedDataLength();
		static dword	getEncryptedDataLength(dword ulDataLength);
		
		/*
		** It is assumed that the key is 16 bytes (128 bit)
		*/
		virtual dword	encrypt(PBYTE pKey, PBYTE pOutputData) = 0;
		virtual dword	decrypt(PBYTE pKey, PBYTE pOutputData) = 0;
};


class AES : public EncryptionAlgorithm
{
	public:
						AES(PBYTE pInputData, dword ulDataLength);
						
		dword			encrypt(PBYTE pKey, PBYTE pOutputData);
		dword			decrypt(PBYTE pKey, PBYTE pOutputData);
};


class XOR : public EncryptionAlgorithm
{
	public:
						XOR(PBYTE pInputData, dword ulDataLength);
						
		dword			encrypt(PBYTE pKey, PBYTE pOutputData);
		dword			decrypt(PBYTE pKey, PBYTE pOutputData);

		dword			encrypt(PBYTE pKey, dword ulKeyLength, PBYTE pOutputData);
		dword			decrypt(PBYTE pKey, dword ulKeyLength, PBYTE pOutputData);
};

#endif