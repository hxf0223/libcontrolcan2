file( GLOB server_srcs async_server.cc )
add_executable( exam_asio_server ${server_srcs} )
target_link_libraries( exam_asio_server pthread )

file( GLOB client_srcs client.cc )
add_executable( exam_asio_client ${client_srcs} )
target_link_libraries( exam_asio_client pthread )

if (CMAKE_HOST_WIN32)
	set_property(TARGET exam_asio_server PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set_property(TARGET exam_asio_client PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
