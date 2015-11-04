/*
 * togo.h
 *
 * Created on: 2015-2-15
 * Author: zhuli
 */
#ifndef TOGO_H
#define TOGO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>
#include <linux/tcp.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <pthread.h>
#include <event2/buffer.h>

#define TOGO_VERSION "1.0.0"

#define TOGO_C_DEFAULT_IP "localhost"
#define TOGO_C_DEFAULT_PORT 8787
#define TOGO_C_DEFAULT_WTN 8
#define TOGO_C_VERSION "version"
#define TOGO_C_IP "ip"
#define TOGO_C_PORT "port"
#define TOGO_C_DAEMON "daemon"
#define TOGO_C_WTN "worker_thread_num"
#define TOGO_C_LOG "log_file"
#define TOGO_C_DEFAULT "conf/togo.conf"
#define TOGO_C_PATH "../conf/togo.conf"

#define TOGO_POOL_SIZE (1024 * 1024)
#define TOGO_WORKER_POOL_SIZE (16 * 1024)

#define BOOL int
#define TRUE 1
#define FALSE 0
#define NUM_NULL -1

enum common_ret {
	SUCCESS, FAIL
};

enum log_level {
	ERROR, DEBUG, INFO, TEST
};

typedef struct togo_c {
	char *version;
	char *ip;
	int port;
	int daemon;
	int worker_thread_num;
	char * log_file;
} TOGO_C;

#endif
