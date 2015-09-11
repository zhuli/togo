/*
 * togo_hashtable.c
 *
 *  Created on: 2015-6-24
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

static uint32_t togo_hashtable_bucket(const TOGO_HASHTABLE * hashtable,
		u_char *key, size_t len);
static void togo_hashtable_lock(const TOGO_HASHTABLE * hashtable,
		uint32_t current_bucket);
static void togo_hashtable_unlock(const TOGO_HASHTABLE * hashtable,
		uint32_t current_bucket);

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

	return hashtable;
}

BOOL togo_hashtable_add(const TOGO_HASHTABLE * hashtable, u_char *key, void * p)
{
	size_t len = togo_strlen(key);

	uint32_t current_bucket = togo_hashtable_bucket(hashtable, key, len);
	TOGO_HASHTABLE_BUCKET * bucket = (hashtable->bucket + current_bucket);
	TOGO_POOL * pool = hashtable->pool;
	TOGO_HASHTABLE_ITEM * item = (TOGO_HASHTABLE_ITEM *) togo_pool_calloc(pool,
			sizeof(TOGO_HASHTABLE_ITEM));
	if (item == NULL) {
		togo_log(INFO, "Malloc TOGO_HASHTABLE_ITEM Fail");
		return FALSE;
	}

	togo_hashtable_lock(hashtable, current_bucket);

	item->key = key;
	item->key_len = len;
	item->p = p;
	item->next = bucket->item;
	bucket->item = item;
	bucket->size++;
	togo_hashtable_unlock(hashtable, current_bucket);

	return TRUE;
}

BOOL togo_hashtable_remove(const TOGO_HASHTABLE * hashtable, u_char *key)
{

	size_t len = togo_strlen(key);
	uint32_t current_bucket = togo_hashtable_bucket(hashtable, key, len);
	TOGO_HASHTABLE_BUCKET * bucket = (hashtable->bucket + current_bucket);

	togo_hashtable_lock(hashtable, current_bucket);

	TOGO_HASHTABLE_ITEM * item = bucket->item;
	TOGO_HASHTABLE_ITEM * pre = NULL;

	while (item != NULL) {
		if (len == item->key_len && strncmp(item->key, key, len) == 0) {

			if (pre == NULL) {
				bucket->item = item->next;
			} else {
				pre->next = item->next;
			}

			togo_pool_free_data(hashtable->pool, (void *) item);
			bucket->size--;

			break;
		}

		pre = item;
		item = item->next;
	}

	togo_hashtable_unlock(hashtable, current_bucket);

	return TRUE;
}

TOGO_HASHTABLE_ITEM * togo_hashtable_get(const TOGO_HASHTABLE * hashtable,
		u_char *key)
{

	size_t len = togo_strlen(key);
	uint32_t current_bucket = togo_hashtable_bucket(hashtable, key, len);
	TOGO_HASHTABLE_BUCKET * bucket = (hashtable->bucket + current_bucket);

	togo_hashtable_lock(hashtable, current_bucket);

	TOGO_HASHTABLE_ITEM * item = bucket->item;
	TOGO_HASHTABLE_ITEM * current = NULL;

	while (item != NULL) {

		if (togo_strcmp(item->key, key) == 0) {
			current = item;
			break;
		}
		item = item->next;
	}

	togo_hashtable_unlock(hashtable, current_bucket);

	return current;
}

BOOL togo_hashtable_flush(TOGO_HASHTABLE * hashtable)
{
	pthread_mutex_t global_lock = hashtable->global_lock;
	pthread_mutex_lock(&global_lock); //lock

	uint32_t i;
	TOGO_HASHTABLE_BUCKET * bucket = hashtable->bucket;
	for (i = 0; i < TOGO_HASHTABLE_BUCKET_NUM; i++) {
		while (bucket->item != NULL) {
			TOGO_HASHTABLE_ITEM * item = bucket->item;
			bucket->item = bucket->item->next;
			free(item);
		}
		bucket->size = 0;
		bucket = bucket++;
	}
	hashtable->total_get = 0;
	hashtable->total_remove = 0;
	hashtable->total_size = 0;

	pthread_mutex_unlock(&global_lock); //unlock

	return TRUE;
}

static void togo_hashtable_lock(const TOGO_HASHTABLE * hashtable,
		uint32_t current_bucket)
{
	uint32_t x =
			togo_hashtable_get_lock(current_bucket, TOGO_HASHTABLE_LOCK_SIZE);
	pthread_mutex_t * lock = hashtable->lock;
	pthread_mutex_lock((lock + x)); //lock
}

static void togo_hashtable_unlock(const TOGO_HASHTABLE * hashtable,
		uint32_t current_bucket)
{
	uint32_t x =
			togo_hashtable_get_lock(current_bucket, TOGO_HASHTABLE_LOCK_SIZE);
	pthread_mutex_t * lock = hashtable->lock;
	pthread_mutex_unlock((lock + x)); //unlock
}

static uint32_t togo_hashtable_bucket(const TOGO_HASHTABLE * hashtable,
		u_char *key, size_t len)
{
	int code = togo_djb_hash(key);
	uint32_t total_bucke = hashtable->total_bucket;
	uint32_t bucket_len = abs(code % total_bucke);
	return bucket_len;
}

