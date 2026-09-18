#pragma once

#include <zip.h>

void zip_get_pattern_from_file(zip_file_t * file, char output[1000], const char * regex);
void zip_get(zip_t *z, size_t i, zip_file_t **zf, const char **filename, size_t *sz);