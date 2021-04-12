# ${CMAKE_SOURCE_DIR}/Examples/Stereo/CMakeLists.txt
if(ENABLE_KITTI)
  add_executable(
    stereo_kitti
    stereo_kitti.cc
  )

  target_link_libraries(
    stereo_kitti
    PRIVATE
    ${PROJECT_NAME}
  )
endif()

if(ENABLE_EuRoC)
  add_executable(
    stereo_euroc
    stereo_euroc.cc
  )

  target_link_libraries(
    stereo_euroc
    PRIVATE
    ${PROJECT_NAME}
  )
endif()

