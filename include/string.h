#ifndef __STRING_H__
#define __STRING_H__

#include <types.h>
#include <compiler.h>

size_t strlen(const char *s) __nonnull;

void *memcpy(void *d, const void *s, size_t n);

void *memset(void *s, int c, size_t n);

#endif

