
#include "adb/adb_query.h"
#include "core/zip_io.h"
#include "core/fs_dir.h"
#include "tree/files.h"
#include "cmdlist.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <fts.h>

static void droid_set_apk_name(droid_bundle_t * bundle, const char * apk_name) {
    strcpy(bundle->context->ray_data.package_name, apk_name);
    if (strcmp(bundle->output_dir, "output-apk-list")==0) {
        free(bundle->output_dir);
        bundle->output_dir= strdup(bundle->context->ray_data.package_name);
    }
}

static void pull_apks(droid_bundle_t *bundle, const char * apk_path) {
    char * apk_name=droid_adb_get_bundle_package_name(apk_path);


    droid_set_apk_name(bundle, apk_name);

    free(apk_name);
    if (access(bundle->output_dir, F_OK)==0) {
        bundle->output_dir_files = fs_list_files(bundle->output_dir);
        return;
    }
    droid_adb_pull(apk_path, bundle->output_dir);
    bundle->output_dir_files = fs_list_files(bundle->output_dir);

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

droid_bundle_t * droid_create(ray_any_t * ray, const char * in_apk, const char *out_dir) {
    droid_bundle_t * bundle = calloc(1, sizeof(droid_bundle_t));

    if (strlen(out_dir)==0) {
        bundle->output_dir = strdup("output-apk-list");
    } else {
        bundle->output_dir = strdup(out_dir);
    }
    bundle->output_dir_files = fs_list_files(out_dir);

    int err=0;
    if (strlen(in_apk)) {
        bundle->pkg_file=zip_open(in_apk, ZIP_RDONLY, &err);
        zip_error_t e; zip_error_init(&e);
        if (!bundle->pkg_file&&err) {
            fprintf(stderr, "cannot open this apk: %s\n", zip_error_strerror(&e));
            zip_error_fini(&e);
        } else {
            if (droid_test_apk_is_apk(bundle))
                bundle->input_file = strdup(in_apk);
            droid_load_files(bundle);
        }
    }
    if (bundle->input_file||bundle->output_dir)
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
    if (strlen(bundle->context->ray_data.package_name)==0) {
        if (bundle->manifest) {
            char * package_name = manifest_get(bundle->manifest, "package");
            strcpy(bundle->context->ray_data.package_name, package_name);
            free(package_name);
        } else if (!bundle->input_file) {
                return bundle->output_dir;
        }
    }

    return bundle->context->ray_data.package_name;
}

static void fs_get_parent_path_only(char *output, const char * path) {
    if (strrchr(path, '/')) {
        const char * parent = strrchr(path, '/');
        const char *end=parent;
        while (*--parent && *(parent-1)!='/' && parent!=path) {}
        strncpy(output, parent, end-parent);
    } else {
        strcpy(output, path);
    }
}

static int locate_cachedir_with(const droid_bundle_t *bundle, const char * pkg_name) {
    char *paths[] = { (char*)".", nullptr };
    FTS * fsdir = fts_open(paths, FTS_LOGICAL|FTS_NOCHDIR, nullptr);
    char * package_name=bundle->context->ray_data.package_name;
    if (fsdir) {
        for (const FTSENT *e_file=nullptr; strlen(package_name)==0 && ((e_file=fts_read(fsdir))); ) {
            if (e_file->fts_info & FTS_F) {
                if (strcmp(e_file->fts_name, "base.apk")==0) {
                    if (strstr(e_file->fts_path, pkg_name)==nullptr)
                        continue;
                    fs_get_parent_path_only(package_name, e_file->fts_path);
                }
            }
        }
        fts_close(fsdir);
    }
    return strlen(package_name)>0;
}

void droid_get_apk(droid_bundle_t * bundle, const char * pkg_ref_name) {
    char * apk_list= droid_adb_get_packages_list();
    char * apk_path=apk_list?droid_adb_get_apk_path(apk_list, pkg_ref_name):nullptr;

    do {
        if (apk_path && !bundle->output_dir_files) {
            pull_apks(bundle, apk_path);
            free(apk_path);
            bundle->proceed=true;
        } else {
            if (!locate_cachedir_with(bundle, pkg_ref_name))
                break;
            ray_file_load(bundle->context);
            if (access(droid_get_package_name(bundle), F_OK))
                break;
            bundle->proceed=true;
        }
    } while (false);
    if (bundle->proceed) {
        droid_fw_check_filename(droid_get_package_name(bundle));

        recompile_apks(bundle, droid_get_package_name(bundle));
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
