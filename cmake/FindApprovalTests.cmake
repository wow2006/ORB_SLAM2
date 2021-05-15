# ${CMAKE_SOURCE_DIR}/cmake/FindApprovalTests.cmake
set(ApprovalTests_URL "https://github.com/approvals/ApprovalTests.cpp/releases/download/v.10.9.1/ApprovalTests.v.10.9.1.hpp")
set(ApprovalTests_DIR "${CMAKE_SOURCE_DIR}/Thirdparty/ApprovalTests/include/ApprovalTests")

if(TARGET ApprovalTests::ApprovalTests-header-only)
  return()
endif()

if(NOT EXISTS ${ApprovalTests_DIR}/ApprovalTests.hpp)
  file(MAKE_DIRECTORY ${ApprovalTests_DIR})
  file(
    DOWNLOAD
    ${ApprovalTests_URL}
    ${ApprovalTests_DIR}/ApprovalTests.hpp
    SHOW_PROGRESS
    EXPECTED_HASH MD5=b6899213a40f36b427210cbdd4ef5266
  )
endif()

add_library(
  ApprovalTests::ApprovalTests-header-only
  INTERFACE IMPORTED
)

set_target_properties(
  ApprovalTests::ApprovalTests-header-only
  PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/Thirdparty/ApprovalTests/include/"
)

