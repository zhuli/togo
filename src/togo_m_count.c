/*
 * togo_m_count.c
 *
 *  Created on: 2015-9-9
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

static TOGO_M_COUNT * togo_m_queue_get(u_char * name);

BOOL togo_m_count_command(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item, int ntag)
{
	BOOL ret = FALSE;
	u_char * action = NULL;
	u_char * cname = NULL;
	uint32_t step = 1;

	/**
	 * command_tag[0] : Module  C
	 * command_tag[1] : Action  PLUS|MINUS|CLEAR
	 * command_tag[2] : Object  Queue name
	 * command_tag[3] : Step    1  MAX:99999999
	 */
	action = command_tag[1].value;
	cname = command_tag[2].value;

	if (ntag > 3) {
		if (command_tag[3].length > 0) {
			int index = togo_strlen(command_tag[3].value);
			if (index > 8) {
				index = 8;
			}
			step = togo_atoi(command_tag[3].value, index);
			if (step < 1) {
				return FALSE;
			}
		}
	}

	if (action == NULL || cname == NULL) {
		return ret;
	}

	if (togo_strcmp(action, "PLUS") == 0) {
		ret = togo_m_count_plus(cname, step, socket_item);

	} else if (togo_strcmp(command_tag[1].value, "MINUS") == 0) {
		ret = togo_m_count_minus(cname, step, socket_item);

	} else if (togo_strcmp(command_tag[1].value, "CLEAR") == 0) {
		ret = togo_m_count_clear(cname);

	}

	return ret;
}

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

BOOL togo_m_count_plus(u_char * name, uint32_t step,
		TOGO_THREAD_ITEM *socket_item)
{
	TOGO_M_COUNT * item;
	char str[20];

	item = togo_m_queue_get(name);
	if (item == NULL) {
		return FALSE;
	}

	pthread_mutex_lock(&item->lock);

	item->count += step;
	togo_itoa(item->count, str, 10);
	togo_send_data(socket_item, str, togo_strlen(str));

	pthread_mutex_unlock(&item->lock);

	return TRUE;
}

BOOL togo_m_count_minus(u_char * name, uint32_t step,
		TOGO_THREAD_ITEM *socket_item)
{
	TOGO_M_COUNT * item;
	char str[20];

	item = togo_m_queue_get(name);
	if (item == NULL) {
		return FALSE;
	}

	pthread_mutex_lock(&item->lock);

	item->count -= step;
	togo_itoa(item->count, str, 10);
	togo_send_data(socket_item, str, togo_strlen(str));

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
	TOGO_HASHTABLE_ITEM * hash_item;
	TOGO_M_COUNT * item;

	hash_item = togo_hashtable_get(togo_m_count_hashtable, name);
	if (hash_item == NULL) {
		/* if does not find a count item, we need to create a new queue!*/
		pthread_mutex_lock(&togo_m_count_glock);

		hash_item = togo_hashtable_get(togo_m_count_hashtable, name);
		if (hash_item == NULL) {
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

			pthread_mutex_init(&item->lock, NULL);
			item->pool = togo_m_count_pool;
			item->name = buf;
			item->count = 0;

			BOOL ret = togo_hashtable_add(togo_m_count_hashtable, name,
					(void *) item);
			if (ret == FALSE) {
				pthread_mutex_unlock(&togo_m_queue_glock);
				return NULL;
			}

			hash_item = togo_hashtable_get(togo_m_count_hashtable, name);
		}

		pthread_mutex_unlock(&togo_m_queue_glock);
	}

	item = (TOGO_M_COUNT *) hash_item->p;

	return item;
}
