#include "zip_io.h"

#include <regex.h>
#include <string.h>

void zip_get_pattern_from_file(zip_file_t * file, char output[1000], const char * regex) {
    regex_t pattern;
    regcomp(&pattern, regex, 0);
    size_t zr=0;
    char buffer[1000];
    do {
        zr=zip_fread(file, buffer, 1000);

        regmatch_t list[100]={};
        const bool regex_ok=regexec(&pattern, buffer, 100, list, 0)==0;
        for (size_t i=0;regex_ok&&i<100;i++) {
            if (list[i].rm_so==list[i].rm_eo)
                break;

            strncpy(output, buffer+ list[i].rm_so, list[i].rm_eo - list[i].rm_so);
        }

    } while (zr>0);
    regfree(&pattern);
}

void zip_get(zip_t *z, const size_t i, zip_file_t **zf, const char **filename, size_t *sz) {
    zip_stat_t index;
    zip_stat_index(z, i, 0, &index);
    *zf=zip_fopen_index(z, i, 0);
    *filename=index.name;
    *sz=index.size;
}

