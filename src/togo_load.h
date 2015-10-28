/*
 * togo_load.h
 *
 *  Created on: 2015-6-29
 *      Author: zhuli
 */

#ifndef TOGO_LOAD_H_
#define TOGO_LOAD_H_

#include "togo_util.h"
#include "togo_alloc.h"
#include "togo_hash.h"
#include "togo_pool.h"
#include "togo_log.h"
#include "togo_string.h"
#include "togo_hashtable.h"
#include "togo_array.h"
#include "togo_config.h"
#include "togo_init.h"
#include "togo_server.h"
#include "togo_command.h"
#include "togo_m_queue.h"
#include "togo_m_counter.h"
#include "togo_m_lock.h"
#include "togo_m_cache.h"
#include "togo_dispatcher.h"

/* Global variable */
TOGO_POOL * togo_global_pool;
TOGO_C * togo_global_c;
TOGO_LOG * togo_global_log;

#endif /* TOGO_LOAD_H_ */
