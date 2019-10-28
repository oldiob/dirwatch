#ifndef CORE_IO_H
#define CORE_IO_H

/**
 * panic() - Log a message before killing the process.
 * @func:  The function that called the panic.
 * @fmt:  Argument format
 *
 * Will log a last message describing why the system panicked.  If
 * errno was set in the calling thread, it will also log it.
 *
 * panic() is brutal and kill all other threads.  This might change.
 */
extern void panic(const char *func, const char *fmt, ...);
#define panic(...) panic(__func__, __VA_ARGS__)

extern void logging(int lvl, const char *fmt, ...);

#define critical(fmt, ...) logging(0, fmt __VA_OPT__(,) __VA_ARGS__)
#define error(fmt, ...) logging(1, fmt __VA_OPT__(,) __VA_ARGS__)
#define warning(fmt, ...) logging(2, fmt __VA_OPT__(,) __VA_ARGS__)
#define info(fmt, ...) logging(3, fmt __VA_OPT__(,) __VA_ARGS__)
#define debug(fmt, ...) logging(4, fmt __VA_OPT__(,) __VA_ARGS__)


#endif
