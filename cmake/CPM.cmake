# CPM.cmake
# ---------
# CMake Package Manager bootstrap.
# This file is intentionally small so the project stays "clone and build" friendly.

set(CPM_DOWNLOAD_VERSION 0.40.2)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake...")
  file(DOWNLOAD
    https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
    ${CPM_DOWNLOAD_LOCATION}
    STATUS CPM_DOWNLOAD_STATUS
  )
  list(GET CPM_DOWNLOAD_STATUS 0 CPM_DOWNLOAD_STATUS_CODE)
  if(NOT CPM_DOWNLOAD_STATUS_CODE EQUAL 0)
    message(FATAL_ERROR "CPM.cmake download failed: ${CPM_DOWNLOAD_STATUS}")
  endif()
endif()

include(${CPM_DOWNLOAD_LOCATION})
