#include "core/types.h"
#include "cmdlist.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glob.h>

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

static int ray_check_context(const ray_state_t * rs) {
    if (rs->type==RAY_BUILD_FOR_APK) {
        for (const list_t *l = rs->droid_bundles; l; l = l->next) {
            const droid_bundle_t * bundle = l->data;
            if (!bundle->pkg_file)
                return printf("droid bundle without a context, apk %s exists?!\n", bundle->input_file);
        }
    }
    return 0;
}

void ray_file_save(const ray_state_t *rs) {
    char path[1000];
    if (ray_check_context(rs))
        return;

    sprintf(path, "%lu.state.dat", rs->r_seed);
    FILE * fp=fopen(path, "w");
    fwrite(&rs->ray_cnt, sizeof(rs->ray_cnt), 1, fp);
    fclose(fp);
}

void ray_file_load(ray_state_t *rs) {
    if (ray_check_context(rs))
        return;
    glob_t g_result;
    glob("*.state.dat", 0, nullptr, &g_result);
    globfree(&g_result);
    for (size_t i=0;i<g_result.gl_pathc; i++) {
        FILE * fp=fopen(g_result.gl_pathv[i], "r");
        fread(&rs->ray_cnt, sizeof(rs->ray_cnt), 1, fp);
        fclose(fp);
    }
}

ray_state_t * ray_build_for(const ray_build_types_e type) {
    ray_state_t * ra = calloc(1, sizeof(ray_state_t));
    srandom(time(nullptr));
    ra->r_seed=random();
    if (type==RAY_BUILD_FOR_APK) {
        char * apk_list=strdup(pa_get("apk_list"));
        char * bak=nullptr;
        for (const char * apk = strtok_r(apk_list,",;|", &bak);
            apk; apk = strtok_r(nullptr, ",;|", &bak)) {

            char output_path[100];
            srandom(time(nullptr));
            sprintf(output_path, "%s-out%ld", apk, random()%1000);

            list_emplace(&ra->droid_bundles, droid_create(apk, output_path, ra->bundle_count++));
        }
        free(apk_list);
    }

    return ra;
}

void ray_any_done(ray_state_t * rs) {
    if (rs->type==RAY_BUILD_FOR_APK) {
        while (rs->droid_bundles) {
            droid_destroy(
                list_erase(&rs->droid_bundles, rs->droid_bundles));
        }
        memset(&rs->ray_cnt, 0, sizeof(rs->ray_cnt));
    }
    free(rs);
}


static void droid_multiple_options(const ray_state_t * rs) {
    for (const list_t * l=rs->droid_bundles; l; l=l->next) {
        droid_bundle_t * bundle = l->data;
        printf("pkg package name: %s\n", droid_get_package_name(bundle));
        if (*(const bool*)pa_get("useful_strings"))
            droid_display_useful_strings(bundle);
        if (*(const char**)pa_get("extract"))
            droid_extract(bundle);
        if (*(const char**)pa_get("list_intents"))
            droid_list_intents(bundle, stdout);
    }
}
static void droid_group_options(const ray_state_t * rs) {
    if (list_size(rs->droid_bundles)==2) {
    }
    if (*(const bool*)pa_get("diff")) {
        const droid_bundle_t * first=list_front(rs->droid_bundles);
        const droid_bundle_t * last=list_back(rs->droid_bundles);

        droid_diff((const droid_bundle_t*[]){first, last},
            *(const bool*)pa_get("exclude_equals"),
        pa_get("only"));
    }
}

static void ray_run(const ray_state_t * rs) {
        if (ray_check_context(rs))
            return;
    if (rs->type==RAY_BUILD_FOR_APK) {
        if (*(const char*)pa_get("get_apk")) {
            droid_get_apk(list_front(rs->droid_bundles), pa_get("get_apk"));
        }
        droid_multiple_options(rs);
        droid_group_options(rs);
    }
}

static void pa_add_droid() {
    pa_string("out_dir");

    pa_string("get_apk");
    pa_bool("list_intents");
    pa_bool("useful_strings");
    pa_set_default(pa_bool("extract"), "true");
}

int main() {
    for (size_t i=0;i<100;i++)
        pa_list[i]=&pa_list_arg[i];
    pa_add_droid();

    pa_set_default(pa_string("apk_list"), "F-Droid.apk|org.fdroid.fdroid_2000010.apk");
    pa_bool("diff");
    pa_set_default(pa_bool("exclude_equals"), "false");
    pa_set_default(pa_string("only"), "classes*.dex|lib/*|assets/*");



    ray_state_t * r_rs = ray_build_for(RAY_BUILD_FOR_APK);
    ray_run(r_rs);

    ray_any_done(r_rs);

    return 0;
}
