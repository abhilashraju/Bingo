
cmake_minimum_required(VERSION 3.15)

# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
project(Bingo CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-O0 -Wall -Wextra -pedantic -fconcepts -fPIC)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    TYPE INCLUDE
    PATTERN "*.in" EXCLUDE)
