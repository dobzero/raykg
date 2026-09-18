#pragma once
#include <stddef.h>
#include <stdio.h>

typedef struct table_str {
    char * strings;
    size_t capacity;
    size_t size;
} table_str_t;

typedef struct elf_strtab {
    table_str_t *tables;
    size_t tables_count;
} elf_strtab_t;

elf_strtab_t * elf_strtab_new(size_t tables_count);
void elf_strtab_emplace(elf_strtab_t * strtab, FILE *fp, size_t offset, size_t size);
void elf_strtab_delete(elf_strtab_t *strtab);
