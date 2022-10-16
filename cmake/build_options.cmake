set( EXTRA_LD_FLAGS "" )

set( ENABLE_PROFILER OFF CACHE BOOL "Enable gperftools profiler" )
if ( CMAKE_HOST_UNIX AND ENABLE_PROFILER )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -Wl,--eh-frame-hdr" )
    set( TEMP_LD_FLAGS "-Wl,--no-as-needed" profiler "-Wl,--as-needed" )
    set( EXTRA_LD_FLAGS ${TEMP_LD_FLAGS} ${EXTRA_LD_FLAGS} )
endif()

