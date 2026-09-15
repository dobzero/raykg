#include "script.h"

#include <stdio.h>
#include <stdlib.h>
script_ctx_t * script_open(const char *path) {
    script_ctx_t * sc = malloc(sizeof(script_ctx_t));
    sc->fp=fopen(path,"r");
    return sc;
}
void script_close(script_ctx_t * sc) {
    fclose(sc->fp);
    free(sc);
}

