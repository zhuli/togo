/*
 * togo_m_cache.c
 *
 *  Created on: 2015-10-20
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

uint32_t togo_m_cache_init_area(void);

void togo_m_cache_init(void)
{
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

	togo_m_cache = togo_pool_calloc(togo_m_cache_pool, sizeof(TOGO_M_CACHE));
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

	togo_m_cache_init_area();
}

uint32_t togo_m_cache_init_area(void)
{
	uint32_t total_area, start, curr, i;
	TOGO_M_CACHE_AREA * area;

	total_area = 0;
	curr = TOGO_M_CACHE_ITEM_START;

	while (curr != TOGO_M_CACHE_BUCKET_MAX_SIZE) {
		total_area++;
		curr = (uint32_t)(curr * TOGO_M_CACHE_ITEM_POWER);
		if (curr >= TOGO_M_CACHE_BUCKET_MAX_SIZE) {
			curr = TOGO_M_CACHE_BUCKET_MAX_SIZE;
		}
	}

	area = togo_pool_calloc(togo_m_cache_pool, sizeof(TOGO_M_CACHE_AREA) * total_area);
	if (area == NULL) {
		togo_log(ERROR, "Initialize modules_cache's TOGO_M_CACHE_AREA fail.");
		togo_exit();
	}
	togo_m_cache->area = area;

	while (curr != TOGO_M_CACHE_BUCKET_MAX_SIZE) {
		total_area++;
		curr = (uint32_t)(curr * TOGO_M_CACHE_ITEM_POWER);
		if (curr >= TOGO_M_CACHE_BUCKET_MAX_SIZE) {
			curr = TOGO_M_CACHE_BUCKET_MAX_SIZE;
		}
	}
}

