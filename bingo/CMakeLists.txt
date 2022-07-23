cmake_minimum_required(VERSION 3.1)

project (unifex)


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

SET (UNIFEX_SOURCE_DIR  "${PROJECT_ROOT}/thirdparty/source")
SET (UNIFEX_INCLUDE_DIR  "${PROJECT_ROOT}/thirdparty/include")
MESSAGE ("Source path: ${PROJECT_SOURCE_DIR}")
MESSAGE ("Binary path: ${PROJECT_BINARY_DIR}")

include_directories(
    "${PROJECT_SOURCE_DIR}" 
    "${UNIFEX_INCLUDE_DIR}"     
    )
    
# Add an executable
add_library( unifex 
        ${UNIFEX_SOURCE_DIR}/async_mutex.cpp
        ${UNIFEX_SOURCE_DIR}/exception.cpp
        ${UNIFEX_SOURCE_DIR}/inplace_stop_token.cpp
        ${UNIFEX_SOURCE_DIR}/manual_event_loop.cpp
        ${UNIFEX_SOURCE_DIR}/static_thread_pool.cpp
        ${UNIFEX_SOURCE_DIR}/thread_unsafe_event_loop.cpp
        ${UNIFEX_SOURCE_DIR}/timed_single_thread_context.cpp
        ${UNIFEX_SOURCE_DIR}/trampoline_scheduler.cpp
        ${UNIFEX_SOURCE_DIR}/async_manual_reset_event.cpp
        ${UNIFEX_SOURCE_DIR}/linux/mmap_region.cpp
        ${UNIFEX_SOURCE_DIR}/linux/monotonic_clock.cpp
        ${UNIFEX_SOURCE_DIR}/linux/safe_file_descriptor.cpp
        ${UNIFEX_SOURCE_DIR}/linux/io_epoll_context.cpp
)
                

# link against the boost libraries
target_link_libraries( unifex
    PRIVATE
        pthread
)