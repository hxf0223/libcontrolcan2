# Android should use 3.1 or above
cmake_minimum_required(VERSION 3.1)


IF(NOT PROJECT_NAME)
  project(gmock_test_all)
ENDIF(NOT PROJECT_NAME)

set(TARGET_SRC ${CMAKE_CURRENT_LIST_DIR}/gmock-gtest-all.cc)
set(TARGET_HDR ${CMAKE_CURRENT_LIST_DIR}/gtest/gtest.h ${CMAKE_CURRENT_LIST_DIR}/gmock/gmock.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# option to build either a shared or a static library. default: shared
option (BUILD_SHARED "Build shared library" ON)

if (BUILD_SHARED)
    add_library(gmock_test_all SHARED ${TARGET_SRC} ${TARGET_HDR})
else (BUILD_SHARED)
    add_library(gmock_test_all STATIC ${TARGET_SRC} ${TARGET_HDR})
endif (BUILD_SHARED)

# when other libraries or executables link to <target>.
target_include_directories(gmock_test_all PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>  # <prefix>/include
    )

#----------------------------------------------------------------------------
# Install instructions.
#----------------------------------------------------------------------------
install(
    TARGETS gmock_test_all
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    )

install(FILES glog/logging.h DESTINATION include/glog)
