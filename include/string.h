#ifndef __STRING_H__
#define __STRING_H__

#include <common.h>

size_t strlen(const char *s) __nonnull;

void *memcpy(void *dest, const void *src, size_t count);

#endif

