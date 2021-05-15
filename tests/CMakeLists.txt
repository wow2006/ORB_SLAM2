# ${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt
if(ENABLE_UNIT_TESTS)
  add_subdirectory(unit-tests)
endif()

if(ENABLE_ACCEPTNACES)
  add_subdirectory(acceptances)
endif()

