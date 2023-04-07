#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

#define BUFFER_SIZE 200

typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} StatementType;

typedef enum {
    STMT_PREPARE_SUCCESS,
    STMT_PREPARE_UNRECOGNIZED,
    STMT_PREPARE_SYNTAX_ERROR
} StatementPrepareResult;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {    
    StatementType type;
    Row insert_row;
} Statement;

typedef struct {
    char *buf;
    size_t len;
    ssize_t input_len;
} InputReader;

InputReader *ir_new_reader(void) {
    InputReader *r = (InputReader *) malloc( sizeof(InputReader) );
    r->buf = NULL;
    r->len = 0;
    r->input_len = 0;
    return r;
}

ssize_t ir_read_line(InputReader *r, FILE* f) {
    int read = getline(&r->buf, &r->len, f);

    if (read < 0) {
        return -1;
    }

    r->input_len = read - 1;
    r->buf[read - 1] = 0;
    return read;
}

void ir_free_buf(InputReader *r) {
    if (r->buf != NULL) {
        free(r->buf);
        r->buf = NULL;
    }
    r->len = 0;
}

void ir_free(InputReader *r) {
    ir_free_buf(r);
    free(r);
}

bool is_meta_command(const char *s) {
    return s[0] == '.';
}

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED,
} MetaCommandExecResult;

MetaCommandExecResult exec_meta_command(InputReader *r) {
    if (strncmp(r->buf, ".exit", 5) == 0) {
        exit(0);
    }
    return META_COMMAND_UNRECOGNIZED;
}

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
