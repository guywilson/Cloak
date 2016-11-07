#include "types.h"

#ifndef _INCL_MD5
#define _INCL_MD5

class MD5
{
	private:
		dword	state[4];
		dword	count[2];
		byte	buffer[64];
		
		void	transform(byte *pBlock);
		void	encode(byte *pOutput, dword *pInput, dword length);
		void	decode(dword *pOutput, byte *pInput, dword length);
		
	public:
				MD5();
				~MD5();
		
		void	initialise();
		void	clear();
		void	update(byte *pInput, dword length);
		void	finalise(byte digest[16]);
};

#endif