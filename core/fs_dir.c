#include "fs_dir.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

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
