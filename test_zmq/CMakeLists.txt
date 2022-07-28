cmake_minimum_required (VERSION 3.15)

include(${PROJECT_SOURCE_DIR}/cmake/platform_include.cmake)

file( GLOB SRC_FILES *.cc )
add_executable( zmq_test ${SRC_FILES} )
target_link_libraries( zmq_test zmq gtest glog )

if (CMAKE_HOST_WIN32)
	set_property(TARGET zmq_test PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
