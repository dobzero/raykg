
#include "adb_query.h"
#include "sign.h"
#include "fw.h"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

typedef enum {
    PROGRAM_ARGTYPE_CHAR
} program_arg_type_e;

typedef struct {
    char arg[100];
    int32_t *flag;
    program_arg_type_e type;
    union {
        char str_[100];
    } value;
} program_arg_t;

static program_arg_t PA_List[100];

static char * pa_string(const char * name) {
    for (program_arg_t *p = PA_List; p != NULL; p++) {
        if (*p->arg!='\0')
            continue;
        p->flag=nullptr;
        strcpy(p->arg, name);
        p->type=PROGRAM_ARGTYPE_CHAR;
        return p->value.str_;
    }
    return nullptr;
}

static void pa_set_default(const void *value, const char * arg) {
    for (program_arg_t *p = PA_List; p != NULL; p++) {
        if ((void*)p->value.str_!=value)
            continue;
        switch (p->type) {
            case PROGRAM_ARGTYPE_CHAR:
                strcpy(p->value.str_, arg);
                break;
        }
        break;
    }
}

void str_append(char **result, const char *a);
static const char * list_files(const char * dir) {
    char * result=nullptr;
    DIR * fp=opendir(dir);
    if (!fp)
        return nullptr;

    struct dirent * entry=calloc(1, sizeof(struct dirent)+255);

    for (struct dirent * ent_=nullptr; readdir_r(fp, entry, &ent_) ==0 && ent_; ) {
        if (*ent_->d_name=='.')
            continue;
        char path[1000];
        if (result)
            sprintf(path, "\n%s/%s", dir, ent_->d_name);
        else sprintf(path, "%s/%s", dir, ent_->d_name);
        str_append(&result, path);

    }

    free(entry);

    closedir(fp);
    return result;
}

static void ray_info_save(ray_bc_t *apkinfo) {
    char path[1000];
    sprintf(path, "%s.rayinfo", apkinfo->packagename);
    FILE * fp=fopen(path, "w");
    fwrite(apkinfo, sizeof(*apkinfo), 1, fp);
    fclose(fp);
}

static void ray_info_load(ray_bc_t *apkinfo) {
    char path[1000];
    sprintf(path, "%s.ri", apkinfo->packagename);
    FILE * fp=fopen(path, "r");
    fread(apkinfo, sizeof(*apkinfo), 1, fp);
    fclose(fp);
}

static void pull_apks(const char * apk_path, const char * out_dir, const char ** files, ray_bc_t * apkinfo) {
    adb_pull(apk_path, out_dir);
    *files = list_files(out_dir);
    const char * apk_name=adb_get_bundle_package_name(apk_path);

    strcpy(apkinfo->packagename, apk_name);
    free((void*)apk_name);
    ray_info_save(apkinfo);
}

static void recompile_apks(const char *out_dir, const char *files, ray_bc_t * apkinfo) {
    char fullpath[1000];
    sprintf(fullpath, "%s.xapk", apkinfo->packagename);
    if (!access(fullpath, F_OK))
        return;
    sign_resign_all(files, "keys-google.keystore", "android-keys", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2");
    adb_compile_bundle(apkinfo->packagename, out_dir, BUNDLE_X_APK_FORMAT);
}

int main() {
    const char * get_apk=pa_string("get_apk");
    const char * out_dir=pa_string("out_dir");
    pa_set_default(out_dir, "com.rockstargames.gtasa");

    char * apk_list= adb_get_packages_list();
    char * apk_path=apk_list?adb_get_apk_path(apk_list, "gtasa"):nullptr;
    const char * files = list_files(out_dir);

    ray_bc_t apkinfo;
    bool proceed=true;
    do {
        if (apk_path && !files) {
            pull_apks(apk_path, out_dir, &files, &apkinfo);
            free(apk_path);
        } else {
            strcpy(apkinfo.packagename, out_dir);
            if (access(apkinfo.packagename, F_OK))
                if (!((proceed=false)))
                    break;
            ray_info_load(&apkinfo);
        }
    } while (false);
    if (proceed) {
        fw_check_filename(apkinfo.packagename);

        recompile_apks(out_dir, files, &apkinfo);
        ray_info_save(&apkinfo);
    }
    free((void*)files);
    if (apk_list)
        free(apk_list);

    return *get_apk!='\0';
}
