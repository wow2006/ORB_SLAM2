# ${CMAKE_SOURCE_DIR}/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(ORB_SLAM2 CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(options)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# ==========================
option(ENABLE_EXAMPLE "Build examples"       OFF)
option(ENABLE_TUM     "Build TUM example"    OFF)
option(ENABLE_KITTI   "Build KITTI example"  OFF)
option(ENABLE_EuRoC   "Build EuRoC example"  OFF)
option(ENABLE_MONO    "Build MONO example"   OFF)
option(ENABLE_STEREO  "Build Stereo example" OFF)
option(ENABLE_RGBD    "Build RGB-D example"  OFF)
# ==========================

find_package(OpenCV 3 REQUIRED imgproc features2d imgcodecs calib3d highgui)
find_package(Eigen3   REQUIRED)
find_package(Pangolin REQUIRED)
find_package(spdlog   REQUIRED)
find_package(range-v3 REQUIRED)
# Thirdparty
find_package(g2o   REQUIRED)
find_package(DBoW2 REQUIRED)

# ==========================
# SLAM Library
# ==========================
add_library(
  ${PROJECT_NAME}
  SHARED
)

target_sources(
  ${PROJECT_NAME}
  PRIVATE
  src/Map.cpp
  src/Frame.cpp
  src/System.cpp
  src/Viewer.cpp
  src/KeyFrame.cpp
  src/MapPoint.cpp
  src/Tracking.cpp
  src/MapDrawer.cpp
  src/Optimizer.cpp
  src/PnPsolver.cpp
  src/Converter.cpp
  src/ORBmatcher.cpp
  src/Sim3Solver.cpp
  src/Initializer.cpp
  src/LoopClosing.cpp
  src/FrameDrawer.cpp
  src/LocalMapping.cpp
  src/ORBextractor.cpp
  src/KeyFrameDatabase.cpp
)

target_include_directories(
  ${PROJECT_NAME}
  SYSTEM PUBLIC
  ${Pangolin_INCLUDE_DIRS}
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC
  spdlog::spdlog
  range-v3
  ${OpenCV_LIBS}
  ${Pangolin_LIBRARIES}
  Eigen3::Eigen
  ORB_SLAM2::g2o
  ORB_SLAM2::DBoW2
  options::options
)

target_compile_definitions(
  ${PROJECT_NAME}
  PUBLIC
  COMPILEDWITHC11
)

target_compile_features(
  ${PROJECT_NAME}
  PUBLIC
  cxx_std_17
)

target_compile_options(
  ${PROJECT_NAME}
  PUBLIC
  -march=native
)

target_precompile_headers(
  ${PROJECT_NAME}
  PUBLIC
  include/precompiled.hpp
)
# ==========================

# ==========================
# Build examples
if(ENABLE_EXAMPLE)
  add_subdirectory(Examples)
endif()
# ==========================

