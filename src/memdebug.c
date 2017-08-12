#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "memdebug.h"


void * malloc_d(size_t size, const char *pszId)
{
	void * rtn;
	byte * p;

	rtn = malloc(size);
	p = (byte *)rtn;

#ifdef DEBUG_MEMORY
	printf("malloc_d() [%s] : Allocating %ld bytes from 0x%09X to 0x%09X\n", pszId, size, (dword)p, (dword)(p+size));
#endif

	return p;
}

void free_d(void *ptr, const char *pszId)
{
#ifdef DEBUG_MEMORY
	byte * p;

	p = (byte *)ptr;

	printf("free_d() [%s] : Freeing memory at 0x%09X\n", pszId, (dword)p);
#endif

	free(ptr);
}

void * memcpy_d(void * d, void * s, size_t size, const char *pszId)
{
	void * rtn;

#ifdef DEBUG_MEMORY
	byte * p;
	byte * r;

	p = (byte *)d;
	r = (byte *)s;

	printf("memcpy_d() [%s] : Copying %ld bytes to address 0x%09X to 0x%09X from address 0x%09X to 0x%09X\n", pszId, size, (dword)p, (dword)(p+size), (dword)r, (dword)(r+size));
#endif

	rtn = memcpy(d, s, size);

	return rtn;
}

size_t fwrite_d(const void * ptr, size_t size, size_t count, FILE * stream, const char *pszId)
{
	size_t rtn;

#ifdef DEBUG_MEMORY
	byte * p;

	p = (byte *)ptr;

	printf("fwrite_d() [%s] : Writing %ld bytes to file from address 0x%09X to 0x%09X\n", pszId, (size * count), (dword)p, (dword)(p+(size * count)));
#endif

	rtn = fwrite(ptr, size, count, stream);

	return rtn;
}

size_t fread_d(void * ptr, size_t size, size_t count, FILE * stream, const char *pszId)
{
	size_t rtn;

#ifdef DEBUG_MEMORY
	byte * p;

	p = (byte *)ptr;

	printf("fread_d() [%s] : Reading %ld bytes from file to address 0x%09X to 0x%09X\n", pszId, (size * count), (dword)p, (dword)(p+(size * count)));
#endif

	rtn = fread(ptr, size, count, stream);

	return rtn;
}
