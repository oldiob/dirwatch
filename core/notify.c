/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2019 Olivier Dion <olivier.dion@polymtl.ca>
 *
 * core/notify.c -
 */


#include <errno.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>

#include "core/io.h"
#include "core/notify.h"
#include "mem/alloc.h"


#define USR_MASK IN_ALL_EVENTS
#define SYS_MASK (IN_ONLYDIR | IN_MASK_CREATE)


struct event {
	struct list_head data;
	struct event *next;
	u32 mask;
};

struct dir {
	struct hlist_node entry;
	struct event *events;
	ustr *name;
	int wd;
};


static struct dir *dir_new(ustr *dirname);
static void dir_del(struct dir *dir);
static struct dir *dir_at(ustr *dirname);
static ustr *dir_name(int wd, ustr *new);

static struct event *event_new(struct dir *dir, u32 mask);
static void event_del(struct event *event);
static struct event *event_at(struct dir *dir, u32 mask);


#define NOTIFY_HSIZE 256
struct hlist_head directories[NOTIFY_HSIZE] = { HLIST_HEAD_INIT };
_Static_assert((NOTIFY_HSIZE) && !(NOTIFY_HSIZE & (NOTIFY_HSIZE - 1)), "NOTIFY_HSIZE is not a power of 2");

int eventQ = -1;

static inline void default_dtor(struct list_head *head) { (void)head; }
void (*data_dtor)(struct list_head *head) = default_dtor;

int count_directories_name = 128;
ustr **directories_name = NULL;


int notify_init(int flags)
{
	if (eventQ > -1)
		return -EBADFD;
try_again:
	eventQ = inotify_init1(flags);
	if (eventQ < 0) {
		switch (errno) {
		case EINVAL:
			flags = 0;
			goto try_again;
		default:
			panic("inotify_init1");
		}
	}
	directories_name = xalloc(count_directories_name, sizeof(ustr*));
	return eventQ;
}

int notify_fini(ustr *dirname)
{
	int ret = 0;
	if (dirname) {
		struct dir *dir = dir_at(dirname);
		if (!dir) {
			ret = -ENOENT;
			goto end;
		}
		dir_del(dir);
		ret = 1;
		goto end;
	}
	for (usize i=0; i<NOTIFY_HSIZE; ++i) {
		struct hlist_head *head = directories + i;
		struct dir *dir;
		struct hlist_node *tmp;
		hlist_for_each_entry_safe(dir, tmp, head, entry) {
			dir_del(dir);
			++ret;
		}
	}
	close(eventQ);
	eventQ = -1;
end:
	return ret;
}

__notnull
int notify_get(ustr *dirname, u32 mask, struct list_head **data)
{
	int ret = 0;
	struct dir *dir = dir_at(dirname);
	struct event *event;
	if (!dir)
		dir = dir_new(dirname);
	mask &= USR_MASK;
	event = event_at(dir, mask);
	if (!event)
		event = event_new(dir, mask);
	*data = &event->data;
	return ret;
}

__notnull
int notify_dtor(void (*dtor)(struct list_head *head))
{
	data_dtor = dtor;
	return 0;
}

ustr *notify_dirname(int wd)
{
	return dir_name(wd, NULL);
}

__notnull
static struct dir *dir_new(ustr *dirname)
{
	u64 index;
	struct dir *dir;
	int wd = inotify_add_watch(eventQ, dirname, USR_MASK | SYS_MASK);
	if (wd < 0)
		warning("inotify_add_watch - %s %s", strerror(errno), dirname);
	else
		info("Add watch on %s", dirname);
	dir_name(wd, dirname);
	index = ustr_h(dirname) & (NOTIFY_HSIZE - 1);
	dir = xmalloc(sizeof(struct dir));
	INIT_HLIST_NODE(&dir->entry);
	hlist_add_head(&dir->entry, directories + index);
	dir->events = NULL;
	dir->name = dirname;
	dir->wd = wd;
	return dir;
}

static void dir_del(struct dir *dir)
{
	struct event *cur = dir->events;
	struct event *next;
	while (cur) {
		next = cur->next;
		event_del(cur);
		cur = next;
	}
	hlist_del(&dir->entry);
	if (inotify_rm_watch(eventQ, dir->wd) < 0)
		warning("inotify_rm_watch");
	xfree(dir);
}

static struct dir *dir_at(ustr *dirname)
{
	u64 index = ustr_h(dirname) & (NOTIFY_HSIZE - 1);
	struct hlist_head *head = directories + index;
	struct dir *dir;
	hlist_for_each_entry(dir, head, entry)
		if (dir->name == dirname)
			return dir;
	return NULL;
}

static ustr *dir_name(int wd, ustr *new)
{
	ustr *old;
	while (wd >= count_directories_name) {
		usize old_count = count_directories_name;
		count_directories_name *= 2;
		directories_name = xrealloc(directories_name, sizeof(ustr*) * count_directories_name);
		for (int i=old_count; i<count_directories_name; ++i)
			directories_name[i] = NULL;
	}
	old = directories_name[wd];
	if (new) {
		if (!old)
			old = new;
		directories_name[wd] = new;
	}
	return old;
}

static struct event *event_new(struct dir *dir, u32 mask)
{
	struct_alloc(struct event, event);
	INIT_LIST_HEAD(&event->data);
	event->next = dir->events;
	dir->events = event;
	event->mask = mask;
	return event;
}

static void event_del(struct event *event)
{
	data_dtor(&event->data);
	xfree(event);
}

static struct event *event_at(struct dir *dir, u32 mask)
{
	struct event *event;
	for (event=dir->events; event; event=event->next) {
		if (event->mask & mask)
			return event;
	}
	return NULL;
}
