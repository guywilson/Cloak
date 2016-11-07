#include <stdlib.h>
#include <string.h>

#include "exception.h"
#include "errorcodes.h"
#include "safebuffer.h"

SafeBuffer::SafeBuffer()
{
	_buffer = NULL;
	_bufferLength = 0L;
	_offset = 0L;
	_isAllocated = false;
}

SafeBuffer::SafeBuffer(dword size) : SafeBuffer()
{
	allocateBuffer(size);
}

SafeBuffer::~SafeBuffer()
{
	if (_isAllocated) {
		free(_buffer);
		_isAllocated = false;
		_bufferLength = 0L;
		_offset = 0L;
	}
}

void SafeBuffer::assertAllocated()
{
	if (!_isAllocated) {
		throw new Exception(
			ERR_MALLOC,
			"Buffer is not allocated",
			__FILE__,
			"SafeBuffer",
			"assertAllocated()",
			__LINE__);
	}
}

void SafeBuffer::assertLessThan(dword a, dword b)
{
	if (a >= b) {
		throw new Exception(
			ERR_VALIDATION,
			"Not less than error",
			__FILE__,
			"SafeBuffer",
			"assertLessThan()",
			__LINE__);
	}
}

void SafeBuffer::allocateBuffer(dword size)
{
	if (!_isAllocated) {
		_buffer = (PBYTE)malloc(size);
		
		if (_buffer == NULL) {
			throw new Exception(
				ERR_MALLOC,
				"Failed to allocate buffer",
				__FILE__,
				"SafeBuffer",
				"allocateBuffer()",
				__LINE__);
		}
		
		memset(_buffer, 0, size);
		
		_bufferLength = size;
		_offset = 0L;
		_isAllocated = true;
	}
	else {
		throw new Exception(
			ERR_MALLOC,
			"Buffer is already allocated",
			__FILE__,
			"SafeBuffer",
			"allocateBuffer()",
			__LINE__);
	}
}

byte SafeBuffer::getByteAt(dword index)
{
	byte		b = 0x00;
	
	assertAllocated();
	assertLessThan(index, _bufferLength);
	
	b = _buffer[index];
	
	return b;
}

PBYTE SafeBuffer::getPtr(dword offset)
{
	PBYTE		p = NULL;
	
	assertAllocated();
	assertLessThan(offset, _bufferLength);

	p = &_buffer[offset];
	_offset = offset;
	
	return p;
}

PBYTE SafeBuffer::getPtr()
{
	return getPtr(0L);
}

dword SafeBuffer::getRemainingSize()
{
	dword	remainder;
	
	remainder = (_bufferLength - _offset);
	
	_offset = 0L;
	
	return remainder;
}

void SafeBuffer::copyFrom(void * pSource, size_t length)
{
	assertAllocated();
	assertLessThan(length, _bufferLength);

	memcpy(_buffer, pSource, length);
}

void SafeBuffer::copyTo(void * pTarget, size_t length)
{
	assertAllocated();
	assertLessThan(length, _bufferLength);

	memcpy(pTarget, _buffer, length);
}

void SafeBuffer::copyFrom(void * pSource, size_t length, dword offset)
{
	assertAllocated();
	assertLessThan(length, _bufferLength);
	assertLessThan(offset, _bufferLength);
	assertLessThan(length, (_bufferLength - offset));

	memcpy(&_buffer[offset], pSource, length);
}

void SafeBuffer::copyTo(void * pTarget, size_t length, dword offset)
{
	assertAllocated();
	assertLessThan(length, _bufferLength);
	assertLessThan(offset, _bufferLength);
	assertLessThan(length, (_bufferLength - offset));

	memcpy(pTarget, &_buffer[offset], length);
}
