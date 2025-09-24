#include "cache.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>

#define DEFAULT_MAX_AGE_SECONDS 3600
#define NS_PER_SECOND 1000000000ULL

static uint64_t get_current_time_ns(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (uint64_t)ts.tv_sec * NS_PER_SECOND + (uint64_t)ts.tv_nsec;
}

static int is_fresh(CacheEntry *entry)
{
	uint64_t current_time_ns = get_current_time_ns();
	uint64_t age_seconds =
		(current_time_ns - entry->storage_time_ns) / NS_PER_SECOND;
	return age_seconds < (uint64_t)entry->max_age_seconds;
}

static int find_entry_index(Cache *cache, const char *url)
{
	for (int i = 0; i < cache->count; i++) {
		if (strcmp(cache->entries[i].request, url) == 0) {
			return i;
		}
	}
	return -1;
}

static int find_stale_entry_index(Cache *cache)
{
	for (int i = 0; i < cache->count; i++) {
		if (!is_fresh(&cache->entries[i])) {
			return i;
		}
	}
	return -1;
}

static int find_lru_entry_index(Cache *cache)
{
	if (cache->count == 0)
		return -1;

	int lru_index = 0;
	uint64_t oldest_access = cache->entries[0].last_retrieved_ns;

	for (int i = 1; i < cache->count; i++) {
		if (cache->entries[i].last_retrieved_ns < oldest_access) {
			oldest_access = cache->entries[i].last_retrieved_ns;
			lru_index = i;
		}
	}
	return lru_index;
}

static void evict_entry(Cache *cache, int index)
{
	assert(index >= 0 && index < cache->count);

	free(cache->entries[index].content);

	for (int i = index; i < cache->count - 1; i++) {
		cache->entries[i] = cache->entries[i + 1];
	}
	cache->count--;
}

Cache *cache_create(int capacity)
{
	Cache *cache = malloc(sizeof(Cache));
	if (!cache)
		return NULL;

	cache->entries = malloc(capacity * sizeof(CacheEntry));
	if (!cache->entries) {
		free(cache);
		return NULL;
	}

	cache->capacity = capacity;
	cache->count = 0;

	return cache;
}

void cache_destroy(Cache *cache)
{
	if (!cache)
		return;

	for (int i = 0; i < cache->count; i++) {
		free(cache->entries[i].content);
	}

	free(cache->entries);
	free(cache);
}

int cache_get(Cache *cache, const char *url)
{
	if (!cache || !url)
		return -1;

	int index = find_entry_index(cache, url);
	if (index == -1)
		return -1;

	CacheEntry *entry = &cache->entries[index];
	entry->last_retrieved_ns = get_current_time_ns();

	if (is_fresh(entry)) {
		return index;
	} else {
		return -2;
	}
}

int cache_put(Cache *cache, const char *url, int max_age_seconds)
{
	if (!cache || !url)
		return -1;

	if (max_age_seconds <= 0) {
		max_age_seconds = DEFAULT_MAX_AGE_SECONDS;
	}

	int existing_index = find_entry_index(cache, url);
	if (existing_index != -1) {
		return existing_index;
	}

	int insert_index;

	if (cache->count < cache->capacity) {
		insert_index = cache->count;
		cache->count++;
	} else {
		int stale_index = find_stale_entry_index(cache);
		if (stale_index != -1) {
			evict_entry(cache, stale_index);
			insert_index = cache->count;
			cache->count++;
		} else {
			int lru_index = find_lru_entry_index(cache);
			evict_entry(cache, lru_index);
			insert_index = cache->count;
			cache->count++;
		}
	}

	CacheEntry *entry = &cache->entries[insert_index];
	strncpy(entry->request, url, sizeof(entry->request) - 1);
	entry->request[sizeof(entry->request) - 1] = '\0';

	uint64_t current_time = get_current_time_ns();
	entry->storage_time_ns = current_time;
	entry->last_retrieved_ns = current_time;
	entry->max_age_seconds = max_age_seconds;
	entry->content = NULL;
	entry->content_size = 0;

	return insert_index;
}

int cache_store_content(Cache *cache, int index, const void *content,
			size_t content_size)
{
	if (!cache || index < 0 || index >= cache->count || !content)
		return -1;

	CacheEntry *entry = &cache->entries[index];

	if (entry->content) {
		free(entry->content);
	}

	entry->content = malloc(content_size);
	if (!entry->content)
		return -1;

	memcpy(entry->content, content, content_size);
	entry->content_size = content_size;

	return 0;
}

void *cache_get_content(Cache *cache, int index, size_t *content_size)
{
	if (!cache || index < 0 || index >= cache->count || !content_size)
		return NULL;

	CacheEntry *entry = &cache->entries[index];
	*content_size = entry->content_size;
	return entry->content;
}

uint64_t cache_get_age_seconds(Cache *cache, int index)
{
	if (!cache || index < 0 || index >= cache->count)
		return 0;

	CacheEntry *entry = &cache->entries[index];
	uint64_t current_time_ns = get_current_time_ns();
	return (current_time_ns - entry->storage_time_ns) / NS_PER_SECOND;
}
