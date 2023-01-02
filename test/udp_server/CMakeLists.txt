cmake_minimum_required (VERSION 3.15)

file( GLOB SRC_FILES *.cc )
add_executable( udp_serv ${SRC_FILES} )

if (CMAKE_HOST_UNIX)
	target_link_libraries( udp_serv ControlCAN glog gtest ${EXTRA_LD_FLAGS} )
else()
	target_link_libraries( udp_serv ControlCAN glog gtest )
endif()

if (CMAKE_HOST_WIN32)
	set_property(TARGET udp_serv PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

