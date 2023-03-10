cmake_minimum_required (VERSION 3.15)
if (POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif (POLICY CMP0091)

project ("ControlCAN" VERSION 0.0.2)

# cmake set info
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin) # set static lib's output dir
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin) # set DSO output dir
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS) # dismiss compile message about boost placeholders

if (CMAKE_HOST_WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-D_WIN32_WINNT=0x0A00) # avoid boost.asio warning: Please define _WIN32_WINNT or _WIN32_WINDOWS appropriately
endif()

include(cmake/build_type.cmake)
include(cmake/build_options.cmake)


# find needed boost from system config
#set(Boost_USE_STATIC_LIBS ON)
set(BOOST_COMPONENTS date_time chrono filesystem iostreams program_options regex system thread)
find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})
if(Boost_FOUND)
	message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
	message(STATUS "Boost_LIB_VERSION: ${Boost_LIB_VERSION}")

	include_directories(${Boost_INCLUDE_DIRS})
	link_directories(${BOOST_LIBRARYDIR})
else()
	message(FATAL_ERROR "Boost not found.")
endif()


include_directories("${PROJECT_SOURCE_DIR}/3rd/include")

include_directories("${PROJECT_SOURCE_DIR}/3rd/libzmq/include")
include_directories("${PROJECT_SOURCE_DIR}/3rd/include/cppzmq")
include_directories("${PROJECT_SOURCE_DIR}/3rd/googletest/include")
include_directories("${PROJECT_SOURCE_DIR}/include")
link_directories("${PROJECT_SOURCE_DIR}/bin")

add_subdirectory("${PROJECT_SOURCE_DIR}/3rd/glog-0.6.0")
add_subdirectory("${PROJECT_SOURCE_DIR}/3rd/googletest")
include_directories(${GLOG_INC_DIR})

add_subdirectory("3rd/libzmq")
add_subdirectory("libControlCAN")

add_subdirectory("can_agent")
add_subdirectory ("test")
add_subdirectory("asio_example")
