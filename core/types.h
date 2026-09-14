#pragma once
#include <stdlib.h>
typedef enum {
    PROGRAM_ARGTYPE_STR,
    PROGRAM_ARGTYPE_BOOL
} program_arg_type_e;

typedef struct {
    char arg[100];
    int32_t *flag;
    program_arg_type_e type;
    union {
        char str_[100];
        bool boolean;
    } value;
} program_arg_t;


typedef struct droid_file droid_file_t;

typedef enum  {
    RAY_BUILD_FOR_APK
}ray_build_types_e;

typedef struct ray_any_ {
    ray_build_types_e type;

    struct {
        union {
            droid_file_t * droid_pkg_file;
        };

        union {
            struct {
                char package_name[1000];
            };
        } ray_data;
    };
} ray_any_t;


ray_any_t * ray_build_for(ray_build_types_e type);

void ray_any_done(ray_any_t * ray);

void ray_file_save(const ray_any_t *ray);
void ray_file_load(ray_any_t *ray);