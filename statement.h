#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>

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
    STMT_PREPARE_SYNTAX_ERROR,
    STMT_PREPARE_STR_TOO_LONG,
    STMT_PREPARE_ID_NEGATIVE,
} StatementPrepareResult;

typedef enum {
    EXECUTE_TABLE_FULL,
    EXECUTE_SUCCESS,
} ExecuteResult;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {    
    StatementType type;
    Row insert_row;
} Statement;

#define sizeof_attr(Struct, Attr) (sizeof(((Struct *)0)->Attr))

#define TABLE_MAX_PAGES 100

typedef struct {
    int fd;
    uint32_t file_len;
    uint32_t num_pages;
    void *pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
    uint32_t num_rows;
    Pager *pager;
    uint32_t root_page;
} Table;

typedef struct {
    Table *table;
    uint32_t page;
    uint32_t cell;
    bool end_of_table;
} Cursor;


Table *open_db(const char *filename);
void close_db(Table *t);
void free_table(Table *t);

void *row_slot(Table *t, uint32_t row);

StatementPrepareResult prepare_statement(InputReader *r, Statement *stmt);
ExecuteResult exec_statement(Table *t, Statement *stmt);
ExecuteResult exec_insert(Table *t, Statement *stmt);
ExecuteResult exec_select(Table *t, Statement *stmt);

void serialize_row(void *buf, Row *r);
void deserialize_row(void *buf, Row *r);

Cursor *table_start(Table *t);
Cursor *table_end(Table *t);

void *get_page(Pager *p, uint32_t page_idx);

#endif
