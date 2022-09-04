cmake_minimum_required (VERSION 3.15)

file( GLOB SRC_FILES *.cc )
add_executable( test_zmq ${SRC_FILES} )
#target_link_libraries( test_zmq gtest glog -Wl,-Bstatic zmq -Wl,-Bdynamic )
if (CMAKE_HOST_UNIX)
	target_link_libraries( test_zmq glog gmock_test_all zmq )
elseif(CMAKE_HOST_WIN32)
	target_link_libraries( test_zmq glog gmock_test_all libzmq )
endif()

if (CMAKE_HOST_WIN32)
	set_property(TARGET test_zmq PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
