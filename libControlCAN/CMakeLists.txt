cmake_minimum_required (VERSION 3.15)
message(STATUS "Select CMake build type: ${CMAKE_BUILD_TYPE}")

add_definitions(-DBUILDING_LIBCC_DLL)

if (CMAKE_HOST_UNIX)
	# there maybe multiple boost install, override the default boost install path
	link_directories("/usr/local/lib")
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
file(GLOB LIB_CTRL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
add_library(ControlCAN SHARED ${LIB_CTRL_SRCS})

if (CMAKE_HOST_UNIX)
	target_link_libraries(ControlCAN dl -Wl,-Bstatic boost_filesystem boost_chrono -Wl,-Bdynamic -Wl,--as-needed)
endif()

set(ControlCAN ${ControlCAN} PARENT_SCOPE)
set_property(TARGET ControlCAN PROPERTY POSITION_INDEPENDENT_CODE ON)

if (CMAKE_HOST_WIN32)
	set_property(TARGET ControlCAN PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
