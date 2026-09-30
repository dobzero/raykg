
#include "unix/elf/elf.h"

static void open_elf_and_print(const char *path) {
    fprintf(stdout, "open binary: %s\n", path);
    const auto linux_zstd = unix_elf_open(path);
    unix_elf_print(linux_zstd);
    unix_elf_close(linux_zstd);
}
int main() {
    open_elf_and_print("/bin/zstd");
    open_elf_and_print("/bin/ls");
    open_elf_and_print("/bin/bash");

}