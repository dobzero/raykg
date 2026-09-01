#include "strtab.h"

#include <stdlib.h>
#include <string.h>

elf_strtab_t * elf_strtab_new(const size_t tables_count) {
    elf_strtab_t * strtab = malloc(sizeof(elf_strtab_t));
    strtab->tables = calloc(tables_count+1, sizeof(table_str_t));
    strtab->tables_count = tables_count+1;
    for (size_t i=0; i < strtab->tables_count; i++) {
        strtab->tables[i].capacity=0xFFF;
        strtab->tables[i].strings = calloc(sizeof(char), strtab->tables[i].capacity);
    }

    return strtab;
}
void elf_strtab_emplace(elf_strtab_t * strtab, FILE *fp, const size_t offset, const size_t size) {
    // first fit
    fseek(fp, (long)offset, SEEK_SET);
    table_str_t * table=nullptr;
    for (size_t i=0; i < strtab->tables_count && !table; i++) {
        if (strtab->tables[i].size)
            continue;
        table=&strtab->tables[i];
    }
    if (!table) {
        const size_t last_table=strtab->tables_count;
        strtab->tables_count=2*last_table;
        table_str_t * tables = realloc(strtab->tables, sizeof(table_str_t) * strtab->tables_count);
        if (tables)
            strtab->tables=tables;
        table=&strtab->tables[last_table];
        memset(table,0,sizeof(table_str_t));
    }

    if (table->capacity<size) {
        char * strings=realloc(table->strings, size);
        if (strings)
            table->strings = strings;
        table->capacity=size;
    }
    fread(table->strings, size, 1, fp);
    table->size=size;
}
void elf_strtab_delete(elf_strtab_t *strtab) {
    for (size_t i=0; i<strtab->tables_count; i++) {
        free(strtab->tables[i].strings);
    }
    free(strtab->tables);
    free(strtab);
}
