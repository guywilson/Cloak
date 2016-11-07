#ifndef _WIN32
#include <bsd/string.h>

#define strcpy_s(t, l, s)	strlcpy(t, s, l);
#define strcat_s(t, l, s)	strlcat(t, s, l);

#endif