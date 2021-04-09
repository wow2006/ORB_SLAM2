# ${CMAKE_SOURCE_DIR}/cmake/Findg2o.cmake
if(TARGET ORB_SLAM2::g2o)
  return()
endif()

add_library(ORB_SLAM2::g2o SHARED IMPORTED)

set_target_properties(
  ORB_SLAM2::g2o
  PROPERTIES
  IMPORTED_LOCATION_RELEASE         "${CMAKE_SOURCE_DIR}/Thirdparty/g2o/lib/libg2o.so"
  IMPORTED_IMPLIB                   "${CMAKE_SOURCE_DIR}/Thirdparty/g2o/lib/libg2o.so"
  INTERFACE_INCLUDE_DIRECTORIES     "${CMAKE_SOURCE_DIR}/;${CMAKE_SOURCE_DIR}/Thirdparty/;${CMAKE_SOURCE_DIR}/Thirdparty/g2o/"
  IMPORTED_CONFIGURATIONS           Release
  IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
)

