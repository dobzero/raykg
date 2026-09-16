#include "droid_file.h"
#include <zip.h>

bool droid_test_apk_is_apk(const droid_file_t * drf) {
    zip_t * pkg_file = drf->pkg_file;

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

    return true;
}
