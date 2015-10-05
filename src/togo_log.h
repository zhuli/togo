/*
 * togo_log.h
 *
 *  Created on: 2015-7-25
 *      Author: zhuli
 */

#ifndef TOGO_LOG_H_
#define TOGO_LOG_H_

FILE * togo_log_file;

typedef struct togo_log TOGO_LOG;

struct togo_log {
	BOOL file_log;
	u_char * file;
	pthread_mutex_t lock;
};

void togo_log_init(u_char * filename);
void togo_log_write(u_char * str);

#endif /* TOGO_LOG_H_ */
