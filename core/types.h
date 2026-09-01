#pragma once

typedef struct ray_bundle_content {
    union {
        char package_name[1000];
    };
} ray_file_t;

void ray_file_save(ray_file_t *apkinfo);
void ray_file_load(ray_file_t *apkinfo);