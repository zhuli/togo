/*
 * togo_m_cache.h
 *
 *  Created on: 2015-10-20
 *      Author: zhuli
 */

#ifndef TOGO_M_CACHE_H_
#define TOGO_M_CACHE_H_

#define TOGO_M_COUNTER_POOL_SIZE (1024 * 1024)
#define TOGO_M_CACHE_CHUNK_SIZE (1024 * 1024)
#define TOGO_M_CACHE_MAX_SIZE (TOGO_M_CACHE_CHUNK_SIZE * 1024)
#define TOGO_M_CACHE_ITEM_START 32
#define TOGO_M_CACHE_ITEM_POWER 1.15
#define TOGO_M_CACHE_AREA_TABLE_DEFAULT_SIZE 120

typedef struct togo_m_cache TOGO_M_CACHE;
typedef struct togo_m_cache_area TOGO_M_CACHE_AREA;
typedef struct togo_m_cache_chunk TOGO_M_CACHE_CHUNK;
typedef struct togo_m_cache_item TOGO_M_CACHE_ITEM;
#define togo_m_cache_item_size(a, b) \
		(a + 1 + b  + sizeof(TOGO_M_CACHE_ITEM))

struct togo_m_cache {
	TOGO_M_CACHE_AREA * area;
	uint32_t * area_table;

	uint32_t total_area;
	uint32_t total_size;
	uint64_t total_hit;
	uint64_t total_write;
	uint64_t total_read;

	pthread_mutex_t glock;
	TOGO_POOL * pool;
};

struct togo_m_cache_area {
	uint32_t msize;

	TOGO_M_CACHE_ITEM * lru_head;
	TOGO_M_CACHE_ITEM * lru_tail;
	TOGO_M_CACHE_ITEM * free_list;
	TOGO_M_CACHE_CHUNK * chunk_list;
	TOGO_M_CACHE_CHUNK * chunk_curr;

	uint32_t chunk_item_curr;
	uint32_t chunk_item_num;
	uint32_t total_size;
	uint32_t total_chunk;
	uint32_t total_item;
	uint32_t used_item;
	uint32_t free_item;
	pthread_mutex_t lock;
};

struct togo_m_cache_chunk {
	TOGO_M_CACHE_CHUNK * next;
	TOGO_M_CACHE_CHUNK * prev;
	TOGO_M_CACHE_AREA * area;
	u_char * p;
};

struct togo_m_cache_item {
	uint32_t klen;
	uint32_t vlen;
	uint64_t expires;
	BOOL status;

	TOGO_M_CACHE_ITEM * prev;
	TOGO_M_CACHE_ITEM * next;
	TOGO_M_CACHE_AREA * area;
};

BOOL togo_m_cache_command(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item, int ntag);
void togo_m_cache_init(void);
BOOL togo_m_cache_set(TOGO_THREAD_ITEM * socket_item, u_char * key,
		uint32_t expires, uint32_t vlen);

TOGO_POOL * togo_m_cache_pool;
TOGO_HASHTABLE * togo_m_cache_hashtable;
TOGO_M_CACHE * togo_m_cache;

#endif /* TOGO_M_CACHE_H_ */
