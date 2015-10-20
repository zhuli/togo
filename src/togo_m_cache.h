/*
 * togo_m_cache.h
 *
 *  Created on: 2015-10-20
 *      Author: zhuli
 */

#ifndef TOGO_M_CACHE_H_
#define TOGO_M_CACHE_H_


#define TOGO_M_CACHE_BUCKET_SIZE (1024 * 1024)
#define TOGO_M_CACHE_BUCKET_MAX_SIZE (TOGO_M_CACHE_BUCKET_SIZE * 1024)
#define TOGO_M_CACHE_ITEM_START 32
#define TOGO_M_CACHE_ITEM_POWER 1.5

typedef struct togo_m_cache TOGO_M_CACHE;
typedef struct togo_m_cache_area TOGO_M_CACHE_AREA;
typedef struct togo_m_cache_bucket TOGO_M_CACHE_BUCKET;
typedef struct togo_m_cache_item TOGO_M_CACHE_ITEM;
#define togo_m_cache_bucket_size() \
	(TOGO_M_CACHE_BUCKET_SIZE + sizeof(TOGO_M_CACHE_BUCKET))

struct togo_m_cache {
	TOGO_M_CACHE_AREA * area;
	uint32_t total_size;
	uint64_t total_hit;
	uint64_t total_write;
	uint64_t total_read;

	pthread_mutex_t glock;
	TOGO_POOL * pool;
};

struct togo_m_cache_area {
	size_t start;
	size_t len;

	TOGO_M_CACHE_ITEM * lru_head;
	TOGO_M_CACHE_ITEM * lru_tail;
	TOGO_M_CACHE_ITEM * free_list;

	TOGO_M_CACHE_BUCKET * bucket_list;
	TOGO_M_CACHE_BUCKET * bucket_curr;

	uint32_t curr;
	uint32_t total_size;
	uint32_t total_bucket;
	uint32_t total_item;
	uint32_t used_item;
	uint32_t free_item;
	pthread_mutex_t lock;

};

struct togo_m_cache_bucket {
	TOGO_M_CACHE_BUCKET * next;
	TOGO_M_CACHE_BUCKET * prev;
	TOGO_M_CACHE_AREA * area;
};

struct togo_m_cache_item {
	size_t klen;
	size_t vlen;
	uint32_t expires;
	BOOL use_lru; /* If not use lru, This item can't be deleted by running the lru*/

	TOGO_M_CACHE_ITEM * prev;
	TOGO_M_CACHE_ITEM * next;

	TOGO_M_CACHE_AREA * area;
};

#endif /* TOGO_M_CACHE_H_ */
