# Define the two required variables before including
# the source code for watching a git repository.
set(PRE_CONFIGURE_FILE "${PROJECT_SOURCE_DIR}/git_ver_src_in/git.c.in")
set(POST_CONFIGURE_FILE "${CMAKE_BINARY_DIR}/git.c")
include("${PROJECT_SOURCE_DIR}/cmake/git_watcher.cmake")

file( GLOB SRC_FILES *.cc )
add_executable( test_git_ver ${SRC_FILES} ${POST_CONFIGURE_FILE} )
target_link_libraries( test_git_ver glog gtest )
target_include_directories(test_git_ver SYSTEM PUBLIC "${PROJECT_SOURCE_DIR}/git_ver_src_in")
