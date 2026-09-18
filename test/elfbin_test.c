
#include "unix/elf/elf.h"

static void open_bin(const char *path) {
    fprintf(stdout, "open binary: %s\n", path);
    const auto linux_zstd = unix_elf_open(path);
    unix_elf_print(linux_zstd);
    unix_elf_close(linux_zstd);
}
int main() {
    open_bin("/bin/zstd");
    open_bin("/bin/ls");
    open_bin("/bin/bash");

}