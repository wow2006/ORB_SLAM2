# ${CMAKE_SOURCE_DIR}/Examples/RGB-D/CMakeLists.txt
if(ENABLE_TUM)
    add_executable(
      rgbd_tum
      rgbd_tum.cc
    )

    target_link_libraries(
      rgbd_tum
      PRIVATE
      ${PROJECT_NAME}
    )
endif()
