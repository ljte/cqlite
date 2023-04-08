#include <inttypes.h>
#include <stdint.h>

#include "reader.h"

#pragma once
#ifndef STATEMENT_H
#define STATEMENT_H


typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
} StatementType;

typedef enum {
    STMT_PREPARE_SUCCESS,
    STMT_PREPARE_UNRECOGNIZED,
    STMT_PREPARE_SYNTAX_ERROR
} StatementPrepareResult;

typedef enum {
    EXECUTE_TABLE_FULL,
    EXECUTE_SUCCESS,
} ExecuteResult;

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

#define sizeof_attr(Struct, Attr) (sizeof(((Struct *)0)->Attr))

#define TABLE_MAX_PAGES 100

typedef struct {
    uint32_t num_rows;
    void *pages[TABLE_MAX_PAGES];
} Table;


Table *new_table(void);
void free_table(Table *t);

void *row_slot(Table *t, uint32_t row);

StatementPrepareResult prepare_statement(InputReader *r, Statement *stmt);
ExecuteResult exec_statement(Table *t, Statement *stmt);
ExecuteResult exec_insert(Table *t, Statement *stmt);
ExecuteResult exec_select(Table *t, Statement *stmt);

void serialize_row(void *buf, Row *r);
void deserialize_row(void *buf, Row *r);

#endif
