file( GLOB server_srcs async_server.cc )
add_executable( asio_server ${server_srcs} )
target_link_libraries( asio_server pthread )

file( GLOB client_srcs client.cc )
add_executable( asio_client ${client_srcs} )
target_link_libraries( asio_client pthread )
