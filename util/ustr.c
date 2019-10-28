/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2019 Olivier Dion <olivier.dion@polymtl.ca>
 */


#include <string.h>

#include "core/io.h"
#include "mem/alloc.h"
#include "util/ustr.h"


#ifndef CONFIG_USTR_ENSURE
#define CONFIG_USTR_ENSURE 1
#endif


/* TODO - Set SYS_PAGE_SIZE in build system */
#define SYS_PAGE_SIZE 4096
#define USTR_HSIZE (SYS_PAGE_SIZE / sizeof(struct ustr))
_Static_assert(USTR_HSIZE && !(USTR_HSIZE & (USTR_HSIZE - 1)), "HMAP must be a power of 2");


static struct ustr *table[USTR_HSIZE] = {NULL};


static usize djb2(const char *str, usize *len)
{
	int c;
	const char *diff = str;
	u64 hash = 5381;
	while ((c = (int)*str++))
		hash = ((hash << 5) + hash) ^ c;
	if (len)
		*len = str - diff;
	return hash;
}

static ustr *get(const char *str, usize len, u64 real_hash)
{
	ustr *ret;
	u64 hash = (real_hash & (USTR_HSIZE - 1));
	struct ustr *e;
	struct ustr *new_e;
	for (e=table[hash]; NULL != e; e=e->next) {
		if (e->hash != real_hash)
			continue;
		if (CONFIG_USTR_ENSURE && (0 != strcmp(str, e->str)))
			continue;
		ret = e->str;
		goto match;
	}
	new_e = xmalloc(sizeof(struct ustr) + len + 1);
	new_e->next = e;
	new_e->hash = real_hash;
	memcpy(new_e->str, str, len);
	new_e->str[len] = '\0';
	table[hash] = new_e;
	ret = new_e->str;
match:
	return ret;
}

__return_notnull
ustr *ustr_mk(const char *str)
{
	usize len;
	u64 real_hash = djb2(str, &len);
	return get(str, len, real_hash);
}


__return_notnull
ustr *ustr_mkn(const char *str, usize len)
{
	u64 real_hash = djb2(str, NULL);
	return get(str, len, real_hash);
}
