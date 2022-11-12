cmake_minimum_required(VERSION 3.1)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
project(bingo_examples)
include_directories(
    ${CONAN_INCLUDE_DIRS}
)
link_directories(
    ${CONAN_LIB_DIRS}
)
add_subdirectory(server)

add_subdirectory(client)
add_subdirectory(http_server)

#add_subdirectory(web_server)

add_subdirectory(aws_service)