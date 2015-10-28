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
static u_char * togo_m_cache_create_item(TOGO_THREAD_ITEM * socket_item,
		TOGO_M_CACHE_ITEM * item, TOGO_M_CACHE_AREA * area, uint32_t klen,
		uint32_t vlen, u_char * key, uint32_t expires);
static int32_t togo_m_cache_area_search(uint32_t * p, uint32_t size,
		uint32_t total);
static void togo_m_cache_set_cb(TOGO_THREAD_ITEM * socket_item);
static int tries = 4;

void togo_m_cache_init(void)
{
	uint32_t total_area, curr, i;
	uint32_t * area_table;
	TOGO_M_CACHE_AREA * area;

	togo_m_cache_pool = togo_pool_create(TOGO_M_COUNTER_POOL_SIZE);
	if (togo_m_cache_pool == NULL) {
		togo_log(ERROR, "Initialize modules_cache's POOL fail.");
		togo_exit();
	}

	togo_m_cache_hashtable = togo_hashtable_init(togo_m_cache_pool);
	if (togo_m_cache_hashtable == NULL) {
		togo_log(ERROR, "Initialize modules_cache's HASHTABLE fail.");
		togo_exit();
	}

	togo_m_cache = (TOGO_M_CACHE *) togo_pool_calloc(togo_m_cache_pool,
			sizeof(TOGO_M_CACHE));
	if (togo_m_cache == NULL) {
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

BOOL togo_m_cache_set(TOGO_THREAD_ITEM * socket_item, u_char * key,
		uint32_t expires, uint32_t vlen)
{
	uint32_t klen, item_size, area_id, i, this_time;
	TOGO_M_CACHE_AREA * area;
	TOGO_M_CACHE_ITEM * item;
	TOGO_M_CACHE_CHUNK * chunk;
	TOGO_M_CACHE_ITEM * temp;
	u_char * new_key;

	klen = togo_strlen(key);
	if (klen == 0 || vlen == 0 || togo_m_cache->area == NULL) {
		return FALSE;
	}

	item_size = togo_m_cache_item_size(klen, vlen);
	area_id = togo_m_cache_area_search(togo_m_cache->area_table, item_size,
			togo_m_cache->total_area);
	if (area_id == -1) {
		return FALSE;
	}

	area = (TOGO_M_CACHE_AREA *) (togo_m_cache->area + area_id);

	pthread_mutex_lock(&area->lock);

	if (area->chunk_item_curr < area->chunk_item_num) {

		chunk = area->chunk_curr;
		item = (TOGO_M_CACHE_ITEM *) (chunk->p
				+ (area->msize * area->chunk_item_curr));

		new_key = togo_m_cache_create_item(socket_item, item, area, klen, vlen,
				key, expires);

		area->chunk_item_curr++;

	} else {

		if (area->free_list != NULL) {
			item = area->free_list;
			if (area->free_list->next == NULL) {
				area->free_list = NULL;
			} else {
				area->free_list->next->prev = NULL;
				area->free_list = area->free_list->next;
			}

			new_key = togo_m_cache_create_item(socket_item, item, area, klen,
					vlen, key, expires);

		} else {
			/* Alloc a new chunk*/
			pthread_mutex_lock(&togo_m_cache->glock);

			if (togo_m_cache->total_size < TOGO_M_CACHE_MAX_SIZE) {

				chunk = togo_m_cache_create_chunk(area);
				if (chunk == NULL) {
					pthread_mutex_unlock(&area->lock);
					return FALSE;
				}

				if (area->chunk_list == NULL) {
					area->chunk_list = chunk;
				} else {
					area->chunk_list->prev = chunk;
					chunk->next = area->chunk_list;
					area->chunk_list = chunk;
				}
				area->chunk_curr = chunk;
				area->chunk_item_curr = 0;

				area->total_chunk++;
				area->total_size += TOGO_M_CACHE_CHUNK_SIZE;
				area->total_item += area->chunk_item_num;
				area->free_item += area->chunk_item_num;

				pthread_mutex_unlock(&togo_m_cache->glock);

				item = (TOGO_M_CACHE_ITEM *) (chunk->p
						+ (area->msize * area->chunk_item_curr));

				new_key = togo_m_cache_create_item(socket_item, item, area,
						klen, vlen, key, expires);

			} else {
				pthread_mutex_unlock(&togo_m_cache->glock);

				/* LRU */
				if (area->lru_tail == NULL) {
					pthread_mutex_unlock(&area->lock);
					return FALSE;
				}

				this_time = togo_get_time();
				item = NULL;
				temp = area->lru_tail;
				for (i = 0; i < tries; i++) {
					if (temp == NULL) {
						break;
					}
					if (temp->expires != 0 && temp->expires < this_time) {
						break;
					}
					temp = temp->prev;
				}
				if (temp == NULL || temp->expires == 0
						|| temp->expires > this_time) {
					item = (TOGO_M_CACHE_ITEM *) area->lru_tail;
					area->lru_tail = area->lru_tail->prev;
				} else {
					item = temp;
					if (temp->next != NULL) {
						temp->next->prev = temp->prev;
					}
					if (temp->prev != NULL) {
						temp->prev->next = temp->next;
					}
					if (area->lru_head == temp) {
						area->lru_head = temp->next;
					}
					if (area->lru_tail == temp) {
						area->lru_tail = temp->prev;
					}
				}

				new_key = togo_m_cache_create_item(socket_item, item, area,
						klen, vlen, key, expires);
			}

		}
	}

	pthread_mutex_unlock(&area->lock);

	/* HashTable */
	togo_hashtable_add(togo_m_cache_hashtable, new_key, (void *) item);
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
		togo_log(INFO, "Initialize modules_cache's TOGO_M_CACHE_CHUNK fail.");
		togo_exit();
	}
	chunk_item_size = TOGO_M_CACHE_CHUNK_SIZE / msize;

	curr_area->msize = msize;

	curr_area->lru_head = NULL;
	curr_area->lru_tail = NULL;
	curr_area->free_list = NULL;
	curr_area->chunk_list = chunk;
	curr_area->chunk_curr = chunk;

	curr_area->chunk_item_curr = 0;
	curr_area->chunk_item_num = chunk_item_size;

	curr_area->total_size = TOGO_M_CACHE_CHUNK_SIZE;
	curr_area->total_chunk = 1;
	curr_area->total_item = chunk_item_size;
	curr_area->used_item = 0;
	curr_area->free_item = chunk_item_size;

	togo_m_cache->total_size += TOGO_M_CACHE_CHUNK_SIZE;

	pthread_mutex_init(&curr_area->lock, NULL);
}

static TOGO_M_CACHE_CHUNK * togo_m_cache_create_chunk(TOGO_M_CACHE_AREA * area)
{
	TOGO_M_CACHE_CHUNK * chunk;
	u_char * p;

	chunk = (TOGO_M_CACHE_CHUNK *) togo_pool_calloc(togo_m_cache_pool,
			sizeof(TOGO_M_CACHE_CHUNK));
	if (chunk == NULL) {
		togo_log(INFO, "create modules_cache's chunk fail.");
		return NULL;
	}

	p = (u_char *) togo_pool_alloc(togo_m_cache_pool,
			sizeof(u_char) * TOGO_M_CACHE_CHUNK_SIZE);
	if (p == NULL) {
		togo_log(INFO, "create modules_cache's chunk fail.");
		return NULL;
	}

	chunk->area = area;
	chunk->next = NULL;
	chunk->prev = NULL;
	chunk->p;

	return chunk;

}

static u_char * togo_m_cache_create_item(TOGO_THREAD_ITEM * socket_item,
		TOGO_M_CACHE_ITEM * item, TOGO_M_CACHE_AREA * area, uint32_t klen,
		uint32_t vlen, u_char * key, uint32_t expires)
{
	u_char * new_key;
	u_char * new_val;
	u_char * nbsp;

	item->area = area;
	item->expires = togo_get_time() + expires;
	item->klen = klen;
	item->vlen = vlen;
	item->next = NULL;
	item->prev = NULL;

	new_key = (u_char *) item + sizeof(TOGO_M_CACHE_ITEM);
	nbsp = (u_char *) new_key + klen + 1;
	*(nbsp) = '\0';
	new_val = nbsp + 1;
	togo_memcpy(new_key, key, klen);

	area->total_item++;
	area->used_item++;
	area->free_item--;

	if (area->lru_head == NULL) {
		area->lru_head = item;
		area->lru_tail = item;
	} else {
		area->lru_head->prev = item;
		item->next = area->lru_head;
		area->lru_head = item;
	}

	togo_read_data(socket_item, togo_m_cache->pool, new_val, vlen,
			togo_m_cache_set_cb);
}

static int32_t togo_m_cache_area_search(uint32_t * p, uint32_t size,
		uint32_t total)
{
	//binary_search
	uint32_t low, high, mid, curr, pre;
	int32_t ret;

	if (size <= TOGO_M_CACHE_CHUNK_SIZE) {
		return 0;
	}
	if (size > TOGO_M_CACHE_MAX_SIZE) {
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

static void togo_m_cache_set_cb(TOGO_THREAD_ITEM * socket_item)
{

}

