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
	togo_log_file = fopen(filename, "wr+");
	if (togo_log_file ==  NULL) {
		printf("Init log Error! Can not open file %s", filename);
		togo_exit();
	}
}



