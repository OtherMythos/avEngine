

FIND_PATH(RLottie_INCLUDE_DIR rlottie.h
    PATHS ${RLottie_ROOT}
        ENV RLottie_ROOT
    PATH_SUFFIXES include)

FIND_LIBRARY(RLottie_LIBRARY NAMES rlottie
    PATHS ${RLottie_ROOT}
        ENV RLottie_ROOT
    PATH_SUFFIXES lib)

SET(RLottie_FOUND FALSE)
IF(RLottie_INCLUDE_DIR AND RLottie_LIBRARY)
    SET(RLottie_FOUND TRUE)
ENDIF(RLottie_INCLUDE_DIR AND RLottie_LIBRARY)

IF(RLottie_FOUND)
   IF (NOT RLottie_FIND_QUIETLY)
	   MESSAGE(STATUS "Found RLottie: ${RLottie_LIBRARY}, ${RLottie_INCLUDE_DIR}")
   ENDIF (NOT RLottie_FIND_QUIETLY)
ELSE(RLottie_FOUND)
   IF (RLottie_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find RLottie")
   ENDIF (RLottie_FIND_REQUIRED)
ENDIF(RLottie_FOUND)

mark_as_advanced( FORCE
    RLottie_INCLUDE_DIR
    RLottie_LIBRARY
)
