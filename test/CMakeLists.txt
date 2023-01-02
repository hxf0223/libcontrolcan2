cmake_minimum_required (VERSION 3.15)

file( GLOB SRC_FILES *.cc )
add_executable( can_test ${SRC_FILES} )

if (CMAKE_HOST_UNIX)
	target_link_libraries( can_test ControlCAN glog gtest ${EXTRA_LD_FLAGS} )
else()
	target_link_libraries( can_test ControlCAN glog gtest )
endif()

if (CMAKE_HOST_WIN32)
	set_property(TARGET can_test PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test_zmq)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test_eventpp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test_git_ver)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/udp_server) # boost.asio upd server demo
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test_asio_server_client) # test for asio server/client comm.

