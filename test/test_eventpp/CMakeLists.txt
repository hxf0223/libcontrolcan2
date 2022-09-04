cmake_minimum_required (VERSION 3.15)

file( GLOB SRC_FILES *.cc )
add_executable( test_eventpp ${SRC_FILES} )
target_link_libraries( test_eventpp glog gmock_test_all )

if (CMAKE_HOST_WIN32)
	set_property(TARGET test_eventpp PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

