FILE(GLOB src_files *.cpp)

add_executable(asio_udp_client ${INC_FILES} ${src_files})
target_link_libraries(asio_udp_client pthread glog)

if (CMAKE_HOST_WIN32)
	set_property(TARGET asio_udp_client PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()