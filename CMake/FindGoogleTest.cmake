set(GoogleTest_LIBRARY "" CACHE STRING "" FORCE)
set(GoogleTest_INCLUDE_DIR "" CACHE STRING "" FORCE)

#Discard cached results so a previously misidentified system executable cannot survive a
#reconfigure. GoogleTest is a prebuilt avEngine dependency and must only be resolved below its
#explicit root; searching PATH can mistake programs named "gtest" for an include directory.
unset(GOOGLE_TEST_INCLUDE_DIR CACHE)
unset(GOOGLE_MOCK_INCLUDE_DIR CACHE)
unset(GOOGLE_TEST_LIBRARY CACHE)
unset(GOOGLE_MOCK_LIBRARY CACHE)

FIND_PATH(GOOGLE_TEST_INCLUDE_DIR NAMES gtest/gtest.h
    PATHS ${GoogleTest_ROOT}
        ENV GoogleTest_ROOT
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)

FIND_PATH(GOOGLE_MOCK_INCLUDE_DIR NAMES gmock/gmock.h
    PATHS ${GoogleTest_ROOT}
        ENV GoogleTest_ROOT
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)

FIND_LIBRARY(GOOGLE_MOCK_LIBRARY NAMES gmock gmockd
    PATHS ${GoogleTest_ROOT}
        ENV GoogleTest_ROOT
    PATH_SUFFIXES lib lib64
    NO_DEFAULT_PATH)

FIND_LIBRARY(GOOGLE_TEST_LIBRARY NAMES gtest gtestd
    PATHS ${GoogleTest_ROOT}
        ENV GoogleTest_ROOT
    PATH_SUFFIXES lib lib64
    NO_DEFAULT_PATH)

set(GoogleTest_LIBRARY "${GOOGLE_MOCK_LIBRARY};${GOOGLE_TEST_LIBRARY}"
CACHE STRING "" FORCE)
set(GoogleTest_INCLUDE_DIR "${GOOGLE_TEST_INCLUDE_DIR};${GOOGLE_MOCK_INCLUDE_DIR}"
CACHE STRING "" FORCE)

SET(GoogleTest_FOUND FALSE)
IF(GOOGLE_TEST_INCLUDE_DIR AND GOOGLE_MOCK_INCLUDE_DIR AND
   GOOGLE_TEST_LIBRARY AND GOOGLE_MOCK_LIBRARY)
   SET(GoogleTest_FOUND TRUE)
ENDIF()

IF(GoogleTest_FOUND)
   IF (NOT GoogleTest_FIND_QUIETLY)
	   MESSAGE(STATUS "Found GoogleTest")
   ENDIF (NOT GoogleTest_FIND_QUIETLY)
ELSE(GoogleTest_FOUND)
   IF (GoogleTest_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find GoogleTest")
   ENDIF (GoogleTest_FIND_REQUIRED)
ENDIF(GoogleTest_FOUND)

mark_as_advanced( FORCE
    GoogleTest_INCLUDE_DIR
    GoogleTest_LIBRARY
)
