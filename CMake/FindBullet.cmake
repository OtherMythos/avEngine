
FIND_PATH(Bullet_INCLUDE_DIR Bullet3Common
    PATHS ${Bullet_ROOT}/include/bullet
    ENV Bullet_ROOT)

FIND_LIBRARY(BULLET_LINEARMATH NAMES LinearMath
    PATHS ${Bullet_ROOT}/lib/LinearMath
    PATH_SUFFIXES a
)
FIND_LIBRARY(BULLET_COLLISION NAMES BulletCollision
    PATHS ${Bullet_ROOT}/lib/BulletCollision
    PATH_SUFFIXES a
)
FIND_LIBRARY(BULLET_DYNAMICS NAMES BulletDynamics
    PATHS ${Bullet_ROOT}/lib/BulletDynamics
    PATH_SUFFIXES a
)

set(Bullet_LIBRARY "" CACHE STRING "" FORCE)
if(BULLET_LINEARMATH)
    set(Bullet_LIBRARY "${Bullet_LIBRARY};${BULLET_LINEARMATH};${BULLET_COLLISION};${BULLET_DYNAMICS}" CACHE STRING "" FORCE)
    #A bit suspicious, but to solve some linker error you need to include this twice.
    set(Bullet_LIBRARY "${Bullet_LIBRARY};${Bullet_LIBRARY};" CACHE STRING "" FORCE)
endif()

SET(Bullet_FOUND FALSE)
IF(Bullet_INCLUDE_DIR AND Bullet_LIBRARY)
    SET(Bullet_FOUND TRUE)
ENDIF(Bullet_INCLUDE_DIR AND Bullet_LIBRARY)

IF(Bullet_FOUND)
    IF (NOT Bullet_FIND_QUIETLY)
        MESSAGE(STATUS "Found bullet: ${Bullet_LIBRARY}")
    ENDIF (NOT Bullet_FIND_QUIETLY)
        ELSE(Bullet_FOUND)
    IF (Bullet_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find bullet")
    ENDIF (Bullet_FIND_REQUIRED)
ENDIF(Bullet_FOUND)

mark_as_advanced( FORCE
    Bullet_INCLUDE_DIR
    Bullet_LIBRARY
)
