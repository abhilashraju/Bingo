cmake_minimum_required(VERSION 3.1)

project (bingoserver)


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

add_compile_options(-O0 -Wall -Wextra -pedantic ) 

SET (PROJECT_ROOT "..")
SET (UNIFEX_INCLUDE_DIR  "${PROJECT_ROOT}/thirdparty/include")
MESSAGE ("Source path: ${PROJECT_SOURCE_DIR}")
MESSAGE ("Binary path: ${PROJECT_BINARY_DIR}")

include_directories(
    "${PROJECT_ROOT}/bingo" 
    "${UNIFEX_INCLUDE_DIR}" 
    )
link_directories(
    "${PROJECT_ROOT}/build/bingo"
)
# Add an executable
add_executable(bingoserver 
                ${PROJECT_SOURCE_DIR}/bingoserver2.cpp 
                )

# link against the boost libraries
target_link_libraries( bingoserver
    PRIVATE
        pthread
        unifex
)