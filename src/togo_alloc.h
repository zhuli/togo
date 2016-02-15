/*
 * togo_alloc.h
 *
 *  Created on: 2015-7-1
 *      Author: zhuli
 */

#ifndef TOGO_ALLOC_H
#define TOGO_ALLOC_H

#define TOGO_ALIGNMENT   sizeof(unsigned long)    /* platform word */
#define togo_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define togo_align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))

#define togo_memzero(buf, n)       (void) memset(buf, 0, n)
#define togo_memset(buf, c, n)     (void) memset(buf, c, n)
#define togo_free free
#define togo_memcpy memcpy
#define togo_realloc realloc
#define togo_memchr memchr
#define togo_memmove memmove

#define togo_align(d, a)     (((d) + (a - 1)) & ~(a - 1))

#define togo_offsetof(q, type, link)         \
    (type *) ((u_char *) q - offsetof(type, link))

void * togo_alloc(size_t size);
void * togo_calloc(size_t size);

#endif /* TOGO_ALLOC_H_ */
