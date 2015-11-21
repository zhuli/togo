/*
 * togo_hashtable.h
 *
 *  Created on: 2015-6-24
 *      Author: zhuli
 */

#ifndef TOGO_HASHTABLE_H
#define TOGO_HASHTABLE_H

#define TOGO_HASHTABLE_BUCKET_NUM 1024
#define TOGO_HASHTABLE_LOCK_SIZE 64
#define TOGO_HASHTABLE_EXPAND_STEP 32
#define TOGO_HASHTABLE_EXPAND_POWER 2
#define togo_hashtable_get_lock(a, b) (a / b)
#define togo_hashtable_if_expand(a, b) ((a * TOGO_HASHTABLE_EXPAND_POWER) <= b)
#define togo_hashtable_expand_size(a) (a *  TOGO_HASHTABLE_EXPAND_POWER)

typedef struct togo_hashtable_item TOGO_HASHTABLE_ITEM;
typedef struct togo_hashtable_bucket TOGO_HASHTABLE_BUCKET;
typedef struct togo_hashtable TOGO_HASHTABLE;

struct togo_hashtable_item {
	u_char * key;
	size_t key_len;
	void * p;
	TOGO_HASHTABLE_ITEM * next;
};

struct togo_hashtable_bucket {
	TOGO_HASHTABLE_ITEM * item;
	uint32_t size;
};

struct togo_hashtable {
	TOGO_HASHTABLE_BUCKET * bucket;
	pthread_mutex_t * lock; /* HashTable lock */
	pthread_mutex_t global_lock; /* global lock */
	uint32_t total_size;
	uint32_t total_get;
	uint32_t total_remove;
	uint32_t total_bucket;
	TOGO_POOL * pool; /* Worker memory pool */

	BOOL expand_status;
	uint32_t expand_success;
	uint32_t expand_curr;
	uint32_t expand_num;
	uint32_t expand_total_bucket;
	TOGO_HASHTABLE_BUCKET * expand_bucket;
	pthread_mutex_t * expand_lock;
};

TOGO_HASHTABLE * togo_hashtable_init(TOGO_POOL * pool);
BOOL togo_hashtable_add(TOGO_HASHTABLE * hashtable, u_char *key, void * p);
BOOL togo_hashtable_remove(TOGO_HASHTABLE * hashtable, u_char *key);
TOGO_HASHTABLE_ITEM * togo_hashtable_get(TOGO_HASHTABLE * hashtable,
		u_char *key);
BOOL togo_hashtable_flush(TOGO_HASHTABLE * hashtable);

#endif
