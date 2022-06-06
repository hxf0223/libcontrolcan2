if (CMAKE_HOST_WIN32)
    include_directories("${PROJECT_SOURCE_DIR}/install_include/win32")
elseif (CMAKE_HOST_UNIX)
    include_directories("${PROJECT_SOURCE_DIR}/install_include/linux")
elseif (CMAKE_HOST_APPLE)
    # TBD
endif()
