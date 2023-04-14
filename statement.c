#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "statement.h"
#include "leaf.h"

Cursor *table_start(Table *t) {
    Cursor *c = malloc(sizeof(Cursor));
    c->table = t;
    c->page = t->root_page;
    c->cell = 0;

    void *root = get_page(t->pager, t->root_page);
    uint32_t num_cells = *leaf_node_num_cells(root);
    c->end_of_table = num_cells == 0;

    return c;
}

Cursor *table_end(Table *t) {
    Cursor *c = malloc(sizeof(Cursor));
    c->table = t;
    c->page = t->root_page;

    void *root = get_page(t->pager, t->root_page);
    c->cell = *leaf_node_num_cells(root);
    c->end_of_table = true;

    return c;
}

void cursor_advance(Cursor *c) {
    void *node = get_page(c->table->pager, c->page);

    c->cell += 1;
    if (c->cell >= *leaf_node_num_cells(node)) {
        c->end_of_table = true;
    }
}

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
    p->num_pages = (file_len / PAGE_SIZE);

    if (file_len % PAGE_SIZE != 0) {
        fprintf(stderr, "Corrupted database file: %s\n", filename);
        exit(1);
    }

    for (int i = 0; i < TABLE_MAX_PAGES; i++) {
        p->pages[i] = NULL;
    }
    return p;
}

Table *open_db(const char *filename) {
    Pager *p = pager_open(filename);

    Table *t = malloc(sizeof(Table));
    t->pager = p;
    t->root_page = 0;

    if (p->num_pages == 0) {
        void *root = get_page(p, 0);
        init_leaf_node(root);
    }
    return t;
}

void pager_flush(Pager *p, uint32_t page_idx) {
    if (p->pages[page_idx] == NULL) {
        fprintf(stderr, "Can not flush to a null page: %u\n", page_idx);
        exit(1);
    }

    if (lseek(p->fd, page_idx * PAGE_SIZE, SEEK_SET) < 0) {
        perror("lseek");
        exit(1);
    }

    if (write(p->fd, p->pages[page_idx], PAGE_SIZE) < 0) {
        perror("pager.write");
        exit(1);
    }
}

void close_db(Table *t) {
    Pager *p = t->pager;

    for (uint32_t i = 0; i < p->num_pages; i++) {
        if (p->pages[i] == NULL) continue;

        pager_flush(p, i);
        free(p->pages[i]);
        p->pages[i] = NULL;
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
        exit(1);
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

        if (page_idx >= p->num_pages) {
            p->num_pages = page_idx + 1;
        }
    }
    return p->pages[page_idx];
}

void *cursor_value(Cursor *c) {
    uint32_t page_idx = c->page;
    void *page = get_page(c->table->pager, page_idx);
    return leaf_node_value(page, c->cell);
}

ExecuteResult exec_insert(Table *t, Statement *stmt) {
    void *node = get_page(t->pager, t->root_page);
    if ((*leaf_node_num_cells(node)) >= LEAF_NODE_MAX_CELLS) {
        return EXECUTE_TABLE_FULL;
    }

    Cursor *c = table_end(t);

    leaf_node_insert(c, stmt->insert_row.id, &stmt->insert_row);

    free(c);
    printf("INSERTED 1\n");
    return EXECUTE_SUCCESS;
}

ExecuteResult exec_select(Table *t, Statement *stmt) {
    Cursor *c = table_start(t);
    Row r;

    while (!c->end_of_table) {
        deserialize_row(cursor_value(c), &r);
        printf("(%d, %s, %s)\n", r.id, r.username, r.email);
        cursor_advance(c);
    }
    free(c);
    return EXECUTE_SUCCESS;
}
