/*
 * togo_m_cache.c
 *
 *  Created on: 2015-10-20
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

static void togo_m_cache_init_area(uint32_t msize, uint32_t i,
		TOGO_M_CACHE_AREA * area);
static TOGO_M_CACHE_CHUNK * togo_m_cache_create_chunk(TOGO_M_CACHE_AREA * area);
static int32_t togo_m_cache_area_search(uint32_t * p, uint32_t size,
		uint32_t total);

void togo_m_cache_init(void)
{
	uint32_t total_area, curr, i;
	uint32_t * area_table;
	TOGO_M_CACHE_AREA * area;

	togo_m_cache_pool = togo_pool_create(TOGO_M_COUNTER_POOL_SIZE);

	if (togo_m_cache_pool == NULL) {
		togo_log(ERROR, "Initialize modules_cache's pool fail.");
		togo_exit();
	}

	togo_m_cache_hashtable = togo_hashtable_init(togo_m_cache_pool);
	if (togo_m_cache_hashtable == NULL) {
		togo_log(ERROR, "Initialize modules_cache's hashtable fail.");
		togo_exit();
	}

	togo_m_cache = (TOGO_M_CACHE *) togo_pool_calloc(togo_m_cache_pool,
			sizeof(TOGO_M_CACHE));
	if (togo_m_cache_hashtable == NULL) {
		togo_log(ERROR, "Initialize modules_cache's TOGO_M_CACHE fail.");
		togo_exit();
	}

	togo_m_cache->pool = togo_m_cache_pool;
	togo_m_cache->total_area = 0;
	togo_m_cache->total_hit = 0;
	togo_m_cache->total_read = 0;
	togo_m_cache->total_size = 0;
	togo_m_cache->total_write = 0;
	pthread_mutex_init(&togo_m_cache->glock, NULL);

	/* Initialize the area table!*/
	area_table = (uint32_t *) togo_pool_alloc(togo_m_cache_pool,
			sizeof(uint32_t) * TOGO_M_CACHE_AREA_TABLE_DEFAULT_SIZE);
	if (area_table == NULL) {
		togo_log(ERROR, "Initialize modules_cache's area_table fail.");
		togo_exit();
	}
	togo_m_cache->area_table = area_table;

	total_area = i = 1;
	curr = TOGO_M_CACHE_ITEM_START;
	*area_table = curr;
	while (curr != TOGO_M_CACHE_CHUNK_SIZE) {

		total_area++;
		curr = (uint32_t)(curr * TOGO_M_CACHE_ITEM_POWER);
		if (curr >= TOGO_M_CACHE_CHUNK_SIZE) {
			curr = TOGO_M_CACHE_CHUNK_SIZE;
		}

		*(area_table + i) = curr;
		i++;
	}
	togo_m_cache->total_area = total_area;

	/* Initialize the memory of area!*/
	area = (TOGO_M_CACHE_AREA *) togo_pool_calloc(togo_m_cache_pool,
			sizeof(TOGO_M_CACHE_AREA) * total_area);
	if (area == NULL) {
		togo_log(ERROR, "Initialize modules_cache's TOGO_M_CACHE_AREA fail.");
		togo_exit();
	}
	togo_m_cache->area = area;

	for (i = 0; i < total_area; i++) {
		curr = *(area_table + i);
		togo_m_cache_init_area(curr, i, area);
	}

}

static void togo_m_cache_init_area(uint32_t msize, uint32_t i,
		TOGO_M_CACHE_AREA * area)
{
	TOGO_M_CACHE_AREA * curr_area;
	TOGO_M_CACHE_CHUNK * chunk;
	uint32_t chunk_item_size;

	curr_area = (TOGO_M_CACHE_AREA *) (area + i);

	chunk = togo_m_cache_create_chunk(curr_area);
	if (chunk == NULL) {
		togo_exit();
	}
	chunk_item_size = TOGO_M_CACHE_CHUNK_SIZE / msize;

	curr_area->msize = msize;

	curr_area->lru_head = NULL;
	curr_area->lru_tail = NULL;
	curr_area->free_list = NULL;
	curr_area->chunk_list = chunk;
	curr_area->chunk_curr = chunk;

	curr_area->curr = 0;
	curr_area->total_size = TOGO_M_CACHE_CHUNK_SIZE;
	curr_area->total_chunk = 1;
	curr_area->total_item = chunk_item_size;
	curr_area->chunk_item_size = chunk_item_size;
	curr_area->used_item = 0;
	curr_area->free_item = chunk_item_size;

	pthread_mutex_init(&curr_area->lock, NULL);
}

static TOGO_M_CACHE_CHUNK * togo_m_cache_create_chunk(TOGO_M_CACHE_AREA * area)
{
	TOGO_M_CACHE_CHUNK * bucket;
	u_char * p;

	bucket = (TOGO_M_CACHE_CHUNK *) togo_pool_calloc(togo_m_cache_pool,
			sizeof(TOGO_M_CACHE_CHUNK));
	if (bucket == NULL) {
		togo_log(INFO, "Initialize modules_cache's TOGO_M_CACHE_CHUNK fail.");
		return NULL;
	}

	p = (u_char *) togo_pool_alloc(togo_m_cache_pool,
			sizeof(u_char) * TOGO_M_CACHE_CHUNK_SIZE);
	if (p == NULL) {
		togo_log(INFO, "Initialize modules_cache's TOGO_M_CACHE_CHUNK fail.");
		return NULL;
	}

	bucket->area = area;
	bucket->next = NULL;
	bucket->prev = NULL;
	bucket->p;

	return bucket;

}

static int32_t togo_m_cache_area_search(uint32_t * p, uint32_t size,
		uint32_t total)
{
	//binary_search
	uint32_t low, high, mid, curr, pre;
	int32_t ret;

	if (size <= 32) {
		return 0;
	}
	if (size > (1024 * 1024)) {
		return -1;
	}

	low = 0;
	high = (total - 1);

	while (low <= high) {
		if (low == (high - 1)) {
			curr = *(p + high);
			pre = *(p + low);
			if (size > pre && size <= curr) {
				ret = curr;
				break;
			}
			ret = -1;
			break;

		} else {
			mid = (low + high) / 2;
			curr = *(p + mid);
			pre = *(p + (mid - 1));
			if (size > pre && size <= curr) {
				ret = mid;
				break;
			}

			if (size > curr) {
				low = mid; //in the right
			} else {
				high = mid; //in the left
			}
		}
	}

	return ret;
}
