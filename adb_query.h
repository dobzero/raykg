#pragma once

char * adb_get_packages_list();
char * adb_get_apk_path(const char * query, const char *apk);
const char * adb_get_bundle_package_name(const char *apks);

void adb_pull(const char *apks, const char *outdir);
typedef enum bundle_type {
    BUNDLE_X_APK_FORMAT
} bundle_type_e;

void adb_compile_bundle(const char * apk_name, const char *outdir, bundle_type_e type);
