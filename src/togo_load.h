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
#include "togo_string.h"
#include "togo_hashtable.h"
#include "togo_array.h"
#include "togo_config.h"
#include "togo_init.h"
#include "togo_server.h"
#include "togo_command.h"
#include "togo_m_queue.h"
#include "togo_dispatcher.h"

TOGO_POOL * togo_pool;
TOGO_C togo_c;
FILE * togo_log_file;

#endif /* TOGO_LOAD_H_ */
