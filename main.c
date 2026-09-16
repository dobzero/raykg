
#include "core/types.h"
#include "cmdlist.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static program_arg_t PA_List[100];

static char * pa_string(const char * name) {
    for (program_arg_t *p = PA_List; p; p++) {
        if (*p->arg!='\0')
            continue;
        p->flag=nullptr;
        strcpy(p->arg, name);
        p->type=PROGRAM_ARGTYPE_STR;
        return p->value.str_;
    }
    return nullptr;
}

static const bool * pa_bool(const char * name) {
    for (program_arg_t *p = PA_List; p; p++) {
        if (*p->arg!='\0')
            continue;
        p->flag=nullptr;
        strcpy(p->arg, name);
        p->type=PROGRAM_ARGTYPE_BOOL;
        return &p->value.boolean;
    }
    return nullptr;
}

static void pa_set_default(const void *value, const char * arg) {
    for (program_arg_t *p = PA_List; p; p++) {
        if ((void*)p->value.str_!=value)
            continue;
        switch (p->type) {
            case PROGRAM_ARGTYPE_STR:
                strcpy(p->value.str_, arg);
                break;
            case PROGRAM_ARGTYPE_BOOL:
                p->value.boolean=strcmp(arg,"true")==0;
                break;
        }
        break;
    }
}
static const void * pa_get(const char *name) {
    for (const program_arg_t *p = PA_List; p; p++) {
        if (strcmp(p->arg, name)!=0)
            continue;
        if (p->type==PROGRAM_ARGTYPE_BOOL)
            return &p->value.boolean;
        if (p->type==PROGRAM_ARGTYPE_STR)
            return &p->value.str_;
    }
    return nullptr;
}

static int ray_check_context(const ray_any_t * ray) {
    if (ray->type==RAY_BUILD_FOR_APK) {
        if (!ray->droid_pkg_file->context) {
            return fprintf(stderr, "no droid pkg context found, apk exists?!\n");
        }
    }
    return 0;
}

void ray_file_save(const ray_any_t *ray) {
    char path[1000];
    if (ray_check_context(ray))
        return;
    if (ray->type==RAY_BUILD_FOR_APK) {

        sprintf(path, "%s.rayinfo", droid_get_package_name(ray->droid_pkg_file));
    }
    FILE * fp=fopen(path, "w");
    fwrite(&ray->ray_data, sizeof(ray->ray_data), 1, fp);
    fclose(fp);
}

void ray_file_load(ray_any_t *ray) {
    char path[1000];
    if (ray_check_context(ray))
        return;
    if (ray->type==RAY_BUILD_FOR_APK) {

        sprintf(path, "%s.rayinfo", droid_get_package_name(ray->droid_pkg_file));
    }
        FILE * fp=fopen(path, "r");
        fread(&ray->ray_data, sizeof(ray->ray_data), 1, fp);
        fclose(fp);
}

ray_any_t * ray_build_for(const ray_build_types_e type) {
    ray_any_t * ray = calloc(1, sizeof(ray_any_t));
    if (type==RAY_BUILD_FOR_APK) {
        ray->droid_pkg_file = droid_create(ray, pa_get("apk_file"), pa_get("out_dir"));
    }

    return ray;
}

void ray_any_done(ray_any_t * ray) {
    if (ray->type==RAY_BUILD_FOR_APK) {

        droid_destroy(ray->droid_pkg_file);
    }
    free(ray);
}


static void ray_run(const ray_any_t * ray) {
        if (ray_check_context(ray))
            return;
    if (ray->type==RAY_BUILD_FOR_APK) {
        printf("pkg package name: %s\n", droid_get_package_name(ray->droid_pkg_file));

        if (*(const char*)pa_get("get_apk")) {
            droid_get_apk(ray->droid_pkg_file);
            return;
        }
        if (*(const bool*)pa_get("useful_strings"))
            droid_display_useful_strings(ray->droid_pkg_file);
        if (*(const char**)pa_get("extract"))
            droid_extract(ray->droid_pkg_file);
    }
}

int main() {
    const char * get_apk=pa_string("get_apk");
    const char * output = pa_string("out_dir");
    const char * apk_file = pa_string("apk_file");

    pa_bool("useful_strings");
    const bool * extract = pa_bool("extract");

    pa_set_default(apk_file, "F-Droid.apk");
    pa_set_default(output, "F-Droid");
    pa_set_default(extract, "true");

    ray_any_t * ray = ray_build_for(RAY_BUILD_FOR_APK);
    ray_run(ray);

    ray_any_done(ray);

    return *get_apk!='\0';
}
