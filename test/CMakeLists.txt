cmake_minimum_required (VERSION 3.15)

include(${PROJECT_SOURCE_DIR}/cmake/platform_include.cmake)

add_executable (can_test "main.cc")
target_link_libraries(can_test libControlCAN gtest glog)

if (WIN32)
	set_property(TARGET can_test PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
