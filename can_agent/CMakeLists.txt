cmake_minimum_required (VERSION 3.15)

add_executable (can_agent "main.cc")
target_link_libraries(can_agent libControlCAN gtest glog)

if (WIN32)
	set_property(TARGET can_agent PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()