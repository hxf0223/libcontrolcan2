cmake_minimum_required (VERSION 3.15)
project ("libControlCAN")

# cmake_policy(SET policy NEW)

# cmake set info
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 17)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin" )

include_directories("$ENV{BOOST_ROOT}/include")
link_directories("$ENV{BOOST_ROOT}/lib")

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/gtest-1.7.0/include")

add_subdirectory("libControlCAN")
add_subdirectory ("test")
