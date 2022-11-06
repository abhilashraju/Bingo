cmake_minimum_required(VERSION 3.1)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
set (appname bingo_web_server)
project (${appname})

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


SET (PROJECT_ROOT "..")
SET (SERVER_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src")

include_directories(
    "${PROJECT_ROOT}/bingo" 
    "${PROJECT_ROOT}/web_server/include" 
    "${CONAN_INCLUDE_DIRS_LIBUNIFEX}"
    "${CONAN_INCLUDE_DIRS_NLOHMANN_JSON}" 
    )

# Add an executable
add_executable(${appname} 
                ${SERVER_SOURCE_DIR}/web_server.cpp 
                )

# link against the boost libraries
target_link_libraries( ${appname}
    PRIVATE
        pthread
        ${CONAN_LIBS_LIBUNIFEX}
)