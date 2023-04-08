#include <string.h>

#include "statement.h"
#include "reader.h"


StatementPrepareResult prepare_statement(InputReader *r,
                                         Statement *stmt) {

    if (strncmp(r->buf, "select", 6) == 0) {
        stmt->type = STATEMENT_SELECT;
        return STMT_PREPARE_SUCCESS;
    }

    if (strncmp(r->buf, "insert", 6) == 0) {
        stmt->type = STATEMENT_INSERT;
        int args_assigned = sscanf(r->buf,
            "insert %u %s %s",
             &stmt->insert_row.id,
             stmt->insert_row.username,
             stmt->insert_row.email);
        if (args_assigned < 3) {
            return STMT_PREPARE_SYNTAX_ERROR;
        }
        return STMT_PREPARE_SUCCESS;
    }
    return STMT_PREPARE_UNRECOGNIZED;
}

void exec_statement(Statement *stmt) {
    switch (stmt->type) {
    case (STATEMENT_SELECT):
        printf("SELECTING DATA\n");
        break;
    case (STATEMENT_INSERT):
        printf("INSERTING USER DATA %d %s %s\n",
               stmt->insert_row.id,
               stmt->insert_row.username,
               stmt->insert_row.email);
        break;
    }
}
