#ifndef METACMD_H
#define METACMD_H

#include <stdbool.h>

#include "reader.h"


typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED,
} MetaCommandExecResult;


MetaCommandExecResult exec_meta_command(InputReader *r);
bool is_meta_command(const char *s);

#endif
