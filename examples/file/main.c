
#include <stdio.h>

#include "spawn.h"


const char *in_file = "in.txt";
const char *out_file = "out.txt";

int main(int argc, char *argv[])
{
    struct cs_pipe inp;
    struct cs_pipe outp;

    if (cs_pipe_create_read_file(&inp, in_file) == -1) {
        printf("Could not read %s\n", in_file);

        return 1;
    }

    if (cs_pipe_create_write_file(&outp, out_file) == -1) {
        printf("Could not write %s\n", out_file);

        return 1;
    }

    SPAWN(&inp, &outp, true,
            SPAWN_ARGS("grep", "Lorem"),
            SPAWN_ARGS("tr", ",", ";"),
            SPAWN_ARGS("tr", ".", "!"));

    return 0;
}


