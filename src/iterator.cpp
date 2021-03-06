#include <stdio.h>
#include <iostream>

#include "iterator.h"

using namespace std;

Iterator::Iterator(byte *pchData, dword ulLength)
{
	this->data = pchData;
	this->ulDataLength = ulLength;

	this->counter = 0L;
}

dword Iterator::getDataLength()
{
	return ulDataLength;
}

byte Iterator::getCurrentByte()
{
	return data[counter];
}

bool Iterator::hasNext()
{
	return counter < ulDataLength ? true : false;
}

BitStreamIterator::BitStreamIterator(byte * pchData, dword ulLength, word usBitsPerByte) : Iterator(pchData, ulLength)
{
	bitCounter = 0;
	bitsPerByte = usBitsPerByte;
}

byte BitStreamIterator::nextBits()
{
	byte	bits = 0x00;

	bits = (getCurrentByte() >> bitCounter) & getBitMask();
	bitCounter += bitsPerByte;

	if (bitCounter == 8) {
		bitCounter = 0;
		counter++;
	}

	return bits;
}

word BitStreamIterator::getBitsPerByte()
{
	return this->bitsPerByte;
}

byte BitStreamIterator::getBitMask()
{
	return BitStreamIterator::getBitMask(getBitsPerByte());
}

byte BitStreamIterator::getBitMask(word usBitsPerByte)
{
	int				i;
	byte			mask = 0x00;

	for (i = 0;i < usBitsPerByte;i++) {
		mask += (0x01 << i) & 0xFF;
	}

	return mask;
}

ByteStreamIterator::ByteStreamIterator(byte * pchData, dword ulLength) : Iterator(pchData, ulLength)
{

}

byte ByteStreamIterator::nextByte()
{
	byte	byte;

	byte = getCurrentByte();
	counter++;

	return byte;
}
