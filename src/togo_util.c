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
	uint32_t len, time_len = 0;
	time_t rawtime;
	struct tm * timeinfo;
	char time_str[30];

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S ", timeinfo);

	switch (level) {
	case ERROR:
		x = "[ERROR] ";
		break;

	case INFO:
		x = "[INFO]  ";
		break;

	case DEBUG:
		x = "[DEBUG] ";
		break;
	}
	len = togo_strlen(x);
	togo_memcpy(str, x, len);

	time_len = togo_strlen(time_str);
	togo_memcpy(str + len, time_str, time_len);

	va_list args;
	va_start(args, fmt);
	vsprintf((str + len + time_len), fmt, args);
	va_end(args);

	len = togo_strlen(str);
	togo_memcpy((str + len), "\r\n\0", 3);
	if (togo_global_log != NULL && togo_global_log->file_log == TRUE) {
		togo_log_write(str);
	} else {
		printf(str);
	}
}

void togo_exit()
{
	exit(1);
}

