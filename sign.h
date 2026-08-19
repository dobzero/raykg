#pragma once
typedef struct ray_bundle_content {
    char packagename[1000];
}ray_bc_t;

void sign_resign_all(const char *apks, const char * keystore, const char * alias, const char *pass_ks, const char * pass_key);
