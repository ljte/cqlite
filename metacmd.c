#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "metacmd.h"
#include "statement.h"
#include "leaf.h"


void print_constants(void);
extern const uint32_t ROW_SIZE;
extern const uint32_t PAGE_SIZE;


MetaCommandExecResult exec_meta_command(InputReader *r, Table* t) {
    if (strncmp(r->buf, ".exit", 5) == 0) {
        ir_free(r);
        close_db(t);
        exit(0);
    }
    if (strncmp(r->buf, ".constants", 10)) {
        print_constants();
        return META_COMMAND_SUCCESS;
    }
    return META_COMMAND_UNRECOGNIZED;
}


bool is_meta_command(const char *s) { return s[0] == '.'; }


void print_constants(void) {
    printf("Row size: %d\n", ROW_SIZE);
    printf("Common node header size: %d\n", COMMON_NODE_HEADER_SIZE);
    printf("Leaf node header size: %d\n", LEAF_NODE_HEADER_SIZE);
    printf("Leaf node cell size: %d\n", LEAF_NODE_CELL_SIZE);
    printf("Leaf node space for cells: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
    printf("Leaf node max cells: %d\n", LEAF_NODE_MAX_CELLS);
}
