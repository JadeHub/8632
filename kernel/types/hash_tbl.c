#include "hash_tbl.h"

#include <kernel/memory/kmalloc.h>

static inline uint32_t _hash_fn(uint32_t value, size_t sz)
{
	return value % sz;
}

hash_tbl_t* hash_tbl_create(size_t sz)
{
	hash_tbl_t* ht = (hash_tbl_t*)kmalloc(sizeof(hash_tbl_t));

	ht->size = sz;
	ht->buckets = (list_head_t*)kmalloc(sizeof(list_head_t) * sz);

	for (size_t i = 0; i < sz; i++)
	{
		INIT_LIST_HEAD(&ht->buckets[i]);
	}
	return ht;
}

void has_tbl_destroy(hash_tbl_t* ht)
{
	kfree(ht->buckets);
	kfree(ht);
}
void hash_tbl_add(hash_tbl_t* ht, uint32_t key, hash_tbl_item_t* item)
{
	item->key = key;
	list_head_t* bucket = &ht->buckets[_hash_fn(key, ht->size)];	
	list_add(&item->list, bucket);
}

bool hash_tbl_empty(hash_tbl_t* ht)
{
	for (size_t i = 0; i < ht->size; i++)
	{
		if (!list_empty(&ht->buckets[i]))
			return false;
	}
	return true;
}

hash_tbl_item_t* hash_tbl_find(hash_tbl_t* ht, uint32_t key)
{
	list_head_t* bucket = &ht->buckets[_hash_fn(key, ht->size)];

	hash_tbl_item_t* item;
	list_for_each_entry(item, bucket, list)
	{
		if (item->key == key)
		{
			return item;
		}
	}
	return NULL;
}

bool hash_tbl_has(hash_tbl_t* ht, uint32_t key)
{
	return hash_tbl_find(ht, key) != NULL;
}

void hash_tbl_delete_item(hash_tbl_t* ht, hash_tbl_item_t* item)
{
	list_delete(&item->list);
}

void hash_tbl_delete(hash_tbl_t* ht, uint32_t key)
{
	hash_tbl_item_t* item = hash_tbl_find(ht, key);
	if (item)
		hash_tbl_delete_item(ht, item);
}
