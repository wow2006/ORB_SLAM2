# ${CMAKE_SOURCE_DIR}/Examples/CMakeLists.txt
if(ENABLE_RGBD)
  add_subdirectory(RGB-D)
endif()

if(ENABLE_STEREO)
  add_subdirectory(Stereo)
endif()

if(ENABLE_MONO)
  add_subdirectory(Monocular)
endif()

if(ENABLE_GUI)
  add_subdirectory(GUI)
endif()

