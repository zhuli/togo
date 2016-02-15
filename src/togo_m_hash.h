/*
 * togo_m_hash.h
 *
 *  Created on: 2016-1-7
 *      Author: zhuli
 */

#ifndef TOGO_M_HASH_H_
#define TOGO_M_HASH_H_

#define TOGO_M_HASH_POOL_SIZE (1024 * 1024)
#define TOGO_M_HASH_HT_ITEM_SIZE 64
#define TOGO_M_HASH_HT_LOCK_SIZE 16
#define TOGO_M_HASH_HT_EXPAND_STEP 16

struct togo_m_hash_node {
	TOGO_HASHTABLE * ht;

	uint32_t total_item;
	uint32_t total_field;
	uint64_t total_hit;
	uint64_t total_write;
	uint64_t total_read;

	pthread_mutex_t glock;
	TOGO_POOL * pool;
};

struct togo_m_hash_item {
	TOGO_HASHTABLE * ht;

	uint32_t total_field;
	uint64_t total_hit;
	uint64_t total_write;
	uint64_t total_read;

	pthread_mutex_t lock;
	TOGO_POOL * pool;
};

void togo_m_hash_init(void);

TOGO_HASHTABLE * togo_m_hash_node;

#endif /* TOGO_M_HASH_H_ */
