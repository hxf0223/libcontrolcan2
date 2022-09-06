# Define the two required variables before including
# the source code for watching a git repository.
set(PRE_CONFIGURE_FILE "${PROJECT_SOURCE_DIR}/git_ver_src_in/git.c.in")
set(POST_CONFIGURE_FILE "${CMAKE_BINARY_DIR}/git.c")
include("${PROJECT_SOURCE_DIR}/cmake/git_watcher.cmake")

file( GLOB SRC_FILES *.cc )
add_executable( test_git_ver ${SRC_FILES} )
#target_include_directories(test_git_ver SYSTEM PUBLIC "${PROJECT_SOURCE_DIR}/cmake/git_watcher.cmake")
#add_dependencies(test_git_ver check_git)
