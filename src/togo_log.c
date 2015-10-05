/*
 * togo_log.c
 *
 *  Created on: 2015-7-25
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

void togo_log_init(u_char * filename)
{
	FILE * file;

	togo_global_log = (TOGO_LOG *) togo_pool_calloc(togo_global_pool,
			sizeof(TOGO_LOG));

	if (filename == NULL) {
		togo_global_log->file_log = FALSE;
		togo_global_log->file = filename;
		return;
	}

	file = fopen(filename, "a+");
	if (file == NULL) {
		togo_global_log->file_log = FALSE;
		togo_global_log->file = filename;
		return;
	}

	togo_global_log->file_log = TRUE;
	togo_global_log->file = filename;
	pthread_mutex_init(&togo_global_log->lock, NULL);
	fclose(file);
}

void togo_log_write(u_char * str)
{
	if (togo_global_log->file_log == TRUE && togo_global_log->file != NULL) {
		pthread_mutex_lock(&togo_global_log->lock);
		FILE * file;
		file = fopen(togo_global_log->file, "a+");
		fputs(str, file);
		fclose(file);
		pthread_mutex_unlock(&togo_global_log->lock);
	}
}

