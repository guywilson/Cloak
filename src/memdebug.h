void * malloc_d(size_t size, char *pszId);
void free_d(void *ptr, char *pszId);
void * memcpy_d(void * d, void * s, size_t size, char *pszId);
size_t fwrite_d(const void * ptr, size_t size, size_t count, FILE * stream, char *pszId);
size_t fread_d(void * ptr, size_t size, size_t count, FILE * stream, char *pszId);
