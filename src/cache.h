#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
	char request[100];
	void *content;
	size_t content_size;
	uint64_t storage_time_ns;
	int max_age_seconds;
	uint64_t last_retrieved_ns;
} CacheEntry;

typedef struct {
	CacheEntry *entries;
	int capacity;
	int count;
} Cache;

Cache *cache_create(int capacity);
void cache_destroy(Cache *cache);

int cache_put(Cache *cache, const char *url, int max_age_seconds);
int cache_get(Cache *cache, const char *url);

int cache_store_content(Cache *cache, int index, const void *content,
			size_t content_size);
void *cache_get_content(Cache *cache, int index, size_t *content_size);
uint64_t cache_get_age_seconds(Cache *cache, int index);

#endif
