#include "types.h"

#ifndef _INCL_SAFEBUFFER
#define _INCL_SAFEBUFFER

class SafeBuffer
{
	private:
		PBYTE		_buffer;
		dword		_bufferLength;
		dword		_offset;
		bool		_isAllocated;
		
		void		assertAllocated();
		void		assertLessThan(dword a, dword b);
		
	public:
					SafeBuffer();
					SafeBuffer(dword size);
					
					~SafeBuffer();
					
		void		allocateBuffer(dword size);
		
		byte		getByteAt(dword index);
		
		PBYTE		getPtr();
		PBYTE		getPtr(dword offset);
		
		dword		getRemainingSize();
		
		void		copyFrom(void * pSource, size_t length);
		void		copyTo(void * pTarget, size_t length);
		
		void		copyFrom(void * pSource, size_t length, dword offset);
		void		copyTo(void * pTarget, size_t length, dword offset);
};

#endif