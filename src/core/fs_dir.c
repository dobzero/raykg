#include "fs_dir.h"

#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void str_append(char **result, const char *a);
static int discard(const struct dirent *del) {
    return *del->d_name!='.';
}

char * fs_list_files(const char * dir) {
    char * result=nullptr;

    struct dirent ** ent_list=nullptr;
    const size_t entries = scandir(dir, &ent_list, discard, alphasort);

    for (size_t e=0; e<entries && ent_list; e++) {
        char path[1000];
        if (result)
            sprintf(path, "\n%s/%s", dir, ent_list[e]->d_name);
        else sprintf(path, "%s/%s", dir, ent_list[e]->d_name);
        str_append(&result, path);

    }

    if (ent_list)
        free(ent_list);

    return result;
}

char * strpath_r(char *src, char **bkp) {
    return strtok_r(src, "/", bkp);
}

void create_dirs(const char * path, const bool isdir) {
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