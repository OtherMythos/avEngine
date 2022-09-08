
FIND_PATH(LibSndFile_INCLUDE_DIR sndfile.h
    NO_DEFAULT_PATH
    PATHS ${LibSndFile_ROOT}/include
    ENV LibSndFile_ROOT)

FIND_LIBRARY(LibSndFile_BASE_LIB NAMES sndfile
    NO_DEFAULT_PATH
    PATHS ${LibSndFile_ROOT}/lib/
    PATH_SUFFIXES a
)

set(LibSndFile_LIBRARY "" CACHE STRING "" FORCE)
if(LibSndFile_BASE_LIB)
    set(LibSndFile_LIBRARY "${LibSndFile_LIBRARY};${LibSndFile_BASE_LIB}" CACHE STRING "" FORCE)
endif()

SET(LibSndFile_FOUND FALSE)
IF(LibSndFile_INCLUDE_DIR AND LibSndFile_LIBRARY)
    SET(LibSndFile_FOUND TRUE)
ENDIF(LibSndFile_INCLUDE_DIR AND LibSndFile_LIBRARY)

IF(LibSndFile_FOUND)
    IF (NOT LibSndFile_FIND_QUIETLY)
        MESSAGE(STATUS "Found LibSndFile: ${LibSndFile_LIBRARY}")
        MESSAGE(STATUS "Found LibSndFile includes: ${LibSndFile_INCLUDE_DIR}")
    ENDIF (NOT LibSndFile_FIND_QUIETLY)
        ELSE(LibSndFile_FOUND)
    IF (LibSndFile_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find LibSndFile")
    ENDIF (LibSndFile_FIND_REQUIRED)
ENDIF(LibSndFile_FOUND)

mark_as_advanced( FORCE
    LibSndFile_INCLUDE_DIR
    LibSndFile_LIBRARY
)
