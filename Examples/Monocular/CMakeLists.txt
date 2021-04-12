# ${CMAKE_SOURCE_DIR}/Monocular/CMakeLists.txt
if(ENABLE_TUM)
  add_executable(
    mono_tum
    mono_tum.cc
  )

  target_link_libraries(
    mono_tum
    PRIVATE
    ${PROJECT_NAME}
  )
endif()

if(ENABLE_KITTI)
  add_executable(
    mono_kitti
    mono_kitti.cc
  )

  target_link_libraries(
    mono_kitti
    PRIVATE
    ${PROJECT_NAME}
  )
endif()

if(ENABLE_EuRoC)
  add_executable(
    mono_euroc
    mono_euroc.cc
  )

  target_link_libraries(
    mono_euroc
    PRIVATE
    ${PROJECT_NAME}
  )
endif()

