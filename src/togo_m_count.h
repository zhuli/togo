/*
 * togo_m_count.h
 *
 *  Created on: 2015-9-9
 *      Author: zhuli
 */

#ifndef TOGO_M_COUNT_H_
#define TOGO_M_COUNT_H_

#define TOGO_M_COUNT_POOL_SIZE (1024 * 1024)

typedef struct togo_m_count TOGO_M_COUNT;

struct togo_m_count {
	u_char * name;
	int64_t count;
	pthread_mutex_t lock;
	TOGO_POOL * pool;
};

void togo_m_count_init(void);
BOOL togo_m_count_plus(u_char * name);
BOOL togo_m_count_minus(u_char * name);

TOGO_POOL * togo_m_count_pool;
TOGO_HASHTABLE * togo_m_count_hashtable;
pthread_mutex_t togo_m_count_glock;

#endif /* TOGO_M_COUNT_H_ */
