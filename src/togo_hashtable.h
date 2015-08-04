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
#define togo_hashtable_get_lock(a, b) (a / b)

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
};

TOGO_HASHTABLE * togo_hashtable_init(TOGO_POOL * pool);
BOOL togo_hashtable_add(const TOGO_HASHTABLE * hashtable, u_char *key,
		void * p);
BOOL togo_hashtable_remove(const TOGO_HASHTABLE * hashtable, u_char *key);
TOGO_HASHTABLE_ITEM * togo_hashtable_get(const TOGO_HASHTABLE * hashtable,
		u_char *key);
BOOL togo_hashtable_flush(TOGO_HASHTABLE * hashtable);

#endif
