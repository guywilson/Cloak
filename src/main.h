#define 	MODE_INFO_ONLY			0x00
#define		MODE_ADD				0x01
#define		MODE_EXTRACT			0x02
#define		MODE_COPY				0x04

#define		KEY_PASSWD				0x01
#define		KEY_STREAM				0x10
#define		KEY_DEBUG				0xFF

#define MAX_PASSWORD_LENGTH			64
#define PASSWORD_BUFFER_LENGTH		65

void printUsage();
void printHeaderInfo(Cloak *cloak);
void processParams(int argc, char *argv[]);
bool processCommand(Cloak * cloak, char *pszCommand);
void getBitsPerByte(Cloak *cloak);
void getCompressionLevel(Cloak *cloak);
int getPassword();
void getpwd(char *pszPassword, int maxLen);
byte * getKeyStream(char *pszKeyFilename, dword * ulKeyLength);

#ifdef __WATCOMC__
int _strcmpi(const char *pszStr, const char *pszCompare);
#endif
