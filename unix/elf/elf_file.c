#include "elf_file.h"

#include <stdlib.h>
#include <elf.h>
#include <string.h>
#include <unistd.h>

static void * exchange(void **v, void *e) {
    void *r = *v;
    *v = e;
    return r;
}

elf_needed_t * elf_needed_new() {
    elf_needed_t * needed = calloc(1, sizeof(elf_needed_t));
    needed->list_capacity=20;
    needed->list=calloc(needed->list_capacity, sizeof(char*));
    for (size_t i=0; i<needed->list_capacity; i++) {
        needed->list[i]=calloc(0x3F, sizeof(char));
    }
    return needed;
}

void elf_needed_print_deps(const elf_needed_t *deps) {
    for (size_t i=0; i<deps->list_size; i++) {
        char *libname = deps->list[i];
        fprintf (stdout, "%s\n", libname);
    }
}

void elf_needed_delete(elf_needed_t *needed) {
    for (size_t i=0; i<needed->list_capacity; i++) {
        free(needed->list[i]);
    }
    free(needed->list);
    free(needed);
}

static void elf_needed_emplace(elf_needed_t * deps, const elf_strtab_t * elf_strtab, const Elf64_Dyn * dyn) {
    table_str_t * table=nullptr;
    for (size_t i=0; i<elf_strtab->tables_count && !table; i++) {
        if (dyn->d_tag==DT_NEEDED)
            if (elf_strtab->tables[i].size>dyn->d_un.d_val)
                table=&elf_strtab->tables[i];
    }
    if (!table)
        return;
    if (deps->list_size==deps->list_capacity)
        return;
    char *libname = deps->list[deps->list_size++];
    strncpy(libname, &table->strings[dyn->d_un.d_val], 0x3F);

}

unix_elf_bin_t * unix_elf_open(const char * path) {
    unix_elf_bin_t * ueb = malloc(sizeof(unix_elf_bin_t));
    ueb->string_table = elf_strtab_new(0);

    ueb->fp = fopen(path, "rb");

    Elf64_Ehdr * hdr = malloc(sizeof(Elf64_Ehdr)+sizeof(Elf64_Shdr));
    fread(hdr, sizeof(Elf64_Ehdr), 1, ueb->fp);

    const size_t sections[] = {
        hdr->e_shentsize, // count of sections
        hdr->e_shstrndx, // string table index

        hdr->e_shoff,

        hdr->e_phoff,
        hdr->e_phentsize
    };
    if (sections[3]&&sections[4]) {
        ueb->deps=elf_needed_new();
    }

    if (hdr->e_shentsize) {
        fseek(ueb->fp, (long)hdr->e_shoff, SEEK_SET);
        Elf64_Shdr * slist = realloc(exchange((void**)&hdr, nullptr), sizeof(Elf64_Shdr)*hdr->e_shentsize);
        if (slist)
            ueb->sections=slist;

        ueb->sections_count = fread(slist, sizeof(Elf64_Shdr), sections[0], ueb->fp);
        for (size_t i=0; i<ueb->sections_count; i++) {
            if (slist[i].sh_type==SHT_STRTAB || i==sections[1]) {
                elf_strtab_emplace(ueb->string_table, ueb->fp, slist[i].sh_offset, slist[i].sh_size);
            }
        }

        fseek(ueb->fp, (long)sections[3], SEEK_SET);
        Elf64_Phdr * plist = calloc(sizeof(Elf64_Phdr), sections[4]);
        const size_t plist_count = fread(plist, sizeof(Elf64_Phdr), sections[4], ueb->fp);

        for (size_t i=0; i<plist_count; i++) {
            if (plist[i].p_type!=PT_DYNAMIC) {
                continue;
            }
            fseek(ueb->fp, (long)plist[i].p_offset, SEEK_SET);
            const size_t dyn_count = plist[i].p_filesz/sizeof(Elf64_Dyn);
            Elf64_Dyn dyn_list[dyn_count];
            fread(dyn_list, sizeof(Elf64_Dyn), dyn_count, ueb->fp);

            for (size_t j = 0; j<dyn_count; j++) {
                elf_needed_emplace(ueb->deps, ueb->string_table, &dyn_list[j]);
            }
        }

        free(plist);
    }

    if (hdr)
        free(hdr);

    return ueb;
}

void unix_elf_print(const unix_elf_bin_t *ueb) {
    printf("deps: \n");
    elf_needed_print_deps(ueb->deps);
}

void unix_elf_close(unix_elf_bin_t *ueb) {
    fclose(ueb->fp);
    elf_strtab_delete(ueb->string_table);
    if (ueb->deps)
        elf_needed_delete(ueb->deps);
    free(ueb->sections);
    free(ueb);
}
