#ifndef __LIST_H__
#define __LIST_H__

#include <types.h>

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

#define LIST_NODE_INIT(name) { &(name), &(name) }

#define LIST_NODE(name) \
	struct list_node name = LIST_NODE_INIT(name)

static inline void INIT_LIST_NODE(struct list_node *list)
{
	list->next = list;
	list->prev = list;
}

#define list_next(node) ((node)->next)
#define list_prev(node) ((node)->prev)

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_node *new,
			      struct list_node *prev,
			      struct list_node *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @node: list node to add it after
 *
 * Insert a new entry after the specified node.
 * This is good for implementing stacks.
 */
static inline void list_add(struct list_node *new, struct list_node *node)
{
	__list_add(new, node, node->next);
}


/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @node: list node to add it before
 *
 * Insert a new entry before the specified node.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_node *new, struct list_node *node)
{
	__list_add(new, node->prev, node);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_node * prev, struct list_node * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void __list_del_entry(struct list_node *entry)
{
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_node *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void list_replace(struct list_node *old,
				struct list_node *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

static inline void list_replace_init(struct list_node *old,
					struct list_node *new)
{
	list_replace(old, new);
	INIT_LIST_NODE(old);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void list_del_init(struct list_node *entry)
{
	__list_del_entry(entry);
	INIT_LIST_NODE(entry);
}

/**
 * list_move - delete from one list and add as another's node
 * @list: the entry to move
 * @node: the node that will precede our entry
 */
static inline void list_move(struct list_node *list, struct list_node *node)
{
	__list_del_entry(list);
	list_add(list, node);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @node: the node that will follow our entry
 */
static inline void list_move_tail(struct list_node *list,
				  struct list_node *node)
{
	__list_del_entry(list);
	list_add_tail(list, node);
}

/**
 * list_is_last - tests whether @list is the last entry in list @node
 * @list: the entry to test
 * @node: the node of the list
 */
static inline int list_is_last(const struct list_node *list,
				const struct list_node *node)
{
	return list->next == node;
}

/**
 * list_is_first - tests whether @list is the first entry in list @node
 * @list: the entry to test
 * @node: the node of the list
 */
static inline int list_is_first(const struct list_node *list,
				const struct list_node *node)
{
	return list->prev == node;
}

/**
 * list_empty - tests whether a list is empty
 * @node: the list to test.
 */
static inline int list_empty(const struct list_node *node)
{
	return node->next == node;
}

/**
 * list_rotate_next - rotate the list with its next
 * @node: the node of the list
 */
static inline void list_rotate_next(struct list_node *node)
{
	struct list_node *next;

	if (!list_empty(node)) {
		next = node->next;
		list_move_tail(next, node);
	}
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @node: the list to test.
 */
static inline int list_is_singular(const struct list_node *node)
{
	return !list_empty(node) && (node->next == node->prev);
}

static inline void __list_cut_position(struct list_node *list,
		struct list_node *node, struct list_node *entry)
{
	struct list_node *new_first = entry->next;
	list->next = node->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	node->next = new_first;
	new_first->prev = node;
}

/**
 * list_cut_position - cut a list into two
 * @list: a new list to add all removed entries
 * @node: a list with entries
 * @entry: an entry within node, could be the node itself
 *	and if so we won't cut the list
 *
 * This helper moves the initial part of @node, up to and
 * including @entry, from @node to @list. You should
 * pass on @entry an element you know is on @node. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void list_cut_position(struct list_node *list,
		struct list_node *node, struct list_node *entry)
{
	if (list_empty(node))
		return;
	if (list_is_singular(node) &&
		(node->next != entry && node != entry))
		return;
	if (entry == node)
		INIT_LIST_NODE(list);
	else
		__list_cut_position(list, node, entry);
}

static inline void __list_splice(const struct list_node *list,
				 struct list_node *prev,
				 struct list_node *next)
{
	struct list_node *first = list->next;
	struct list_node *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @node: the place to add it in the first list.
 */
static inline void list_splice(const struct list_node *list,
				struct list_node *node)
{
	if (!list_empty(list))
		__list_splice(list, node, node->next);
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @node: the place to add it in the first list.
 */
static inline void list_splice_tail(struct list_node *list,
				struct list_node *node)
{
	if (!list_empty(list))
		__list_splice(list, node->prev, node);
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @node: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void list_splice_init(struct list_node *list,
				    struct list_node *node)
{
	if (!list_empty(list)) {
		__list_splice(list, node, node->next);
		INIT_LIST_NODE(list);
	}
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @node: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void list_splice_tail_init(struct list_node *list,
					 struct list_node *node)
{
	if (!list_empty(list)) {
		__list_splice(list, node->prev, node);
		INIT_LIST_NODE(list);
	}
}

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_node pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_prev_entry(ptr, type, member) \
	container_of((ptr)->prev, type, member)

#define list_next_entry(ptr, type, member) \
	container_of((ptr)->next, type, member)
/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list node to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_node to use as a loop cursor.
 * @node:	the node for your list.
 */
#define list_for_each(pos, node) \
	for (pos = (node)->next; pos != (node); pos = pos->next)

/**
 * __list_for_each	-	iterate over a list
 * @pos:	the &struct list_node to use as a loop cursor.
 * @node:	the node for your list.
 *
 * This variant doesn't differ from list_for_each() any more.
 * We don't do prefetching in either case.
 */
#define __list_for_each(pos, node) \
	for (pos = (node)->next; pos != (node); pos = pos->next)

/**
 * list_for_each_prev	-	iterate over a list backwards
 * @pos:	the &struct list_node to use as a loop cursor.
 * @node:	the node for your list.
 */
#define list_for_each_prev(pos, node) \
	for (pos = (node)->prev; pos != (node); pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:	the &struct list_node to use as a loop cursor.
 * @n:		another &struct list_node to use as temporary storage
 * @node:	the node for your list.
 */
#define list_for_each_safe(pos, n, node) \
	for (pos = (node)->next, n = pos->next; pos != (node); \
		pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &struct list_node to use as a loop cursor.
 * @n:		another &struct list_node to use as temporary storage
 * @node:	the node for your list.
 */
#define list_for_each_prev_safe(pos, n, node) \
	for (pos = (node)->prev, n = pos->prev; \
	     pos != (node); \
	     pos = n, n = pos->prev)

/**
 * list_for_each_entry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @node:	the node for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, node, member)				\
	for (pos = list_entry((node)->next, typeof(*pos), member);	\
	     &pos->member != (node); 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @node:	the node for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, node, member)			\
	for (pos = list_entry((node)->prev, typeof(*pos), member);	\
	     &pos->member != (node); 	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:	the type * to use as a start point
 * @node:	the node of the list
 * @member:	the name of the list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, node, member) \
	((pos) ? : list_entry(node, typeof(*pos), member))

/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @node:	the node for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, node, member) 		\
	for (pos = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (node);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @node:	the node for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, node, member)		\
	for (pos = list_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (node);	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @node:	the node for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, node, member) 			\
	for (; &pos->member != (node);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @node:	the node for your list.
 * @member:	the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, node, member)			\
	for (pos = list_entry((node)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (node); 					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_continue - continue list iteration safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @node:	the node for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, node, member) 		\
	for (pos = list_entry(pos->member.next, typeof(*pos), member), 		\
		n = list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (node);						\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_from - iterate over list from current point safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @node:	the node for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, node, member) 			\
	for (n = list_entry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (node);						\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @node:	the node for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, node, member)		\
	for (pos = list_entry((node)->prev, typeof(*pos), member),	\
		n = list_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (node); 					\
	     pos = n, n = list_entry(n->member.prev, typeof(*n), member))

/**
 * list_safe_reset_next - reset a stale list_for_each_entry_safe loop
 * @pos:	the loop cursor used in the list_for_each_entry_safe loop
 * @n:		temporary storage used in list_for_each_entry_safe
 * @member:	the name of the list_struct within the struct.
 *
 * list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define list_safe_reset_next(pos, n, member)				\
	n = list_entry(pos->member.next, typeof(*pos), member)

#endif
