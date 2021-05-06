set(GoogleTest_LIBRARY "" CACHE STRING "" FORCE)
set(GoogleTest_INCLUDE_DIR "" CACHE STRING "" FORCE)

FIND_PATH(GOOGLE_TEST_INCLUDE_DIR gtest
    PATHS ${GoogleTest_ROOT}
        ENV GoogleTest_ROOT
    PATH_SUFFIXES googletest/include)

FIND_PATH(GOOGLE_MOCK_INCLUDE_DIR gmock
    PATHS ${GoogleTest_ROOT}
        ENV GoogleTest_ROOT
    PATH_SUFFIXES googlemock/include)

FIND_LIBRARY(GOOGLE_MOCK_LIBRARY NAMES gmock gmockd
    PATHS ${GoogleTest_ROOT}/build/${CMAKE_BUILD_TYPE}/lib
        ENV GoogleTest_ROOT
    PATH_SUFFIXES lib)

FIND_LIBRARY(GOOGLE_TEST_LIBRARY NAMES gtest gtestd
    PATHS ${GoogleTest_ROOT}/build/${CMAKE_BUILD_TYPE}/lib
        ENV GoogleTest_ROOT
    PATH_SUFFIXES lib)

set(GoogleTest_LIBRARY "${GoogleTest_LIBRARY};${GOOGLE_MOCK_LIBRARY};${GOOGLE_TEST_LIBRARY}"
CACHE STRING "" FORCE)
set(GoogleTest_INCLUDE_DIR "${GoogleTest_INCLUDE_DIR};${GOOGLE_TEST_INCLUDE_DIR};${GOOGLE_MOCK_INCLUDE_DIR}"
CACHE STRING "" FORCE)

SET(GoogleTest_FOUND FALSE)
IF(GoogleTest_INCLUDE_DIR AND GoogleTest_LIBRARY)
   SET(GoogleTest_FOUND TRUE)
ENDIF(GoogleTest_INCLUDE_DIR AND GoogleTest_LIBRARY)

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
