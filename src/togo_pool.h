/*
 * togo_pool.h
 *
 *  Created on: 2015-7-1
 *      Author: zhuli
 */

#ifndef TOGO_POOL_H_
#define TOGO_POOL_H_

#define TOGO_POOL_ALIGNMENT       		16
#define TOGO_DEFAULT_POOL_SIZE    		(16 * 1024)
#define TOGO_DEFAULT_POOL_FREE_SIZE    	1024
#define TOGO_DEFAULT_POOL_MIN_SIZE    	(2 * 1024)

#define TOGO_POOL_WASTE_SIZE 128

typedef struct togo_pool_data TOGO_POOL_DATA;
typedef struct togo_pool_block TOGO_POOL_BLOCK;
typedef struct togo_pool_large TOGO_POOL_LARGE;
typedef struct togo_pool TOGO_POOL;

#define togo_pool_strlen(a) \
	(strlen(a) + 1)
#define togo_pool_size(a) \
	(a + sizeof(TOGO_POOL_BLOCK))
#define togo_pool_data_size(a) \
	(a + sizeof(TOGO_POOL_DATA))

/**
 * The data header.
 * Every data have a head to help us to record the address.
 * The size of data equal (sizeof(TOGO_POOL_DATA) + size)
 */
struct togo_pool_data {
	size_t size;
	TOGO_POOL * pool;
	TOGO_POOL_BLOCK * block;
	TOGO_POOL_DATA * next;
	TOGO_POOL_DATA * prev;
};

/**
 * This struct Used to store small data.
 * It will store in the Memory pool.
 */
struct togo_pool_block {
	size_t size;
	u_char * start;
	u_char * end;
	u_char * used;
	TOGO_POOL_BLOCK * next;
	TOGO_POOL_BLOCK * prev;
};

/**
 * This struct Used to store big data.
 * It will malloc a new memory page.
 */
struct togo_pool_large {
	size_t size;
	void *p;
	TOGO_POOL_LARGE *next;
	TOGO_POOL_LARGE *prev;
};

/**
 * Memory pool
 */
struct togo_pool {
	size_t size;
	size_t max;
	size_t total_size;

	uint32_t total_block;
	TOGO_POOL_BLOCK * block;
	TOGO_POOL_BLOCK * block_current;

	TOGO_POOL_DATA * free;
	TOGO_POOL_DATA * free_current;

	TOGO_POOL_LARGE * large;

	pthread_mutex_t mlock;
};

TOGO_POOL * togo_pool_create(size_t size);
void togo_pool_destroy(TOGO_POOL * pool);
void * togo_pool_calloc(TOGO_POOL * pool, size_t size);
void * togo_pool_alloc(TOGO_POOL * pool, size_t size);
void * togo_pool_realloc(TOGO_POOL * pool, void * p, size_t size,
		size_t new_size);
void togo_pool_free_data(TOGO_POOL * pool, void * p);
void togo_pool_free_large(TOGO_POOL * pool, void * large);
BOOL togo_pool_islarge(TOGO_POOL * pool, size_t size);

#endif /* TOGO_POOL_H_ */
