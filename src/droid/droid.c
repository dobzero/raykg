
#include "adb/adb_query.h"
#include "core/zip_io.h"
#include "core/fs_dir.h"
#include "cmdlist.h"
#include "droid.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>

static void pull_apks(droid_bundle_t *bundle, const char * apk_path, char * package_name) {
    droid_adb_pull(apk_path, bundle->output_dir);
    bundle->output_dir_files = fs_list_files(bundle->output_dir);
    char * apk_name=droid_adb_get_bundle_package_name(apk_path);

    strcpy(package_name, apk_name);
    free(apk_name);
    ray_file_save(bundle->context);
}

static void recompile_apks(const droid_bundle_t *bundle, const char * package_name) {
    char fullpath[1000];
    sprintf(fullpath, "%s.xapk", package_name);
    if (!access(fullpath, F_OK))
        return;
    droid_sign_resign_all(bundle->output_dir_files, "keys-google.keystore", "android-keys", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2", "Scion4-Gloss3-Nicotine6-Liquid2-Sequel2");
    droid_adb_compile_bundle(package_name, bundle->output_dir, BUNDLE_X_APK_FORMAT);
}

static void droid_load_files(droid_bundle_t *bundle) {
    bundle->manifest = manifest_from_archive(bundle->pkg_file);
    manifest_prepare(bundle->manifest);
}
static void droid_unload_files(droid_bundle_t *bundle) {
    if (bundle->manifest)
        manifest_destroy(bundle->manifest);
    bundle->manifest=nullptr;
}

droid_bundle_t * droid_create(ray_any_t * ray, const char * in_apk, const char *out_dir) {
    droid_bundle_t * bundle = calloc(1, sizeof(droid_bundle_t));

    if (strlen(out_dir)) {
        bundle->output_dir = strdup(out_dir);
        bundle->output_dir_files = fs_list_files(out_dir);
    }
    int err=0;
    if (strlen(in_apk)) {
        bundle->pkg_file=zip_open(in_apk, ZIP_RDONLY, &err);
        zip_error_t e; zip_error_init(&e);
        if (!bundle->pkg_file&&err) {
            fprintf(stderr, "cannot open this apk: %s\n", zip_error_strerror(&e));
            zip_error_fini(&e);
            return nullptr;
        }

        if (droid_test_apk_is_apk(bundle))
            bundle->input_file = strdup(in_apk);
        droid_load_files(bundle);
    }
    if (bundle->input_file)
        bundle->context = ray;
    return bundle;
}

void droid_destroy(droid_bundle_t * bundle) {
    droid_unload_files(bundle);

    if (bundle->output_dir_files)
        free(bundle->output_dir_files);
    if (bundle->output_dir)
        free(bundle->output_dir);

    if (bundle->input_file) {
        free(bundle->input_file);
        zip_close(bundle->pkg_file);
    }

    free(bundle);
}

char * droid_get_package_name(const droid_bundle_t *bundle) {
    if (!bundle->input_file)
        return bundle->output_dir;

    if (strlen(bundle->context->ray_data.package_name)==0) {
        char * package_name = manifest_get(bundle->manifest, "package");
        strcpy(bundle->context->ray_data.package_name, package_name);
        free(package_name);
    }

    return bundle->context->ray_data.package_name;
}

void droid_get_apk(droid_bundle_t * bundle) {
    char * apk_list= droid_adb_get_packages_list();
    char * apk_path=apk_list?droid_adb_get_apk_path(apk_list, "gtasa"):nullptr;

    char * package_name = droid_get_package_name(bundle);
    do {
        if (apk_path && !bundle->output_dir_files) {
            pull_apks(bundle, apk_path, package_name);
            free(apk_path);
        } else {
            strcpy(package_name, bundle->output_dir);
            if (access(package_name, F_OK))
                if (!((bundle->proceed=false)))
                    break;
            ray_file_load(bundle->context);
        }
    } while (false);
    if (bundle->proceed) {
        droid_fw_check_filename(package_name);

        recompile_apks(bundle, package_name);
        ray_file_save(bundle->context);
    }

    if (apk_list)
        free(apk_list);
}

void droid_display_useful_strings(const droid_bundle_t *bundle) {
    if (!bundle->input_file)
        return;
    FILE * fp = fopen(bundle->input_file, "r");
    if (!fp)
        return;

    char buffer[4096 + 1] = {};
    for (size_t rfp=0, buf=0;
        (rfp = fread(buffer, 1, 4096, fp)); buf=0) {

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
                "http", ".com", ".io", ".org", ".so", // shared objects


                "android", "github" // domains
            };
            for (int p=0;p<sizeof(pattern_and_highlights)/sizeof(void*);p++)
                if (strstr(target, pattern_and_highlights[p]))
                    if (printf("%#lx: %.*s\n", ftell(fp), i, buffer + buf-i-1))
                        break;
        } while (buf < rfp);
    }

    fclose(fp);
}
