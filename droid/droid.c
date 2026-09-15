
#include "adb/adb_query.h"
#include "droid_file.h"
#include "../core/fs_dir.h"
#include "../cmdlist.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>

#include <zip.h>

static void pull_apks(droid_file_t *drf, const char * apk_path, char * package_name) {
    droid_adb_pull(apk_path, drf->output_dir);
    drf->output_dir_files = fs_list_files(drf->output_dir);
    char * apk_name=droid_adb_get_bundle_package_name(apk_path);

    strcpy(package_name, apk_name);
    free(apk_name);
    ray_file_save(drf->context);
}

static void recompile_apks(const droid_file_t *drf, const char * package_name) {
    char fullpath[1000];
    sprintf(fullpath, "%s.xapk", package_name);
    if (!access(fullpath, F_OK))
        return;
    droid_sign_resign_all(drf->output_dir_files, "keys-google.keystore", "android-keys", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2");
    droid_adb_compile_bundle(package_name, drf->output_dir, BUNDLE_X_APK_FORMAT);
}

droid_file_t * droid_create(ray_any_t * ray, const char * in_apk, const char *out_dir) {
    droid_file_t * drf = calloc(1, sizeof(droid_file_t));

    if (strlen(out_dir)) {
        drf->output_dir = strdup(out_dir);
        drf->output_dir_files = fs_list_files(out_dir);
    }
    if (strlen(in_apk)) {
        int err=0;
        zip_error_t e;
        zip_t * pkg_file = zip_open(in_apk, ZIP_RDONLY, &err);
        zip_error_init_with_code(&e, err);
        if (!pkg_file&&err) {
            fprintf(stderr, "this isn't a apk file: %s\n", zip_error_strerror(&e));
            goto NoContext;
        }
        zip_close(pkg_file);
        drf->input_file = strdup(in_apk);
    }
    drf->context = ray;
    NoContext:
    return drf;
}

void droid_destroy(droid_file_t * drf) {
    if (drf->output_dir_files)
        free(drf->output_dir_files);
    if (drf->output_dir)
        free(drf->output_dir);
    free(drf->input_file);

    free(drf);
}

char * droid_get_package_name(const droid_file_t *drf) {
    if (*drf->context->ray_data.package_name)
        return drf->context->ray_data.package_name;

    return drf->output_dir; // todo: search for android:package in Manifest later, but for now
}

void droid_get_apk(droid_file_t * drf) {
    char * apk_list= droid_adb_get_packages_list();
    char * apk_path=apk_list?droid_adb_get_apk_path(apk_list, "gtasa"):nullptr;

    char * package_name = drf->context->ray_data.package_name;
    do {
        if (apk_path && !drf->output_dir_files) {
            pull_apks(drf, apk_path, package_name);
            free(apk_path);
        } else {
            strcpy(package_name, drf->output_dir);
            if (access(package_name, F_OK))
                if (!((drf->proceed=false)))
                    break;
            ray_file_load(drf->context);
        }
    } while (false);
    if (drf->proceed) {
        droid_fw_check_filename(package_name);

        recompile_apks(drf, package_name);
        ray_file_save(drf->context);
    }

    if (apk_list)
        free(apk_list);
}

void droid_display_useful_strings(const droid_file_t *drf) {
    if (!drf->input_file)
        return;
    FILE * fp = fopen(drf->input_file, "r");
    if (!fp)
        return;

    char buffer[1024 + 1] = {};
    for (size_t rfp=0, buf=0;
        (rfp = fread(buffer, 1, 1024, fp)); buf=0) {

        do {
            int i =0;
            for(;buf<rfp;i++)
                if (!isprint(buffer[buf++]))
                    break;
            if (i<10)
                continue;
            char * target = buffer + buf-i;
            assert(isprint(*target));

            const char * pattern_and_highlights[] = {
                "http", ".com", ".io", ".org",


                "android", "github"
            };
            for (int p=0;p<6;p++)
                if (strstr(target, pattern_and_highlights[p]))
                    printf("%#lx: %.*s\n", ftell(fp), i, buffer + buf-i-1);
        } while (buf < rfp);
    }

    fclose(fp);
}
