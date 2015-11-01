/*
 * togo_m_cache.c
 *
 *  Created on: 2015-10-20
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

static void togo_m_cache_create_area(uint32_t msize, uint32_t i,
		TOGO_M_CACHE_AREA * area);
static TOGO_M_CACHE_CHUNK * togo_m_cache_create_chunk(TOGO_M_CACHE_AREA * area);
static BOOL togo_m_cache_create_item(TOGO_THREAD_ITEM * socket_item,
		TOGO_M_CACHE_ITEM * item, TOGO_M_CACHE_AREA * area, uint32_t klen,
		uint32_t vlen, u_char * key, uint32_t expires);
static int32_t togo_m_cache_area_search(uint32_t * p, uint32_t size,
		uint32_t total);
static BOOL togo_m_cache_set_comm(TOGO_THREAD_ITEM * socket_item, u_char * key,
		uint32_t expires, uint32_t vlen);
static void togo_m_cache_set_cb(TOGO_THREAD_ITEM * socket_item);
static void togo_m_cache_get_cb(TOGO_THREAD_ITEM * socket_item);
static BOOL togo_m_cache_delete_comm(TOGO_M_CACHE_ITEM * item);
static int tries = 4;

BOOL togo_m_cache_command(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item, int ntag)
{
	u_char * action = NULL;
	u_char * key = NULL;
	u_char * temp;
	size_t vlen = 0;
	size_t expires = 0;
	BOOL ret = FALSE;

	/**
	 * command_tag[0] : Module  CACHE
	 * command_tag[1] : Action  SET|ADD|REPLACE|DELETE|GET|FLUSH
	 * command_tag[2] : Key     key
	 * command_tag[3] : Expires expires time
	 * command_tag[4] : Vlen    The length of value
	 */
	action = command_tag[1].value;
	if (ntag > 2) {
		key = command_tag[2].value;
	}
	if (ntag > 3) {
		temp = command_tag[3].value;
		if (command_tag[3].length > 0 && command_tag[3].length < 6) {
			expires = togo_atoi(temp, command_tag[3].length);
		}
	}
	if (ntag > 4) {
		temp = command_tag[4].value;
		if (command_tag[4].length > 0 && command_tag[4].length < 8) {
			vlen = togo_atoi(temp, command_tag[4].length);
		}
	}

	if (action == NULL) {
		return ret;
	}

	if (togo_strcmp(action, "SET") == 0) {
		if (key == NULL || vlen == 0) {
			return ret;
		}
		ret = togo_m_cache_set(socket_item, key, expires, vlen);

	} else if (togo_strcmp(action, "ADD") == 0) {
		if (key == NULL || vlen == 0) {
			return ret;
		}
		ret = togo_m_cache_add(socket_item, key, expires, vlen);

	} else if (togo_strcmp(action, "REPLACE") == 0) {
		if (key == NULL || vlen == 0) {
			return ret;
		}
		ret = togo_m_cache_replace(socket_item, key, expires, vlen);

	} else if (togo_strcmp(action, "DELETE") == 0) {
		if (key == NULL) {
			return ret;
		}
		ret = togo_m_cache_delete(socket_item, key);

	} else if (togo_strcmp(action, "GET") == 0) {
		if (key == NULL) {
			return ret;
		}
		ret = togo_m_cache_get(socket_item, key);

	} else if (togo_strcmp(action, "FLUSH") == 0) {

		ret = togo_m_cache_flush(socket_item);

	}

	return ret;
}

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
	togo_m_cache->is_flush = FALSE;
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
		togo_m_cache_create_area(curr, i, area);
	}

}

BOOL togo_m_cache_set(TOGO_THREAD_ITEM * socket_item, u_char * key,
		uint32_t expires, uint32_t vlen)
{
	TOGO_M_CACHE_ITEM * item;
	TOGO_HASHTABLE_ITEM * hitem;
	hitem = togo_hashtable_get(togo_m_cache_hashtable, key);
	if (hitem != NULL) {
		item = (TOGO_M_CACHE_ITEM *) hitem->p;
		togo_m_cache_delete_comm(item);
	}

	return togo_m_cache_set_comm(socket_item, key, expires, vlen);
}

BOOL togo_m_cache_add(TOGO_THREAD_ITEM * socket_item, u_char * key,
		uint32_t expires, uint32_t vlen)
{
	TOGO_M_CACHE_ITEM * item;
	TOGO_HASHTABLE_ITEM * hitem;

	hitem = togo_hashtable_get(togo_m_cache_hashtable, key);
	if (hitem != NULL) {
		item = (TOGO_M_CACHE_ITEM *) hitem->p;
		if (item->expires == 0 || item->expires > togo_get_time()) {
			togo_send_data(socket_item, TOGO_SBUF_EXIST,
					togo_strlen(TOGO_SBUF_EXIST));
			return TRUE;
		}

		togo_m_cache_delete_comm(item);
	}

	return togo_m_cache_set_comm(socket_item, key, expires, vlen);
}

BOOL togo_m_cache_replace(TOGO_THREAD_ITEM * socket_item, u_char * key,
		uint32_t expires, uint32_t vlen)
{
	TOGO_M_CACHE_ITEM * item;
	TOGO_HASHTABLE_ITEM * hitem;

	hitem = togo_hashtable_get(togo_m_cache_hashtable, key);
	if (hitem == NULL) {

		togo_send_data(socket_item, TOGO_SBUF_NOT_EXIST,
				togo_strlen(TOGO_SBUF_NOT_EXIST));
		return TRUE;

	} else {

		item = (TOGO_M_CACHE_ITEM *) hitem->p;
		if (item->expires != 0 && item->expires < togo_get_time()) {

			togo_m_cache_delete_comm(item);
			togo_send_data(socket_item, TOGO_SBUF_NOT_EXIST,
					togo_strlen(TOGO_SBUF_NOT_EXIST));
			return TRUE;
		}
		return TRUE;
	}

	return togo_m_cache_set_comm(socket_item, key, expires, vlen);
}

BOOL togo_m_cache_delete(TOGO_THREAD_ITEM * socket_item, u_char * key)
{
	TOGO_M_CACHE_ITEM * item;
	TOGO_HASHTABLE_ITEM * hitem;

	hitem = togo_hashtable_get(togo_m_cache_hashtable, key);
	if (hitem == NULL) {

		togo_send_data(socket_item, TOGO_SBUF_NOT_EXIST,
				togo_strlen(TOGO_SBUF_NOT_EXIST));
		return TRUE;
	}

	item = (TOGO_M_CACHE_ITEM *) hitem->p;
	return togo_m_cache_delete_comm(item);
}

BOOL togo_m_cache_get(TOGO_THREAD_ITEM * socket_item, u_char * key)
{
	TOGO_M_CACHE_ITEM * item;
	TOGO_HASHTABLE_ITEM * hitem;
	uint32_t vlen;
	void * buf;

	if (togo_m_cache->is_flush == TRUE) {
		return FALSE;
	}

	hitem = togo_hashtable_get(togo_m_cache_hashtable, key);
	if (hitem == NULL) {
		togo_send_data(socket_item, TOGO_SBUF_NOT_EXIST,
				togo_strlen(TOGO_SBUF_NOT_EXIST));
		return TRUE;
	}

	item = (TOGO_M_CACHE_ITEM *) hitem->p;
	if (item->expires != 0 && item->expires < togo_get_time()) {
		togo_m_cache_delete_comm(item);
		togo_send_data(socket_item, TOGO_SBUF_NOT_EXIST,
				togo_strlen(TOGO_SBUF_NOT_EXIST));
		return TRUE;
	}

	buf = (void *) item + sizeof(TOGO_M_CACHE_ITEM) + item->klen + 1;
	vlen = item->vlen;

	togo_send_dbig(socket_item, buf, vlen, togo_m_cache_get_cb, (void *) item);

	return TRUE;

}

BOOL togo_m_cache_flush(TOGO_THREAD_ITEM * socket_item)
{
	uint32_t total_area, i;
	TOGO_M_CACHE_AREA * area_curr;
	TOGO_M_CACHE_ITEM * temp;

	pthread_mutex_lock(&togo_m_cache->glock);
	togo_m_cache->is_flush = TRUE;

	togo_m_cache->total_hit = 0;
	togo_m_cache->total_read = 0;
	togo_m_cache->total_write = 0;

	total_area = togo_m_cache->total_area;

	for (i = 0; i < total_area; i++) {
		area_curr = (TOGO_M_CACHE_AREA *) (togo_m_cache->area + i);

		pthread_mutex_lock(&area_curr->lock);

		if (area_curr->lru_head != NULL && area_curr->lru_tail != NULL) {
			if (area_curr->free_list == NULL) {
				area_curr->free_list = area_curr->lru_head;
			} else {
				area_curr->lru_tail->next = area_curr->free_list;
				area_curr->free_list->prev = area_curr->lru_tail;
				area_curr->free_list = area_curr->lru_head;
			}
			area_curr->lru_head = NULL;
			area_curr->lru_tail = NULL;
		}

		area_curr->used_item = 0;
		area_curr->free_item = area_curr->total_item;

		pthread_mutex_unlock(&area_curr->lock);

	}

	togo_m_cache->is_flush = FALSE;
	pthread_mutex_unlock(&togo_m_cache->glock);

}

static void togo_m_cache_create_area(uint32_t msize, uint32_t i,
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
	chunk->p = p;

	togo_m_cache->total_size += TOGO_M_CACHE_CHUNK_SIZE;

	return chunk;

}

static BOOL togo_m_cache_create_item(TOGO_THREAD_ITEM * socket_item,
		TOGO_M_CACHE_ITEM * item, TOGO_M_CACHE_AREA * area, uint32_t klen,
		uint32_t vlen, u_char * key, uint32_t expires)
{
	u_char * new_key;
	u_char * new_val;
	u_char * nbsp;

	item->area = area;
	item->expires = (expires == 0) ? 0 : togo_get_time() + expires;
	item->klen = klen;
	item->vlen = vlen;
	item->next = NULL;
	item->prev = NULL;
	item->status = FALSE;

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
			togo_m_cache_set_cb, (void *) item);

	return TRUE;
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

static BOOL togo_m_cache_set_comm(TOGO_THREAD_ITEM * socket_item, u_char * key,
		uint32_t expires, uint32_t vlen)
{
	uint32_t klen, item_size, area_id, i, this_time;
	TOGO_M_CACHE_AREA * area;
	TOGO_M_CACHE_ITEM * item;
	TOGO_M_CACHE_CHUNK * chunk;
	TOGO_M_CACHE_ITEM * temp;

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

	if (togo_m_cache->is_flush == TRUE) {
		pthread_mutex_unlock(&area->lock);
		return FALSE;
	}

	if (area->chunk_item_curr < area->chunk_item_num) {

		chunk = area->chunk_curr;
		item = (TOGO_M_CACHE_ITEM *) ((TOGO_M_CACHE_ITEM *) chunk->p
				+ area->chunk_item_curr);
		togo_m_cache_create_item(socket_item, item, area, klen, vlen, key,
				expires);

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

			togo_m_cache_create_item(socket_item, item, area, klen, vlen, key,
					expires);

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

				pthread_mutex_unlock(&togo_m_cache->glock);

				item = (TOGO_M_CACHE_ITEM *) ((TOGO_M_CACHE_ITEM *) chunk->p
						+ area->chunk_item_curr);

				togo_m_cache_create_item(socket_item, item, area, klen, vlen,
						key, expires);

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

				togo_m_cache_create_item(socket_item, item, area, klen, vlen,
						key, expires);
			}

		}
	}

	togo_m_cache->total_write++;
	togo_m_cache->total_hit++;

	pthread_mutex_unlock(&area->lock);

	return TRUE;

}

static void togo_m_cache_set_cb(TOGO_THREAD_ITEM * socket_item)
{
	TOGO_M_CACHE_ITEM * item;
	u_char * new_key;

	item = (TOGO_M_CACHE_ITEM *) socket_item->bparam;
	item->status = 1;

	/* HashTable */
	new_key = (u_char *) item + sizeof(TOGO_M_CACHE_ITEM);
	togo_hashtable_add(togo_m_cache_hashtable, new_key, (void *) item);
}

static void togo_m_cache_get_cb(TOGO_THREAD_ITEM * socket_item)
{
	TOGO_M_CACHE_ITEM * item;

	item = (TOGO_M_CACHE_ITEM *) socket_item->bsparam;
	togo_m_cache->total_read++;
	togo_m_cache->total_hit++;
}

static BOOL togo_m_cache_delete_comm(TOGO_M_CACHE_ITEM * item)
{
	TOGO_M_CACHE_AREA * area;
	u_char * new_key;

	if (item == NULL) {
		return FALSE;
	}
	area = item->area;

	new_key = (u_char *) item + sizeof(TOGO_M_CACHE_ITEM);
	togo_hashtable_remove(togo_m_cache_hashtable, new_key);

	pthread_mutex_lock(&area->lock);

	if (togo_m_cache->is_flush == TRUE) {
		pthread_mutex_unlock(&area->lock);
		return FALSE;
	}
	if (item->status == 1) {
		pthread_mutex_unlock(&area->lock);
		return FALSE;
	}

	if (item->next != NULL) {
		item->next->prev = item->prev;
	}
	if (item->prev != NULL) {
		item->prev->next = item->next;
	}
	if (area->lru_head == item) {
		area->lru_head = item->next;
	}
	if (area->lru_tail == item) {
		area->lru_tail = item->prev;
	}

	if (area->free_list == NULL) {
		area->free_list = item;
		item->next = NULL;
		item->prev = NULL;
	} else {
		item->next = area->free_list;
		area->free_list->prev = item;
		area->free_list = item;
	}

	item->vlen = 0;
	item->klen = 0;
	item->expires = 0;
	item->status = 0;

	area->free_item++;
	area->used_item--;

	pthread_mutex_unlock(&area->lock);

	return TRUE;
}

