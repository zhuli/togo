/*
 * togo_log.h
 *
 *  Created on: 2015-7-25
 *      Author: zhuli
 */

#ifndef TOGO_LOG_H_
#define TOGO_LOG_H_

FILE * togo_log_file;

typedef struct togo_log_s TOGO_LOG;

struct togo_log_s {
	BOOL file_log;
	FILE * file;
};

void togo_log_init(char * filename);

#endif /* TOGO_LOG_H_ */
