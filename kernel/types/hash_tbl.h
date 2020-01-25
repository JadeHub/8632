#pragma once

#include <stdint.h>

#include <kernel/types/list.h>

typedef struct hash_tbl
{
	size_t size;
	list_head_t* buckets;
}hash_tbl_t;

typedef struct hash_tbl_item
{
	uint32_t key;
	list_head_t list;
}hash_tbl_item_t;

hash_tbl_t* hash_tbl_create(size_t sz);
void has_tbl_destroy(hash_tbl_t* ht);
void hash_tbl_add(hash_tbl_t* ht, uint32_t key, hash_tbl_item_t* item);
bool hash_tbl_empty(hash_tbl_t* ht);
hash_tbl_item_t* hash_tbl_find(hash_tbl_t* ht, uint32_t key);
bool hash_tbl_has(hash_tbl_t* ht, uint32_t key);
void hash_tbl_delete_item(hash_tbl_t* ht, hash_tbl_item_t* item);
hash_tbl_item_t* hash_tbl_delete(hash_tbl_t* ht, uint32_t key);

#define hash_tbl_lookup(ht, key, type, member) ({		\
	container_of(hash_tbl_find(ht, key), type, member); })
