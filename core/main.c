/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2019 Olivier Dion <olivier.dion@polymtl.ca>
 */


#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "core/conf.h"
#include "core/daemon.h"
#include "core/io.h"
#include "core/notify.h"
#include "mem/alloc.h"
#include "util/list.h"
#include "util/ustr.h"


static const char tmp_dir[] = "/tmp/dirwatch/";
static const char var_dir[] = ".local/share/dirwatch/";
static int eventQ;


static void show_usage(void);
static void install_sighandler(void);
static void poll(void);
static void do_cmd(struct inotify_event *event, ustr *dirname, struct conf *conf);
static void handle_notify_event(struct inotify_event *event, struct list_head *head, ustr *dirname);
static void do_notify(void);


int main(int argc, char *argv[])
{
	int err;
	char buf[PATH_MAX];
	int log;
	if (argc < 3)
		show_usage();
	/* Make sure that /tmp/dirwatch exists */
	err = mkdir(tmp_dir, S_IRUSR | S_IWUSR | S_IXUSR);
	if (err && errno != EEXIST)
		panic("mkdir %s", tmp_dir);
	/* Create daemon */
	snprintf(buf, sizeof(buf), "%s%s", tmp_dir, argv[1]);
	err = daemonize(DAEMON_SYSV, NULL, NULL, buf, NULL);
	if (err & DAEMON_LOCK)
		panic("Failed to lock '%s'.  Daemon probably already exits", buf);

	/* Initialize global variables */
	info("Initializing inotify cached");
	eventQ = notify_init(IN_NONBLOCK);

	snprintf(buf, sizeof(buf), "%s/%s", getenv("HOME"), var_dir);
	mkdir(buf, S_IRUSR | S_IWUSR | S_IXUSR);
	snprintf(buf, sizeof(buf), "%s/%s%s", getenv("HOME"), var_dir, argv[1]);
	log = open(buf, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
	if (log < -1)
		goto no_logging;
	if (dup2(log, STDOUT_FILENO) < 0)
		warning("dup2 STDOUT_FILENO");
	if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0)
		warning("dup2 STDERR_FILENO");
	close(log);
	info("New dirwatch daemon '%s' %d", argv[1], getpid());
no_logging:
	/* Read configs */
	for (int i=2; i<argc; ++i) {
		if (conf_read(argv[i]))
			error("Failed to read config file %s", argv[i]);
	}
	info("Installing signal handler");
	install_sighandler();

	/* Heavy work */
	poll();
	return 0;
}


__noreturn
static void show_usage(void)
{
	printf("Usage: dirwatch NAME [FILE]...\n");
	exit(1);
}

static void terminate(int sig)
{
	(void)sig;
}

static void install_sighandler(void)
{
	struct sigaction act;
	act.sa_handler = terminate;
	sigaction(SIGTERM, &act, NULL);
}

static void poll(void)
{
	struct epoll_event events[1];
	int epoll = epoll_create1(0);
	sigset_t mask;
	if (epoll < 0)
		panic("Failed to create epoll");
	sigfillset(&mask);
	sigdelset(&mask, SIGTERM);
	events[0].data.fd = eventQ;
	events[0].events = EPOLLIN;
	if (epoll_ctl(epoll, EPOLL_CTL_ADD, eventQ, events))
		panic("epoll_ctl on eventQ %d", eventQ);
	info("Polling ...");
	while (1) {
		int len = epoll_pwait(epoll, events,
				      array_size(events), -1, &mask);
		if (len < 0) {
			if (errno == EINTR)
				break;
			panic("epoll_wait");
		}
		for (int i=0; i<len; ++i) {
			if (events[i].data.fd == eventQ)
				do_notify();
		}
	}
	info("Received SIGTERM ...");
}

static void do_cmd(struct inotify_event *event, ustr *dirname, struct conf *conf)
{
	char match[PATH_MAX];
	snprintf(match, sizeof(match), "%s", event->name);
	chdir(dirname);
	setenv("DW_MATCH", match, 1);
	execl("/bin/sh", "sh", "-c", conf->cmd, (char*)NULL);
	panic("execl on command %s", conf->cmd);
}

static void handle_notify_event(struct inotify_event *event, struct list_head *head, ustr *dirname)
{
	struct conf *conf;
	int status;
	pid_t cpid;
	list_for_each_entry(conf, head, node) {
		if (regexec(conf->regex, event->name, 0, NULL, 0))
			continue;
		cpid = fork();
		switch (cpid) {
		case 0:
			do_cmd(event, dirname, conf);
			break;
		case -1:
			error("Failed to fork for command %s", conf->cmd);
			break;
		}
		waitpid(cpid, &status, 0);
		if (!status)
			info("At '%s' DO '%s' on '%s'", dirname, conf->cmd, event->name);
		else
			error("At %s' '%s' ON '%s'", dirname, conf->cmd, event->name);
	}
}

static void do_notify()
{
	static char buf[4096] __align(alignof(struct inotify_event));
	char *ptr;
	struct inotify_event *event;
	isize len;
read_again:
	len = read(eventQ, buf, sizeof(buf));
	if (len <= 0)
		goto end;
	for (ptr=buf; ptr<buf+len; ptr+=sizeof(struct inotify_event) + event->len) {
		ustr *dirname;
		struct list_head *head;
		event = (struct inotify_event*)ptr;
		if (event->len == 0)
			continue;
		dirname = notify_dirname(event->wd);
		if (unlikely(!dirname)) {
			critical("Inconstancy in cached dirname for watch descriptor %d", event->wd);
			continue;
		}
		notify_get(dirname, event->mask, &head);
		handle_notify_event(event, head, dirname);
	}
	goto read_again;
end:
	{};
}
