#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

#include "metacmd.h"
#include "reader.h"
#include "statement.h"


int main() {
    InputReader *r = ir_new_reader();
    while (true) {
        printf(">>> ");
        if ( ir_read_line(r, stdin) < 0 ) {
            break;
        }
        if (is_meta_command(r->buf)) {
            switch (exec_meta_command(r)) {
            case (META_COMMAND_SUCCESS):
                continue;
            case (META_COMMAND_UNRECOGNIZED):
                fprintf(stderr, "Unrecognized command: %s\n", r->buf);
                continue;
            }
        }
        Statement stmt;
        switch (prepare_statement(r, &stmt)) {
        case STMT_PREPARE_SUCCESS:
            break;
        case STMT_PREPARE_UNRECOGNIZED:
            fprintf(stderr, "Unrecognized keyword: %s\n", r->buf);
            continue;
        case STMT_PREPARE_SYNTAX_ERROR:
            fprintf(stderr, "Syntax error: %s\n", r->buf);
            continue;
        }
        exec_statement(&stmt);
    }
    ir_free(r);
}
