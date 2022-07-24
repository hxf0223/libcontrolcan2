cmake_minimum_required (VERSION 3.15)

file(GLOB_RECURSE srcs *.cc server/*.cc)

add_executable (can_agent ${srcs})
target_link_libraries(can_agent ControlCAN gtest glog)

if (WIN32)
	set_property(TARGET can_agent PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()