#include "types.h"

#ifndef _INCL_ITERATOR
#define _INCL_ITERATOR

class Iterator
{
	private:
		byte * 			data;
		dword			ulDataLength;
		
	protected:
		dword			getDataLength();
		byte			getCurrentByte();
		
		dword			counter;
		
	public:
						Iterator(byte *pchData, dword ulLength);
						
		bool			hasNext();
};

class BitStreamIterator : public Iterator
{
	private:
		int			bitCounter;
		word		bitsPerByte;
		
	public:
					BitStreamIterator(byte * pchData, dword ulLength, word usBitsPerByte);
						
		word		getBitsPerByte();
		
		byte		nextBits();
		
		byte		getBitMask();
		static byte	getBitMask(word usBitsPerByte);
};

class ByteStreamIterator : public Iterator
{
	public:
				ByteStreamIterator(byte * pchData, dword ulLength) : Iterator(pchData, ulLength) {}
						
		byte	nextByte();
};

#endif