#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "droid.h"
#include "core/fs_dir.h"
#include "core/zip_io.h"

static void droid_copy_resources(const droid_bundle_t *bundle) {
    char manifest_path[1000];
    char manifest_target[1000];
    sprintf(manifest_path, "%s/%s", bundle->output_dir, "AndroidManifest.xml");
    sprintf(manifest_target, "%s/%s", bundle->output_dir, "original/AndroidManifest.xml");
    create_dirs(manifest_target, false);

    rename(manifest_path, manifest_target);
    FILE *fp = fopen(manifest_path, "w");
    manifest_write(bundle->manifest, fp);
    fclose(fp);
}

#define MIN(x, y) (x<y ? x : y)
void droid_extract(const droid_bundle_t *bundle) {
    const auto files=zip_get_num_entries(bundle->pkg_file, ZIP_FL_UNCHANGED);

    if (access(bundle->output_dir, F_OK)) {
        create_dirs(bundle->output_dir, true);
    }

    char target[1000];
    for(size_t i=0;i<files;i++) {
        zip_file_t * zf =nullptr;
        const char *filename=nullptr;
        size_t size=0;
        zip_get(bundle->pkg_file, i, &zf, &filename, &size);

        snprintf(target, sizeof(target), "%s/%s", bundle->output_dir, filename);

        create_dirs(target, false);
        FILE * fp = fopen(target, "r");
        if (!fp) {
            fp = fopen(target, "w");

            uint8_t buffer[4096];
            for (size_t z_off=0;z_off<size;) {
                const size_t n = MIN(size-z_off, 4096);
                const size_t r=zip_fread(zf, buffer, n);

                fwrite(buffer, 1, r, fp);

                z_off+=r;
            }
        }

        fclose(fp);
        zip_fclose(zf);
    }

    droid_copy_resources(bundle);
}

typedef struct diff_item {
    char filepath[100];
    size_t size;
    uint32_t comp_type;
    uint32_t crc;
} diff_item_t;

static const char * interesting[] = {
    "lib/", "assets/", "META-INF/services/", "classes"
};

static list_t * snapshot(const droid_bundle_t * bundle, const char * only) {
    zip_t * zp = bundle->pkg_file;
    list_t * list=nullptr;
    for (size_t i=0;i<zip_get_num_entries(zp, ZIP_FL_UNCHANGED);i++) {
        const char * filename=zip_get_name(zp, i, ZIP_FL_UNCHANGED);
        bool snap_file=false;

        if (*only=='\0') {
            for (size_t j=0;j<3&&!snap_file;j++) {
                if (strstr(filename, interesting[j]))
                    snap_file=true;
            }
        } else {
            if (strstr(filename, only))
                snap_file=true;
        }
        if (!snap_file)
            continue;

        diff_item_t * diff = calloc(1, sizeof(diff_item_t));
        zip_stat_t zs;
        zip_stat_index(zp, i, 0, &zs);
        zip_file_t * zf = zip_fopen(zp, filename, ZIP_FL_UNCHANGED);

        strcpy(diff->filepath, filename);
        diff->size=zs.size;
        diff->comp_type=zs.comp_method;
        diff->crc=zs.crc;

        zip_fclose(zf);
        list_emplace(&list, diff);
    }
    return list;
}

static void snapshot_free(list_t *list) {
    while (list) {
        free (list_remove(&list, list));
    }
}

static void display_files(const list_t *l, const char * fmt, ...) {
    va_list va = {};
    va_start(va, fmt);
    vprintf(fmt, va);
    for (; l; l=l->next) {
        const diff_item_t * l_d = l->data;
        printf("filepath: %s, crc: %u\n", l_d->filepath, l_d->crc);
    }

    va_end(va);
}

void droid_diff(const droid_bundle_t *bundle[2], const bool exclude_eq, const char * only) {
    list_t * snap_first=snapshot(bundle[0], only);
    list_t * snap_last=snapshot(bundle[1], only);

    list_t * first_only=nullptr;
    list_t * last_only=list_dup(snap_last);

    list_t * common=nullptr;
    for (const list_t *f=snap_first; f; f=f->next) {
            diff_item_t * f_d = f->data;

        bool file_found_in_l=false;
        for (list_t *l=last_only; l; l=l->next) {
            const diff_item_t * l_d = l->data;

            if (strcmp(f_d->filepath, l_d->filepath)!=0) {
                continue;
            }
            file_found_in_l=true;
            if (f_d->crc!=l_d->crc) {

            } else {
                list_emplace(&common, list_remove(&last_only, l));
            }
            break;
        }
        if (!file_found_in_l)
            list_emplace(&first_only, f_d);
    }


    display_files(first_only, "files present in the first apk %s:\n", bundle[0]->input_file);

    display_files(last_only, "files present in the last apk %s:\n", bundle[1]->input_file);

    if (!exclude_eq)
        display_files(common, "common files between them:\n");

    list_destroy(first_only);
    list_destroy(last_only);
    list_destroy(common);

    snapshot_free(snap_first);
    snapshot_free(snap_last);

}
