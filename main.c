#include <stdio.h>

#include "reader.h"
#include "metacmd.h"
#include "statement.h"


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("%s: file", argv[0]);
        return 0;
    }
    InputReader *r = ir_new_reader();
    Table *t = open_db(argv[1]);

    while (1) {
        printf(">>> ");
        if (ir_read_line(r, stdin) < 0) {
            break;
        }
        if (is_meta_command(r->buf)) {
            switch (exec_meta_command(r, t)) {
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
        case STMT_PREPARE_STR_TOO_LONG:
            fprintf(stderr, "Input too long: %s\n", r->buf);
            continue;
        case STMT_PREPARE_ID_NEGATIVE:
            fprintf(stderr, "Id must be positive: %s\n", r->buf);
            continue;
        }
        switch (exec_statement(t, &stmt)) {
        case EXECUTE_SUCCESS:
            break;
        case EXECUTE_TABLE_FULL:
            fprintf(stderr, "Error: table full\n");
            break;
        }
    }
    ir_free(r);
    close_db(t);
}
