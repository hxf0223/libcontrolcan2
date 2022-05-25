cmake_minimum_required (VERSION 3.15)

message(STATUS "Select CMake build type: ${CMAKE_BUILD_TYPE}")

add_definitions(-DBUILDING_LIBCC_DLL)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

file(GLOB LIB_CTRL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
add_library(libControlCAN SHARED ${LIB_CTRL_SRCS})
set(libControlCAN ${libControlCAN} PARENT_SCOPE)

if (WIN32)
	set_property(TARGET libControlCAN PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
