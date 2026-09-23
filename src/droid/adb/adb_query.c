#include "adb_query.h"
#include "droid/droid.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <glob.h>
#include <fts.h>

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
    char target[0xFFF]={0};
    sprintf(target, "adb shell pm path %.*s", (int)(strchr(apk_name, '\n')-apk_name), apk_name);
    FILE * adb=popen(target, "r");
    fread(target, 0xFFF, 1, adb);
    pclose(adb);
    return strdup(target);
}

char * droid_adb_get_bundle_package_name(const char *apks) {
    const char * pkg_ns_begin = strstr(apks, "==/");
    pkg_ns_begin+=3;
    const char * pkg_ns_end = strchr(pkg_ns_begin, '-');
    char * result;
    asprintf(&result, "%.*s", (int)(pkg_ns_end-pkg_ns_begin), pkg_ns_begin);
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
            continue; // already extracted, we're done

        FILE *adb=popen(cmdls, "r");
        pclose(adb);
        }
    free(editable);
}

static void zip_compress_all_files(const char * filename, const char * dir) {
    zip_t * zf = zip_open(filename, ZIP_CREATE, nullptr);
    if (!zf)
        return;

    char *paths[] = { (char*)dir, nullptr };
    FTS * fsdir = fts_open(paths, FTS_LOGICAL|FTS_NOCHDIR, nullptr);
    if (fsdir) {
        for (const FTSENT *e_file=nullptr;
            (e_file=fts_read(fsdir)); ) {

            if (e_file->fts_info & FTS_F) {
                zip_source_t * zs = zip_source_file(zf, e_file->fts_path, 0, 0);
                const zip_uint64_t index = zip_file_add(zf, e_file->fts_name, zs, ZIP_FL_OVERWRITE);
                zip_set_file_compression(zf, index, ZIP_CM_DEFLATE, 6);
            }
        }
        fts_close(fsdir);
    }

    zip_close(zf);
}

void droid_adb_compile_bundle(const char * apk_name, const char *outdir, const bundle_type_e type) {

    const char * extension=adb_get_extension(type);
    char apk_filename[1000];
    sprintf(apk_filename, "%s%s", apk_name, extension);
    if (access(apk_filename, F_OK)==0)
        return;

    glob_t results;
    char idsig_glob[1000];
    sprintf(idsig_glob, "%s/*.idsig", outdir);
    if (glob(idsig_glob, 0, nullptr, &results)==0) {
        for (size_t i=0;i<results.gl_pathc;i++) {
            remove(results.gl_pathv[i]);
        }

    }

    switch (type) {
        case BUNDLE_X_APK_FORMAT:
            zip_compress_all_files(apk_filename, outdir);
            break;
    }
}
