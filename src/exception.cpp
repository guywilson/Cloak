#include <string.h>
#include <stdio.h>

#include "secure_func.h"
#include "exception.h"
#include "errorcodes.h"


void Exception::_initialise()
{
	this->szMessage[0] = '\0';
	this->szFileName[0] = '\0';
	this->szClassName[0] = '\0';
	this->szMethodName[0] = '\0';

	this->errorCode = 0L;
	this->lineNumber = 0L;
}

Exception::Exception()
{
	_initialise();
	this->errorCode = ERR_UNDEFINED;
	strcpy_s(this->szMessage, MESSAGE_BUFFER_SIZE, "Undefined exeption");
}

Exception::Exception(const char *pszMessage)
{
	_initialise();
	strcpy_s(this->szMessage, MESSAGE_BUFFER_SIZE, pszMessage);
}

Exception::Exception(dword errorCode, const char *pszMessage)
{
	_initialise();
	this->errorCode = errorCode;
	strcpy_s(this->szMessage, MESSAGE_BUFFER_SIZE, pszMessage);
}

Exception::Exception(dword errorCode, const char *pszMessage, const char *pszFileName, const char *pszClassName, const char *pszMethodName, dword lineNumber)
{
	_initialise();
	this->errorCode = errorCode;
	strcpy_s(this->szMessage, MESSAGE_BUFFER_SIZE, pszMessage);
	strcpy_s(this->szFileName, MESSAGE_BUFFER_SIZE, pszFileName);
	strcpy_s(this->szClassName, MESSAGE_BUFFER_SIZE, pszClassName);
	strcpy_s(this->szMethodName, MESSAGE_BUFFER_SIZE, pszMethodName);
	this->lineNumber = lineNumber;
}

dword Exception::getErrorCode()
{
	return this->errorCode;
}

dword Exception::getLineNumber()
{
	return this->lineNumber;
}

char *Exception::getFileName()
{
	return this->szFileName;
}

char *Exception::getClassName()
{
	return this->szClassName;
}

char *Exception::getMethodName()
{
	return this->szMethodName;
}

char *Exception::getMessage()
{
	return this->szMessage;
}

char *Exception::getExceptionString()
{
	char		szLineNum[8];

	exception[0] = '\0';

	if (errorCode) {
	#ifdef _WIN32
		sprintf_s(exception, EXCEPTION_BUFFER_SIZE, "*** Exception (0x%04X) : ", (unsigned int)errorCode);
	#else
		sprintf(exception, "*** Exception (0x%04X) : ", (unsigned int)errorCode);
	#endif
	}
	else {
		strcat_s(exception, EXCEPTION_BUFFER_SIZE, "*** Exception : ");
	}

	strcat_s(exception, EXCEPTION_BUFFER_SIZE, szMessage);
	strcat_s(exception, EXCEPTION_BUFFER_SIZE, " ***");

	if (strlen(szFileName) > 0 && strlen(szClassName) > 0 && strlen(szMethodName) > 0) {
		strcat_s(exception, EXCEPTION_BUFFER_SIZE, "\nIn ");
		strcat_s(exception, EXCEPTION_BUFFER_SIZE, szFileName);
		strcat_s(exception, EXCEPTION_BUFFER_SIZE, " - ");
		strcat_s(exception, EXCEPTION_BUFFER_SIZE, szClassName);
		strcat_s(exception, EXCEPTION_BUFFER_SIZE, "::");
		strcat_s(exception, EXCEPTION_BUFFER_SIZE, szMethodName);
	}

	if (lineNumber > 0) {
		strcat_s(exception, EXCEPTION_BUFFER_SIZE, " at line ");
	#ifdef _WIN32
		sprintf_s(szLineNum, 8, "%lu", lineNumber);
	#else
		sprintf(szLineNum, "%lu", lineNumber);
	#endif
		strcat_s(exception, EXCEPTION_BUFFER_SIZE, szLineNum);
	}

	return exception;
}
