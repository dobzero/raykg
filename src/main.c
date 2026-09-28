
#include "core/types.h"
#include "cmdlist.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static program_arg_t pa_list_arg[100];
static program_arg_t *pa_list[100 + 1];

static char * pa_string(const char * name) {
    for (program_arg_t **p = pa_list; *p; p++) {
        if (*(*p)->arg!='\0')
            continue;
        (*p)->flag=nullptr;
        strcpy((*p)->arg, name);
        (*p)->type=PROGRAM_ARGTYPE_STR;
        return (*p)->value.str_;
    }
    return nullptr;
}

static const bool * pa_bool(const char * name) {
    for (program_arg_t **p = pa_list; *p; p++) {
        if (*(*p)->arg!='\0')
            continue;
        (*p)->flag=nullptr;
        strcpy((*p)->arg, name);
        (*p)->type=PROGRAM_ARGTYPE_BOOL;
        return &(*p)->value.boolean;
    }
    return nullptr;
}

static void pa_set_default(const void *value, const char * arg) {
    for (program_arg_t **p = pa_list; *p; p++) {
        if ((void*)(*p)->value.str_!=value)
            continue;
        switch ((*p)->type) {
            case PROGRAM_ARGTYPE_STR:
                strcpy((*p)->value.str_, arg);
                break;
            case PROGRAM_ARGTYPE_BOOL:
                (*p)->value.boolean=strcmp(arg,"true")==0;
                break;
        }
        break;
    }
}
static const void * pa_get(const char *name) {
    for (program_arg_t **p = pa_list; *p; p++) {
        if (strcmp((*p)->arg, name)!=0)
            continue;
        if ((*p)->type==PROGRAM_ARGTYPE_BOOL)
            return &(*p)->value.boolean;
        if ((*p)->type==PROGRAM_ARGTYPE_STR)
            return &(*p)->value.str_;
    }
    return nullptr;
}

static int ray_check_context(const ray_any_t * ra) {
    if (ra->type==RAY_BUILD_FOR_APK) {
        for (const list_t *l = ra->droid_bundles; l; l = l->next) {
            const droid_bundle_t * bundle = l->data;
            if (!bundle->context)
                return fprintf(stderr, "droid bundle without a context, apk %s exists?!\n", bundle->input_file);
        }
    }
    return 0;
}

void ray_file_save(const ray_any_t *ra) {
    char path[1000];
    if (ray_check_context(ra))
        return;
    if (ra->type==RAY_BUILD_FOR_APK) {

        sprintf(path, "%s.rayinfo", droid_get_package_name(ra->tier_bundle));
    }
    FILE * fp=fopen(path, "w");
    fwrite(&ra->ray_cnt, sizeof(ra->ray_cnt), 1, fp);
    fclose(fp);
}

void ray_file_load(ray_any_t *ra) {
    char path[1000];
    if (ray_check_context(ra))
        return;
    if (ra->type==RAY_BUILD_FOR_APK) {

        sprintf(path, "%s.rayinfo", droid_get_package_name(ra->tier_bundle));
    }
        FILE * fp=fopen(path, "r");
        fread(&ra->ray_cnt, sizeof(ra->ray_cnt), 1, fp);
        fclose(fp);
}

ray_any_t * ray_build_for(const ray_build_types_e type) {
    ray_any_t * ra = calloc(1, sizeof(ray_any_t));
    if (type==RAY_BUILD_FOR_APK) {
        char * apk_list=strdup(pa_get("apk_list"));
        char * bak=nullptr;
        for (const char * apk = strtok_r(apk_list,",;|", &bak);
            apk; apk = strtok_r(nullptr, ",;|", &bak)) {

            char output_path[100];
            sprintf(output_path, "%s-out%ld", apk, random()%100);

            list_emplace(&ra->droid_bundles, droid_create(ra, apk, output_path));
        }
        free(apk_list);
    }

    return ra;
}

void ray_any_done(ray_any_t * ra) {
    if (ra->type==RAY_BUILD_FOR_APK) {

        while (ra->droid_bundles) {
            droid_destroy(list_erase(&ra->droid_bundles, ra->droid_bundles));
        }
    }
    free(ra);
}


static void ray_run(const ray_any_t * ra) {
        if (ray_check_context(ra))
            return;
    if (ra->type==RAY_BUILD_FOR_APK) {
        if (*(const char*)pa_get("get_apk")) {
            droid_get_apk(ra->tier_bundle, pa_get("get_apk"));
            return;
        }
        for (const list_t * l=ra->droid_bundles; l; l=l->next) {
            droid_bundle_t * bundle = l->data;
            printf("pkg package name: %s\n", droid_get_package_name(bundle));
            if (*(const bool*)pa_get("useful_strings"))
                droid_display_useful_strings(bundle);
            if (*(const char**)pa_get("extract"))
                droid_extract(bundle);
            if (*(const char**)pa_get("list_intents"))
                droid_list_intents(bundle, stdout);
        }
        if (list_size(ra->droid_bundles)==2) {
            if (*(const bool*)pa_get("diff")) {
                const droid_bundle_t * first=list_front(ra->droid_bundles);
                const droid_bundle_t * last=list_back(ra->droid_bundles);

                droid_diff((const droid_bundle_t*[]){first, last},
                    *(const bool*)pa_get("exclude_equals"),
                    pa_get("only")
                );
            }
        }
    }
}

static void pa_add_droid() {
    pa_string("out_dir");

    pa_string("get_apk");
    pa_bool("list_intents");
    pa_bool("useful_strings");
    pa_bool("extract");
}

int main() {
    for (size_t i=0;i<100;i++)
        pa_list[i]=&pa_list_arg[i];
    pa_add_droid();

    pa_set_default(pa_string("apk_list"), "F-Droid.apk|org.fdroid.fdroid_2000010.apk");
    pa_set_default(pa_bool("diff"), "true");
    pa_set_default(pa_bool("exclude_equals"), "true");
    pa_set_default(pa_string("only"), "classes");



    ray_any_t * ra = ray_build_for(RAY_BUILD_FOR_APK);
    ray_run(ra);

    ray_any_done(ra);

    return 0;
}
