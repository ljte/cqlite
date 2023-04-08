#include <stdlib.h>

#include "reader.h"


InputReader *ir_new_reader(void) {
    InputReader *r = (InputReader *) malloc(sizeof(InputReader));
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

