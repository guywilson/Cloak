#include "iterator.h"
#include "types.h"

#ifndef _INCL_DATA
#define _INCL_DATA

#define FILENAME_BUFFER_LENGTH		512
#define EXTENSION_BUFFER_LENGTH		16

#define ZLIB_CHUNK_SIZE				65536

#define FILE_OPEN_READ				0x01
#define FILE_OPEN_WRITE				0x02
#define FILE_OPEN_APPEND			0x04

#define FILE_OPEN_UPDATE			0x08

#define FILE_MODE_BINARY			0x10
#define FILE_MODE_TEXT				0x20

#define DATA_HEADER_SIZE			8

class Data
{
	private:
		byte *				_data;
		dword				_length;

		dword				_blockSize;
		dword				_blockPtr;

		bool				_isInitialised = false;

		void				_initialise();

	protected:
		void				setOriginalLength(dword len);
		void				setCompressedLength(dword len);

		void				setIsCompressed(bool yn);
		void				setIsEncrypted(bool yn);

	public:
							Data();
							Data(dword size);
							Data(Data & src);
							Data(byte * header, byte * data, dword length);

							~Data();

		byte *				getData();

		dword				getLength();
		dword				getTotalLength();

		void				initiateBlockIterator(dword blockSize);
		bool				hasNextBlock();
		dword				nextBlock(byte * block);

		BitStreamIterator *	iterator(word usBitsPerByte);

		void				encrypt();
		void				encrypt(byte * pbKey, dword ulKeyLength);

		void				decrypt();
		void				decrypt(byte * pbKey, dword ulKeyLength);

		dword				compress(int level);
		void				decompress();

		dword				getOriginalLength();
		dword				getCompressedLength();
		dword				getEncryptedLength();

		static dword		getOriginalLength(byte * header);
		static dword		getCompressedLength(byte * header);
		static dword		getEncryptedLength(byte * header);

		bool				isCompressed();
		bool				isEncrypted();

		static bool			isCompressed(byte * header);
		static bool			isEncrypted(byte * header);
};

class DataFile
{
	private:
		FILE *				fptr;
		int					mode;

		bool				isRead();
		bool				isWrite();
		bool				isAppend();

		bool				isUpdate();

		bool				isBinary();
		bool				isText();

	public:
							DataFile(char * pszFileName, int mode);

							~DataFile();

		dword				getLength();

		Data *				read(dword numBytes);
		void				write(Data * data, dword numBytes);
};

#endif
