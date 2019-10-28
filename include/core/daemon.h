#ifndef CORE_DAEMON_H
#define CORE_DAEMON_H


/**
 * enum daemon_policy
 *
 * @DAEMON_FORK: Double fork (return value only).
 * @DAEMON_FD:  Close all file descriptor except stdin, stdout, stderr.
 * @DAEMON_SIGHAND:  Reset all signal handlers.
 * @DAEMON_ENV:  Purge environment variables (return value only).
 * @DAEMON_LOCK:  Lock file (return value only).
 * @DAEMON_UMASK:  Set umask to 0.
 * @DAEMON_CHDIR:  Change working directory (return value only).
 * @DAEMON_DEVNULL:  Redirect stdin, stdout, stderr to /dev/null.
 * @DAEMON_SYSV:  Be SysV compliant.
 */
enum daemon_policy {
	DAEMON_FORK = 0x1,
	DAEMON_FD = 0x2,
	DAEMON_SIGHAND = 0x4,
	DAEMON_SIGMASK = 0x8,
	DAEMON_ENV = 0x10,
	DAEMON_LOCK = 0x20,
	DAEMON_UMASK = 0x40,
	DAEMON_CHDIR = 0x80,
	DAEMON_DEVNULL = 0x100,
	DAEMON_SYSV = (DAEMON_FD |
		       DAEMON_SIGHAND |
		       DAEMON_SIGMASK |
		       DAEMON_DEVNULL |
		       DAEMON_UMASK)
};


/**
 * daemonize() - Create a daemon process.
 *
 * @flags: Policy for the daemon.  See enum daemon_policy.  Flags that
 * are label (return value only) are ignored in @flags.
 *
 * @purge: Array terminated by %NULL of environment variable to purge,
 *         or %NULL.
 *
 * @wd: The working directory of the daemon, or %NULL for the root
 *      directory.
 *
 * @lock: File to lock for the daemon.
 *
 * @fd_lock: Pointer filled with the file descriptor to the locked
 *           file @lock or %NULL if you don't need it.
 *
 * Return: On success, 0 is returned.  Otherwise, the flag of the step
 * that has failed is set to 1.
 *
 * For example, if the locking step has failed, the %DAEMON_LOCK will
 * be set. And further step are not done.
 */
extern int daemonize(int flags, const char *purge[], const char *wd, const char *lock, int *fd_lock);


#endif
