cmake_minimum_required(VERSION 4.3)
project(raykg C)

set(CMAKE_C_STANDARD 23)

add_executable(raykg main.c adb_query.c adb_query.h sign.c sign.h fw.c fw.h)
