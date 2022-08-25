cmake_minimum_required (VERSION 3.15)

add_definitions(-DWIN32_LEAN_AND_MEAN)

#set (CMAKE_CXX_STANDARD 17)
link_directories(${PROJECT_BINARY_DIR})

file(GLOB_RECURSE srcs *.cc server/*.cc)

add_executable (can_agent ${srcs})
if (CMAKE_HOST_UNIX)
	target_link_libraries(can_agent ControlCAN gtest glog pthread)
elseif(CMAKE_HOST_WIN32)
	target_link_libraries(can_agent ControlCAN gtest glog)
endif()

if (WIN32)
	set_property(TARGET can_agent PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
