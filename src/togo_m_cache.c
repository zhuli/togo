/*
 * togo_m_cache.c
 *
 *  Created on: 2015-10-20
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

void togo_m_cache_init(void)
{
	togo_m_cache_pool = togo_pool_create(TOGO_M_COUNTER_POOL_SIZE);
}

