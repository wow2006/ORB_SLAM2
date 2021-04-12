# ${CMAKE_SOURCE_DIR}/cmake/download_file.cmake
function(download_file FILE_NAME URL MD5)
  if(EXISTS "${FILE_NAME}")
    return()
  endif()

  file(
    DOWNLOAD
    ${URL}
    ${FILE_NAME}
    EXPECTED_MD5 ${MD5}
  )
endfunction()

function(decompress FILE_NAME DEST_DIR)
  if(EXISTS "${DEST_DIR}")
    return()
  endif()

  file(ARCHIVE_EXTRACT
       INPUT       "${FILE_NAME}"
       DESTINATION "${DEST_DIR}"
  )
endfunction()

