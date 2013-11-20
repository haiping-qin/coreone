#include <sys/heap.h>
#include <malloc.h>
#include <string.h>

void *malloc(size_t size)
{
	return heap_alloc(size);
}

void *zalloc(size_t size)
{
	void *p;

	p = heap_alloc(size);
	if (p)
		memset(p, 0, size);

	return p;
}

void free(void *p)
{
	heap_free(p);
}
