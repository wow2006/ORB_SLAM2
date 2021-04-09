# ${CMAKE_SOURCE_DIR}/cmake/FindDBoW2.cmake
if(TARGET ORB_SLAM2::DBoW2)
  return()
endif()

add_library(ORB_SLAM2::DBoW2 SHARED IMPORTED)

set_target_properties(
  ORB_SLAM2::DBoW2
  PROPERTIES
  IMPORTED_LOCATION_RELEASE         "${CMAKE_SOURCE_DIR}/Thirdparty/DBoW2/lib/libDBoW2.so"
  IMPORTED_IMPLIB                   "${CMAKE_SOURCE_DIR}/Thirdparty/DBoW2/lib/libg2o.so"
  INTERFACE_INCLUDE_DIRECTORIES     "${CMAKE_SOURCE_DIR}/;${CMAKE_SOURCE_DIR}/Thirdparty/;${CMAKE_SOURCE_DIR}/Thirdparty/DBoW2/"
  IMPORTED_CONFIGURATIONS           Release
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
)

