#include "adb_query.h"
#include "droid/droid.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


// ReSharper disable once CppUseInternalLinkage
void str_append(char **result, const char *a) {
    if (*result) {
        char * r=nullptr;
        asprintf(&r, "%s%s", *result, a);
        free(*result); *result = r;
    } else {
        asprintf(result, "%s", a);
    }
}

char * droid_adb_get_packages_list() {
    FILE * adb=popen("adb shell pm list packages", "r");

    char apk[512];
    char *result=nullptr;
    while (fgets(apk, 512, adb)) {
        str_append(&result, apk);
    }

    pclose(adb);
    return result;
}

char * droid_adb_get_apk_path(const char *query, const char *apk) {
    const char * apk_name = strstr(query, apk);
    if (!apk_name)
        return nullptr;
    for (; *(apk_name-1)!=':'; --apk_name) {}
    char target[0xFFF];
    sprintf(target, "adb shell pm path %.*s", (int)(strchr(apk_name, '\n')-apk_name), apk_name);
    FILE * adb=popen(target, "r");
    fread(target, 0xFFF, 1, adb);
    pclose(adb);
    return strdup(target);
}

char * droid_adb_get_bundle_package_name(const char *apks) {
    const char *baseapk=strstr(apks, "/base.apk");
    if (!baseapk)
        return nullptr;
    baseapk--;
    for (; *(baseapk-1)!='/'; --baseapk) {}
    char *result=nullptr;
    asprintf(&result, "%.*s", (int)(strchr(baseapk, '-')-baseapk), baseapk);
    return result;
}

static const char * adb_get_extension(const bundle_type_e type) {
    if (type==BUNDLE_X_APK_FORMAT)
        return ".xapk";
    return nullptr;
}

void droid_adb_pull(const char *apks, const char *outdir) {
    char cmdls[10000];
    char *editable=strdup(apks);
    char *bk=nullptr;
    if (access(outdir, F_OK))
        mkdir(outdir, 0755);
    for (char *tok=strtok_r(editable, "\n", &bk);
        tok && isprint(*tok); tok=strtok_r(nullptr, "\n", &bk)) {

        if (strncmp(tok, "package:", strlen("package:"))==0)
            tok=strchr(tok, ':')+1;
        if (!droid_fw_check_filename(strrchr(tok, '/'))) {
            return;
        }
        sprintf(cmdls, "adb pull %s %s/%s", tok, outdir, strrchr(tok, '/')+1);
        if (access(strrchr(cmdls, ' ')+1, F_OK)==0)
            continue; // already extracted, we just get out now

        FILE *adb=popen(cmdls, "r");
        pclose(adb);
        }
    free(editable);
}

void droid_adb_compile_bundle(const char * apk_name, const char *outdir, const bundle_type_e type) {

    const char * extension=adb_get_extension(type);
    char cmdls[1000];
    sprintf(cmdls, "%s%s", apk_name, extension);
    if (access(cmdls, F_OK)==0)
        return;
    sprintf(cmdls, "rm %s/*.idsig", outdir);
    pclose(popen(cmdls, "r"));

    switch (type) {
        case BUNDLE_X_APK_FORMAT:
            sprintf(cmdls, "zip -j %s%s %s/*", apk_name, adb_get_extension(type), outdir);
            FILE * zip =popen(cmdls, "r");
            while (fgets(cmdls, 100, zip))
                sleep(1);
            pclose(zip);
    }
}
