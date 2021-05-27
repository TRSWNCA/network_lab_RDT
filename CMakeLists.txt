cmake_minimum_required(VERSION 3.19)
project(RDT)

set(CMAKE_CXX_STANDARD 14)

add_executable(RDT
        src/prog2.c)

add_executable(rdt_1
        src/rdt_1.c)

add_executable(rdt_2_0
        src/rdt_2_0.c)

add_executable(rdt_2_1
        src/rdt_2_1.c)

add_executable(rdt_2_2
        src/rdt_2_2.c)

add_executable(rdt_3
        src/rdt_3.c)

add_executable(stopAndWait
        src/RDT_stopAndWait_ver.c)

add_executable(stopAndWait_KRC
        src/RDT_stopAndWait_KRC_ver.c)

add_executable(goBackN
        src/RDT_goBackN_ver.c)

add_executable(goBackN_KRC
        src/RDT_goBackN_KRC_ver.c)