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
