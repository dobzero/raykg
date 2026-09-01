#include <stdio.h>

#include "core/fs_dir.h"
#include "droid/adb_query.h"
#include "droid/sign.h"
#include "droid/fw.h"

#include "core/types.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void pull_apks(const char * apk_path, const char * out_dir, const char ** files, ray_file_t * apkinfo) {
    droid_adb_pull(apk_path, out_dir);
    *files = fs_list_files(out_dir);
    const char * apk_name=droid_adb_get_bundle_package_name(apk_path);

    strcpy(apkinfo->package_name, apk_name);
    free((void*)apk_name);
    ray_file_save(apkinfo);
}

static void recompile_apks(const char *out_dir, const char *files, ray_file_t * apkinfo) {
    char fullpath[1000];
    sprintf(fullpath, "%s.xapk", apkinfo->package_name);
    if (!access(fullpath, F_OK))
        return;
    droid_sign_resign_all(files, "keys-google.keystore", "android-keys", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2");
    droid_adb_compile_bundle(apkinfo->package_name, out_dir, BUNDLE_X_APK_FORMAT);
}

void droid_get_apk(const char *out_dir) {
    char * apk_list= droid_adb_get_packages_list();
    char * apk_path=apk_list?droid_adb_get_apk_path(apk_list, "gtasa"):nullptr;
    const char * files = fs_list_files(out_dir);

    ray_file_t apkinfo;
    bool proceed=true;
    do {
        if (apk_path && !files) {
            pull_apks(apk_path, out_dir, &files, &apkinfo);
            free(apk_path);
        } else {
            strcpy(apkinfo.package_name, out_dir);
            if (access(apkinfo.package_name, F_OK))
                if (!((proceed=false)))
                    break;
            ray_file_load(&apkinfo);
        }
    } while (false);
    if (proceed) {
        droid_fw_check_filename(apkinfo.package_name);

        recompile_apks(out_dir, files, &apkinfo);
        ray_file_save(&apkinfo);
    }
    free((void*)files);
    if (apk_list)
        free(apk_list);
}