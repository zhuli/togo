/*
 * togo_m_count.c
 *
 *  Created on: 2015-9-9
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

static TOGO_M_COUNT * togo_m_queue_get(u_char * name);

void togo_m_count_init(void)
{
	togo_m_count_pool = togo_pool_create(TOGO_M_QUEUE_POOL_SIZE);
	if (togo_m_count_pool == NULL) {
		togo_log(ERROR, "Initialize modules_count's pool fail.");
		togo_exit();
	}

	togo_m_count_hashtable = togo_hashtable_init(togo_m_count_pool);
	if (togo_m_count_hashtable == NULL) {
		togo_log(ERROR, "Initialize modules_count's count fail.");
		togo_exit();
	}

	pthread_mutex_init(&togo_m_count_glock, NULL);
}

BOOL togo_m_count_plus(u_char * name, uint32_t step)
{
	TOGO_M_COUNT * item;

	item = togo_m_queue_get(name);
	if (item == NULL) {
		return FALSE;
	}

	pthread_mutex_lock(&item->lock);

	item->count += step;

	pthread_mutex_unlock(&item->lock);

	return TRUE;
}

BOOL togo_m_count_minus(u_char * name, uint32_t step)
{
	TOGO_M_COUNT * item;

	item = togo_m_queue_get(name);
	if (item == NULL) {
		return FALSE;
	}

	pthread_mutex_lock(&item->lock);

	item->count -= step;

	pthread_mutex_unlock(&item->lock);

	return TRUE;
}

BOOL togo_m_count_clear(u_char * name)
{
	TOGO_M_COUNT * item;
	TOGO_POOL * pool;

	item = togo_m_queue_get(name);
	if (item == NULL) {
		return FALSE;
	}

	pthread_mutex_lock(&togo_m_count_glock);
	togo_hashtable_remove(togo_m_count_hashtable, name);
	pool = item->pool;
	togo_pool_free_data(pool, (void *) item->name);
	pthread_mutex_unlock(&togo_m_queue_glock);
}

static TOGO_M_COUNT * togo_m_queue_get(u_char * name)
{
	TOGO_M_COUNT * item;

	item = togo_hashtable_get(togo_m_count_hashtable, name);
	if (item == NULL) {
		/* if does not find a count item, we need to create a new queue!*/
		pthread_mutex_lock(&togo_m_count_glock);

		item = togo_hashtable_get(togo_m_count_hashtable, name);
		if (item == NULL) {
			item = (TOGO_M_COUNT *) togo_pool_calloc(togo_m_count_pool,
					sizeof(TOGO_M_COUNT));
			if (item == NULL) {
				pthread_mutex_unlock(&togo_m_count_glock);
				return NULL;
			}

			u_char * buf = (u_char *) togo_pool_alloc(togo_m_count_pool,
					togo_pool_strlen(name));
			if (buf == NULL) {
				pthread_mutex_unlock(&togo_m_count_glock);
				return NULL;
			}
			togo_strcpy(buf, name);

			pthread_mutex_init(&item->lock);
			item->pool = togo_m_count_pool;
			item->name = buf;
			item->count = 0;

			BOOL ret = togo_hashtable_add(togo_m_count_hashtable, name,
					(void *) item);
			if (ret == FALSE) {
				pthread_mutex_unlock(&togo_m_queue_glock);
				return NULL;
			}

			item = togo_hashtable_get(togo_m_count_hashtable, name);
		}

		pthread_mutex_unlock(&togo_m_queue_glock);
	}

	return item;
}
