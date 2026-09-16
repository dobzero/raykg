cmake_minimum_required(VERSION 4.3)
project(raykg C)

set(CMAKE_C_STANDARD 23)

add_library(droid STATIC
        droid/adb/adb_query.c droid/adb/adb_query.h
        droid/sign.c
        droid/fw.c
        droid/droid_file.h
        droid/integrity.c
        droid/pkg_dir.c)
add_library(unix_elf STATIC unix/elf/elf_file.c unix/elf/elf_file.h unix/elf/strtab.c unix/elf/strtab.h)

add_executable(raykg
        main.c
        droid/droid.c
        core/fs_dir.c core/fs_dir.h
        core/types.h
        cmdlist.h
        auto/script.c auto/script.h
    )

set(BUILD_REGRESS OFF CACHE BOOL "Build regression tests" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(BUILD_DOC OFF CACHE BOOL "Build documentation" FORCE)
add_subdirectory(libzip)

enable_testing()
add_executable(bin_test test/bin_test.c)
target_link_libraries(bin_test unix_elf)
add_test(bin_test "open bin files for testing prp")

target_link_libraries(raykg droid unix_elf libzip::zip)
