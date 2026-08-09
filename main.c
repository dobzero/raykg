
#include "ADB_query.h"
#include "SIGN.h"
#include "FW.h"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

typedef enum {
    PROGRAM_ARGTYPE_CHAR
} Program_ArgType_e;

typedef struct {
    char arg[100];
    int32_t *flag;
    Program_ArgType_e type;
    union {
        char str_[100];
    } value;
} Program_Arg_t;

static Program_Arg_t PA_List[100];

static char * PA_string(const char * name) {
    for (Program_Arg_t *p = PA_List; p != NULL; p++) {
        if (*p->arg!='\0')
            continue;
        p->flag=nullptr;
        strcpy(p->arg, name);
        p->type=PROGRAM_ARGTYPE_CHAR;
        return p->value.str_;
    }
    return nullptr;
}

static void PA_set_default(const void *value, const char * arg) {
    for (Program_Arg_t *p = PA_List; p != NULL; p++) {
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

void STR_append(char **result, const char *a);
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
        STR_append(&result, path);

    }

    free(entry);

    closedir(fp);
    return result;
}

static void RayInfo_save(RayInfo_BC_t *apkinfo) {
    char path[1000];
    sprintf(path, "%s.rayinfo", apkinfo->packagename);
    FILE * fp=fopen(path, "w");
    fwrite(apkinfo, sizeof(*apkinfo), 1, fp);
    fclose(fp);
}

static void RayInfo_load(RayInfo_BC_t *apkinfo) {
    char path[1000];
    sprintf(path, "%s.rayinfo", apkinfo->packagename);
    FILE * fp=fopen(path, "r");
    fread(apkinfo, sizeof(*apkinfo), 1, fp);
    fclose(fp);
}

static void pull_apks(const char * apk_path, const char * out_dir, const char ** files, RayInfo_BC_t * apkinfo) {
    ADB_pull(apk_path, out_dir);
    *files = list_files(out_dir);
    const char * apk_name=ADB_get_bundle_package_name(apk_path);

    strcpy(apkinfo->packagename, apk_name);
    free((void*)apk_name);
    RayInfo_save(apkinfo);
}

static void recompile_apks(const char *out_dir, const char *files, RayInfo_BC_t * apkinfo) {
    char fullpath[1000];
    sprintf(fullpath, "%s.xapk", apkinfo->packagename);
    if (!access(fullpath, F_OK))
        return;
    SIGN_resign_all(files, "keys-google.keystore", "android-keys", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2");
    ADB_compile_bundle(apkinfo->packagename, out_dir, BUNDLE_X_APK_FORMAT);
}

int main() {
    const char * get_apk=PA_string("get_apk");
    const char * out_dir=PA_string("out_dir");
    PA_set_default(out_dir, "com.rockstargames.gtasa");

    char * apk_list= ADB_get_packages_list();
    char * apk_path=apk_list?ADB_get_apk_path(apk_list, "gtasa"):nullptr;
    const char * files = list_files(out_dir);

    RayInfo_BC_t apkinfo;
    if (apk_path && !files) {
        pull_apks(apk_path, out_dir, &files, &apkinfo);
        free(apk_path);
    } else {
        strcpy(apkinfo.packagename, out_dir);
        RayInfo_load(&apkinfo);
    }
    FW_check_filename(apkinfo.packagename);

    recompile_apks(out_dir, files, &apkinfo);
    free((void*)files);
    if (apk_list)
        free(apk_list);

    RayInfo_save(&apkinfo);

    return *get_apk!='\0';
}
