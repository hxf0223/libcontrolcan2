cmake_minimum_required (VERSION 3.8)

add_executable (can_test "main.cc" ) # "${PROJECT_SOURCE_DIR}/gtest-1.7.0/gtest_all.cc"
target_link_libraries(can_test libControlCAN)

if (WIN32)
	set_property(TARGET can_test PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()