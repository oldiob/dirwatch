#ifndef INCLUDE_DEF_H
#define INCLUDE_DEF_H

#define asm __asm__
#define typeof __typeof__
#define inline __inline__
#define restrict __restrict__

/*
 * Taken from: Linux System Programming - Robert Love.  With some
 * little extra.
 */
#if __GNUC__ >= 3
# define __align_cache __attribute__((__aligned__(64)))
# define __always_inline inline __attribute__((__always_inline__))
# define __force
# define __fallthrough __attribute__((__fallthrought__))
# define __on_init        __attribute__((__constructor__))
# define __on_fini        __attribute__((__destructor__))
# define __return_notnull __attribute__((__returns_nonnull__))
# define __notnull        __attribute__ ((__nonnull__))
# define __noinline       __attribute__ ((__noinline__))
# define __pure           __attribute__ ((__pure__))
# define __const        __attribute__ ((__const__))
# define __format(f, n) __attribute__ ((__format__(f, n, n+1)))
# define __noreturn     __attribute__ ((__noreturn__))
# define __malloc       __attribute__ ((__malloc__))
# define __must_check   __attribute__ ((__warn_unused_result__))
# define __deprecated   __attribute__ ((__deprecated__))
# define __used         __attribute__ ((used))
# define __unused       __attribute__ ((__unused__))
# define __packed       __attribute__ ((__packed__))
# define __align(x)     __attribute__ ((__aligned__(x)))
# define __align_max    __attribute__ ((__aligned__))
# define likely(x)      __builtin_expect (!!(x), 1)
# define unlikely(x)    __builtin_expect (!!(x), 0)
#else
# define __align_cache
# define __always_inline
# define __fallthrough
# define __on_init
# define __on_fini
# define __return_notnull
# define __notnull
# define __noinline
# define __pure
# define __const
# define __format(f, n, m)
# define __noreturn
# define __malloc
# define __must_check
# define __deprecated
# define __used
# define __unused
# define __packed
# define __align(x)
# define __align_max
# define likely(x) (x)
# define unlikely(x) (x)
#endif	/* __GNUC__ */

#include <sys/types.h>

typedef size_t usize;
typedef ssize_t isize;

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef unsigned __int128 u128;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef __int128 s128;

#include <stddef.h>

#ifndef NULL
#  define NULL ((void*)0)
#endif

#ifndef offsetof
#  define offsetof(type, member) __builtin_offsetof (type, member)
#endif

typedef ptrdiff_t pdiff;
typedef wchar_t rune;


typedef _Bool bool;
#define true 1
#define false 0

typedef unsigned char uchar;
typedef unsigned int uint;


#define _Atomic atomic


/* From the Kernel */
#define container_of(ptr, type, member)					\
	({								\
		void *__$ptr = (void *)(ptr);				\
		((type *)(__$ptr - offsetof(type, member)));		\
	})
#define typecheck(x, y) (!!(sizeof((typeof(x)*)1 == (typeof(y)*)1)))
#define field_sizeof(type, field) (sizeof((type*)0)->field)
#define field_size(x, field) (sizeof((typeof(x)*)0)->field)
#define array_size(arr) (sizeof(arr) / sizeof((arr)[0]))

#define paddingof(A, B) ((alignof(A) - alignof(B)) % alignof(A));
#define alignof(x) _Alignof(x)
#define range(a, b) (a) ... (b)


#ifdef WITH_ASSERT
#  include <assert.h>
#else
#  define assert(...) do{}while(0)
#endif


#define assert_align(__$ptr) assert_align_void(__$ptr, _Alignof(typeof(*__$ptr)))
static __always_inline bool assert_align_void(void *ptr, unsigned long align)
{
	assert(((unsigned long)ptr % align) == 0);
	return true;
}

static __always_inline bool assert_pow2(unsigned long N)
{
	assert(N && !(N & (N-1)));
	return true;
}

#ifndef barrier
# define barrier() asm volatile ("":::"memory")
#endif

#define smp_read_barrier_depends() do {} while(0)

#define __READ_ONCE_SIZE						\
({									\
	switch (size) {							\
	case 1: *(u8 *)res = *(volatile u8 *)p; break;		\
	case 2: *(u16 *)res = *(volatile u16 *)p; break;		\
	case 4: *(u32 *)res = *(volatile u32 *)p; break;		\
	case 8: *(u64 *)res = *(volatile u64 *)p; break;		\
	default:							\
		barrier();						\
		__builtin_memcpy((void *)res, (const void *)p, size);	\
		barrier();						\
	}								\
})

static __always_inline
void __read_once_size(const volatile void *p, void *res, int size)
{
	__READ_ONCE_SIZE;
}


#define __READ_ONCE(x, check)						\
({									\
	union { typeof(x) __val; char __c[1]; } __u;			\
									\
	__read_once_size(&(x), __u.__c, sizeof(x));			\
	smp_read_barrier_depends(); /* Enforce dependency ordering from x */ \
	__u.__val;							\
})
#define READ_ONCE(x) __READ_ONCE(x, 1)

static __always_inline void __write_once_size(volatile void *p, void *res, int size)
{
	switch (size) {
	case 1: *(volatile u8 *)p = *(u8 *)res; break;
	case 2: *(volatile u16 *)p = *(u16 *)res; break;
	case 4: *(volatile u32 *)p = *(u32 *)res; break;
	case 8: *(volatile u64 *)p = *(u64 *)res; break;
	default:
		barrier();
		__builtin_memcpy((void *)p, (const void *)res, size);
		barrier();
	}
}

#define WRITE_ONCE(x, val) \
({							\
	union { typeof(x) __val; char __c[1]; } __u =	\
		{ .__val = (__force typeof(x)) (val) }; \
	__write_once_size(&(x), __u.__c, sizeof(x));	\
	__u.__val;					\
})

#endif
