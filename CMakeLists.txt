cmake_minimum_required(VERSION 4.3)
project(raykg C)

set(CMAKE_C_STANDARD 23)

file(GLOB CORE_SRC src/core/*.c src/auto/*.c src/core/*.h src/auto/*.h)
file(GLOB DROID_SRC src/droid/*.c src/droid/adb/*.c drc/droid/*.h src/droid/adb/*.h)
file(GLOB UNIX_SRC src/unix/elf/*.c drc/unix/elf/*.h)

add_library(droid STATIC ${DROID_SRC})
add_library(unix_elf STATIC ${UNIX_SRC})
include_directories("src")

add_executable(raykg src/main.c src/cmdlist.h ${CORE_SRC})

set(BUILD_REGRESS OFF CACHE BOOL "Build regression tests" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples" FORCE)
set(BUILD_DOC OFF CACHE BOOL "Build documentation" FORCE)
add_subdirectory(libzip)

enable_testing()
add_executable(elfbin_test test/elfbin_test.c)
target_link_libraries(elfbin_test unix_elf)
add_test(elfbin_test "open bin files for testing prp")

target_link_libraries(raykg droid unix_elf libzip::zip)
