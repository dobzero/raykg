#pragma once

char * fs_list_files(const char * dir);

char * strpath_r(char *src, char **bkp);
void create_dirs(const char * path, bool isdir);