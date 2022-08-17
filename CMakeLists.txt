cmake_minimum_required (VERSION 3.15)
if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif (POLICY CMP0091)

project ("ControlCAN")

# cmake_policy(SET policy NEW)

# cmake set info
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 17)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin ) # set static lib's output dir
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin ) # set DSO output dir
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS) # dismiss compile message about boost placeholders

# for windows compile or cross-platform compile
include_directories("$ENV{BOOST_ROOT}/include")
link_directories("$ENV{BOOST_ROOT}/lib")

include_directories("${PROJECT_SOURCE_DIR}/3rd/include")
include_directories("${PROJECT_SOURCE_DIR}/libzmq/include")
include_directories("${PROJECT_SOURCE_DIR}/3rd/include/cppzmq")
include_directories("${PROJECT_SOURCE_DIR}/googletest/include")
#include_directories("${PROJECT_SOURCE_DIR}/install_include")
include_directories("${PROJECT_SOURCE_DIR}/include")
link_directories("${PROJECT_SOURCE_DIR}/bin")

add_subdirectory("libzmq")
add_subdirectory("glog")
add_subdirectory("googletest")
add_subdirectory("libControlCAN")

add_subdirectory("can_agent")
add_subdirectory ("test")
add_subdirectory ("test_zmq")
add_subdirectory("test_asio_server")
add_subdirectory("test_asio_udp")
add_subdirectory("asio_example")

