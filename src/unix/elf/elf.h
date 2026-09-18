#pragma once
#include "strtab.h"
#include <stdio.h>

typedef struct elf_needed {
    char ** list;
    size_t list_capacity;
    size_t list_size;
} elf_needed_t;

elf_needed_t * elf_needed_new();
void elf_needed_print_deps(const elf_needed_t *deps);
void elf_needed_delete(elf_needed_t*);

typedef struct unix_elf_bin {
    FILE * fp;
    void * sections; // section table list read from file
    size_t sections_count; // // count of section entries read

    elf_strtab_t * string_table;
    elf_needed_t * deps;
} unix_elf_bin_t;


unix_elf_bin_t * unix_elf_open(const char * path);
void unix_elf_print(const unix_elf_bin_t *ueb);
void unix_elf_close(unix_elf_bin_t *ueb);