/*
 * togo_array.c
 *
 *  Created on: 2015-7-9
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

TOGO_ARRAY * togo_array_create(TOGO_POOL * pool, size_t size, uint32_t n)
{
	TOGO_ARRAY * arr;
	void * elt;

	if (n < 1 || size == 0) {
		return NULL;
	}

	arr = (TOGO_ARRAY *) togo_pool_calloc(pool, sizeof(TOGO_ARRAY));
	if (arr == NULL) {
		togo_log(ERROR, "malloc a array fail");
		return NULL;
	}

	elt = togo_pool_calloc(pool, size * n);
	if (elt == NULL) {
		togo_log(ERROR, "malloc a array fail");
		togo_pool_free_data(pool, (void *) arr);
		return NULL;
	}

	arr->elt = elt;
	arr->size = size;
	arr->n = n;
	arr->used = 0;
	arr->pool = pool;
	pthread_mutex_init(&arr->lock, NULL);

	return arr;

}

void togo_array_destory(TOGO_ARRAY * arr)
{
	void * elt;
	TOGO_POOL * pool;

	if (arr == NULL) {
		return;
	}

	pthread_mutex_lock(&arr->lock);

	elt = arr->elt;
	pool = arr->pool;

	togo_pool_free_data(pool, (void *) arr);
	togo_pool_free_data(pool, (void *) elt);

	pthread_mutex_unlock(&arr->lock);

}

void * togo_array_push(TOGO_ARRAY * arr)
{
	void * elt;
	void * new_elt;
	size_t osize;
	size_t nsize;
	TOGO_POOL * pool;

	pthread_mutex_lock(&arr->lock);

	if (arr->used == arr->n) {

		osize = arr->n * arr->size;
		nsize = osize * 2;
		pool = arr->pool;
		elt = arr->elt;

		new_elt = togo_pool_realloc(pool, (void *) elt, osize, nsize);
		if (new_elt == NULL) {
			togo_log(ERROR, "realloc a array fail");
			pthread_mutex_unlock(&arr->lock);
			return NULL;
		}

		arr->elt = new_elt;
		arr->n = arr->n * 2;

	} else {

		new_elt = arr->elt;
	}

	new_elt = arr->elt + (arr->size * arr->used);
	arr->used++;

	pthread_mutex_unlock(&arr->lock);

	return new_elt;
}

void * togo_array_push_n(TOGO_ARRAY * arr, uint32_t n)
{
	void * elt;
	void * new_elt;
	size_t osize;
	size_t nsize;
	TOGO_POOL * pool;

	if (n < 1) {
		return NULL;
	}

	n = (n > arr->n) ? n : arr->n;

	pthread_mutex_lock(&arr->lock);

	if (arr->used == arr->n) {

		osize = arr->n * arr->size;
		nsize = osize + (n * arr->size);
		pool = arr->pool;
		elt = arr->elt;

		new_elt = togo_pool_realloc(pool, (void *) elt, osize, nsize);
		if (new_elt == NULL) {
			togo_log(ERROR, "realloc a array fail");
			pthread_mutex_unlock(&arr->lock);
			return NULL;
		}

		arr->elt = new_elt;
		arr->n = arr->n + n;

	} else {

		new_elt = arr->elt;
	}

	new_elt = arr->elt + (arr->size * arr->used);
	arr->used = arr->used + n;

	pthread_mutex_unlock(&arr->lock);

	return new_elt;
}

