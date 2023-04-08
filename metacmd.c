#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "metacmd.h"

MetaCommandExecResult exec_meta_command(InputReader *r) {
    if (strncmp(r->buf, ".exit", 5) == 0) {
        exit(0);
    }
    return META_COMMAND_UNRECOGNIZED;
}


bool is_meta_command(const char *s) { return s[0] == '.'; }
