include(${PROJECT_SOURCE_DIR}/cmake/platform_include.cmake)

file( GLOB SRC_FILES *.cc )
add_executable( asio_example ${SRC_FILES} )
target_link_libraries( asio_example ControlCAN gtest glog )

if (CMAKE_HOST_WIN32)
	set_property(TARGET asio_example PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()