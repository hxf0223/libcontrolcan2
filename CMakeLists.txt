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
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fno-omit-frame-pointer -Wl,--eh-frame-hdr" )

if (CMAKE_HOST_WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-D_WIN32_WINNT=0x0A00) # avoid boost.asio warning: Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately
endif()

include(cmake/build_type.cmake)

# for windows compile or cross-platform compile
include_directories("$ENV{BOOST_ROOT}/include")
link_directories("$ENV{BOOST_ROOT}/lib")

include_directories(${GLOG_INC_DIR})
include_directories("${PROJECT_SOURCE_DIR}/3rd/include")
include_directories("${PROJECT_SOURCE_DIR}/libzmq/include")
include_directories("${PROJECT_SOURCE_DIR}/3rd/include/cppzmq")
include_directories("${PROJECT_SOURCE_DIR}/googletest/include")
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
add_subdirectory("asio_example")

