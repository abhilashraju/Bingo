cmake_minimum_required(VERSION 3.1)

set (appname aws_sns_publisher)
project (${appname})

find_package( Boost )
find_package(AWSSDK REQUIRED)
include_directories( ${Boost_INCLUDE_DIRS} )

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
SET (SERVER_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src")
MESSAGE ("Source path: ${SERVER_SOURCE_DIR}")
MESSAGE ("Binary path: ${PROJECT_BINARY_DIR}")
MESSAGE ("AWS libraries: ${AWSSDK_LINK_LIBRARIES}")

include_directories(
    "${PROJECT_ROOT}/bingo" 
    "${PROJECT_ROOT}/web_server/include" 
    "${UNIFEX_INCLUDE_DIR}" 
    )
link_directories(
    "${PROJECT_ROOT}/build/bingo"
    "/usr/local/lib/"
)
# Add an executable
add_executable(${appname} 
                ${SERVER_SOURCE_DIR}/aws_service.cpp 
                )

# link against the boost libraries
target_link_libraries( ${appname}
    PRIVATE
        pthread
        unifex
        aws-cpp-sdk-core
        aws-cpp-sdk-sns
)