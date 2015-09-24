/*
 * togo_m_counter.h
 *
 *  Created on: 2015-9-9
 *      Author: zhuli
 */

#ifndef TOGO_M_COUNTER_H_
#define TOGO_M_COUNTER_H_

#define TOGO_M_COUNTER_POOL_SIZE (1024 * 1024)

typedef struct togo_m_counter TOGO_M_COUNTER;

struct togo_m_counter {
	u_char * name;
	int64_t count;
	pthread_mutex_t lock;
	TOGO_POOL * pool;
	BOOL clear;
};

BOOL togo_m_counter_command(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item, int ntag);
void togo_m_counter_init(void);
BOOL togo_m_counter_plus(u_char * name, int32_t step,
		TOGO_THREAD_ITEM *socket_item);
BOOL togo_m_counter_minus(u_char * name, int32_t step,
		TOGO_THREAD_ITEM *socket_item);
BOOL togo_m_counter_get(u_char * name, TOGO_THREAD_ITEM *socket_item);
BOOL togo_m_counter_reset(u_char * name, TOGO_THREAD_ITEM *socket_item);

TOGO_POOL * togo_m_counter_pool;
TOGO_HASHTABLE * togo_m_counter_hashtable;
pthread_mutex_t togo_m_counter_glock;

#endif /* TOGO_M_COUNTER_H_ */
