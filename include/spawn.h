
#include <stdbool.h>
#include <stdlib.h>

#ifndef SPAWN_H
#define SPAWN_H

struct cs_pipe {
    int fds[2];
};

struct cs_process {
    pid_t pid;
    int exit_status;
};

/**
 * Control structure that describes how to execute a process.
 *
 * @member args A NULL terminated list that gives the arguments for the process.
 *              The first argument is the process file.
 * @member in A pointer to a pipe for stdin, or NULL for no redirect.
 * @member out A pointer to a pipe for stdout, or NULL for no redirect.
 * @member err A pointer to a pipe for stderr, or NULL for no redirect.
 */
struct cs_control {
    char **args;
    struct cs_pipe *in;
    struct cs_pipe *out;
    struct cs_pipe *err;
};


/**
 * Initializes a pipe structure with a new pipe.
 *
 * @return 0 on success, -1 of failure.
 */
int cs_pipe_create(struct cs_pipe *p);

/**
 * Initializes a pipe structure with a new pipe whose input is connected to a
 * file.
 *
 * @return 0 on success, -1 of failure.
 */
int cs_pipe_create_read_file(struct cs_pipe *p, const char *file);

/**
 * Initializes a pipe structure with a new pipe whose output is connected to a
 * file.
 *
 * @return 0 on success, -1 of failure.
 */
int cs_pipe_create_write_file(struct cs_pipe *p, const char *file);

/**
 * Clean up a pipe's resources.
 */
void cs_pipe_destroy(struct cs_pipe *p);

/**
 * Read up to `len` bytes into `buf`.
 *
 * @return The number of bytes read or -1 on error.
 */
ssize_t cs_pipe_read(struct cs_pipe *p, char *buf, size_t len);

/**
 * Write up to `len` bytes from `buf`.
 *
 * @return The number of bytes written or -1 on error.
 */
ssize_t cs_pipe_write(struct cs_pipe *p, char *buf, size_t len);


/**
 * Spawn a new process as specified by `ctrl`. `proc` is initialized to track
 * the process.
 *
 * @return 0 on success, -1 of failure.
 */
int cs_spawn(struct cs_process *proc, struct cs_control *ctrl);

/**
 * Wait for a process to terminate.
 *
 * @return 0 on success, -1 of failure.
 */
int cs_wait(struct cs_process *proc);

/**
 * Stop a process from running (i.e. kill the process).
 *
 * @return 0 on success, -1 of failure.
 */
int cs_kill(struct cs_process *proc);

/**
 * Poll if the process is still running.
 */
bool cs_poll(struct cs_process *proc);

/**
 * Return the exit status of a process. Contains garbage is `cs_wait` or
 * `cs_kill` haven't been called.
 */
int cs_exit_status(struct cs_process *proc);

/**
 * See the SPAWN macro.
 */
int cs_spawn_helper(char ***args, struct cs_pipe *in, struct cs_pipe *out, bool wait);


#define __SPAWN_STATIC_LIST(type, ...) ((type[]) {__VA_ARGS__, NULL})
#define SPAWN_ARGS(...) __SPAWN_STATIC_LIST(char *, __VA_ARGS__)

/**
 * Used to run a sequence of piped processes. Each process' output is piped to
 * the next's input. `in` is used for the initial input, and `out` is used for
 * the final output. If and only if `wait` is true, then it waits until all
 * processes finish running to return.
 *
 * The varargs are in the form SPAWN_ARGS(command, arg1, arg2, ...).
 */
#define SPAWN(in, out, wait, ...) cs_spawn_helper(\
        (char***) __SPAWN_STATIC_LIST(char **, __VA_ARGS__), in, out, wait)

#define SPAWN_NIO(wait, ...) SPAWN(NULL, NULL, wait, __VA_ARGS__)

#endif /* end of include guard: SPAWN_H */
