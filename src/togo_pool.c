/*
 * togo_pool.c
 *
 *  Created on: 2015-7-1
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

static void * togo_pool_build_data(TOGO_POOL * pool, size_t size, u_char * p,
		TOGO_POOL_BLOCK * block);
static void * togo_pool_alloc_block(TOGO_POOL * pool, size_t size);
static void * togo_pool_alloc_large(TOGO_POOL * pool, size_t size);

int search = 10;
int tries = 4;

TOGO_POOL * togo_pool_create(size_t size)
{
	TOGO_POOL * pool;
	size_t pool_size;
	TOGO_POOL_BLOCK * block;

	/* The min size of pool is TOGO_DEFAULT_POOL_MIN_SIZE */
	size = togo_pool_size(size);
	if (TOGO_DEFAULT_POOL_MIN_SIZE > size) {
		size = TOGO_DEFAULT_POOL_SIZE;
	}

	pool_size = sizeof(TOGO_POOL);
	pool = togo_alloc(pool_size);
	if (pool == NULL) {
		togo_log(ERROR, "malloc a pool fail");
		return NULL;
	}
	togo_memzero(pool, pool_size);

	block = togo_alloc(size);
	if (block == NULL) {
		togo_free(pool);
		togo_log(ERROR, "malloc a pool_block fail");
		return NULL;
	}
	togo_memzero(block, sizeof(TOGO_POOL_BLOCK));

	pool->size = size;
	pool->max = size - sizeof(TOGO_POOL_BLOCK);
	pool->total_size = size;
	pool->total_block = 1;

	pool->block = block;
	pool->block_current = pool->block;

	pool->block->start = (u_char *) pool->block + sizeof(TOGO_POOL_BLOCK);
	pool->block->end = (u_char *) pool->block + size;
	pool->block->used = pool->block->start;
	pool->block->size = pool->max;
	pool->block->prev = NULL;
	pool->block->next = NULL;

	pool->free = NULL;
	pool->free_current = NULL;
	pool->large = NULL;

	pthread_mutex_init(&pool->mlock, NULL);

	return pool;
}

void togo_pool_destroy(TOGO_POOL * pool)
{
	pthread_mutex_lock(&pool->mlock);

	TOGO_POOL_LARGE * large = pool->large;
	TOGO_POOL_LARGE * temp_l;
	TOGO_POOL_BLOCK * block = pool->block;
	TOGO_POOL_BLOCK * temp_b;

	while (large != NULL) {

		temp_l = large;
		large = large->next;
		togo_free(temp_l);
	}

	while (block != NULL) {

		temp_b = block;
		block = block->next;
		togo_free(temp_b);
	}

	pthread_mutex_unlock(&pool->mlock);
	togo_free(pool);
	pool = NULL;

}

void * togo_pool_calloc(TOGO_POOL * pool, size_t size)
{
	void * p;
	p = togo_pool_alloc(pool, size);
	if (p) {
		togo_memzero(p, size);
	}
	return p;
}

void * togo_pool_alloc(TOGO_POOL * pool, size_t size)
{
	void * p;
	size_t dsize;
	TOGO_POOL_DATA * free_current;
	TOGO_POOL_DATA * last;
	TOGO_POOL_BLOCK * block;

	pthread_mutex_lock(&pool->mlock);
	block = pool->block_current;
	if (block == NULL) {
		return NULL;
	}

	dsize = size + sizeof(TOGO_POOL_DATA);
	if (dsize < pool->max) {

		/* Search the free list */
		free_current = pool->free_current;
		last = NULL;
		int i;
		size_t space;
		size_t ssize;
		for (i = 0; i < search; i++) {
			if (free_current == NULL) {
				break;
			}

			ssize = free_current->size;
			if (size <= ssize && ((ssize - size) < TOGO_POOL_WASTE_SIZE)) {

				p = (void *) free_current + sizeof(TOGO_POOL_DATA);
				last = free_current->prev;
				if (free_current->prev != NULL) {
					free_current->prev->next = free_current->next;
				}
				if (free_current->next != NULL) {
					free_current->next->prev = free_current->prev;
				}
				if (free_current == pool->free) {
					pool->free = free_current->next;
				}
				pool->free_current =
						(free_current->next == NULL) ?
								pool->free : free_current->next;

				pthread_mutex_unlock(&pool->mlock);
				return p;
			}

			free_current = free_current->next;
			last = free_current;

		}
		pool->free_current = (last == NULL) ? pool->free : last;

		/* Search the free block */
		do {
			p = block->used;
			space = (size_t) abs(block->end - block->used);

			if (space > dsize && (block->end > block->used)) {

				block->used = p + dsize;
				p = togo_pool_build_data(pool, size, p, block);
				pthread_mutex_unlock(&pool->mlock);
				return p;
			}

			block = block->next;

		} while (block);

		/* Create a new block */
		p = togo_pool_alloc_block(pool, size);
		pthread_mutex_unlock(&pool->mlock);
		return p;
	}

	/* Create a new large Memory */
	p = togo_pool_alloc_large(pool, size);
	pthread_mutex_unlock(&pool->mlock);
	return p;
}

void * togo_pool_realloc(TOGO_POOL * pool, void * p, size_t size,
		size_t new_size)
{
	size_t space;
	size_t add_size;
	TOGO_POOL_DATA * data;
	u_char * end;
	TOGO_POOL_BLOCK * block;

	pthread_mutex_lock(&pool->mlock);

	block = pool->block_current;
	if (block == NULL || size >= new_size) {
		return NULL;
	}

	data = (TOGO_POOL_DATA *) (p - sizeof(TOGO_POOL_DATA));

	space = abs(block->end - block->used);
	add_size = new_size - size;
	end = ((u_char *) data) + data->size;

	if (block->used == end && (space > add_size)) {

		block->used = block->used + add_size;
		data->size = data->size + add_size;
		pthread_mutex_unlock(&pool->mlock);
		return p;
	}
	pthread_mutex_unlock(&pool->mlock);

	void * new = togo_pool_calloc(pool, new_size);
	TOGO_POOL_DATA * new_data =
			(TOGO_POOL_DATA *) (new - sizeof(TOGO_POOL_DATA));
	if (new == NULL) {
		return NULL;
	}
	togo_memcpy(new, p, size);

	togo_pool_free_data(pool, (void *) p);

	return new;

}

void togo_pool_free_data(TOGO_POOL * pool, void * p)
{
	TOGO_POOL_DATA * next;
	TOGO_POOL_DATA * prev;

	pthread_mutex_lock(&pool->mlock);
	TOGO_POOL_DATA * data = (TOGO_POOL_DATA *) (p - sizeof(TOGO_POOL_DATA));
	TOGO_POOL_DATA * free = pool->free;

	if (data) {
		if (free == NULL) {
			data->next = NULL;
			data->prev = NULL;
			pool->free = data;
			pool->free_current = data;
			pthread_mutex_unlock(&pool->mlock);
			return;
		}

		data->next = free;
		data->prev = NULL;
		free->prev = data;
		pool->free = data;
	}

	pthread_mutex_unlock(&pool->mlock);

	return;
}

void togo_pool_free_large(TOGO_POOL * pool, void * lp)
{
	TOGO_POOL_LARGE * next;
	TOGO_POOL_LARGE * prev;
	pthread_mutex_lock(&pool->mlock);
	TOGO_POOL_LARGE * large = (TOGO_POOL_LARGE *) (lp - sizeof(TOGO_POOL_LARGE));
	if (large) {
		next = large->next;
		prev = large->prev;

		if (next != NULL) {
			next->prev = prev;
		}
		if (prev != NULL) {
			prev->next = next;
		}
		if (pool->large == large) {
			pool->large = next;
		}
		pool->total_size -= large->size + sizeof(TOGO_POOL_LARGE);
		togo_free(large);
	}

	pthread_mutex_unlock(&pool->mlock);
}

BOOL togo_pool_islarge(TOGO_POOL * pool, size_t size)
{
	size_t dsize;
	dsize = size + sizeof(TOGO_POOL_DATA);

	if (dsize < pool->max) {
		return FALSE;
	}

	return TRUE;
}

static void * togo_pool_build_data(TOGO_POOL * pool, size_t size, u_char * p,
		TOGO_POOL_BLOCK * block)
{
	void * ret;
	TOGO_POOL_DATA * data = (TOGO_POOL_DATA *) p;
	togo_memzero(data, sizeof(TOGO_POOL_DATA));

	data->pool = pool;
	data->block = block;
	data->size = size;
	data->next = NULL;
	data->prev = NULL;

	ret = p + sizeof(TOGO_POOL_DATA);
	return ret;
}

static void * togo_pool_alloc_block(TOGO_POOL * pool, size_t size)
{
	TOGO_POOL_BLOCK * current_block;
	TOGO_POOL_BLOCK * new_block;
	size_t block_size = pool->size;
	u_char * p = NULL;
	int try = 0;

	current_block = pool->block_current;
	if (current_block == NULL) {
		return NULL;
	}

	new_block = togo_alloc(block_size);
	if (new_block == NULL) {
		togo_log(ERROR, "malloc a pool_block fail");
		return NULL;
	}

	pool->total_block++;

	new_block->start = (u_char *) new_block + sizeof(TOGO_POOL_BLOCK);
	new_block->end = (u_char *) new_block + block_size;
	new_block->used = new_block->start + size + sizeof(TOGO_POOL_DATA);
	new_block->size = pool->max;

	p = new_block->start;

	do {
		try++;

		if (current_block->next == NULL) {

			current_block->next = new_block;
			new_block->prev = current_block;
			new_block->next = NULL;
			break;
		}

		current_block = current_block->next;

	} while (current_block);

	if (try >= tries) {
		pool->block_current = new_block;
	}

	pool->total_size += pool->size;

	return togo_pool_build_data(pool, size, p, new_block);
}

static void * togo_pool_alloc_large(TOGO_POOL * pool, size_t size)
{
	void * p;
	TOGO_POOL_LARGE * new_large;
	size_t all_size;

	all_size = sizeof(TOGO_POOL_LARGE) + size;
	new_large = togo_alloc(all_size);
	if (new_large == NULL) {
		togo_log(ERROR, "malloc a pool_large fail");
		pthread_mutex_unlock(&pool->mlock);
		return NULL;
	}
	togo_memzero(new_large, sizeof(TOGO_POOL_LARGE));

	p = (void *) new_large + sizeof(TOGO_POOL_LARGE);

	new_large->size = size;
	new_large->p = p;
	new_large->next = pool->large;
	new_large->prev = NULL;
	if (pool->large != NULL) {
		pool->large->prev = new_large;
	}

	pool->total_size += all_size;
	pool->large = new_large;

	return p;
}

