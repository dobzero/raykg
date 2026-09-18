#pragma once

char * droid_adb_get_packages_list();
char * droid_adb_get_apk_path(const char * query, const char *apk);
char * droid_adb_get_bundle_package_name(const char *apks);

void droid_adb_pull(const char *apks, const char *outdir);
typedef enum bundle_type {
    BUNDLE_X_APK_FORMAT
} bundle_type_e;

void droid_adb_compile_bundle(const char * apk_name, const char *outdir, bundle_type_e type);
