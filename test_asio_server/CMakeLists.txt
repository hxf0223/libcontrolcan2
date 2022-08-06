cmake_minimum_required (VERSION 3.15)

include_directories(${PROJECT_SOURCE_DIR}/libzmq/include)
link_directories(${PROJECT_BINARY_DIR})

file(GLOB_RECURSE srcs server.cc server/*.cc)

add_executable (test_asio_server ${srcs})
target_link_libraries(test_asio_server ControlCAN gtest glog -Wl,-Bstatic zmq -Wl,-Bdynamic)

add_executable(test_asio_client client.cc)
target_link_libraries(test_asio_client pthread)

if (WIN32)
	set_property(TARGET test_asio_server PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
	set_property(TARGET test_asio_client PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
