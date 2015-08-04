/*
 * togo_m_queue.c
 *
 *  Created on: 2015-7-11
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

static TOGO_M_QUEUE * togo_m_queue_create(u_char * name);
static TOGO_M_QUEUE * togo_m_queue_get(u_char * name);
static void togo_m_queue_block_create();
static TOGO_M_QUEUE_BLOCK * togo_m_queue_block_get();
static void togo_m_queue_block_free(TOGO_M_QUEUE_BLOCK * block,
		TOGO_POOL * pool, TOGO_M_QUEUE * queue);

void togo_m_queue_command(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item)
{
	u_char * action;
	u_char * qname;
	u_char * value;
	size_t len;

	if (command_tag[1].value) {
		action = command_tag[1].value;
	}
	if (command_tag[2].value) {
		qname = command_tag[2].value;
	}
	if (command_tag[3].value) {
		value = command_tag[3].value;
		len = command_tag[3].length;
	}
	if (action == NULL || qname == NULL) {
		return;
	}

	if (strcmp(action, "RPUSH") == 0) {
		if (value == NULL) {
			return;
		}
		togo_m_queue_rpush(qname, value, len);

	} else if (strcmp(command_tag[1].value, "LPUSH") == 0) {
		if (value == NULL) {
			return;
		}
		togo_m_queue_lpush(qname, value, len);

	} else if (strcmp(command_tag[1].value, "LPOP") == 0) {
		togo_m_queue_lpop(qname, socket_item);

	} else if (strcmp(command_tag[1].value, "RPOP") == 0) {
		togo_m_queue_rpop(qname, socket_item);

	} else if (strcmp(command_tag[1].value, "COUNT") == 0) {
		togo_m_queue_count(qname, socket_item);
	}
}

void togo_m_queue_init(void)
{
	togo_m_queue_pool = togo_pool_create(
			togo_pool_size(TOGO_M_QUEUE_POOL_SIZE));
	if (togo_m_queue_pool == NULL) {
		togo_log(ERROR, "Initialize modules_queue's pool fail.");
		togo_exit();
	}

	togo_m_queue_hashtable = togo_hashtable_init(togo_m_queue_pool);
	if (togo_m_queue_hashtable == NULL) {
		togo_log(ERROR, "Initialize modules_queue's hashtable fail.");
		togo_exit();
	}

	togo_m_queue_fblock = togo_pool_calloc(togo_m_queue_pool,
			sizeof(TOGO_M_QUEUE_FBLOCK));
	if (togo_m_queue_fblock == NULL) {
		togo_log(ERROR, "Initialize modules_queue's free block fail.");
		togo_exit();
	}
	pthread_mutex_init(&togo_m_queue_fblock->flock, NULL);

}

BOOL togo_m_queue_rpush(u_char * name, u_char * val, size_t len)
{
	TOGO_M_QUEUE_ITEM * item;
	TOGO_M_QUEUE_BLOCK * block;
	TOGO_M_QUEUE * queue;
	size_t space;

	queue = togo_m_queue_get(name);
	if (queue == NULL) {
		return FALSE;
	}

	pthread_mutex_lock(&queue->qlock);

	if (queue->block == NULL) {
		block = togo_m_queue_block_get();
		if (block == NULL) {
			pthread_mutex_unlock(&queue->qlock);
			return FALSE;
		}
		block->next = NULL;
		block->prev = NULL;
		block->curr = block->buf;

		queue->block = block;
		queue->total_block++;

	} else {
		block = queue->block;
		space = abs((block->buf + block->size) - block->curr);

		/* We need to create a new block when have no enough space! */
		if (space < (len + sizeof(TOGO_M_QUEUE_ITEM))) {

			block = togo_m_queue_block_get();
			if (block == NULL) {
				pthread_mutex_unlock(&queue->qlock);
				return FALSE;
			}

			block->next = queue->block;
			block->prev = NULL;
			block->curr = block->buf;
			queue->block->prev = block;

			queue->block = block;
			queue->total_block++;

		}
	}

	item = (TOGO_M_QUEUE_ITEM *) block->curr;
	togo_memzero(block->curr, sizeof(TOGO_M_QUEUE_ITEM));
	block->curr = block->curr + sizeof(TOGO_M_QUEUE_ITEM);

	item->block = block;
	item->val = block->curr;
	item->size = len;
	item->next = NULL;
	item->prev = queue->tail;

	if (queue->tail != NULL) {
		queue->tail->next = item;
	}
	queue->tail = item;
	if (queue->head == NULL) {
		queue->head = item;
	}
	block->nelt++;
	queue->total_elt++;

	togo_memcpy(item->val, val, len);
	block->curr = block->curr + len;

	pthread_mutex_unlock(&queue->qlock);

	return TRUE;
}

BOOL togo_m_queue_lpush(u_char * name, u_char * val, size_t len)
{
	TOGO_M_QUEUE_ITEM * item;
	TOGO_M_QUEUE_BLOCK * block;
	TOGO_M_QUEUE * queue;
	size_t space;

	queue = togo_m_queue_get(name);
	if (queue == NULL) {
		return FALSE;
	}

	pthread_mutex_lock(&queue->qlock);

	if (queue->block == NULL) {
		block = togo_m_queue_block_get();
		if (block == NULL) {
			pthread_mutex_unlock(&queue->qlock);
			return FALSE;
		}

		block->next = NULL;
		block->prev = NULL;
		block->curr = block->buf;

		queue->block = block;
		queue->total_block++;

	} else {
		block = queue->block;
		space = abs((block->buf + block->size) - block->curr);

		/* We need to create a new block when have no enough space! */
		if (space < (len + sizeof(TOGO_M_QUEUE_ITEM))) {

			block = togo_m_queue_block_get();
			if (block == NULL) {
				pthread_mutex_unlock(&queue->qlock);
				return FALSE;
			}

			block->next = queue->block;
			block->prev = NULL;
			queue->block->prev = block;
			block->curr = block->buf;

			queue->block = block;
			queue->total_block++;

		}
	}

	item = (TOGO_M_QUEUE_ITEM *) block->curr;
	togo_memzero(block->curr, sizeof(TOGO_M_QUEUE_ITEM));
	block->curr = block->curr + sizeof(TOGO_M_QUEUE_ITEM);

	item->block = block;
	item->next = queue->head;
	item->prev = NULL;
	item->val = block->curr;
	item->size = len;

	if (queue->head) {
		queue->head->prev = item;
	}
	queue->head = item;
	if (queue->tail == NULL) {
		queue->tail = item;
	}
	block->nelt++;
	queue->total_elt++;

	togo_memcpy(item->val, val, len);
	block->curr = block->curr + len;

	pthread_mutex_unlock(&queue->qlock);

	return TRUE;
}

void togo_m_queue_rpop(u_char * name, TOGO_THREAD_ITEM *socket_item)
{
	TOGO_HASHTABLE_ITEM * hash_item;
	TOGO_M_QUEUE_ITEM * item;
	TOGO_M_QUEUE_BLOCK * block;
	TOGO_M_QUEUE * queue;

	queue = togo_m_queue_get(name);
	if (queue == NULL) {
		return;
	}

	pthread_mutex_lock(&queue->qlock);

	if (queue->tail == NULL) {
		queue->head = NULL;
		pthread_mutex_unlock(&queue->qlock);
		return;
	}

	item = queue->tail;
	if (item->prev != NULL) {
		item->prev->next = NULL;
	}
	queue->tail = item->prev;
	queue->total_elt--;
	block = item->block;
	block->nelt--;

	if (queue->total_elt == 0) {
		queue->tail = NULL;
		queue->head = NULL;
	}

	togo_send_data(socket_item, item->val, item->size);

	if (block->nelt == 0) {
		togo_m_queue_block_free(block, queue->pool, queue);
		queue->total_block--;
	}

	pthread_mutex_unlock(&queue->qlock);

}

void togo_m_queue_lpop(u_char * name, TOGO_THREAD_ITEM *socket_item)
{
	TOGO_HASHTABLE_ITEM * hash_item;
	TOGO_M_QUEUE_ITEM * item;
	TOGO_M_QUEUE_BLOCK * block;
	TOGO_M_QUEUE * queue;

	queue = togo_m_queue_get(name);
	if (queue == NULL) {
		return;
	}

	pthread_mutex_lock(&queue->qlock);

	if (queue->head == NULL) {
		queue->tail = NULL;
		pthread_mutex_unlock(&queue->qlock);
		return;
	}
	item = queue->head;
	if (item->next != NULL) {
		item->next->prev = NULL;
	}
	queue->head = item->next;
	queue->total_elt--;
	block = item->block;
	block->nelt--;

	if (queue->total_elt == 0) {
		queue->tail = NULL;
		queue->head = NULL;
	}

	togo_send_data(socket_item, item->val, item->size);

	if (block->nelt == 0) {
		togo_m_queue_block_free(block, queue->pool, queue);
		queue->total_block--;
	}

	pthread_mutex_unlock(&queue->qlock);

}

void togo_m_queue_count(u_char * name, TOGO_THREAD_ITEM * socket_item)
{
	uint32_t count;
	TOGO_M_QUEUE * queue;
	size_t str_size;

	queue = togo_m_queue_get(name);
	count = (queue == NULL) ? 0 : queue->total_elt;

	str_size = 20;
	u_char * str = togo_pool_calloc(queue->pool, str_size);
	togo_itoa(count, str, 10);

	togo_send_data(socket_item, str, strlen(str));

	togo_pool_free_data(queue->pool, str);
}

static TOGO_M_QUEUE * togo_m_queue_create(u_char * name)
{
	TOGO_M_QUEUE * queue;
	TOGO_M_QUEUE_BLOCK * block;

	queue = togo_pool_calloc(togo_m_queue_pool, sizeof(TOGO_M_QUEUE));
	if (queue == NULL) {
		return NULL;
	}

	u_char * buf = togo_pool_alloc(togo_m_queue_pool, togo_pool_strlen(name));
	if (buf == NULL) {
		return NULL;
	}
	togo_strcpy(buf, name);

	block = togo_m_queue_block_get();
	if (block == NULL) {
		return NULL;
	}

	queue->name = buf;
	queue->block = block;
	queue->total_block = 1;
	queue->total_elt = 0;
	queue->tail = NULL;
	queue->head = NULL;
	queue->pool = togo_m_queue_pool;

	pthread_mutex_init(&queue->qlock, NULL);

	return queue;
}

static TOGO_M_QUEUE * togo_m_queue_get(u_char * name)
{
	TOGO_HASHTABLE_ITEM * hash_item;
	TOGO_M_QUEUE * queue;

	hash_item = togo_hashtable_get(togo_m_queue_hashtable, name);

	/* if does not find a queue, we need to create a new queue!*/
	if (hash_item == NULL) {
		TOGO_M_QUEUE * queue = togo_m_queue_create(name);
		if (queue == NULL) {
			return NULL;
		}

		BOOL ret = togo_hashtable_add(togo_m_queue_hashtable, queue->name,
				(void *) queue);
		if (ret == FALSE) {
			return NULL;
		}
		hash_item = togo_hashtable_get(togo_m_queue_hashtable, name);
	}

	queue = (TOGO_M_QUEUE *) hash_item->p;
	return queue;
}

static void togo_m_queue_block_create()
{
	TOGO_M_QUEUE_BLOCK * block_s;
	u_char * block;
	int i;

	for (i = 0; i < TOGO_M_QUEUE_BLOCK_NUM; i++) {

		/* Alloc a large memory to store blocks.
		 * Each large memory can store TOGO_M_QUEUE_BLOCK_NUM block.*/
		block_s = togo_pool_calloc(togo_m_queue_pool,
				sizeof(TOGO_M_QUEUE_BLOCK));
		block = togo_pool_calloc(togo_m_queue_pool, TOGO_M_QUEUE_BLOCK_SIZE);
		if (block_s == NULL || block == NULL) {

			if (block != NULL) {
				togo_pool_free_large(togo_m_queue_pool, block);
			}
			if (block_s != NULL) {
				togo_pool_free_data(togo_m_queue_pool, block_s);
			}
			togo_log(ERROR, "Initialize modules_queue's block fail.");
			return;
		}

		block_s->buf = block;
		block_s->nelt = 0;
		block_s->next = (TOGO_M_QUEUE_BLOCK *) togo_m_queue_fblock->block;
		block_s->prev = NULL;
		block_s->size = TOGO_M_QUEUE_BLOCK_SIZE;
		block_s->curr = block;

		togo_m_queue_fblock->block = block_s;
		togo_m_queue_fblock->total++;
	}
}

static TOGO_M_QUEUE_BLOCK * togo_m_queue_block_get()
{
	TOGO_M_QUEUE_BLOCK * block;
	TOGO_M_QUEUE_BLOCK * block_next;

	pthread_mutex_lock(&togo_m_queue_fblock->flock);

	if (togo_m_queue_fblock->block == NULL || togo_m_queue_fblock->total == 0) {
		togo_m_queue_fblock->total = 0;
		togo_m_queue_block_create();
		if (togo_m_queue_fblock->block == NULL) {

			togo_log(ERROR, "alloc a modules_queue's block fail.");
			pthread_mutex_unlock(&togo_m_queue_fblock->flock);
			return NULL;
		}
	}

	block_next = togo_m_queue_fblock->block->next;
	block = togo_m_queue_fblock->block;
	togo_m_queue_fblock->block = block_next;
	togo_m_queue_fblock->total--;

	pthread_mutex_unlock(&togo_m_queue_fblock->flock);

	return block;
}

static void togo_m_queue_block_free(TOGO_M_QUEUE_BLOCK * block,
		TOGO_POOL * pool, TOGO_M_QUEUE * queue)
{
	TOGO_M_QUEUE_BLOCK * prev;
	TOGO_M_QUEUE_BLOCK * next;
	TOGO_M_QUEUE_BLOCK * temp;
	pthread_mutex_lock(&togo_m_queue_fblock->flock);

	prev = block->prev;
	next = block->next;

	if (queue->total_elt == 0) {
		queue->block = NULL;
	} else {
		if (block == queue->block) {
			if (next != NULL) {
				next->prev = NULL;
			}
			queue->block = next;
		} else {
			if (next != NULL) {
				next->prev = prev;
			}
			if (prev != NULL) {
				prev->next = next;
			}
		}
	}

	block->curr = block->buf;
	block->nelt = 0;
	block->prev = NULL;
	block->next = NULL;

	if (block->buf == NULL) {
		togo_pool_free_data(pool, block);
	} else {
		if (togo_m_queue_fblock->total >= TOGO_M_QUEUE_BLOCK_FREELIST_MAX) {
			togo_pool_free_large(pool, block->buf);
			togo_pool_free_data(pool, block);

		} else {
			block->next = togo_m_queue_fblock->block;

			togo_m_queue_fblock->block = block;
			togo_m_queue_fblock->total++;
		}
	}

	pthread_mutex_unlock(&togo_m_queue_fblock->flock);
}

