
FIND_PATH(OGRE_MAIN_INCLUDE Ogre.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES OgreMain/include)

FIND_PATH(OGRE_BUILD_INCLUDE OgreBuildSettings.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES build/${CMAKE_BUILD_TYPE}/include)

FIND_PATH(OGRE_RENDER_OPENGL_INCLUDE OgreGL3PlusSupport.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES RenderSystems/GL3Plus/include)

FIND_PATH(OGRE_HLMS_PBS_INCLUDE OgreHlmsPbs.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES Components/Hlms/Pbs/include)

FIND_PATH(OGRE_HLMS_UNLIT_INCLUDE OgreHlmsUnlit.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES Components/Hlms/Unlit/include)

FIND_PATH(OGRE_HLMS_COMMON_INCLUDE OgreHlmsBufferManager.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES Components/Hlms/Common/include)



FIND_LIBRARY(LIB_HLMS_PBS NAMES OgreHlmsPbs_d OgreHlmsPbs
    PATHS ${Ogre_ROOT}/build/${CMAKE_BUILD_TYPE}/lib
    PATH_SUFFIXES a lib
    )

FIND_LIBRARY(LIB_HLMS_UNLIT NAMES OgreHlmsUnlit_d OgreHlmsUnlit
    PATHS ${Ogre_ROOT}/build/${CMAKE_BUILD_TYPE}/lib
    PATH_SUFFIXES a lib
    )

FIND_LIBRARY(LIB_OGRE_MAIN NAMES OgreMain_d OgreMain
    PATHS ${Ogre_ROOT}/build/${CMAKE_BUILD_TYPE}/lib
    PATH_SUFFIXES a lib
    )

#TODO will want to sort out the .so
FIND_LIBRARY(LIB_RENDERSYSTEM_OPENGL NAMES RenderSystem_GL3Plus_d.so RenderSystem_GL3Plus.so
    PATHS ${Ogre_ROOT}/build/${CMAKE_BUILD_TYPE}/lib
    ENV Ogre_ROOT
    )

set(Ogre_LIBRARY "${Ogre_LIBRARY};${LIB_OGRE_MAIN};${LIB_HLMS_PBS};${LIB_HLMS_UNLIT}"
    CACHE STRING "" FORCE)
set(Ogre_INCLUDE_DIR "${Ogre_INCLUDE_DIR};${OGRE_MAIN_INCLUDE};${OGRE_BUILD_INCLUDE};${OGRE_RENDER_OPENGL_INCLUDE};${OGRE_RENDER_OPENGL_INCLUDE}/GLSL;${OGRE_HLMS_PBS_INCLUDE};${OGRE_HLMS_UNLIT_INCLUDE};${OGRE_HLMS_COMMON_INCLUDE}"
    CACHE STRING "" FORCE)

SET(Ogre_FOUND FALSE)
IF(Ogre_INCLUDE_DIR AND Ogre_LIBRARY)
   SET(Ogre_FOUND TRUE)
ENDIF(Ogre_INCLUDE_DIR AND Ogre_LIBRARY)

IF(Ogre_FOUND)
   IF (NOT Ogre_FIND_QUIETLY)
	   MESSAGE(STATUS "Found Ogre: ${Ogre_LIBRARY}")
       MESSAGE(STATUS "Found Ogre includes: ${Ogre_INCLUDE_DIR}")
   ENDIF (NOT Ogre_FIND_QUIETLY)
ELSE(Ogre_FOUND)
   IF (Ogre_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find Ogre")
   ENDIF (Ogre_FIND_REQUIRED)
ENDIF(Ogre_FOUND)

mark_as_advanced( FORCE
    Ogre_INCLUDE_DIR
    Ogre_LIBRARY
)
