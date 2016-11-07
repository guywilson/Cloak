#ifndef _INCL_TYPES
#include "types.h"
#endif

#define MESSAGE_BUFFER_SIZE			1024
#define EXCEPTION_BUFFER_SIZE		2048

class Exception
{
	private:
		dword		errorCode;
		char		szMessage[MESSAGE_BUFFER_SIZE];
		
		char		szFileName[256];
		char		szClassName[128];
		char		szMethodName[128];
		
		char 		exception[EXCEPTION_BUFFER_SIZE];
		
		dword		lineNumber;
		
		void		_initialise();
		
	public:
					Exception();
					Exception(const char *pszMessage);
					Exception(dword errorCode, const char *pszMessage);
					Exception(
							dword errorCode, 
							const char *pszMessage, 
							const char *pszFileName, 
							const char *pszClassName, 
							const char *pszMethodName, 
							dword lineNumber);
							
		dword		getErrorCode();
		dword		getLineNumber();
		
		char *		getFileName();
		char *		getClassName();
		char *		getMethodName();
		
		char *		getMessage();
		
		char *		getExceptionString();
};
