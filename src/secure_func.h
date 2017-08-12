#ifndef _WIN32
    #ifdef __linux__
    #include <bsd/string.h>
    #endif
#define strcpy_s(t, l, s)	strlcpy(t, s, l);
#define strcat_s(t, l, s)	strlcat(t, s, l);
#endif
