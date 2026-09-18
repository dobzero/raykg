#include "droid.h"

#include <string.h>

// https://android.googlesource.com/platform/frameworks/base/+/refs/heads/main/libs/androidfw/ApkParsing.cpp
bool droid_fw_check_filename(const char *file_a) {
    for (;;file_a++) {
        switch (*file_a) {
            case '\0':
                return true;
            case 'A' ... 'Z':
            case 'a' ... 'z':
            case '0' ... '9':
                break;
            default:
                char file_z[2]={*file_a, '\0'};
                if (strpbrk(file_z, "+,./=_"))
                    break;
                return false;
        }
    }
}
