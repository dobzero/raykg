#pragma once

#include <stdio.h>

typedef struct script_context {
    FILE * fp;
} script_ctx_t;

script_ctx_t * script_open(const char *path);

void script_close(script_ctx_t * sc);
