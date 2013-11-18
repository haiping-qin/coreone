#include <string.h>

size_t strlen(const char *s)
{
	size_t n = 0;

	while (*s++)
		n++;

	return n;
}

void *memcpy(void *dest, const void *src, size_t count)
{
	char *d = dest;
	const char *s = src;

	while (count--)
		*d++ = *s++;
	return dest;
}
