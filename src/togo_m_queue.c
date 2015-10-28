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
static void togo_m_queue_pi_put(size_t priority, TOGO_M_QUEUE * queue,
		TOGO_M_QUEUE_ITEM * item);
static TOGO_M_QUEUE_ITEM * togo_m_queue_pi_get(TOGO_M_QUEUE * queue);

BOOL togo_m_queue_command(TOGO_COMMAND_TAG command_tag[],
		TOGO_THREAD_ITEM *socket_item, int ntag)
{
	u_char * action = NULL;
	u_char * qname = NULL;
	u_char * value = NULL;
	size_t len = 0;
	BOOL ret = FALSE;
	uint32_t priority = 0;

	/**
	 * command_tag[0] : Module  QUEUE
	 * command_tag[1] : Action  RPUSH|LPUSH|LPOP|RPOP|COUNT|STATUS
	 * command_tag[2] : Object  Queue name
	 * command_tag[3] : Value   value
	 * command_tag[4] : Option  priority:1|2|3
	 */
	action = command_tag[1].value;
	qname = command_tag[2].value;

	if (ntag > 3) {
		value = command_tag[3].value;
		len = command_tag[3].length;
	}
	if (ntag > 4) {
		if (command_tag[4].length > 0) {
			priority = togo_atoi(command_tag[4].value, 1);
			if (priority < 1) {
				priority = 0;
			}
			if (priority > 3) {
				priority = 3;
			}
		}
	}

	if (action == NULL || qname == NULL) {
		return ret;
	}

	if (togo_strcmp(action, "RPUSH") == 0) {
		if (value == NULL || len == 0) {
			return ret;
		}
		ret = togo_m_queue_rpush(qname, value, len, priority);

	} else if (togo_strcmp(action, "LPUSH") == 0) {
		if (value == NULL || len == 0) {
			return ret;
		}
		ret = togo_m_queue_lpush(qname, value, len, priority);

	} else if (togo_strcmp(action, "LPOP") == 0) {
		ret = togo_m_queue_lpop(qname, socket_item);

	} else if (togo_strcmp(action, "RPOP") == 0) {
		ret = togo_m_queue_rpop(qname, socket_item);

	} else if (togo_strcmp(action, "COUNT") == 0) {
		ret = togo_m_queue_count(qname, socket_item);

	} else if (togo_strcmp(action, "STATUS") == 0) {
		ret = togo_m_queue_status(qname, socket_item);
	}

	return ret;
}

void togo_m_queue_init(void)
{
	togo_m_queue_pool = togo_pool_create(TOGO_M_QUEUE_POOL_SIZE);
	if (togo_m_queue_pool == NULL) {
		togo_log(ERROR, "Initialize modules_queue's pool fail.");
		togo_exit();
	}

	togo_m_queue_hashtable = togo_hashtable_init(togo_m_queue_pool);
	if (togo_m_queue_hashtable == NULL) {
		togo_log(ERROR, "Initialize modules_queue's hashtable fail.");
		togo_exit();
	}

	togo_m_queue_fblock = (TOGO_M_QUEUE_FBLOCK *) togo_pool_calloc(
			togo_m_queue_pool, sizeof(TOGO_M_QUEUE_FBLOCK));
	if (togo_m_queue_fblock == NULL) {
		togo_log(ERROR, "Initialize modules_queue's free block fail.");
		togo_exit();
	}
	pthread_mutex_init(&togo_m_queue_fblock->flock, NULL);
	pthread_mutex_init(&togo_m_queue_glock, NULL);

}

BOOL togo_m_queue_rpush(u_char * name, u_char * val, size_t len,
		uint32_t priority)
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
		queue->total_size += togo_m_queue_block_size();

	} else {
		block = queue->block;
		space = abs((block->buf + block->size) - block->curr);

		/* We need to create a new block when have no enough space! */
		if (space < (len + sizeof(TOGO_M_QUEUE_ITEM))) {

			if (queue->total_size >= TOGO_M_QUEUE_MAX_SIZE) {
				pthread_mutex_unlock(&queue->qlock);
				return FALSE;
			}

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
			queue->total_size += togo_m_queue_block_size();

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

	/* Put the item into priority queue, If set the priority */
	if (priority > 0) {
		item->next = NULL;
		item->prev = NULL;
		togo_m_queue_pi_put(priority, queue, item);

	} else {
		if (queue->tail != NULL) {
			queue->tail->next = item;
		}
		queue->tail = item;
		if (queue->head == NULL) {
			queue->head = item;
		}
	}

	block->nelt++;
	queue->total_elt++;
	queue->total_hit++;
	queue->total_write++;

	togo_memcpy(item->val, val, len);
	block->curr = block->curr + len;

	pthread_mutex_unlock(&queue->qlock);

	return TRUE;
}

BOOL togo_m_queue_lpush(u_char * name, u_char * val, size_t len,
		uint32_t priority)
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
		queue->total_size += togo_m_queue_block_size();

	} else {
		block = queue->block;
		space = abs((block->buf + block->size) - block->curr);

		/* We need to create a new block when have no enough space! */
		if (space < (len + sizeof(TOGO_M_QUEUE_ITEM))) {

			if (queue->total_size >= TOGO_M_QUEUE_MAX_SIZE) {
				pthread_mutex_unlock(&queue->qlock);
				return FALSE;
			}

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
			queue->total_size += togo_m_queue_block_size();

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

	/* Put the item into priority queue, If set the priority */
	if (priority > 0) {
		item->next = NULL;
		item->prev = NULL;
		togo_m_queue_pi_put(priority, queue, item);

	} else {
		if (queue->head) {
			queue->head->prev = item;
		}
		queue->head = item;
		if (queue->tail == NULL) {
			queue->tail = item;
		}
	}

	block->nelt++;
	queue->total_elt++;
	queue->total_hit++;
	queue->total_write++;

	togo_memcpy(item->val, val, len);
	block->curr = block->curr + len;

	pthread_mutex_unlock(&queue->qlock);

	return TRUE;
}

BOOL togo_m_queue_rpop(u_char * name, TOGO_THREAD_ITEM *socket_item)
{
	TOGO_M_QUEUE_ITEM * item;
	TOGO_M_QUEUE_BLOCK * block;
	TOGO_M_QUEUE * queue;
	TOGO_M_QUEUE_ITEM * pi;

	queue = togo_m_queue_get(name);
	if (queue == NULL) {
		return FALSE;
	}

	pthread_mutex_lock(&queue->qlock);

	/* Search the priority list */
	pi = togo_m_queue_pi_get(queue);
	if (pi != NULL) {
		item = pi;
	} else {
		if (queue->tail == NULL) {
			queue->head = NULL;
			pthread_mutex_unlock(&queue->qlock);
			togo_send_null(socket_item); /* Send "TOGO_NULL"*/
			return TRUE;
		}

		item = queue->tail;
		if (item->prev != NULL) {
			item->prev->next = NULL;
		}
		queue->tail = item->prev;
	}

	queue->total_elt--;
	queue->total_hit++;
	queue->total_read++;
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
		queue->total_size -= togo_m_queue_block_size();
	}

	pthread_mutex_unlock(&queue->qlock);
	return TRUE;
}

BOOL togo_m_queue_lpop(u_char * name, TOGO_THREAD_ITEM *socket_item)
{
	TOGO_M_QUEUE_ITEM * item;
	TOGO_M_QUEUE_BLOCK * block;
	TOGO_M_QUEUE * queue;
	TOGO_M_QUEUE_ITEM * pi;

	queue = togo_m_queue_get(name);
	if (queue == NULL) {
		return FALSE;
	}

	pthread_mutex_lock(&queue->qlock);

	/* Search the priority list */
	pi = togo_m_queue_pi_get(queue);
	if (pi != NULL) {
		item = pi;
	} else {
		if (queue->head == NULL) {
			queue->tail = NULL;
			pthread_mutex_unlock(&queue->qlock);
			togo_send_null(socket_item); /* Send "TOGO_NULL"*/
			return TRUE;
		}
		item = queue->head;
		if (item->next != NULL) {
			item->next->prev = NULL;
		}
		queue->head = item->next;
	}

	queue->total_elt--;
	queue->total_hit++;
	queue->total_read++;
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
		queue->total_size -= togo_m_queue_block_size();
	}

	pthread_mutex_unlock(&queue->qlock);
	return TRUE;
}

BOOL togo_m_queue_count(u_char * name, TOGO_THREAD_ITEM * socket_item)
{
	uint32_t count;
	TOGO_M_QUEUE * queue;
	size_t str_size;

	queue = togo_m_queue_get(name);
	count = (queue == NULL) ? 0 : queue->total_elt;

	u_char * str = (u_char *) togo_pool_alloc(queue->pool, 12);
	togo_itoa(count, str, 10);

	togo_send_data(socket_item, str, togo_strlen(str));
	togo_pool_free_data(queue->pool, (void *) str);
	return TRUE;
}

BOOL togo_m_queue_status(u_char * name, TOGO_THREAD_ITEM * socket_item)
{
	TOGO_M_QUEUE * queue;
	TOGO_STRING * togo_str;

	queue = togo_m_queue_get(name);
	if (queue == NULL) {
		return FALSE;
	}
	pthread_mutex_lock(&queue->qlock);

	togo_str = togo_string_init(queue->pool, TOGO_STRING_DEFAULT_SIZE);
	if (togo_str == NULL) {
		return FALSE;
	}

	togo_string_append_s(&togo_str, "total_elt:");
	togo_string_append_i(&togo_str, queue->total_elt);
	togo_string_append_s(&togo_str, ";total_block:");
	togo_string_append_i(&togo_str, queue->total_block);
	togo_string_append_s(&togo_str, ";total_hit:");
	togo_string_append_i(&togo_str, queue->total_hit);
	togo_string_append_s(&togo_str, ";total_write:");
	togo_string_append_i(&togo_str, queue->total_write);
	togo_string_append_s(&togo_str, ";total_read:");
	togo_string_append_i(&togo_str, queue->total_read);
	togo_string_append_s(&togo_str, ";total_size:");
	togo_string_append_i(&togo_str, queue->total_size);

	togo_send_data(socket_item, togo_str->buf, togo_str->str_size);

	togo_string_destroy(togo_str);

	pthread_mutex_unlock(&queue->qlock);

	return TRUE;
}

static TOGO_M_QUEUE * togo_m_queue_create(u_char * name)
{
	TOGO_M_QUEUE * queue;
	TOGO_M_QUEUE_BLOCK * block;
	uint32_t klen;

	klen = togo_strlen(name);
	queue = (TOGO_M_QUEUE *) togo_pool_calloc(togo_m_queue_pool,
			sizeof(TOGO_M_QUEUE));
	if (queue == NULL) {
		return NULL;
	}

	u_char * buf = (u_char *) togo_pool_alloc(togo_m_queue_pool, klen + 1);
	if (buf == NULL) {
		return NULL;
	}
	togo_memcpy(buf, name, klen);
	*(buf + klen + 1) = '\0';

	block = togo_m_queue_block_get();
	if (block == NULL) {
		return NULL;
	}

	queue->name = buf;
	queue->block = block;
	queue->total_block = 1;
	queue->total_elt = 0;
	queue->total_size = togo_m_queue_block_size();
	queue->total_hit = 0;
	queue->total_read = 0;
	queue->total_write = 0;
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
	if (hash_item == NULL) {
		/* if does not find a queue, we need to create a new queue!*/
		pthread_mutex_lock(&togo_m_queue_glock);

		hash_item = togo_hashtable_get(togo_m_queue_hashtable, name);

		if (hash_item == NULL) {
			TOGO_M_QUEUE * queue = togo_m_queue_create(name);
			if (queue == NULL) {
				pthread_mutex_unlock(&togo_m_queue_glock);
				return NULL;
			}

			BOOL ret = togo_hashtable_add(togo_m_queue_hashtable, queue->name,
					(void *) queue);
			if (ret == FALSE) {
				pthread_mutex_unlock(&togo_m_queue_glock);
				return NULL;
			}
			hash_item = togo_hashtable_get(togo_m_queue_hashtable, name);
		}

		pthread_mutex_unlock(&togo_m_queue_glock);
	}

	queue = (TOGO_M_QUEUE *) hash_item->p;

	return queue;
}

static void togo_m_queue_block_create()
{
	TOGO_M_QUEUE_BLOCK * block_s;
	u_char * block;
	uint32_t i;

	for (i = 0; i < TOGO_M_QUEUE_BLOCK_NUM; i++) {

		/* Alloc a large memory to store blocks.
		 * Each large memory can store TOGO_M_QUEUE_BLOCK_NUM block.*/
		block_s = (TOGO_M_QUEUE_BLOCK *) togo_pool_calloc(togo_m_queue_pool,
				sizeof(TOGO_M_QUEUE_BLOCK));
		block = (u_char *) togo_pool_calloc(togo_m_queue_pool,
				TOGO_M_QUEUE_BLOCK_SIZE);
		if (block_s == NULL || block == NULL) {

			if (block != NULL) {
				togo_pool_free_large(togo_m_queue_pool, (void *) block);
			}
			if (block_s != NULL) {
				togo_pool_free_data(togo_m_queue_pool, (void *) block_s);
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
		togo_pool_free_data(pool, (void *) block);
	} else {
		if (togo_m_queue_fblock->total >= TOGO_M_QUEUE_BLOCK_FREELIST_MAX) {
			togo_pool_free_large(pool, (void *) block->buf);
			togo_pool_free_data(pool, (void *) block);

		} else {
			block->next = togo_m_queue_fblock->block;

			togo_m_queue_fblock->block = block;
			togo_m_queue_fblock->total++;
		}
	}

	pthread_mutex_unlock(&togo_m_queue_fblock->flock);
}

static void togo_m_queue_pi_put(size_t priority, TOGO_M_QUEUE * queue,
		TOGO_M_QUEUE_ITEM * item)
{
	/* Level equal to 1*/
	if (priority == 1) {
		if (queue->pi_1 == NULL) {
			queue->pi_1 = item;
		} else {
			item->next = queue->pi_1;
			queue->pi_1 = item;
		}
	}

	/* Level equal to 2*/
	if (priority == 2) {
		if (queue->pi_2 == NULL) {
			queue->pi_2 = item;
		} else {
			item->next = queue->pi_2;
			queue->pi_2 = item;
		}
	}

	if (priority == 3) {
		if (queue->pi_3 == NULL) {
			queue->pi_3 = item;
		} else {
			item->next = queue->pi_3;
			queue->pi_3 = item;
		}
	}
}

static TOGO_M_QUEUE_ITEM * togo_m_queue_pi_get(TOGO_M_QUEUE * queue)
{
	TOGO_M_QUEUE_ITEM * item;

	if (queue->pi_3 != NULL) {
		item = queue->pi_3;
		queue->pi_3 = item->next;
		return item;
	}

	if (queue->pi_2 != NULL) {
		item = queue->pi_2;
		queue->pi_2 = item->next;
		return item;
	}

	if (queue->pi_1 != NULL) {
		item = queue->pi_1;
		queue->pi_1 = item->next;
		return item;
	}

	return NULL;
}
