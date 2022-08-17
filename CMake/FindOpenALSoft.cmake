
FIND_PATH(OpenALSoft_INCLUDE_DIR AL
    NO_DEFAULT_PATH
    PATHS ${OpenALSoft_ROOT}/include
    ENV OpenALSoft_ROOT)

FIND_LIBRARY(OpenALSoft_BASE_LIB NAMES openal
    NO_DEFAULT_PATH
    PATHS ${OpenALSoft_ROOT}/lib/
    PATH_SUFFIXES a
)

set(OpenALSoft_LIBRARY "" CACHE STRING "" FORCE)
if(OpenALSoft_BASE_LIB)
    set(OpenALSoft_LIBRARY "${OpenALSoft_LIBRARY};${OpenALSoft_BASE_LIB}" CACHE STRING "" FORCE)
endif()

SET(OpenALSoft_FOUND FALSE)
IF(OpenALSoft_INCLUDE_DIR AND OpenALSoft_LIBRARY)
    SET(OpenALSoft_FOUND TRUE)
ENDIF(OpenALSoft_INCLUDE_DIR AND OpenALSoft_LIBRARY)

IF(OpenALSoft_FOUND)
    IF (NOT OpenALSoft_FIND_QUIETLY)
        MESSAGE(STATUS "Found OpenAlSoft: ${OpenALSoft_LIBRARY}")
        MESSAGE(STATUS "Found OpenAlSoft includes: ${OpenALSoft_INCLUDE_DIR}")
    ENDIF (NOT OpenALSoft_FIND_QUIETLY)
        ELSE(OpenALSoft_FOUND)
    IF (OpenALSoft_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find OpenAlSoft")
    ENDIF (OpenALSoft_FIND_REQUIRED)
ENDIF(OpenALSoft_FOUND)

mark_as_advanced( FORCE
    OpenALSoft_INCLUDE_DIR
    OpenALSoft_LIBRARY
)
