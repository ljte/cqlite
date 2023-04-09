#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

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
    strncpy(buf + USERNAME_OFFSET, r->username, USERNAME_SIZE);
    strncpy(buf + EMAIL_OFFSET, r->email, EMAIL_SIZE);
}

void deserialize_row(void *buf, Row *r) {
    memcpy(&r->id, buf + ID_OFFSET, ID_SIZE);
    memcpy(&r->username, buf + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&r->email, buf + EMAIL_OFFSET, EMAIL_SIZE);
}

Pager *pager_open(const char *filename) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    
    int file_len = lseek(fd, 0, SEEK_END);

    Pager *p = (Pager *) malloc(sizeof(Pager));

    p->fd = fd;
    p->file_len = file_len;
    for (int i = 0; i < TABLE_MAX_PAGES; i++) {
        p->pages[i] = NULL;
    }
    return p;
}

Table *open_db(const char *filename) {
    Pager *p = pager_open(filename);

    Table *t = malloc(sizeof(Table));
    t->pager = p;
    t->num_rows = p->file_len / ROW_SIZE;
    return t;
}

void pager_flush(Pager *p, uint32_t page_idx, uint32_t size) {
    if (p->pages[page_idx] == NULL) {
        fprintf(stderr, "Can not flush to a null page: %u\n", page_idx);
        exit(1);
    }

    if (lseek(p->fd, page_idx * PAGE_SIZE, SEEK_SET) < 0) {
        perror("lseek");
        exit(1);
    }

    if (write(p->fd, p->pages[page_idx], size) < 0) {
        perror("pager.write");
        exit(1);
    }
}

void close_db(Table *t) {
    Pager *p = t->pager;
    uint32_t all_pages = t->num_rows / ROWS_PER_PAGE;

    for (uint32_t i = 0; i < all_pages; i++) {
        if (p->pages[i] == NULL) continue;
        printf("FLUSHING %u\n", i);
        pager_flush(p, i, PAGE_SIZE);
        free(p->pages[i]);
        p->pages[i] = NULL;
    }

    uint32_t partial_page_rows = t->num_rows % ROWS_PER_PAGE;

    if (partial_page_rows > 0) {
        uint32_t page_idx = all_pages;
        if (p->pages[page_idx] != NULL) {
            pager_flush(p, page_idx, partial_page_rows * ROW_SIZE);
            free(p->pages[page_idx]);
            p->pages[page_idx] = NULL;
        }
    }

    if (close(p->fd) < 0) {
        perror("close");
        exit(1);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
        if (p->pages[i] != NULL) {
            free(p->pages[i]);
            p->pages[i] = NULL;
        }
    }

    free(p);
    free(t);
}

void *get_page(Pager *p, uint32_t page_idx) {
    if (page_idx > TABLE_MAX_PAGES) {
        fprintf(stderr, "Page index out of bounds: %u\n", page_idx);
    }

    if (p->pages[page_idx] == NULL) {
        void *page = malloc(PAGE_SIZE);
        uint32_t n_pages = p->file_len / PAGE_SIZE;
        if (p->file_len % PAGE_SIZE) {
            n_pages++;
        }

        if (page_idx <= n_pages) {
            lseek(p->fd, page_idx * PAGE_SIZE, SEEK_SET);

            if (read(p->fd, page, PAGE_SIZE) < 0) {
                perror("read");
                exit(1);
            }
        }
        p->pages[page_idx] = page;
    }
    return p->pages[page_idx];
}

void *row_slot(Table *t, uint32_t row) {
    uint32_t page_idx = row / ROWS_PER_PAGE;
    void *page = get_page(t->pager, page_idx);
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
