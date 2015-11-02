/*
 * togo_m_counter.c
 *
 *  Created on: 2015-9-9
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

static TOGO_M_COUNTER * togo_m_counter_item(u_char * name);
static void togo_m_counter_send(TOGO_THREAD_ITEM *socket_item,
		TOGO_M_COUNTER * item);

BOOL togo_m_counter_command(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item, int ntag)
{
	BOOL ret = FALSE;
	u_char * action = NULL;
	u_char * cname = NULL;
	int32_t step = 1;

	if (ntag < 3) {
		return FALSE;
	}

	/**
	 * command_tag[0] : Module  COUNTER
	 * command_tag[1] : Action  PLUS|MINUS|GET|RESET
	 * command_tag[2] : Object  Count name
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
				return ret;
			}
		}
	}

	if (action == NULL || cname == NULL) {
		return ret;
	}

	if (togo_strcmp(action, "PLUS") == 0) {
		ret = togo_m_counter_plus(cname, step, socket_item);

	} else if (togo_strcmp(action, "MINUS") == 0) {
		ret = togo_m_counter_minus(cname, step, socket_item);

	} else if (togo_strcmp(action, "GET") == 0) {
		ret = togo_m_counter_get(cname, socket_item);

	} else if (togo_strcmp(action, "RESET") == 0) {
		ret = togo_m_counter_reset(cname, socket_item);

	}

	return ret;
}

void togo_m_counter_init(void)
{
	togo_m_counter_pool = togo_pool_create(TOGO_M_COUNTER_POOL_SIZE);
	if (togo_m_counter_pool == NULL) {
		togo_log(ERROR, "Initialize modules_counter's pool fail.");
		togo_exit();
	}

	togo_m_counter_hashtable = togo_hashtable_init(togo_m_counter_pool);
	if (togo_m_counter_hashtable == NULL) {
		togo_log(ERROR, "Initialize modules_counter's hashtable fail.");
		togo_exit();
	}

	pthread_mutex_init(&togo_m_counter_glock, NULL);
}

BOOL togo_m_counter_plus(u_char * name, int32_t step,
		TOGO_THREAD_ITEM *socket_item)
{
	TOGO_M_COUNTER * item;

	item = togo_m_counter_item(name);
	if (item == NULL) {
		return FALSE;
	}
	pthread_mutex_lock(&item->lock);

	item->count += step;
	togo_m_counter_send(socket_item, item);

	pthread_mutex_unlock(&item->lock);

	return TRUE;
}

BOOL togo_m_counter_minus(u_char * name, int32_t step,
		TOGO_THREAD_ITEM *socket_item)
{
	TOGO_M_COUNTER * item;

	item = togo_m_counter_item(name);
	if (item == NULL) {
		return FALSE;
	}
	pthread_mutex_lock(&item->lock);

	item->count -= step;
	togo_m_counter_send(socket_item, item);

	pthread_mutex_unlock(&item->lock);

	return TRUE;
}

BOOL togo_m_counter_get(u_char * name, TOGO_THREAD_ITEM *socket_item)
{
	TOGO_M_COUNTER * item;

	item = togo_m_counter_item(name);
	if (item == NULL) {
		return FALSE;
	}
	pthread_mutex_lock(&item->lock);

	togo_m_counter_send(socket_item, item);

	pthread_mutex_unlock(&item->lock);

	return TRUE;
}

BOOL togo_m_counter_reset(u_char * name, TOGO_THREAD_ITEM *socket_item)
{
	TOGO_M_COUNTER * item;

	item = togo_m_counter_item(name);
	if (item == NULL) {
		return FALSE;
	}
	pthread_mutex_lock(&item->lock);
	item->count = 0;
	togo_m_counter_send(socket_item, item);

	pthread_mutex_unlock(&item->lock);

	return TRUE;
}

static void togo_m_counter_send(TOGO_THREAD_ITEM *socket_item,
		TOGO_M_COUNTER * item)
{
	char str[20];
	togo_itoa(item->count, str, 10);
	togo_send_data(socket_item, str, togo_strlen(str));
}

static TOGO_M_COUNTER * togo_m_counter_item(u_char * name)
{
	TOGO_HASHTABLE_ITEM * hash_item;
	TOGO_M_COUNTER * item;

	hash_item = togo_hashtable_get(togo_m_counter_hashtable, name);

	if (hash_item == NULL) {
		/* if does not find a count item, we need to create a new queue!*/
		pthread_mutex_lock(&togo_m_counter_glock);

		hash_item = togo_hashtable_get(togo_m_counter_hashtable, name);
		if (hash_item == NULL) {
			item = (TOGO_M_COUNTER *) togo_pool_calloc(togo_m_counter_pool,
					sizeof(TOGO_M_COUNTER));
			if (item == NULL) {
				pthread_mutex_unlock(&togo_m_counter_glock);
				return NULL;
			}

			u_char * buf = (u_char *) togo_pool_alloc(togo_m_counter_pool,
					togo_pool_strlen(name));
			if (buf == NULL) {
				pthread_mutex_unlock(&togo_m_counter_glock);
				return NULL;
			}
			togo_strcpy(buf, name);

			pthread_mutex_init(&item->lock, NULL);
			item->pool = togo_m_counter_pool;
			item->name = buf;
			item->count = 0;
			item->clear = FALSE;

			BOOL ret = togo_hashtable_add(togo_m_counter_hashtable, item->name,
					(void *) item);
			if (ret == FALSE) {
				pthread_mutex_unlock(&togo_m_counter_glock);
				return NULL;
			}

			hash_item = togo_hashtable_get(togo_m_counter_hashtable, name);
		}

		pthread_mutex_unlock(&togo_m_counter_glock);
	}

	item = (TOGO_M_COUNTER *) hash_item->p;
	return item;
}
