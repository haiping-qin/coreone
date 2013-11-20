#ifndef __SYS_HEAP_H__
#define __SYS_HEAP_H__

#include <types.h>

void *heap_alloc(size_t size);
void heap_free(void *p);

#endif
