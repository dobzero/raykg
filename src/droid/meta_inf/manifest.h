#pragma once

#include <zip.h>
typedef struct manifest {
    char * origin_file;
    char * plain_file;
    uint64_t origin_size;

    bool is_xml_binary;
} manifest_t;

manifest_t * manifest_from_archive(zip_t *z);
char * manifest_get(const manifest_t *man, const char *ns_list);
char ** manifest_get_all_intents(const manifest_t *man);
const char * manifest_get_value(const manifest_t *man, const char * attr);

void manifest_prepare(manifest_t *man);
void manifest_write(const manifest_t *man, FILE *fp);

void manifest_encode();
void manifest_destroy(manifest_t *man);