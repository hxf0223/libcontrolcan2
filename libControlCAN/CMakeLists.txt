cmake_minimum_required (VERSION 3.15)

message(STATUS "Select CMake build type: ${CMAKE_BUILD_TYPE}")

add_definitions(-DBUILDING_LIBCC_DLL)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/easy_socket)

file(GLOB LIB_CTRL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/easy_socket/*.cc)
add_library(ControlCAN SHARED ${LIB_CTRL_SRCS})
if (CMAKE_HOST_UNIX)
	target_link_libraries(ControlCAN dl boost_filesystem boost_chrono)
endif()

#set_target_properties( ControlCAN PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )
set(ControlCAN ${ControlCAN} PARENT_SCOPE)

if (CMAKE_HOST_WIN32)
	set_property(TARGET ControlCAN PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
