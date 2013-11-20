#include <string.h>

size_t strlen(const char *s)
{
	size_t n = 0;

	while (*s++)
		n++;

	return n;
}

void *memcpy(void *d, const void *s, size_t n)
{
	const char *_s = s;
	char *_d = d;

	while (n--)
		*_d++ = *_s++;

	return d;
}

void *memset(void *s, int c, size_t n)
{
	char *_s = s;
	
	while (n--)
		*_s++ = c;

	return s;
}
