/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2019 Olivier Dion <olivier.dion@polymtl.ca>
 *
 * mem/alloc.c - Malloc family wrapper.
 *
 * Work like malloc, but if failed will call a list of procedure to
 * free some memory and try again.  If it fail again, the system is in
 * panic.
 *
 * All allocations, excepted xrealloc, are zeroed throught calloc.
 *
 * xmalloc  -> malloc
 * xalloc   -> calloc
 * xrealloc -> realloc
 * xfree    -> free
 *
 * One shall register a procedure to call to try freeing some memory
 * by using the mem_register_procedure() procedure.  Once a procedure
 * is registered, it can not be unregistered.  Thus, you should
 * register your procedures at the very begining of your program.
 */


#include "core/io.h"
#include "mem/alloc.h"

struct free_procedure {
	void *next;
	void (*procedure)(void);
};

static void *procedures_list = NULL;
static void do_free_mem()
{
	for (struct free_procedure *proc=procedures_list;
	     proc != NULL; proc=proc->next)
		(*(proc->procedure))();
}

void mem_register_proc(void (*proc)(void))
{
	struct free_procedure *fp = malloc(sizeof(struct free_procedure));
	if (unlikely(!fp))
		panic("Failed to register procedure");
	fp->next = procedures_list;
	fp->procedure = proc;
	procedures_list = fp;
}

__return_notnull __malloc
void *xmalloc(size_t size)
{
	void *ptr;
	ptr = calloc(1, size);
	if (likely(ptr != NULL))
		goto success;

	do_free_mem();

	ptr = calloc(1, size);
	if (likely(ptr != NULL))
		goto success;

	panic("Can't allocated memory of size %ld", size);

success:
	return ptr;
}

__return_notnull
void *xrealloc(void *ptr, size_t size)
{
	void *saved_ptr = ptr;
	ptr = realloc(saved_ptr, size);
	if (likely(ptr != NULL))
		goto success;

	do_free_mem();

	ptr = realloc(saved_ptr, size);
	if (likely(ptr != NULL))
		goto success;

	panic("Can't reallocated pointer %p of size %ld", saved_ptr, size);

success:
	return ptr;
}


#ifdef MALLOC_INFO
#include <stdio.h>
#include <malloc.h>
__on_init __on_fini
static void do_malloc_stats()
{
	#ifndef ASAN
	#pragma GCC diagnostic ignored "-Waggregate-return"
	struct mallinfo info = mallinfo();
	fprintf(stderr,
		"Malloc Infos:\n"
		"Arena:         %d bytes\n"
		"Mapped:        %d regions\n"
		"Mapped Used:   %d bytes\n"
		"Max Used:      %d bytes\n"
		"In Used:       %d bytes\n"
		"Freed:         %d bytes\n"
		"Keep Cost:     %d bytes\n",

		info.arena,
		info.hblks,
		info.hblkhd,
		info.usmblks,
		info.uordblks,
		info.fordblks,
		info.keepcost
		);
	#else
	fputs("Binary linked against ASAN.  Malloc info not available.\n", stderr);
	#endif
}
#endif
