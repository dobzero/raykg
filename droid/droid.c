#include <stdio.h>

#include "adb/adb_query.h"
#include "droid_file.h"
#include "../core/fs_dir.h"
#include "../cmdlist.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void pull_apks(droid_file_t *drf, const char * apk_path) {
    droid_adb_pull(apk_path, drf->output_dir);
    drf->output_dir_files = fs_list_files(drf->output_dir);
    char * apk_name=droid_adb_get_bundle_package_name(apk_path);

    strcpy(drf->apk_info.package_name, apk_name);
    free(apk_name);
    ray_file_save(&drf->apk_info);
}

static void recompile_apks(const droid_file_t *drf) {
    char fullpath[1000];
    sprintf(fullpath, "%s.xapk", drf->apk_info.package_name);
    if (!access(fullpath, F_OK))
        return;
    droid_sign_resign_all(drf->output_dir_files, "keys-google.keystore", "android-keys", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2");
    droid_adb_compile_bundle(drf->apk_info.package_name, drf->output_dir, BUNDLE_X_APK_FORMAT);
}

droid_file_t * droid_create(const char *out_dir) {
    droid_file_t * drf = calloc(1, sizeof(droid_file_t));

    drf->output_dir = strdup(out_dir);
    drf->output_dir_files = fs_list_files(out_dir);
    return drf;
}

void droid_destroy(droid_file_t * drf) {
    free(drf->output_dir_files);
    free(drf->output_dir);
    free(drf);
}

void droid_get_apk(droid_file_t * drf) {
    char * apk_list= droid_adb_get_packages_list();
    char * apk_path=apk_list?droid_adb_get_apk_path(apk_list, "gtasa"):nullptr;

    const auto apk_info = &drf->apk_info;
    do {
        if (apk_path && !drf->output_dir_files) {
            pull_apks(drf, apk_path);
            free(apk_path);
        } else {
            strcpy(apk_info->package_name, drf->output_dir);
            if (access(apk_info->package_name, F_OK))
                if (!((drf->proceed=false)))
                    break;
            ray_file_load(apk_info);
        }
    } while (false);
    if (drf->proceed) {
        droid_fw_check_filename(apk_info->package_name);

        recompile_apks(drf);
        ray_file_save(apk_info);
    }

    if (apk_list)
        free(apk_list);
}