
#include <stdio.h>

#include "spawn.h"


int main(int argc, char *argv[])
{
    SPAWN_NIO(true, SPAWN_ARGS("ls"), SPAWN_ARGS("cat"));

    return 0;
}
