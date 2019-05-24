#include <stdio.h>
#include <string.h>
#include <iostream>

#include "types.h"
#include "pwdmgr.h"
#include "encryption.h"
#include "errorcodes.h"
#include "exception.h"
#include "key.h"

using namespace std;

PasswordManager::PasswordManager()
{
	this->passwordSupplied = false;
}

PasswordManager::~PasswordManager()
{
	this->cleanUp();
}

void PasswordManager::wipePassword()
{
	if (this->pszPassword != nullptr) {
		memset(this->pszPassword, 0x00, PASSWORD_BUFFER_LEN);
		memset(this->pszPassword, 0xFF, PASSWORD_BUFFER_LEN);
		memset(this->pszPassword, 0x00, PASSWORD_BUFFER_LEN);
		memset(this->pszPassword, 0xFF, PASSWORD_BUFFER_LEN);
		memset(this->pszPassword, 0x00, PASSWORD_BUFFER_LEN);
	}
}

void PasswordManager::wipeKeys()
{
	memset(this->bKey, 0x00, KEY_BUFFER_LEN);
	memset(this->bKey, 0xFF, KEY_BUFFER_LEN);
	memset(this->bKey, 0x00, KEY_BUFFER_LEN);
	memset(this->bKey, 0xFF, KEY_BUFFER_LEN);
	memset(this->bKey, 0x00, KEY_BUFFER_LEN);

	memset(this->bIV, 0x00, IV_BUFFER_LEN);
	memset(this->bIV, 0xFF, IV_BUFFER_LEN);
	memset(this->bIV, 0x00, IV_BUFFER_LEN);
	memset(this->bIV, 0xFF, IV_BUFFER_LEN);
	memset(this->bIV, 0x00, IV_BUFFER_LEN);
}

void PasswordManager::cleanUp()
{
	if (this->pszPassword != nullptr) {
		this->wipePassword();

		free(this->pszPassword);
	}

	this->wipeKeys();
}

char * PasswordManager::getPasswordBuffer()
{
	this->cleanUp();

	this->pszPassword = (char *)malloc(sizeof(char) * PASSWORD_BUFFER_LEN);

	if (this->pszPassword == NULL) {
		throw new Exception(
				ERR_MALLOC,
				"Error allocating memory for password",
				__FILE__,
				"PasswordManager",
				"getPasswordBuffer()",
				__LINE__);
	}

	this->wipePassword();

	return this->pszPassword;
}

byte * PasswordManager::getKey()
{
	return this->bKey;
}

byte * PasswordManager::getIV()
{
	return this->bIV;
}

void PasswordManager::finalise()
{
	if (this->pszPassword != nullptr) {
		if (strlen(this->pszPassword) > 0) {
			this->passwordSupplied = true;
		}

		wipeKeys();

		EncryptionAlgorithm::generateKeyFromPassword(this->pszPassword, this->getKey(), KEY_BUFFER_LEN);
		EncryptionAlgorithm::getSecondaryKey(this->pszPassword, this->getIV(), IV_BUFFER_LEN);

		wipePassword();
	}
}
