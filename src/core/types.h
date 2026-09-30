#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct list list_t;
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

typedef enum  {
    RAY_BUILD_FOR_APK
}ray_build_types_e;

typedef struct ray_state {
    ray_build_types_e type;
    uint64_t r_seed;
    size_t bundle_count;
    list_t * droid_bundles;

    struct {

        union {
        } ray_cnt;
    };
} ray_state_t;


ray_state_t * ray_build_for(ray_build_types_e type);

void ray_any_done(ray_state_t * rs);

void ray_file_save(const ray_state_t *rs);
void ray_file_load(ray_state_t *rs);

typedef struct list {
    void * data;
    struct list *next;
} list_t;

list_t * list_create(void *data);
list_t * list_dup(const list_t *list);

void * list_erase(list_t **list, list_t *item);
void list_emplace(list_t **list, void *data);

void list_insert(list_t *list, void *data);
list_t * list_find(list_t *list, const void *data);
void list_destroy(list_t *list);

uint64_t list_size(const list_t * list);
void *list_front(const list_t *list);
void *list_back(const list_t *list);

