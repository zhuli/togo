/*
 * togo_m_lock.c
 *
 *  Created on: 2015-9-25
 *      Author: zhuli
 */
#include "togo.h"
#include "togo_load.h"

static TOGO_M_LOCK * togo_m_lock_item(u_char * name);
static void togo_m_lock_send(TOGO_THREAD_ITEM *socket_item, TOGO_M_LOCK * item);

BOOL togo_m_lock_command(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item, int ntag)
{
	BOOL ret = FALSE;
	u_char * action = NULL;
	u_char * cname = NULL;

	if (ntag < 3) {
		return FALSE;
	}

	/**
	 * command_tag[0] : Module  LOCK
	 * command_tag[1] : Action  LOCK|UNLOCK|STATUS
	 * command_tag[2] : Object  Count name
	 */
	action = command_tag[1].value;
	cname = command_tag[2].value;

	if (action == NULL || cname == NULL) {
		return ret;
	}

	if (togo_strcmp(action, "LOCK") == 0) {
		ret = togo_m_lock_lock(cname, socket_item);

	} else if (togo_strcmp(action, "UNLOCK") == 0) {
		ret = togo_m_lock_unlock(cname, socket_item);

	} else if (togo_strcmp(action, "STATUS") == 0) {
		ret = togo_m_lock_status(cname, socket_item);

	}

	return ret;
}

void togo_m_lock_init(void)
{
	togo_m_lock_pool = togo_pool_create(TOGO_M_LOCK_POOL_SIZE);
	if (togo_m_lock_pool == NULL) {
		togo_log(ERROR, "Initialize modules_lock's pool fail.");
		togo_exit();
	}

	togo_m_lock_hashtable = togo_hashtable_init(togo_m_lock_pool);
	if (togo_m_lock_hashtable == NULL) {
		togo_log(ERROR, "Initialize modules_lock's hashtable fail.");
		togo_exit();
	}

	pthread_mutex_init(&togo_m_lock_glock, NULL);
}

BOOL togo_m_lock_lock(u_char * name, TOGO_THREAD_ITEM *socket_item)
{
	TOGO_M_LOCK * item;

	item = togo_m_lock_item(name);
	if (item == NULL) {
		return FALSE;
	}
	pthread_mutex_lock(&item->lock);

	item->total++;
	item->total_lock++;
	item->locked = TRUE;
	togo_m_lock_send(socket_item, item);

	pthread_mutex_unlock(&item->lock);

	return TRUE;
}

BOOL togo_m_lock_unlock(u_char * name, TOGO_THREAD_ITEM *socket_item)
{
	TOGO_M_LOCK * item;

	item = togo_m_lock_item(name);
	if (item == NULL) {
		return FALSE;
	}
	pthread_mutex_lock(&item->lock);

	item->total++;
	item->total_unlock++;
	item->locked = FALSE;
	togo_m_lock_send(socket_item, item);

	pthread_mutex_unlock(&item->lock);

	return TRUE;
}

BOOL togo_m_lock_status(u_char * name, TOGO_THREAD_ITEM * socket_item)
{
	TOGO_M_LOCK * item;
	TOGO_STRING * togo_str;

	item = togo_m_lock_item(name);
	if (item == NULL) {
		return FALSE;
	}
	pthread_mutex_lock(&item->lock);

	togo_str = togo_string_init(item->pool, TOGO_STRING_DEFAULT_SIZE);
	if (togo_str == NULL) {
		return FALSE;
	}

	togo_string_append_s(&togo_str, "status:");
	togo_string_append_i(&togo_str, item->locked);
	togo_string_append_s(&togo_str, ";total:");
	togo_string_append_i(&togo_str, item->total);
	togo_string_append_s(&togo_str, ";total_lock:");
	togo_string_append_i(&togo_str, item->total_lock);
	togo_string_append_s(&togo_str, ";total_unlock:");
	togo_string_append_i(&togo_str, item->total_unlock);

	togo_send_data(socket_item, togo_str->buf, togo_str->str_size);
	togo_string_destroy(togo_str);

	pthread_mutex_unlock(&item->lock);

	return TRUE;
}

static void togo_m_lock_send(TOGO_THREAD_ITEM *socket_item, TOGO_M_LOCK * item)
{
	char str[1];
	togo_itoa(item->locked, str, 10);
	togo_send_data(socket_item, str, togo_strlen(str));
}

static TOGO_M_LOCK * togo_m_lock_item(u_char * name)
{
	TOGO_HASHTABLE_ITEM * hash_item;
	TOGO_M_LOCK * item;

	hash_item = togo_hashtable_get(togo_m_lock_hashtable, name);

	if (hash_item == NULL) {
		/* if does not find a lock item, we need to create a new queue!*/
		pthread_mutex_lock(&togo_m_lock_glock);

		hash_item = togo_hashtable_get(togo_m_lock_hashtable, name);
		if (hash_item == NULL) {
			item = (TOGO_M_LOCK *) togo_pool_calloc(togo_m_lock_pool,
					sizeof(TOGO_M_LOCK));
			if (item == NULL) {
				pthread_mutex_unlock(&togo_m_lock_glock);
				return NULL;
			}

			u_char * buf = (u_char *) togo_pool_alloc(togo_m_lock_pool,
					togo_pool_strlen(name));
			if (buf == NULL) {
				pthread_mutex_unlock(&togo_m_lock_glock);
				return NULL;
			}
			togo_strcpy(buf, name);

			pthread_mutex_init(&item->lock, NULL);
			item->pool = togo_m_lock_pool;
			item->total = 0;
			item->total_lock = 0;
			item->total_unlock = 0;
			item->name = buf;
			item->locked = FALSE;

			BOOL ret = togo_hashtable_add(togo_m_lock_hashtable, item->name,
					(void *) item);
			if (ret == FALSE) {
				pthread_mutex_unlock(&togo_m_lock_glock);
				return NULL;
			}

			hash_item = togo_hashtable_get(togo_m_lock_hashtable, name);
		}

		pthread_mutex_unlock(&togo_m_lock_glock);
	}

	item = (TOGO_M_LOCK *) hash_item->p;
	return item;
}
