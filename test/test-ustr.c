/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2019 Olivier Dion <olivier.dion@polymtl.ca>
 */


#include "./common.h"
#include "../util/ustr.c"

static const char *strings[] =
{
	"Hello World!",
	"No éàîö ASCII",
	"Foo Bar",
	"The quick brown fox jumps over the lazy dog"
};


int main()
{
	ustr *prev = NULL;
	for (usize i=0; i<array_size(strings); ++i) {
		const char *str = strings[i];
		ustr *next = ustr_mk(str);
		assert(strlen(next) == strlen(str));
		assert(!strcmp(next, str));
		assert(next != prev);
		assert(ustr_mk(strings[i]) == next);
		prev = next;
	}
	return 0;
}
