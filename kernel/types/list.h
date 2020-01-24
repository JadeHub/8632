#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct list_head
{
    struct list_head* next, * prev;
}list_head_t;

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	list_head_t name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(list_head_t* list)
{
    list->next = list;
    list->prev = list;
}

/*
Add item between prev and next
*/
static inline void _list_add(list_head_t* item,
    list_head_t* prev,
    list_head_t* next)
{
    next->prev = item;
    item->next = next;
    item->prev = prev;
    prev->next = item;
}

/*
Add item at start of list
*/
static inline void list_add(list_head_t* item, list_head_t* head)
{
    _list_add(item, head, head->next);
}

/*
Add item at end of list
*/
static inline void list_add_tail(list_head_t* item, list_head_t* head)
{
    _list_add(item, head->prev, head);
}

static inline void _list_delete_between(list_head_t* prev, list_head_t* next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void _list_delete_entry(list_head_t* item)
{
    _list_delete_between(item->prev, item->next);
}

static inline void list_delete(list_head_t* item)
{
    _list_delete_entry(item);
    item->next = item->prev = NULL;
}

static inline bool list_empty(list_head_t* list)
{
    return list->next == list;
}

/*
Return the offset of MEMBER within struct TYPE
*/
#define _offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/*
Return a pointer to the struct of which ptr is a member
*/
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

/*
Return the struct for the given item
ptr - list_head_t* of item
type - struct type
member - name of list_head_t item in struct
*/
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

/*
Given a Foo* return the next Foo*
*/
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

/*
Given a Foo* return the previous Foo*
*/
#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)

/*
For each
pos - list_head_t* to use for iteration
head - list head
*/
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_rev(pos, head) \
    for(pos = (head)->prev; pos != (head); pos = pos->prev)

/*
For each Foo
pos - Foo* to use for iteration
head - list_head
member - name of list_head_t item in struct
*/
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_first_entry(head, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = list_next_entry(pos, member))

