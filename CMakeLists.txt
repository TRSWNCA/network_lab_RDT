cmake_minimum_required(VERSION 3.19)
project(RDT)

set(CMAKE_CXX_STANDARD 14)

add_executable(RDT_KRC
        src/prog2.c)

add_executable(rdt_1_KRC
        src/rdt_1.c)

add_executable(rdt_2_0_KRC
        src/rdt_2_0.c)

add_executable(rdt_2_1_KRC
        src/rdt_2_1.c)

add_executable(rdt_2_2_KRC
        src/rdt_2_2.c)

add_executable(rdt_3_KRC
        src/rdt_3.c)

add_executable(stopAndWait_modern
        src/RDT_stopAndWait_modern_ver.c)

add_executable(stopAndWait_KRC
        src/RDT_stopAndWait_KRC_ver.c)

add_executable(goBackN_modern
        src/RDT_goBackN_modern_ver.c)

add_executable(goBackN_KRC
        src/RDT_goBackN_KRC_ver.c)