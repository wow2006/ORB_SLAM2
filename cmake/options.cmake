# ${CMAKE_SOURCE_DIR}/cmake/options.cmake
if(TARGET options::options)
  return()
endif()

add_library(
  options
  INTERFACE
)

add_library(
  options::options
  ALIAS
  options
)

target_compile_options(
  options
  INTERFACE
  $<$<CXX_COMPILER_ID:Clang>:-Weverything;-Wno-c++98-compat-pedantic;-fcolor-diagnostics;-Wno-unused-macros>
  $<$<CXX_COMPILER_ID:GNU>:-Wall;-W;-Wpedantic;-Wshadow;-Wnon-virtual-dtor;-Wold-style-cast;-Wunused;-Wformat=2>
  $<$<CXX_COMPILER_ID:MSVC>:/W3;/permissive-;/bigobj>
)

