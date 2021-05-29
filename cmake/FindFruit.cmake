# You can set your own route by setting FRUIT_INSTALLED_DIR:
# set(ENV{FRUIT_INSTALLED_DIR} "/path/to/fruit/build")

if(EXISTS "${CMAKE_TOOLCHAIN_FILE}")
  get_filename_component(VCPKG_DIR "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
  get_filename_component(VCPKG_DIR "${VCPKG_DIR}/../.." ABSOLUTE)
  if(WIN32)
    set(FRUIT_INSTALLED_DIR "${VCPKG_DIR}/installed/x64-windows/")
  else()
    set(FRUIT_INSTALLED_DIR "${VCPKG_DIR}/installed/x64-linux/")
  endif()
endif()

find_path(FRUIT_INCLUDE_DIR fruit.h
        HINTS (
            ${FRUIT_INSTALLED_DIR}
            /usr
            /usr/local
            )
        PATH_SUFFIXES include/fruit
        )

find_library(FRUIT_LIBRARY
        NAMES fruit
        HINTS (
            ${FRUIT_INSTALLED_DIR}
            /usr
            /usr/local
            )
        PATH_SUFFIXES lib lib64
        )

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Fruit DEFAULT_MSG FRUIT_LIBRARY FRUIT_INCLUDE_DIR)
