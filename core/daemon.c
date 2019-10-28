/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2019 Olivier Dion <olivier.dion@polymtl.ca>
 *
 * core/daemon.c - See daemon(7)
 */


#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include "core/daemon.h"
#include "core/io.h"


static int close_all_files(void);
static int reset_sighandlers(void);
static int reset_sigmask(void);
static void purge_env(const char *env[]);
static int double_fork(void);
static int redirect_to_devnull(void);
static int reset_umask(void);
static int change_working_dir(const char *wd);
static int lock_file(const char *lock, int *fd_lock);


static inline void ensure_close(int fd)
{
	int err;
again:
	err = close(fd);
	if (!err)
		return;
	if (errno == EINTR)
		goto again;
	panic("close");
}

static inline void ensure_dup2(int fd1, int fd2)
{
	int err;
again:
	err = dup2(fd1, fd2);
	if (-1 < err)
		return;
	if (errno == EINTR)
		goto again;
	panic("dup2");
}


int daemonize(int flags, const char *purge[], const char *wd, const char *lock, int *fd_lock)
{
#define IF_OPT(opt, call) if (flags & DAEMON_##opt) if (call()) return DAEMON_##opt
	if (double_fork())
		return DAEMON_FORK;
	IF_OPT(FD, close_all_files);
	IF_OPT(SIGHAND, reset_sighandlers);
	IF_OPT(SIGMASK, reset_sigmask);
	if (purge)
		purge_env(purge);
	if (lock && lock_file(lock, fd_lock))
		return DAEMON_LOCK;
	IF_OPT(UMASK, reset_umask);
	if (change_working_dir(wd))
		return DAEMON_CHDIR;
	IF_OPT(DEVNULL, redirect_to_devnull);
	return 0;
#undef IF_OPT
}


static int close_all_files(void)
{
	char buf[PATH_MAX];
	DIR *self;
	int fd;
	int err = 0;
	struct dirent *dirent;

	snprintf(buf, sizeof(buf), "/proc/%d/fd", getpid());
	self = opendir(buf);

	if (unlikely(!self)) {
		err = DAEMON_FD;
		goto bad_proc;
	}

	fd = dirfd(self);
	while ((dirent = readdir(self))) {
		int tmp;
		tmp = atoi(dirent->d_name);
		if (tmp != fd && tmp > STDERR_FILENO)
			ensure_close(tmp);
	}
	ensure_close(fd);
bad_proc:
	return err;
}

static int reset_sighandlers(void)
{
	struct sigaction action;
	action.sa_handler = SIG_DFL;
	for (int i=1; i<NSIG; ++i)
		sigaction(i, &action, NULL);
	return 0;
}

static int reset_sigmask(void)
{
	sigset_t set;
	sigemptyset(&set);
	if (unlikely(sigprocmask(SIG_SETMASK, &set, NULL) < 0))
		return 1;
	return 0;
}

static void purge_env(const char *env[])
{
	while (*env)
		unsetenv(*(env++));
}

static int double_fork(void)
{
	int err = 0;
	switch (fork()) {
	case -1:
		err = -1;
		break;
	case 0:
		setsid();
		switch (fork()) {
		default: exit(0); break;
		case -1: err = -1; break;
		case 0: break;
		}
		break;
	default:
		exit(0);
	}
	return err;
}

static int redirect_to_devnull(void)
{
	static const char DEVNULL[] = "/dev/null";
	int rnull;
	int wnull;
	int err = -1;
	rnull = open(DEVNULL, O_RDONLY);
	if (unlikely(rnull < 0))
		goto no_read_access;
	wnull = open(DEVNULL, O_WRONLY);
	if (unlikely(wnull < 0))
		goto no_write_access;
	ensure_dup2(rnull, STDIN_FILENO);
	ensure_dup2(wnull, STDOUT_FILENO);
	ensure_dup2(wnull, STDERR_FILENO);
	err = 0;
	ensure_close(wnull);
no_write_access:
	ensure_close(rnull);
no_read_access:
	return err;
}

static int reset_umask(void)
{
	umask(0);
	return 0;
}

static int change_working_dir(const char *wd)
{
	static const char root[] = "/";
	if (!wd)
		wd = root;
	return chdir(wd);
}

static int lock_file(const char *lock, int *fd_lock)
{
	int fd = open(lock, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	int err;
again:
	err = flock(fd, LOCK_EX | LOCK_NB);
	if (err && (errno == EINTR))
		goto again;
	if (fd_lock)
		*fd_lock = fd;
	return err;
}
