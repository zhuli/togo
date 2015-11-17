/*
 * togo_hashtable.c
 *
 *  Created on: 2015-6-24
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

static void togo_hashtable_expand_init(TOGO_HASHTABLE * hashtable);
static void togo_hashtable_expand_do(TOGO_HASHTABLE * hashtable);
static void togo_hashtable_expand_finish(TOGO_HASHTABLE * hashtable);
static void togo_hashtable_add_general(TOGO_HASHTABLE * hashtable,
		TOGO_HASHTABLE_ITEM * item);
static BOOL togo_hashtable_remove_general(TOGO_HASHTABLE * hashtable,
		u_char *key);
static TOGO_HASHTABLE_ITEM * togo_hashtable_get_general(
		TOGO_HASHTABLE * hashtable, u_char *key);
static uint32_t togo_hashtable_bucket(uint32_t total, u_char *key, size_t len);
static pthread_mutex_t * togo_hashtable_get_general_lock(
		TOGO_HASHTABLE * hashtable, uint32_t current_bucket);
static pthread_mutex_t * togo_hashtable_get_expand_lock(
		TOGO_HASHTABLE * hashtable, uint32_t current_bucket);

TOGO_HASHTABLE * togo_hashtable_init(TOGO_POOL * pool)
{
	uint32_t i;

	TOGO_HASHTABLE_BUCKET * bucket = (TOGO_HASHTABLE_BUCKET *) togo_pool_calloc(
			pool, sizeof(TOGO_HASHTABLE_BUCKET) * TOGO_HASHTABLE_BUCKET_NUM);
	if (bucket == NULL) {
		togo_log(ERROR, "malloc TOGO_HASHTABLE_BUCKET fail");
		togo_exit();
	}

	TOGO_HASHTABLE *hashtable = (TOGO_HASHTABLE *) togo_pool_calloc(pool,
			sizeof(TOGO_HASHTABLE));
	if (hashtable == NULL) {
		togo_log(ERROR, "malloc TOGO_HASHTABLE fail");
		togo_exit();
	}

	/* Init Hashtable lock */
	uint32_t lock_num = TOGO_HASHTABLE_BUCKET_NUM / TOGO_HASHTABLE_LOCK_SIZE;
	pthread_mutex_t * lock = (pthread_mutex_t *) togo_pool_calloc(pool,
			sizeof(pthread_mutex_t) * lock_num);
	if (lock == NULL) {
		togo_log(ERROR, "malloc TOGO_HASHTABLE_BUCKET lock fail");
		togo_exit();
	}

	for (i = 0; i < lock_num; i++) {
		pthread_mutex_init((lock + i), NULL);
	}

	pthread_mutex_init(&hashtable->global_lock, NULL);

	hashtable->pool = pool;
	hashtable->lock = lock;
	hashtable->bucket = bucket;
	hashtable->total_get = 0;
	hashtable->total_remove = 0;
	hashtable->total_size = 0;
	hashtable->total_bucket = TOGO_HASHTABLE_BUCKET_NUM; //default 1024

	hashtable->expand_status = FALSE;
	hashtable->expand_success = 0;
	hashtable->expand_curr = 0;
	hashtable->expand_num = 0;
	hashtable->expand_total_bucket = 0;
	hashtable->expand_bucket = NULL;
	hashtable->expand_lock = NULL;

	return hashtable;
}

BOOL togo_hashtable_add(TOGO_HASHTABLE * hashtable, u_char *key, void * p)
{
	uint32_t current_bucket;
	TOGO_HASHTABLE_BUCKET * bucket;
	TOGO_HASHTABLE_ITEM * item;
	pthread_mutex_t * lock;

	size_t len = togo_strlen(key);
	item = (TOGO_HASHTABLE_ITEM *) togo_pool_calloc(hashtable->pool,
			sizeof(TOGO_HASHTABLE_ITEM));
	if (item == NULL) {
		togo_log(INFO, "Malloc TOGO_HASHTABLE_ITEM Fail");
		return FALSE;
	} else {
		item->key = key;
		item->key_len = len;
		item->p = p;
	}

	if (hashtable->expand_status == TRUE) {

		uint32_t expand_success = hashtable->expand_success;
		current_bucket = togo_hashtable_bucket(hashtable->expand_total_bucket,
				key, len);
		bucket = (hashtable->expand_bucket + current_bucket);

		/* If other thread run the function of "togo_hashtable_expand_finish"*/
		if (expand_success
				!= hashtable->expand_success|| hashtable->expand_status == FALSE) {

			pthread_mutex_lock(&hashtable->global_lock);
			togo_hashtable_add_general(hashtable, item);
			pthread_mutex_unlock(&hashtable->global_lock);

		} else {

			lock = togo_hashtable_get_expand_lock(hashtable, current_bucket);
			if (lock == NULL) {
				togo_log(INFO,
						"togo_hashtable_add:togo_hashtable_get_expand_lock NULL");
				return FALSE;
			}
			pthread_mutex_lock(lock);

			item->next = bucket->item;
			bucket->item = item;
			bucket->size++;
			hashtable->total_size++;

			pthread_mutex_unlock(lock);

			/* Every operation of add,
			 * We move TOGO_HASHTABLE_EXPAND_STEP bucket
			 * from old table to new table!*/
			togo_hashtable_expand_do(hashtable);
		}

	} else {
		togo_hashtable_add_general(hashtable, item);
	}

	return TRUE;
}

BOOL togo_hashtable_remove(TOGO_HASHTABLE * hashtable, u_char *key)
{
	uint32_t current_bucket;
	TOGO_HASHTABLE_BUCKET * bucket;
	BOOL is_remove = FALSE;
	TOGO_HASHTABLE_ITEM * item;
	TOGO_HASHTABLE_ITEM * pre = NULL;
	pthread_mutex_t * lock;

	size_t len = togo_strlen(key);
	is_remove = togo_hashtable_remove_general(hashtable, key);

	/**
	 * If the hashtable expanding and We can't remove from the old table,
	 * We try to remove item from the new table;
	 */
	if (hashtable->expand_status == TRUE && is_remove == FALSE) {

		uint32_t expand_success = hashtable->expand_success;
		current_bucket = togo_hashtable_bucket(hashtable->expand_total_bucket,
				key, len);
		bucket = (hashtable->expand_bucket + current_bucket);

		/* If other thread run the function of "togo_hashtable_expand_finish"*/
		if (expand_success
				!= hashtable->expand_success|| hashtable->expand_status == FALSE) {

			pthread_mutex_lock(&hashtable->global_lock);
			togo_hashtable_remove_general(hashtable, key);
			pthread_mutex_unlock(&hashtable->global_lock);

		} else {

			lock = togo_hashtable_get_expand_lock(hashtable, current_bucket);
			if (lock == NULL) {
				togo_log(INFO,
						"togo_hashtable_remove:togo_hashtable_get_expand_lock NULL");
				return FALSE;
			}
			pthread_mutex_lock(lock);

			item = bucket->item;
			while (item != NULL) {
				if (len == item->key_len && strncmp(item->key, key, len) == 0) {

					if (pre == NULL) {
						bucket->item = item->next;
					} else {
						pre->next = item->next;
					}

					togo_pool_free_data(hashtable->pool, (void *) item);
					bucket->size--;
					hashtable->total_size--;
					break;
				}

				pre = item;
				item = item->next;
			}

			pthread_mutex_unlock(lock);
		}
	}

	return TRUE;
}

TOGO_HASHTABLE_ITEM * togo_hashtable_get(TOGO_HASHTABLE * hashtable,
		u_char *key)
{
	uint32_t current_bucket;
	TOGO_HASHTABLE_BUCKET * bucket;
	TOGO_HASHTABLE_ITEM * current = NULL;
	TOGO_HASHTABLE_ITEM * item;
	pthread_mutex_t * lock;

	size_t len = togo_strlen(key);
	current = togo_hashtable_get_general(hashtable, key);

	/**
	 * If the hashtable expanding and We can't remove from the old table,
	 * We try to remove item from the new table;
	 */
	if (hashtable->expand_status == TRUE && current == NULL) {
		uint32_t expand_success = hashtable->expand_success;
		current_bucket = togo_hashtable_bucket(hashtable->expand_total_bucket,
				key, len);
		bucket = (hashtable->expand_bucket + current_bucket);

		/* If other thread run the function of "togo_hashtable_expand_finish"*/
		if (expand_success
				!= hashtable->expand_success|| hashtable->expand_status == FALSE) {
			pthread_mutex_lock(&hashtable->global_lock);
			current = togo_hashtable_get_general(hashtable, key);
			pthread_mutex_unlock(&hashtable->global_lock);

		} else {
			lock = togo_hashtable_get_expand_lock(hashtable, current_bucket);
			if (lock == NULL) {
				togo_log(INFO,
						"togo_hashtable_get:togo_hashtable_get_expand_lock NULL");
				return FALSE;
			}
			pthread_mutex_lock(lock);

			item = bucket->item;
			while (item != NULL) {

				if (togo_strncmp(item->key, key, len) == 0) {
					current = item;
					break;
				}
				item = item->next;
			}

			pthread_mutex_unlock(lock);
		}
	}

	return current;
}

BOOL togo_hashtable_flush(TOGO_HASHTABLE * hashtable)
{
	TOGO_HASHTABLE_BUCKET * bucket;
	TOGO_HASHTABLE_ITEM * item;
	uint32_t i, j;

	pthread_mutex_lock(&hashtable->global_lock);

	bucket = hashtable->bucket;

	for (i = 0; i < hashtable->total_bucket; i++) {
		while (bucket->item != NULL) {
			item = bucket->item;
			bucket->item = item->next;
			togo_pool_free_data(hashtable->pool, (void *) item);
		}
		bucket->size = 0;
		bucket = (hashtable->bucket + i);
	}

	/* If the hashtable is expanding, need to flush the expand table*/
	if (hashtable->expand_status == TRUE) {
		bucket = hashtable->expand_bucket;
		for (j = 0; j < hashtable->expand_total_bucket; j++) {
			while (bucket->item != NULL) {
				item = bucket->item;
				bucket->item = item->next;
				togo_pool_free_data(hashtable->pool, (void *) item);
			}
			bucket->size = 0;
			bucket = (hashtable->expand_bucket + j);
		}

		togo_pool_free_data(hashtable->pool, (void *) hashtable->expand_bucket);
		togo_pool_free_data(hashtable->pool, (void *) hashtable->expand_lock);

		hashtable->expand_status = FALSE;
		hashtable->expand_success = 0;
		hashtable->expand_curr = 0;
		hashtable->expand_num = 0;
		hashtable->expand_bucket = NULL;
		hashtable->expand_lock = NULL;
		hashtable->expand_total_bucket = 0;
	}

	hashtable->total_get = 0;
	hashtable->total_remove = 0;
	hashtable->total_size = 0;

	pthread_mutex_unlock(&hashtable->global_lock);

	return TRUE;
}

static void togo_hashtable_add_general(TOGO_HASHTABLE * hashtable,
		TOGO_HASHTABLE_ITEM * item)
{
	uint32_t current_bucket;
	TOGO_HASHTABLE_BUCKET * bucket;
	pthread_mutex_t * lock;

	current_bucket = togo_hashtable_bucket(hashtable->total_bucket, item->key,
			item->key_len);
	bucket = (hashtable->bucket + current_bucket);

	lock = togo_hashtable_get_general_lock(hashtable, current_bucket);
	pthread_mutex_lock(lock);

	item->next = bucket->item;
	bucket->item = item;
	bucket->size++;
	hashtable->total_size++;

	pthread_mutex_unlock(lock);

	if (togo_hashtable_if_expand(hashtable->total_bucket, hashtable->total_size)) {
		togo_hashtable_expand_init(hashtable);
	}
}

static BOOL togo_hashtable_remove_general(TOGO_HASHTABLE * hashtable,
		u_char *key)
{
	uint32_t current_bucket;
	TOGO_HASHTABLE_BUCKET * bucket;
	BOOL is_remove = FALSE;
	TOGO_HASHTABLE_ITEM * item;
	TOGO_HASHTABLE_ITEM * pre = NULL;
	pthread_mutex_t * lock;

	size_t len = togo_strlen(key);
	current_bucket = togo_hashtable_bucket(hashtable->total_bucket, key, len);
	bucket = (hashtable->bucket + current_bucket);

	lock = togo_hashtable_get_general_lock(hashtable, current_bucket);
	pthread_mutex_lock(lock);

	item = bucket->item;
	while (item != NULL) {
		if (len == item->key_len && strncmp(item->key, key, len) == 0) {

			if (pre == NULL) {
				bucket->item = item->next;
			} else {
				pre->next = item->next;
			}

			togo_pool_free_data(hashtable->pool, (void *) item);
			bucket->size--;
			hashtable->total_size--;
			is_remove = TRUE;
			break;
		}

		pre = item;
		item = item->next;
	}

	pthread_mutex_unlock(lock);

	return is_remove;
}

static TOGO_HASHTABLE_ITEM * togo_hashtable_get_general(
		TOGO_HASHTABLE * hashtable, u_char *key)
{
	uint32_t current_bucket;
	TOGO_HASHTABLE_BUCKET * bucket;
	TOGO_HASHTABLE_ITEM * item;
	TOGO_HASHTABLE_ITEM * current = NULL;
	pthread_mutex_t * lock;

	size_t len = togo_strlen(key);
	current_bucket = togo_hashtable_bucket(hashtable->total_bucket, key, len);
	bucket = (hashtable->bucket + current_bucket);

	lock = togo_hashtable_get_general_lock(hashtable, current_bucket);
	pthread_mutex_lock(lock);

	item = bucket->item;
	while (item != NULL) {
		if (togo_strncmp(item->key, key, len) == 0) {
			current = item;
			break;
		}
		item = item->next;
	}

	pthread_mutex_unlock(lock);

	return current;
}

static void togo_hashtable_expand_init(TOGO_HASHTABLE * hashtable)
{
	uint32_t expand_total_bucket, lock_num, i;
	pthread_mutex_t * lock;
	TOGO_HASHTABLE_BUCKET * bucket;

	pthread_mutex_lock(&hashtable->global_lock);

	if (hashtable->expand_status == TRUE
			|| !togo_hashtable_if_expand(hashtable->total_bucket, hashtable->total_size)) {
		pthread_mutex_unlock(&hashtable->global_lock);
		return;
	}

	/* Lock */
	expand_total_bucket = togo_hashtable_expand_size(hashtable->total_bucket);
	lock_num = expand_total_bucket / TOGO_HASHTABLE_LOCK_SIZE;
	lock = (pthread_mutex_t *) togo_pool_calloc(hashtable->pool,
			sizeof(pthread_mutex_t) * lock_num);
	if (lock == NULL) {

		togo_log(ERROR, "malloc TOGO_HASHTABLE_BUCKET lock fail");
		pthread_mutex_unlock(&hashtable->global_lock);
		return;
	} else {

		for (i = 0; i < lock_num; i++) {
			pthread_mutex_init((lock + i), NULL);
		}

	}

	/* Bucket */
	bucket = (TOGO_HASHTABLE_BUCKET *) togo_pool_calloc(hashtable->pool,
			sizeof(TOGO_HASHTABLE_BUCKET) * expand_total_bucket);
	if (bucket == NULL) {
		togo_log(ERROR, "malloc TOGO_HASHTABLE_BUCKET fail");
		pthread_mutex_unlock(&hashtable->global_lock);
		return;
	}

	hashtable->expand_total_bucket = expand_total_bucket;
	hashtable->expand_status = TRUE;
	hashtable->expand_curr = 0;
	hashtable->expand_num++;
	hashtable->expand_success = 0;
	hashtable->expand_lock = lock;
	hashtable->expand_bucket = bucket;

	pthread_mutex_unlock(&hashtable->global_lock);
}

static void togo_hashtable_expand_do(TOGO_HASHTABLE * hashtable)
{
	uint32_t current_bucket, curr, step;
	TOGO_HASHTABLE_BUCKET * old_bucket;
	TOGO_HASHTABLE_BUCKET * bucket;
	TOGO_HASHTABLE_ITEM * item;
	TOGO_HASHTABLE_ITEM * old_item;
	pthread_mutex_t * lock;
	pthread_mutex_t * expand_lock;

	pthread_mutex_lock(&hashtable->global_lock);

	if (hashtable->expand_status == FALSE) {
		pthread_mutex_unlock(&hashtable->global_lock);
		return;
	}

	curr = hashtable->expand_curr;
	step = curr + TOGO_HASHTABLE_EXPAND_STEP;
	for (curr; curr < step; curr++) {

		if (curr == hashtable->total_bucket) {
			/**
			 * When move all bucket from old table,
			 * We need to stop expanding and the hashtable->expand_status will
			 * be set FALSE.
			 */
			togo_hashtable_expand_finish(hashtable);
			break;
		}

		lock = togo_hashtable_get_general_lock(hashtable, curr);
		pthread_mutex_lock(lock);

		old_bucket = (hashtable->bucket + curr);
		old_item = old_bucket->item;
		while (old_item != NULL) {
			item = old_item;
			old_item = old_item->next;
			current_bucket = togo_hashtable_bucket(
					hashtable->expand_total_bucket, item->key, item->key_len);
			bucket = (hashtable->expand_bucket + current_bucket);

			expand_lock = togo_hashtable_get_expand_lock(hashtable,
					current_bucket);
			if (expand_lock == NULL) {
				continue;
			}
			pthread_mutex_lock(expand_lock);

			item->next = bucket->item;
			bucket->item = item;
			bucket->size++;

			pthread_mutex_unlock(expand_lock);
		}

		pthread_mutex_unlock(lock);
		hashtable->expand_curr++;
	}

	pthread_mutex_unlock(&hashtable->global_lock);
}

static void togo_hashtable_expand_finish(TOGO_HASHTABLE * hashtable)
{
	TOGO_HASHTABLE_BUCKET * old_bucket;
	pthread_mutex_t * old_lock;

	if (hashtable->expand_status == FALSE
			|| hashtable->expand_curr < hashtable->total_bucket) {
		return;
	}

	old_bucket = hashtable->bucket;
	old_lock = hashtable->lock;

	hashtable->expand_success++;
	hashtable->bucket = hashtable->expand_bucket;
	hashtable->lock = hashtable->expand_lock;
	hashtable->total_bucket = hashtable->expand_total_bucket;
	hashtable->expand_curr = 0;
	hashtable->expand_total_bucket = 0;
	hashtable->expand_status = FALSE;

	/* Free the old bucket and old lock !*/
	togo_pool_free_data(hashtable->pool, (void *) old_bucket);
	togo_pool_free_data(hashtable->pool, (void *) old_lock);

}

static pthread_mutex_t * togo_hashtable_get_general_lock(
		TOGO_HASHTABLE * hashtable, uint32_t current_bucket)
{
	uint32_t x;
	pthread_mutex_t * lock;
	x = togo_hashtable_get_lock(current_bucket, TOGO_HASHTABLE_LOCK_SIZE);
	lock = (pthread_mutex_t *) (hashtable->lock + x);
	return lock;
}

static pthread_mutex_t * togo_hashtable_get_expand_lock(
		TOGO_HASHTABLE * hashtable, uint32_t current_bucket)
{
	uint32_t x;
	pthread_mutex_t * lock;
	pthread_mutex_t * expand_lock;
	expand_lock = hashtable->expand_lock;
	x = togo_hashtable_get_lock(current_bucket, TOGO_HASHTABLE_LOCK_SIZE);
	if (hashtable->expand_status == FALSE || expand_lock == NULL) {
		return NULL;
	} else {
		lock = (pthread_mutex_t *) (expand_lock + x);
	}
	return lock;
}

static uint32_t togo_hashtable_bucket(uint32_t total, u_char *key, size_t len)
{
	int code = togo_djb_hash(key);
	uint32_t bucket_len = abs(code % total);
	return bucket_len;
}

