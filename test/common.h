#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "../core/io.c"
#define panic(...) panic(__func__, __VA_ARGS__)
#include "../mem/alloc.c"

#endif
