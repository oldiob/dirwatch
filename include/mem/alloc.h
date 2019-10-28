#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H


#include <malloc.h>


#define array_alloc(type, name, n) type * name = xmalloc( n * sizeof( type ))
#define struct_alloc(__$type, __$name) array_alloc(__$type, __$name, 1)


/**
 * mem_register_proc() - Register a procedure to call when out of memory.
 * @proc:  The procedure to call.
 *
 * NOTE!  Procedure should be registered at the begining of the
 * execution to ensure enough memory is available.
 */
extern void mem_register_proc(void (*proc)(void));

extern void *xmalloc(usize size);

extern void *xrealloc(void *ptr, usize size);

static inline void *xalloc(usize count, usize size)
{
	return xmalloc(count * size);
}

static inline void xfree(void *ptr)
{
	free(ptr);
}


#endif
