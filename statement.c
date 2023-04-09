#include <string.h>
#include <stdlib.h>

#include "statement.h"

const uint32_t ID_SIZE = sizeof_attr(Row, id) * 8;
const uint32_t USERNAME_SIZE = sizeof_attr(Row, username);
const uint32_t EMAIL_SIZE = sizeof_attr(Row, email);
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;


StatementPrepareResult prepare_statement(InputReader *r,
                                         Statement *stmt) {

    if (strncmp(r->buf, "select", 6) == 0) {
        stmt->type = STATEMENT_SELECT;
        return STMT_PREPARE_SUCCESS;
    }

    if (strncmp(r->buf, "insert", 6) == 0) {
        stmt->type = STATEMENT_INSERT;

        if (strtok(r->buf, " ") == NULL) {
            return STMT_PREPARE_SYNTAX_ERROR;
        }

        char *id_str = strtok(NULL, " ");
        char *username = strtok(NULL, " ");
        char *email = strtok(NULL, " ");

        if (id_str == NULL || username == NULL || email == NULL) {
            return STMT_PREPARE_SYNTAX_ERROR;
        }

        int64_t id = atoi(id_str);

        if (id < 0) {
            return STMT_PREPARE_ID_NEGATIVE;
        }

        if (strlen(username) > COLUMN_USERNAME_SIZE) {
            return STMT_PREPARE_STR_TOO_LONG;
        }
        if (strlen(email) > COLUMN_EMAIL_SIZE) {
            return STMT_PREPARE_STR_TOO_LONG;
        }

        stmt->insert_row.id = (uint32_t) id;
        strcpy(stmt->insert_row.username, username);
        strcpy(stmt->insert_row.email, email);

        return STMT_PREPARE_SUCCESS;
    }
    return STMT_PREPARE_UNRECOGNIZED;
}

ExecuteResult exec_statement(Table *t, Statement *stmt) {
    switch (stmt->type) {
    case (STATEMENT_SELECT):
        return exec_select(t, stmt);
    case (STATEMENT_INSERT):
        return exec_insert(t, stmt);
    }
    return EXECUTE_SUCCESS;
}

void serialize_row(void *buf, Row *r) {
    memcpy(buf + ID_OFFSET, &r->id, ID_SIZE);
    memcpy(buf + USERNAME_OFFSET, &r->username, USERNAME_SIZE);
    memcpy(buf + EMAIL_OFFSET, &r->email, EMAIL_SIZE);
}

void deserialize_row(void *buf, Row *r) {
    memcpy(&r->id, buf + ID_OFFSET, ID_SIZE);
    memcpy(&r->username, buf + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&r->email, buf + EMAIL_OFFSET, EMAIL_SIZE);
}

Table *new_table(void) {
    Table *t = malloc(sizeof(Table));
    t->num_rows = 0;
    for (int i = 0; i < TABLE_MAX_PAGES; i++) {
        t->pages[i] = NULL;
    }
    return t;
}

void free_table(Table *t) {
    for (int i = 0; t->pages[i]; i++) {
        free(t->pages[i]);
    }
    free(t);
}

void *row_slot(Table *t, uint32_t row) {
    uint32_t page_idx = row / ROWS_PER_PAGE;
    void *page = t->pages[page_idx];
    if (page == NULL) {
        page = malloc(PAGE_SIZE);
        t->pages[page_idx] = page;
    }
    uint32_t row_offset = row % ROWS_PER_PAGE;
    return page + row_offset * ROW_SIZE;
}

ExecuteResult exec_insert(Table *t, Statement *stmt) {
    if (t->num_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    void *slot = row_slot(t, t->num_rows);

    serialize_row(slot, &stmt->insert_row);
    t->num_rows++;
    printf("INSERTED 1\n");
    return EXECUTE_SUCCESS;
}

ExecuteResult exec_select(Table *t, Statement *stmt) {
    Row r;

    for (uint32_t i = 0; i < t->num_rows; i++) {
        deserialize_row(row_slot(t, i), &r);
        printf("(%d, %s, %s)\n", r.id, r.username, r.email);
    }
    return EXECUTE_SUCCESS;
}
