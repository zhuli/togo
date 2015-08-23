/*
 * init.h
 * Initialize the Togo system.
 * Created on: 2014-2-20
 * Author: zhuli
 */
#ifndef TOGO_SERVER_H
#define TOGO_SERVER_H

#define TOGO_S_MSG_TAG '\n'
#define TOGO_S_MAX_LINE    1024
#define TOGO_S_RBUF_INIT_SIZE 1024
#define TOGO_S_RBUF_MAX_SIZE 8192
#define TOGO_S_SBUF_INIT_SIZE 1024
#define TOGO_S_SBUF_MAX_SIZE 8192
#define TOGO_S_SBUF_SPACE_SIZE 128

typedef struct togo_thread_item TOGO_THREAD_ITEM;
typedef struct togo_thread_queue TOGO_THREAD_QUEUE;
typedef struct togo_worker_threads TOGO_WORKER_THREAD;
typedef void (*BDATA_CALLBACK)(TOGO_THREAD_ITEM * socket_item);
typedef int (*SEND_CALLBACK)(TOGO_THREAD_ITEM * socket_item);

struct togo_thread_item {
	int sfd; /* socket ID */
	struct bufferevent *bev;

	u_char * rbuf; /* The buffer stores client data*/
	int rsize; /* The size of the rbuf */
	u_char * rcurr; /* Record the usage of read_buf */
	int rbytes;

	int sstatus; /* Send data status! 0-nomal; 1-not need send data; 2-send fail*/
	u_char * sbuf; /* The buffer to send data */
	int sbuf_size; /* The buffer size */
	int ssize; /* The length of the send data */

	int bstatus; /* The status of when to read the big data! 0-nomal;1-read big data*/
	void * bbuf; /* The buffer to read the big data !*/
	size_t bsize; /* The size of the bbuf */
	void * bcurr; /* Record the usage of bbuf */
	TOGO_POOL * bpool; /* pool */
	BDATA_CALLBACK bcb; /* The callback function when read the end of the big data*/

	TOGO_THREAD_ITEM *next; /* a list of TOGO_THREAD_ITEM struct */
	TOGO_POOL * worker_pool; /* Worker memory pool */
};

struct togo_thread_queue {
	TOGO_THREAD_ITEM * head; /* Queue head */
	TOGO_THREAD_ITEM * tail; /* Queue tail */
	pthread_mutex_t queue_lock; /* Queue lock */
};

struct togo_worker_threads {
	int tid; /* Thread id */
	pthread_mutex_t mutex_lock; /* Worker lock */
	struct event_base * base; /* libevent event_base，every thread have a event_base*/
	struct event *pipe_event; /* pipe event */
	int notify_receive_fd; /* pipe receive*/
	int notify_send_fd; /* pipe send*/
	TOGO_THREAD_QUEUE * queue; /* Work thread queue */
};
TOGO_WORKER_THREAD * togo_worker_threads;

/**
 * Run The Togo Tcp Server!
 */
BOOL togo_server_init(void);

#endif
