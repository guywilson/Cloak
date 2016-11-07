#include "iterator.h"
#include "safebuffer.h"
#include "types.h"

#ifndef _INCL_DATA
#define _INCL_DATA

#define FILENAME_BUFFER_LENGTH		512
#define EXTENSION_BUFFER_LENGTH		16

class DataFile
{
	protected:
		char				szFilename[FILENAME_BUFFER_LENGTH];
		PBYTE				data;
		dword				ulFileLength;
		
		dword				_getFileLength(FILE *fptr);
	
	public:
							DataFile(char *pszFilename);
							DataFile(byte *pchData, dword ulLength);
							DataFile();
							
							~DataFile();
		
		char		  *		getFilename();
		void				setFilename(char *pszFilename);
		
		dword				getFileLength();
		
		void				getExtension(char *pszExtension);
		
		byte *				getData();
		
		void				read();
		void				write();
};

class EncryptedDataFile : public DataFile
{
	private:
		char				szDefaultExtension[EXTENSION_BUFFER_LENGTH];
		dword				ulEncryptedDataLength;
		
	public:
							EncryptedDataFile(char *pszFilename);
							EncryptedDataFile(byte *pchData, dword ulLength);
							EncryptedDataFile();

							~EncryptedDataFile();

		dword				getEncryptedDataLength();
		
		char *				getDefaultExtension();
		void				setDefaultExtension(char *pszExtension);
		
		BitStreamIterator	*iterator(word usBitsPerByte);
		
		void				encrypt(char *pszPassword);
		void				encrypt(byte * pbKey, dword ulKeyLength);

		void				decrypt(char *pszPassword);
		void				decrypt(byte * pbKey, dword ulKeyLength);
		
		void				read();
		void				write();
};

#endif
