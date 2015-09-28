/*
 * togo_m_counter.h
 *
 *  Created on: 2015-9-9
 *      Author: zhuli
 */

#ifndef TOGO_M_LOCK_H_
#define TOGO_M_LOCK_H_

#define TOGO_M_LOCK_POOL_SIZE (1024 * 1024)

typedef struct togo_m_lock TOGO_M_LOCK;

struct togo_m_lock {
	u_char * name;
	pthread_mutex_t lock;
	BOOL locked;
	uint32_t total;
	uint32_t total_lock;
	uint32_t total_unlock;
	TOGO_POOL * pool;
};

BOOL togo_m_lock_command(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item, int ntag);
void togo_m_lock_init(void);
BOOL togo_m_lock_lock(u_char * name, TOGO_THREAD_ITEM *socket_item);
BOOL togo_m_lock_unlock(u_char * name, TOGO_THREAD_ITEM *socket_item);
BOOL togo_m_lock_status(u_char * name, TOGO_THREAD_ITEM *socket_item);

TOGO_POOL * togo_m_lock_pool;
TOGO_HASHTABLE * togo_m_lock_hashtable;
pthread_mutex_t togo_m_lock_glock;

#endif /* TOGO_M_LOCK_H_ */
