#ifndef CORE_NOTIFY_H
#define CORE_NOTIFY_H


#include <sys/inotify.h>

#include "util/ustr.h"
#include "util/list.h"


struct notify_event {
	struct list_head data;
	struct notify_event *next;
	u32 mask;
};

struct notify_it {
	struct notify_event *event;
	u32 mask;
};


/**
 * notify_init() - Create a new eventQ.
 *
 * @flags:  Same flags passed to inotify_init1.
 *
 * Return: The file descriptor of the new eventQ on success or -EBADFD
 * if an eventQ is already created.
 *
 * NOTE!  This function might call panic() on failure.
 */
extern int notify_init(int flags);

/**
 * notify_fini() - Remove a directory entry or close an eventQ.
 *
 * @dirname: The directory entry name to remove from watching or %NULL
 * to close the eventQ.
 *
 * Return: The number of directory remove from watching.
 */
extern int notify_fini(ustr *dirname);

/**
 *
 * notify_get() - Get the data associated with a directory that's been watched.
 * @dirname:  The directory name.
 *
 * @mask:  The event mask matching the desired events.
 *
 * @data: A pointer filled with a pointer to a struct list_head to add
 * private data.
 *
 * Return: Always %0.
 *
 * NOTE!
 *
 * If no directory named @dirname is currently been watched, a new
 * directory entry is add to the eventQ with inotify_add_watch().
 * Thus, it might call panic() on failure.
 *
 * If no event mask match @mask, a new event entry is created for the
 * watched directory named @dirname.
 */
extern int notify_get(ustr *dirname, u32 mask, struct list_head **data);

/**
 * notify_dtor() - Install a destructor for cleaning data.
 *
 * @dtor:  The destructor to install.
 *
 * Return: Always %0.
 *
 * @dtor will be call with the data head of every event of a directory
 * that is been removed from the eventQ.
 */
extern int notify_dtor(void (*dtor)(struct list_head *head));

/**
 * notify_dirname() - Get cached directory name of inotify.
 *
 * @wd:  The watch descriptor.
 *
 * Return an unique string of the name on success, %NULL if the watch
 * descriptor provided doesn't have a directory named associated with
 * it.
 */
extern ustr* notify_dirname(int wd);


extern struct list_head *notify_it_start(struct notify_it *it, ustr *dirname, u32 mask);
extern struct list_head *notify_it_next(struct notify_it *it);

#define notify_for_each_event(pos, it, dirname, mask) for (pos=notify_it_start(it, dirname, mask); pos; pos=notify_it_next(it))

#endif
