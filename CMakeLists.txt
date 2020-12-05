cmake_minimum_required(VERSION 3.17)
project(hash_map)

set(CMAKE_CXX_STANDARD 14)

add_executable(hash_map main.cpp)
# add coverage
# https://plugins.jetbrains.com/plugin/11031-c-c--cover..
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fprofile-instr-generate)
    #Uncomment in case of linker errors
    #link_libraries(clang_rt.profile-x86_64)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(--coverage)
    #Uncomment in case of linker errors
    #link_libraries(gcov)
endif ()