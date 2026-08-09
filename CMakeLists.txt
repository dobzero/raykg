cmake_minimum_required(VERSION 4.3)
project(raykg C)

set(CMAKE_C_STANDARD 23)

add_executable(raykg main.c ADB_query.c ADB_query.h SIGN.c SIGN.h FW.c FW.h)
