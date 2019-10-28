/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2019 Olivier Dion <olivier.dion@polymtl.ca>
 *
 * core/io.c - Simple routines for error reporting.
 *
 */


#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "core/io.h"
#undef panic

static pthread_mutex_t io_lock = PTHREAD_MUTEX_INITIALIZER;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=format"
__notnull
static void do_logging(const char *header, const char *fmt, va_list ap)
{
	static char buff[4096];
	pthread_mutex_lock(&io_lock);
	vsnprintf(buff, sizeof(buff), fmt, ap);
	printf("[%s %d] %s\n", header, getpid(), buff);
	fflush(NULL);
	pthread_mutex_unlock(&io_lock);
}
#pragma GCC diagnostic pop


__format(printf, 2) __notnull __noreturn
void panic(const char *func, const char *fmt, ...)
{
	static const char header[] = "PANIC";

	char buf[4096];
	va_list ap;
	int saved_errno = errno;
	va_start(ap, fmt);

	if (saved_errno)
		snprintf(buf, sizeof(buf), "%s : %s - %s", func, strerror(saved_errno), fmt);
	else
		snprintf(buf, sizeof(buf), "%s - %s", func, fmt);
	do_logging(header, buf, ap);
	exit(1);
}

__format(printf, 2) __notnull
void logging(int lvl, const char *fmt, ...)
{
	static const char *headers[] = {
		"CRITICAL",
		"ERROR",
		"WARNING",
		"INFO",
	};
	va_list ap;
	va_start(ap, fmt);
	do_logging(headers[lvl], fmt, ap);
}
