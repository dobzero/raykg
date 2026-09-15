#include "droid_file.h"
#include <zip.h>

bool droid_test_apk_integrity(const char * filename) {
    zip_error_t e;
    int err=0;
    zip_t * pkg_file = zip_open(filename, ZIP_RDONLY, &err);
    zip_error_init_with_code(&e, err);
    if (!pkg_file&&err) {
        fprintf(stderr, "cannot open this apk: %s\n", zip_error_strerror(&e));
        zip_error_fini(&e);
        return false;
    }
    const auto files=zip_get_num_entries(pkg_file, ZIP_FL_UNCHANGED);
    printf("count of files in this apk: %lu\n", files);

    const char * package_root_structure[] = {
        "AndroidManifest.xml", "classes.dex", "resources.arsc"
    };
    for(int i=0;i<sizeof(package_root_structure)/sizeof(void*);i++) {
        zip_file_t * zf = zip_fopen(pkg_file, package_root_structure[i], 0);
        if (!zf) {
            fprintf(stderr, "cannot open %s\n", package_root_structure[i]);
            return false;
        }
        zip_fclose(zf);
    }

    zip_close(pkg_file);
    return true;
}