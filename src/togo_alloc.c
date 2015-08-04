/*
 * togo_alloc.c
 *
 *  Created on: 2015-7-1
 *      Author: zhuli
 */

#include "togo.h"
#include "togo_load.h"

void * togo_alloc(size_t size)
{
	void * p;

	p = malloc(size);
	if (p == NULL) {
		togo_log(ERROR, "malloc fail");
	}

	return p;
}

void * togo_calloc(size_t size)
{
	void * p;

	p = togo_alloc(size);
	if (p) {
		togo_memzero(p, size);
	}

	return p;
}

