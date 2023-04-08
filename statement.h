#ifndef STATEMENT_H
#define STATEMENT_H

#include <inttypes.h>

#include "reader.h"


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


StatementPrepareResult prepare_statement(InputReader *r, Statement *stmt);
void exec_statement(Statement *stmt);


#endif
