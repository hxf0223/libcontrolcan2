include(${PROJECT_SOURCE_DIR}/cmake/platform_include.cmake)

file( GLOB SRC_FILES *.cc )
add_executable( exam_asio_work_share_ptr ${SRC_FILES} )
target_link_libraries( exam_asio_work_share_ptr ControlCAN gtest glog )

if (CMAKE_HOST_WIN32)
	set_property(TARGET exam_asio_work_share_ptr PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tcp_serv_client)
