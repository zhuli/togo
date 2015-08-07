/*
 * command.h
 *
 * Created on: 2015-6-16
 * Author: zhuli
 */
#ifndef TOGO_COMMAND_H
#define TOGO_COMMAND_H

#define TOGO_COMMAND_TAG_MAX 10
#define TOGO_COMMAND_MAX_SIZE 25

#define TOGO_SBUF_START "TOGO_S"
#define TOGO_SBUF_END "TOGO_E\r\n"
#define TOGO_SBUF_OK "OK"
#define TOGO_SBUF_ERROR "ERROR"
#define TOGO_SBUF_NULL "NULL"
#define TOGO_SBUF_COMMAND_TOO_BIG "COMMAND_TOO_BIG"
#define TOGO_SBUF_TOO_BIG "TOO_BIG\r\n"

#define togo_send_data togo_command_build_send
#define togo_not_send_data togo_command_build_not_send
#define togo_read_data togo_command_build_read

/**
 * command sign
 */
typedef struct togo_command_tag TOGO_COMMAND_TAG;

struct togo_command_tag {
	u_char *value;
	size_t length;
};

enum TOGO_READ_NETWORK {
	READ_DATA_RECEIVED, READ_NO_DATA_RECEIVED, READ_ERROR
};

/**
 * Through TCP to read the network data.
 * The data will be stored in the socket_item->rbuf.
 */
enum TOGO_READ_NETWORK togo_command_read_network(struct bufferevent *bev,
		TOGO_THREAD_ITEM * socket_item);
/**
 * Parse client's command.
 * If we can't parse one command, we need to wait more data.
 * If we can parse one command, we need to process to this command.
 * We will be a command to parse it out through TOGO_S_MSG_TAG sign
 */
BOOL togo_command_parse_command(TOGO_THREAD_ITEM * socket_item,
		SEND_CALLBACK togo_wt_send_cb);
/**
 * When setting the socket_item->bstatus=1, We will reading the big data
 * from socket_item->rbuf. The socket_item->bstatus will be set 0
 * Only read enough data.
 */
void togo_command_read_big_data(TOGO_THREAD_ITEM * socket_item,
		SEND_CALLBACK togo_wt_send_cb);
void togo_command_build_read(TOGO_THREAD_ITEM * socket_item, TOGO_POOL * bpool,
		u_char * buf, size_t len, BDATA_CALLBACK callback);
void togo_command_build_send(TOGO_THREAD_ITEM * socket_item, u_char * buf,
		size_t len);
void togo_command_build_not_send(TOGO_THREAD_ITEM * socket_item);

#endif
