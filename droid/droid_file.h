#pragma once
#include "../core/types.h"

typedef struct droid_file {
    char *output_dir_files;
    char *output_dir;
    char *input_file;
    ray_any_t * context;

    bool proceed;
} droid_file_t;
droid_file_t * droid_create(ray_any_t * ray, const char * in_apk, const char *out_dir);
void droid_destroy(droid_file_t * drf);
char * droid_get_package_name(const droid_file_t * drf);

bool droid_fw_check_filename(const char *file_a);
void droid_sign_resign_all(const char *apks, const char * keystore, const char * alias, const char *pass_ks, const char * pass_key);

