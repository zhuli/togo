/*
 * togo_m_count.c
 *
 *  Created on: 2015-9-9
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

static TOGO_M_COUNT * togo_m_queue_get(u_char * name);

void togo_m_count_init(void)
{
	togo_m_count_pool = togo_pool_create(TOGO_M_QUEUE_POOL_SIZE);
	if (togo_m_count_pool == NULL) {
		togo_log(ERROR, "Initialize modules_count's pool fail.");
		togo_exit();
	}

	togo_m_count_hashtable = togo_hashtable_init(togo_m_count_pool);
	if (togo_m_count_hashtable == NULL) {
		togo_log(ERROR, "Initialize modules_queue's count fail.");
		togo_exit();
	}

	pthread_mutex_init(&togo_m_count_glock, NULL);
}

BOOL togo_m_count_plus(u_char * name)
{
	TOGO_M_COUNT * item;


}

static TOGO_M_COUNT * togo_m_queue_get(u_char * name) {
	TOGO_M_COUNT * item;

	item = togo_hashtable_get(togo_m_count_hashtable, name);
	if (item == NULL) {
		/* if does not find a count item, we need to create a new queue!*/
		pthread_mutex_lock(&togo_m_count_glock);

		item = togo_hashtable_get(togo_m_count_hashtable, name);
		if (item == NULL) {

		}
	}
}
