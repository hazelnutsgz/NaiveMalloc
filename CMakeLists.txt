cmake_minimum_required(VERSION 3.10)
project(malloc C)

set(CMAKE_C_STANDARD 11)

add_library(malloc library.c library.h)