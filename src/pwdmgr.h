#ifndef _INCL_PWDMGR
#define _INCL_PWDMGR

#define MAX_PASSWORD_LEN			64
#define PASSWORD_BUFFER_LEN			(MAX_PASSWORD_LEN + 1)

#define MAX_KEY_LEN					32
#define KEY_BUFFER_LEN				MAX_KEY_LEN

#define MAX_IV_LEN					16
#define IV_BUFFER_LEN				MAX_IV_LEN


class PasswordManager
{
	public:
		static PasswordManager & getInstance()
		{
			static PasswordManager instance;
			return instance;
		}

	private:
		char *				pszPassword = nullptr;

		byte				bKey[KEY_BUFFER_LEN];
		byte				bIV[IV_BUFFER_LEN];

		bool				passwordSupplied = false;

		void				wipePassword();
		void				wipeKeys();

		void				cleanUp();

		PasswordManager();

	public:
		~PasswordManager();

		PasswordManager(PasswordManager const &) 	= delete;
		void operator = (PasswordManager const &)	= delete;

		char *				getPasswordBuffer();
		int					getPasswordBufferLen() {return PASSWORD_BUFFER_LEN;}

		byte *				getKey();
		int					getKeyBufferLen() {return KEY_BUFFER_LEN;}

		byte *				getIV();
		int					getIVBufferLen() {return IV_BUFFER_LEN;}

		bool				isPasswordSupplied() {return this->passwordSupplied;}

		void				finalise();
};

#endif
