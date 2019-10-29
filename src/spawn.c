
#include "spawn.h"


#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>


void close_read(struct cs_pipe *p) {
    if (p->fds[0] != -1) {
        close(p->fds[0]);
        p->fds[0] = -1;
    }
}

void close_write(struct cs_pipe *p) {
    if (p->fds[1] != -1) {
        close(p->fds[1]);
        p->fds[1] = -1;
    }
}


int cs_pipe_create(struct cs_pipe *p) {
    if (pipe(p->fds) == -1)
        return -1;

    return 0;
}

int cs_pipe_create_read_file(struct cs_pipe *p, const char *file) {
    p->fds[0] = open(file, O_RDONLY);
    p->fds[1] = -1;

    if (p->fds[0] == -1)
        return -1;

    return 0;
}

int cs_pipe_create_write_file(struct cs_pipe *p, const char *file) {
    p->fds[0] = -1;
    p->fds[1] = open(file, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

    if (p->fds[1] == -1)
        return -1;

    return 0;
}


void cs_pipe_destroy(struct cs_pipe *p) {
    close_read(p);
    close_write(p);
}


ssize_t cs_pipe_read(struct cs_pipe *p, char *buf, size_t len) {
    return read(p->fds[0], buf, len);
}

ssize_t cs_pipe_write(struct cs_pipe *p, char *buf, size_t len) {
    return write(p->fds[0], buf, len);
}


int cs_spawn(struct cs_process *proc, struct cs_control *ctrl) {
    pid_t pid = fork();

    if (pid == 0) {
        if (ctrl->in) {
            dup2(ctrl->in->fds[0], STDIN_FILENO);
            cs_pipe_destroy(ctrl->in);
        }

        if (ctrl->out) {
            dup2(ctrl->out->fds[1], STDOUT_FILENO);
            cs_pipe_destroy(ctrl->out);
        }

        if (ctrl->err) {
            dup2(ctrl->err->fds[1], STDERR_FILENO);
            cs_pipe_destroy(ctrl->err);
        }

        execvp(ctrl->args[0], ctrl->args);
    } else {
        proc->pid = pid;

        if (ctrl->in)
            close_read(ctrl->in);
        if (ctrl->out)
            close_write(ctrl->out);
        if (ctrl->err)
            close_write(ctrl->err);
    }

    return 0;
}


int cs_wait(struct cs_process *proc) {
    int stat;

    if (waitpid(proc->pid, &stat, 0) == -1)
        return -1;

    proc->exit_status = WEXITSTATUS(stat);

    return 0;
}


int cs_kill(struct cs_process *proc) {
    proc->exit_status = 143;
    return kill(proc->pid, SIGKILL);
}


bool cs_poll(struct cs_process *proc) {
    if (kill(proc->pid, 0) == 0)
        return true;
    else
        return false;
}


int cs_exit_status(struct cs_process *proc) {
    return proc->exit_status;
}


int cs_spawn_helper(char ***args, struct cs_pipe *in, struct cs_pipe *out, bool wait) {
    struct cs_pipe last_pipe;
    struct cs_pipe next_pipe;
    size_t len;

    while (args[len] != NULL)
        len++;

    struct cs_process *procs = malloc(len*sizeof(struct cs_process));

    if (procs == NULL)
        return -1;

    struct cs_control *ctrls = malloc(len*sizeof(struct cs_control));

    if (ctrls == NULL) {
        free(procs);
        return -1;
    }

    struct cs_pipe *pipes = malloc((len - 1)*sizeof(struct cs_pipe));

    if (pipes == NULL) {
        free(procs);
        free(ctrls);
        return -1;
    }


    for (size_t i = 0; i < len - 1; i++) {
        cs_pipe_create(&pipes[i]);
    }

    for (size_t i = 0; i < len; i++) {
        struct cs_pipe *last_pipe;
        struct cs_pipe *next_pipe;

        if (i == 0) {
            last_pipe = in;
            next_pipe = &pipes[i];
        } else if (i < len - 1) {
            last_pipe = &pipes[i - 1];
            next_pipe = &pipes[i];
        } else {
            last_pipe = &pipes[i - 1];
            next_pipe = out;
        }

        ctrls[i] = (struct cs_control) {args[i], last_pipe, next_pipe, NULL};
    }

    for (size_t i = 0; i < len; i++) {
        if (cs_spawn(&procs[i], &ctrls[i]) == -1) {
            cs_pipe_destroy(&last_pipe);
            return -1;
        }
    }


    if (wait) {
        for (size_t i = 0; i < len; i++)
            cs_wait(&procs[i]);
    }


    free(procs);
    free(ctrls);
    free(pipes);

    return 0;
}
