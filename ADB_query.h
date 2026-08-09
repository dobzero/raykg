#pragma once

char * ADB_get_packages_list();
char * ADB_get_apk_path(const char * query, const char *apk);
const char * ADB_get_bundle_package_name(const char *apks);

void ADB_pull(const char *apks, const char *outdir);
typedef enum Bundle_Type {
    BUNDLE_X_APK_FORMAT
} Bundle_Type_e;

void ADB_compile_bundle(const char * apk_name, const char *outdir, Bundle_Type_e type);
