#ifndef READER_H
#define READER_H

#include <stdio.h>

typedef struct {
    char *buf;
    size_t len;
    ssize_t input_len;
} InputReader;

InputReader *ir_new_reader(void);
ssize_t ir_read_line(InputReader *r, FILE* f);
void ir_free_buf(InputReader *r);
void ir_free(InputReader *r);


#endif
