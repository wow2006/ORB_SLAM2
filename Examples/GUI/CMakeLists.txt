# ${CMAKE_SOURCE_DIR}/Examples/GUI/CMakeLists.txt

find_package(range-v3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Qt5 REQUIRED Widgets Xml OpenGL)
find_package(QGLViewer REQUIRED)

add_executable(
  ORB_SLAM3_GUI
  main.cpp
  Scene.hpp
  Scene.cpp
  mainwindow.hpp
  mainwindow.cpp
  mainwindow.ui
  dialogEuRoC.hpp
  dialogEuRoC.cpp
  dialogEuRoC.ui
)

target_include_directories(
  ORB_SLAM3_GUI
  PRIVATE
  ${QGLVIEWER_INCLUDE_DIR}
)

target_link_libraries(
  ORB_SLAM3_GUI
  PRIVATE
  ORB_SLAM2
  OpenGL::GLU
  OpenGL::GL
  Qt::Xml
  Qt::Xml
  Qt::OpenGL
  Qt::Widgets
  ${QGLVIEWER_LIBRARY}
  range-v3
)

set_target_properties(
  ORB_SLAM3_GUI
  PROPERTIES
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
  CXX_STANDARD_REQUIRED ON
  AUTOMOC ON
  AUTOUIC ON
)

