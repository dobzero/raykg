#pragma once
typedef struct RayInfo_Bundle_Content {
    char packagename[1000];
}RayInfo_BC_t;

void SIGN_resign_all(const char *apks, const char * keystore, const char * alias, const char *pass_ks, const char * pass_key);
