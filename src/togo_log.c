/*
 * togo_log.c
 *
 *  Created on: 2015-7-25
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

void togo_log_init(char * filename)
{
	FILE * log_file;

	togo_global_log = togo_pool_calloc(togo_global_pool, sizeof(TOGO_LOG));

	log_file = fopen(filename, "wr+");
	if (log_file == NULL) {
		togo_global_log->file_log = FALSE;
		togo_global_log->file = NULL;
		return;
	}

}

