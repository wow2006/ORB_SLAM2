# ${CMAKE_SOURCE_DIR}/tests/acceptances/CMakeLists.txt
add_executable(
  testTracking
  testTracking.cpp
)

target_link_libraries(
  testTracking
  PRIVATE
  ORB_SLAM2
  ApprovalTests::ApprovalTests-header-only
)

if(FALSE)
  add_executable(
      acceptanceFrame
      acceptanceFrame.cpp
  )

  target_link_libraries(
      acceptanceFrame
      PRIVATE
      ORB_SLAM2
  )
endif()

