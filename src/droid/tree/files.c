#include "droid/meta_inf/manifest.h"

#include "unix/elf/elf.h"
#include "files.h"

#include <string.h>
static void droid_dump_all_libs(zip_t *zp, const zip_uint64_t index) {
    zip_file_t *zf = zip_fopen_index(zp, index, 0);
    zip_stat_t st;
    zip_stat_index(zp, index, 0, &st);

    char * lib_so = malloc(st.size);

    zip_fread(zf, lib_so, st.size);
    zip_fclose(zf);

    FILE * fp = fmemopen(lib_so, st.size, "r");

    unix_elf_bin_t * elf = unix_elf_open_fp(fp);

    unix_elf_print(elf);
    unix_elf_close(elf);

    free(lib_so);
}

void droid_load_files(droid_bundle_t *bundle) {
    bundle->manifest = manifest_from_archive(bundle->pkg_file);
    manifest_prepare(bundle->manifest);

    for (size_t i=0;
        i<zip_get_num_entries(bundle->pkg_file, 0); i++) {

        const char * libname = zip_get_name(bundle->pkg_file, i, 0);
        if (strstr(libname, "lib")&&strstr(libname, ".so")) {
            printf("info of %s\n", libname);
            droid_dump_all_libs(bundle->pkg_file, i);
        }
    }
}
void droid_unload_files(droid_bundle_t *bundle) {
    if (bundle->manifest)
        manifest_destroy(bundle->manifest);
    bundle->manifest=nullptr;
}