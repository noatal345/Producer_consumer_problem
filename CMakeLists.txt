cmake_minimum_required(VERSION 3.25)
project(os_ex3 C)

set(CMAKE_C_FLAGS -pthread)


add_executable(os_ex3 main.c noa.c)
