/*
 * togo_array.h
 *
 *  Created on: 2015-7-9
 *      Author: zhuli
 */

#ifndef TOGO_ARRAY_H_
#define TOGO_ARRAY_H_

#define TOGO_ARRAY_DEFAULT_NUM 8

typedef struct togo_array TOGO_ARRAY;

#define togo_array_space(arr) \
		(arr->n)

struct togo_array {
	void * elt;
	size_t size;
	uint32_t n;
	uint32_t used;
	TOGO_POOL * pool;
	pthread_mutex_t lock;
};

TOGO_ARRAY * togo_array_create(TOGO_POOL * pool, size_t size, uint32_t n);
void togo_array_destory(TOGO_ARRAY * arr);
void * togo_array_push(TOGO_ARRAY * arr);
void * togo_array_push_n(TOGO_ARRAY * arr, uint32_t n);

#endif /* TOGO_ARRAY_H_ */
