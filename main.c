
#include "core/types.h"
#include "cmdlist.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    PROGRAM_ARGTYPE_CHAR
} program_arg_type_e;

typedef struct {
    char arg[100];
    int32_t *flag;
    program_arg_type_e type;
    union {
        char str_[100];
    } value;
} program_arg_t;

static program_arg_t PA_List[100];

static char * pa_string(const char * name) {
    for (program_arg_t *p = PA_List; p != NULL; p++) {
        if (*p->arg!='\0')
            continue;
        p->flag=nullptr;
        strcpy(p->arg, name);
        p->type=PROGRAM_ARGTYPE_CHAR;
        return p->value.str_;
    }
    return nullptr;
}

static void pa_set_default(const void *value, const char * arg) {
    for (program_arg_t *p = PA_List; p != NULL; p++) {
        if ((void*)p->value.str_!=value)
            continue;
        switch (p->type) {
            case PROGRAM_ARGTYPE_CHAR:
                strcpy(p->value.str_, arg);
                break;
        }
        break;
    }
}

void ray_file_save(ray_file_t *apkinfo) {
    char path[1000];
    sprintf(path, "%s.rayinfo", apkinfo->package_name);
    FILE * fp=fopen(path, "w");
    fwrite(apkinfo, sizeof(*apkinfo), 1, fp);
    fclose(fp);
}

void ray_file_load(ray_file_t *apkinfo) {
    char path[1000];
    sprintf(path, "%s.ri", apkinfo->package_name);
    FILE * fp=fopen(path, "r");
    fread(apkinfo, sizeof(*apkinfo), 1, fp);
    fclose(fp);
}


int main() {
    const char * get_apk=pa_string("get_apk");
    const char * out_dir=pa_string("out_dir");
    pa_set_default(out_dir, "com.rockstargames.gtasa");

    const auto droid_do = droid_create(out_dir);
    if (get_apk)
        droid_get_apk(droid_do);
    droid_destroy(droid_do);

    return *get_apk!='\0';
}
