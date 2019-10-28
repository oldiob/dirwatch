#ifndef CORE_CONF_H
#define CORE_CONF_H


#include <sys/types.h>
#include <regex.h>

#include "util/list.h"
#include "util/ustr.h"

/**
 * struct conf - Configuration data.
 *
 * @node:  Node to add to a list of configurations.
 * @cmd:  Command to execute.
 * @regex:  Pattern matching files.
 */
struct conf {
	struct list_head node;
	ustr *cmd;
	regex_t *regex;
};

/**
 * conf_read() - Read a configuration file.
 *
 * @filepath:  File path to the configuration file.
 *
 * Return:  0 on success.
 */
extern int conf_read(const char *filepath);

#endif
