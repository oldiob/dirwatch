/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2019 Olivier Dion <olivier.dion@polymtl.ca>
 *
 * core/conf.c - Configuration file parser wrapper.
 */


#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "core/io.h"
#include "core/conf.h"


extern int conf_parse(FILE *fp);


int conf_read(const char *filepath)
{
	FILE *fp = fopen(filepath, "r");
	int err;
	if (!fp)
		return errno;
	err =conf_parse(fp);
	fclose(fp);
	return err;
}
