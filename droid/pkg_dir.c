#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "droid_file.h"
static void zip_get(zip_t *z, const size_t i, zip_file_t **zf, const char **filename, size_t *sz) {
    zip_stat_t index;
    zip_stat_index(z, i, 0, &index);
    *zf=zip_fopen_index(z, i, 0);
    *filename=index.name;
    *sz=index.size;
}

static char * strpath_r(char *src, char **bkp) {
    return strtok_r(src, "/", bkp);
}

static void create_dirs(const char * path, const bool isdir) {
    char fullpath[4000], walkdir[4000]={0};
    strcpy(fullpath, path);
    if (!isdir)
        *strrchr(fullpath, '/')='\0';
    char *bkp=nullptr;
    for (const char *parent = strpath_r(fullpath, &bkp);
        parent; parent=strpath_r(nullptr, &bkp)) {
        char * dir=walkdir+strlen(walkdir);
        strcpy(dir, parent);
        mkdir(walkdir, 0777);
        dir[strlen(parent)]='/';
    }

}

#define MIN(x, y) (x<y ? x : y)
void droid_extract(const droid_file_t *drf) {
    const auto files=zip_get_num_entries(drf->pkg_file, ZIP_FL_UNCHANGED);

    if (access(drf->output_dir, F_OK))
        create_dirs(drf->output_dir, true);

    char target[1000];
    for(size_t i=0;i<files;i++) {
        zip_file_t * zf =nullptr;
        const char *filename=nullptr;
        size_t size=0;
        zip_get(drf->pkg_file, i, &zf, &filename, &size);

        snprintf(target, sizeof(target), "%s/%s", drf->output_dir, filename);

        create_dirs(target, false);
        FILE * fp = fopen(target, "w");
        uint8_t buffer[4096];
        for (size_t z_off=0;z_off<size;) {
            const size_t n = MIN(size-z_off, 4096);
            const size_t r=zip_fread(zf, buffer, n);

            fwrite(buffer, 1, r, fp);

            z_off+=r;
        }

        fclose(fp);
        zip_fclose(zf);
    }
}
