cmake_minimum_required(VERSION 3.1)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
project (bingoclient)


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

add_compile_options(-O0 -Wall -Wextra -pedantic -fconcepts -fPIE) 

SET (PROJECT_ROOT "..")

MESSAGE ("Source path: ${PROJECT_SOURCE_DIR}")
MESSAGE ("Binary path: ${PROJECT_BINARY_DIR}")

include_directories(
    "${PROJECT_ROOT}/bingo"
    )

# Add an executable
add_executable(bingoclient 
                ${PROJECT_SOURCE_DIR}/bingoclient2.cpp 
                )

# link against the boost libraries
target_link_libraries( bingoclient
    PRIVATE
        pthread
        ${CONAN_LIBS}
)