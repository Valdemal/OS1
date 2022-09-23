cmake_minimum_required(VERSION 3.23)
project(OS1 C)

set(CMAKE_C_STANDARD 99)

add_executable(OS1 main.c test.c my_syscalls.c)
