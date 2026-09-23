#pragma once
#include "core/types.h"
#include "meta_inf/manifest.h"
#include <zip.h>


typedef struct droid_bundle {
    char *output_dir_files;
    char *output_dir;
    char *input_file;
    zip_t * pkg_file;

    ray_any_t * context;

    manifest_t * manifest;

    bool proceed;
} droid_bundle_t;
droid_bundle_t * droid_create(ray_any_t * ray, const char * in_apk, const char *out_dir);
void droid_destroy(droid_bundle_t * bundle);
char * droid_get_package_name(const droid_bundle_t * bundle);

bool droid_fw_check_filename(const char *file_a);
void droid_sign_resign_all(const char *apks, const char * keystore, const char * alias, const char *pass_ks, const char * pass_key);

bool droid_test_apk_is_apk(const droid_bundle_t * bundle);
void droid_extract(const droid_bundle_t * bundle);
void droid_list_intents(const droid_bundle_t *bundle, FILE *fp);