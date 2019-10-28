/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2019 Olivier Dion <olivier.dion@polymtl.ca>
 */


#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "./common.h"
#include "../util/ustr.c"
#include "../core/notify.c"

int dir_fd;
int fd;
char template[] = "/tmp/XXXXXX";
char to_create[] = "I-was-created";
int eventQ;
ustr *dirname;
struct list_head *create_addr;
struct list_head *delete_addr;

void rmtmp(void)
{
	unlink(template);
}


void install_dir()
{
	assert(mkdtemp(template));
	dir_fd = open(template, O_DIRECTORY | O_RDONLY , S_IRUSR | S_IXUSR);
	assert(0 <= dir_fd);
	atexit(rmtmp);
	dirname = ustr_mk(template);
	eventQ = notify_init(IN_NONBLOCK);
	assert(0 <= eventQ);
	assert(0 == notify_get(dirname, IN_CREATE, &create_addr));
	assert(0 == notify_get(dirname, IN_DELETE, &delete_addr));
}

void do_notify()
{
	char buf[4096] __align(alignof(struct inotify_event));
	char *ptr;
	struct inotify_event *event;
	struct list_head *head;
	isize len;
	for (;;) {
		len = read(eventQ, buf, sizeof(buf));
		if (len == -1 && errno != EAGAIN)
			panic("read");

		if (len <= 0)
			break;

		for (ptr=buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
			event = (struct inotify_event*)ptr;
			notify_get(notify_dirname(event->wd), event->mask, &head);
			if (event->mask & IN_CREATE)
				assert(head == create_addr);
			else if (event->mask & IN_DELETE)
				assert(head == delete_addr);
		}
	}
	assert(notify_fini(NULL) > 0);
}

/*
 * Let's create	a temporary folder TMP_DIR.
 */
int main()
{
	install_dir();
	switch (fork()) {
	case -1:
		exit(EXIT_FAILURE);
	case 0:
		fd = openat(dir_fd, to_create, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
		unlinkat(dir_fd, to_create, 0);
		exit(EXIT_SUCCESS);
	}
	do_notify(eventQ);
	return EXIT_SUCCESS;
}
