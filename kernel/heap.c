#include <common.h>
#include <sys/heap.h>
#include <list.h>

#if defined(CONFIG_HEAP_BASE)
#if !defined(CONFIG_HEAP_SIZE)
#error "CONFIG_HEAP_SIZE not defined!"
#endif
#define HEAP_BASE	CONFIG_HEAP_BASE
#define HEAP_SIZE	CONFIG_HEAP_SIZE

#else
extern int _end;
extern int _end_of_ram;

#define HEAP_BASE	((unsigned long)&_end)
#define HEAP_SIZE	((size_t)&_end_of_ram - (size_t)&_end)
#endif

#define HEAP_MAGIC	'HEAP'

struct heap_chunk {
	u32			magic;
	size_t			size;
	struct list_node	node;
};

static LIST_NODE(heap_free_list);
static LIST_NODE(heap_allo_list);

static struct heap_chunk *heap_create_chunk(void *base, size_t size)
{
	struct heap_chunk *chunk;

	chunk = (struct heap_chunk *)base;
	chunk->size = size;

	return chunk;
}

static struct heap_chunk *heap_free_chunk(struct heap_chunk *chunk)
{
	struct heap_chunk *pos;

	chunk->magic = 0;

	list_for_each_entry(pos, &heap_free_list, node) {
		if (chunk < pos) {
			list_add_tail(&chunk->node, &pos->node);
			goto try_merge;
		}
	}
	list_add_tail(&chunk->node, &heap_free_list);

try_merge:
	if (!list_is_last(&chunk->node, &heap_free_list)) {
		if ((addr_t)chunk + chunk->size == (addr_t)pos) {
			chunk->size += pos->size;
			list_del(&pos->node);
		}
	}

	if (!list_is_first(&chunk->node, &heap_free_list)) {
		pos = list_prev_entry(&chunk->node,
				typeof(struct heap_chunk), node);
		if ((addr_t)pos + pos->size == (addr_t)chunk) {
			pos->size += chunk->size;
			list_del(&chunk->node);
		}
	}

	return chunk;
}

static struct heap_chunk *heap_alloc_chunk(size_t size)
{
	struct heap_chunk *chunk, *next, *new;

	list_for_each_entry_safe(chunk, next, &heap_free_list, node) {
		if (chunk->size >= size) {
			list_del(&chunk->node);

			if (chunk->size > size + sizeof(struct heap_chunk)) {
				new = heap_create_chunk((u8 *)chunk + size,
							chunk->size - size);
				list_add_tail(&new->node, &next->node);
				chunk->size = size;
			}
			return chunk;
		}
	}

	return NULL;
}

void *heap_alloc(size_t size)
{
	struct heap_chunk *chunk, *pos;

	size += sizeof(struct heap_chunk);
	size = roundup(size, sizeof(void *));

	chunk = heap_alloc_chunk(size);
	if (chunk == NULL)
		return NULL;

	chunk->magic = HEAP_MAGIC;

	list_for_each_entry(pos, &heap_allo_list, node) {
		if (chunk < pos) {
			list_add(&chunk->node, &pos->node);
			goto done;
		}
	}
	list_add_tail(&chunk->node, &heap_allo_list);

done:
	return (u8 *)chunk + sizeof(struct heap_chunk);
}

void heap_free(void *p)
{
	struct heap_chunk *chunk;

       	chunk = (struct heap_chunk *)p;
	chunk--;

	if (chunk->magic != HEAP_MAGIC) {
		printf(PR_ERR "Invalid heap pointer: %8p\n", chunk);
		return;
	}

	list_del(&chunk->node);

	heap_free_chunk(chunk);
}

int heap_init(void)
{
	struct heap_chunk *first_chunk;

	first_chunk = heap_create_chunk((void *)HEAP_BASE, (size_t)HEAP_SIZE);
	heap_free_chunk(first_chunk);

	return 0;
}
