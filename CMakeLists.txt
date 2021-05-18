cmake_minimum_required(VERSION 3.19)
project(RDT)

set(CMAKE_CXX_STANDARD 14)

add_executable(RDT
        prog2.c)

add_executable(rdt_1
        rdt_1.c)

add_executable(rdt_2_0
        rdt_2_0.c)

add_executable(rdt_2_1
        rdt_2_1.c)

add_executable(rdt_2_2
        rdt_2_2.c)

add_executable(rdt_3
        rdt_3.c)

add_executable(stopAndWait
        RDT_stopAndWait_ver.c)