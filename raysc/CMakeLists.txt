cmake_minimum_required(VERSION 4.3)
project(raysc C)

set(CMAKE_C_STANDARD 11)

add_library(raysc STATIC library.c)
