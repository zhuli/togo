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
	u_char str[1024];
	u_char *x = NULL;
	uint32_t len = 0;
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
	len = togo_strlen(x);
	togo_memcpy(str, x, len);

	va_list args;
	va_start(args, fmt);
	vsprintf((str + len), fmt, args);
	va_end(args);

	len = togo_strlen(str);
 	togo_memcpy((str + len), "\r\n\0", 3);
	printf(str);
}

void togo_exit()
{
	exit(1);
}

