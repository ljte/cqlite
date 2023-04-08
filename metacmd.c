#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "metacmd.h"
#include "statement.h"

MetaCommandExecResult exec_meta_command(InputReader *r, Table* t) {
    if (strncmp(r->buf, ".exit", 5) == 0) {
        ir_free(r);
        free_table(t);
        exit(0);
    }
    return META_COMMAND_UNRECOGNIZED;
}


bool is_meta_command(const char *s) { return s[0] == '.'; }
