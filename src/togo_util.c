/*
 * util.c
 * Created on: 2014-2-20
 * Author: zhuli
 */
#include "togo.h"
#include "togo_load.h"

int togo_get_time()
{
	time_t now;
	time(&now);
	return now;
}

int togo_log(enum log_level level, const u_char *fmt, ...)
{
	u_char *x = NULL;
	switch (level) {
	case ERROR:
		x = "[ERROR] ";
		break;

	case INFO:
		x = "[INFO] ";
		break;

	case DEBUG:
		x = "[DEBUG] ";
		break;
	}
	printf(x);
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	printf("\r\n");
}

void togo_exit()
{
	exit(1);
}

