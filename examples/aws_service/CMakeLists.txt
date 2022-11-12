cmake_minimum_required(VERSION 3.1)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
set(appname aws_sns_publisher)
project(appname)

# set the C++ standard to C++ 17
set(CMAKE_CXX_STANDARD 17)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message("Setting build type to 'RelWithDebInfo' as none was specified.")
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)

    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
        "MinSizeRel" "RelWithDebInfo")
endif()

add_compile_options(-O0 -Wall -Wextra -pedantic)

include_directories(
    ${CONAN_INCLUDE_DIRS_BOOST}
    ${CONAN_INCLUDE_DIRS_LIBUNIFEX}
    ${CONAN_INCLUDE_DIRS_BINGO}
    ${CONAN_INCLUDE_DIRS_BONGO}
    aws-cpp-sdk-core
)
link_directories(
    "${PROJECT_ROOT}/build/bingo"
)

# Add an executable
add_executable(${appname}
    ${PROJECT_SOURCE_DIR}/src/aws_service.cpp
)

# link against the boost libraries
target_link_libraries(${appname}
    PRIVATE
    pthread
    ${CONAN_LIBS_LIBUNIFEX}
    aws-cpp-sdk-core
    aws-cpp-sdk-sns
    curl
    ssl
    crypto
)