#include "droid.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glob.h>

static const char * apksigner_path() {
    static char path[1000]={};
    if (strlen(path))
        return path;
    strcpy(path, "~/Android/Sdk/build-tools/*/apksigner");
    glob_t sdks;
    if (!glob(path, GLOB_TILDE, nullptr, &sdks)) {
        strcpy(path, sdks.gl_pathv[0]);
        globfree(&sdks);
    } else {
        memset(path, 0, sizeof(path));
    }
    return *path?path:nullptr;
}

void droid_sign_resign_all(const char *apks, const char * keystore, const char * alias, const char *pass_ks, const char * pass_key) {
    char * files_dup=strdup(apks);

    char *back=nullptr;
    const char * signer_path = apksigner_path();
    if (!signer_path)
        return;
    for (char * file = strtok_r(files_dup, "\n", &back); file;
        file = strtok_r(nullptr, "\n", &back)) {
        char sign[4000];
        if (strcmp(strrchr(file, '.'), ".apk")!=0)
            continue;
        sprintf(sign, "%s sign --ks %s --ks-key-alias %s --ks-pass \"pass:%s\" --key-pass \"pass:%s\" %s", signer_path, keystore, alias, pass_ks, pass_key, file);

        FILE * signer = popen(sign, "r");
        if (!signer)
            return;
        fclose(signer);

        if (!droid_fw_check_filename(file)) {
            return;
        }
        sprintf(sign, "%s verify --print-certs %s", signer_path, file);
        printf("checking filename: %s\n", file);
        signer=popen(sign, "r");
        while (fgets(sign, sizeof(sign), signer)) {
            printf("%s", sign);
        }
        pclose(signer);

        }

    free(files_dup);
}
