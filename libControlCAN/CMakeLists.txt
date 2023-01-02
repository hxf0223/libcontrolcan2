cmake_minimum_required (VERSION 3.15)
message(STATUS "Select CMake build type: ${CMAKE_BUILD_TYPE}")

add_definitions(-DBUILDING_LIBCC_DLL)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
file(GLOB LIB_CTRL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
if (CMAKE_HOST_WIN32)
    file(GLOB LIB_DEF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/libControlCan.def)
    list(APPEND LIB_CTRL_SRCS ${LIB_DEF_FILE})
endif()
add_library(ControlCAN SHARED ${LIB_CTRL_SRCS})

if (CMAKE_HOST_UNIX) # TBD: there's some link conflict with find() when using static link with boost under linux
    #target_link_libraries(ControlCAN dl atomic -Wl,-Bstatic Boost::filesystem Boost::chrono -Wl,-Bdynamic -Wl,--as-needed)
    target_link_libraries(ControlCAN dl atomic Boost::filesystem Boost::chrono -Wl,--as-needed)
endif()

set(ControlCAN ${ControlCAN} PARENT_SCOPE)
set_property(TARGET ControlCAN PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties(ControlCAN PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

if (CMAKE_HOST_WIN32)
    set_property(TARGET ControlCAN PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()
