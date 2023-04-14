#include <stdlib.h>
#include <string.h>

#include "leaf.h"
#include "statement.h"

uint32_t *leaf_node_num_cells(void *node) {
    return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void *leaf_node_cell(void *node, uint32_t cell_num) {
    return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t *leaf_node_key(void *node, uint32_t cell_num) {
    return leaf_node_cell(node, cell_num);
}

void *leaf_node_value(void *node, uint32_t cell_num) {
    return leaf_node_cell(node, cell_num) + LEAF_NODE_KEY_SIZE;
}

void init_leaf_node(void *node) {
    uint32_t *ncells = leaf_node_num_cells(node);
    *ncells = 0;
}

void leaf_node_insert(Cursor *c, uint32_t key, Row *value) {
    void *node = get_page(c->table->pager, c->page);

    uint32_t ncells = *leaf_node_num_cells(node);

    if (ncells > LEAF_NODE_MAX_CELLS) {
        fprintf(stderr, "TODO: split nodes\n");
        exit(1);
    }

    if (c->cell < ncells) {
        for (uint32_t i = ncells; i > c->cell; i--) {
            memcpy(leaf_node_cell(node, i),
                   leaf_node_cell(node, i - 1),
                   LEAF_NODE_CELL_SIZE);
        }
    }

    (*leaf_node_num_cells(node))++;
    uint32_t *cur_key = leaf_node_key(node, c->cell);
    *cur_key = key;

    serialize_row(value, leaf_node_value(node, c->cell));
}
