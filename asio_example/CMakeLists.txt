file( GLOB_RECURSE  SRC_FILES *.cc )
add_executable( exam_asio ${SRC_FILES} )
target_link_libraries( exam_asio glog gtest )

if (CMAKE_HOST_WIN32)
	set_property(TARGET exam_asio PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

