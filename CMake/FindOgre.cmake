set(Ogre_LIBRARY "" CACHE STRING "" FORCE)
set(Ogre_INCLUDE_DIR "" CACHE STRING "" FORCE)

FIND_PATH(OGRE_MAIN_INCLUDE Ogre.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES include/OGRE)

if(ANDROID)
    FIND_PATH(OGRE_RENDER_OPENGL_INCLUDE OgreGL3PlusSupport.h
        PATHS ${Ogre_ROOT}
            ENV Ogre_ROOT
        PATH_SUFFIXES include/OGRE/RenderSystems/GL3Plus/)
endif()

FIND_PATH(OGRE_HLMS_PBS_INCLUDE OgreHlmsPbs.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES include/OGRE/Hlms/Pbs/)

FIND_PATH(OGRE_HLMS_UNLIT_INCLUDE OgreHlmsUnlit.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES include/OGRE/Hlms/Unlit/)

FIND_PATH(OGRE_HLMS_COMMON_INCLUDE OgreHlmsBufferManager.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES include/OGRE/Hlms/Common)

FIND_PATH(OGRE_PLUGIN_PARTICLE_INCLUDE OgreParticleFXPlugin.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES include/OGRE/Plugins/ParticleFX/)

FIND_PATH(OGRE_PLUGIN_VULKAN_INCLUDE OgreVulkanPlugin.h
    PATHS ${Ogre_ROOT}
        ENV Ogre_ROOT
    PATH_SUFFIXES include/OGRE/RenderSystems/Vulkan)

if(APPLE)
    FIND_PATH(OGRE_RENDER_METAL_INCLUDE OgreMetalPlugin.h
        PATHS ${Ogre_ROOT}
            ENV Ogre_ROOT
        PATH_SUFFIXES include/OGRE/RenderSystems/Metal/)
endif()


FIND_LIBRARY(LIB_HLMS_PBS NAMES OgreHlmsPbs_d OgreHlmsPbs OgreHlmsPbsStatic OgreHlmsPbsStatic_d
    PATHS ${Ogre_ROOT}/lib ${Ogre_ROOT}/lib/${CMAKE_BUILD_TYPE}
    PATH_SUFFIXES a lib
    )

FIND_LIBRARY(LIB_HLMS_UNLIT NAMES OgreHlmsUnlit_d OgreHlmsUnlit OgreHlmsUnlitStatic OgreHlmsUnlitStatic_d
    PATHS ${Ogre_ROOT}/lib ${Ogre_ROOT}/lib/${CMAKE_BUILD_TYPE}
    PATH_SUFFIXES a lib
    )

FIND_LIBRARY(LIB_OGRE_MAIN NAMES OgreMain_d OgreMain Ogre OgreMainStatic OgreMainStatic_d
    PATHS ${Ogre_ROOT}/lib ${Ogre_ROOT}/lib/${CMAKE_BUILD_TYPE}
    PATH_SUFFIXES a lib
    )

FIND_LIBRARY(LIB_PARTICLE_FX NAMES Plugin_ParticleFX_d.so Plugin_ParticleFX.so Plugin_ParticleFX Plugin_ParticleFXStatic Plugin_ParticleFXStatic_d
    PATHS ${Ogre_ROOT}/lib/OGRE ${Ogre_ROOT}/lib/${CMAKE_BUILD_TYPE}/opt ${Ogre_ROOT}/lib/${CMAKE_BUILD_TYPE}
    ENV Ogre_ROOT
    )

if("${PLATFORM}" STREQUAL "android")
    FIND_LIBRARY(LIB_DEP_FREEIMAGE NAMES FreeImage FreeImage_d
        PATHS ${Ogre_ROOT}/androidDependencies/
        PATH_SUFFIXES a
        )
    FIND_LIBRARY(LIB_DEP_ZZIP NAMES zziplib zziplib_d
        PATHS ${Ogre_ROOT}/androidDependencies/
        PATH_SUFFIXES a
        )
    FIND_LIBRARY(LIB_DEP_SHADERC NAMES shaderc_combined
        PATHS ${Ogre_ROOT}/androidDependencies/
        PATH_SUFFIXES a
        NO_DEFAULT_PATH
        )
endif()



IF(WIN32)
    FIND_FILE(OGRE_HLMS_PBS_DLL OgreHlmsPbs.dll OgreHlmsPbs_d.dll
        HINTS ${Ogre_ROOT}/bin/${CMAKE_BUILD_TYPE}
        NO_DEFAULT_PATH
        )
    FIND_FILE(OGRE_HLMS_UNLIT_DLL OgreHlmsUnlit.dll OgreHlmsUnlit_d.dll
        HINTS ${Ogre_ROOT}/bin/${CMAKE_BUILD_TYPE}
        NO_DEFAULT_PATH
        )
    FIND_FILE(OGRE_MAIN_DLL OgreMain.dll OgreMain_d.dll
        HINTS ${Ogre_ROOT}/bin/${CMAKE_BUILD_TYPE}
        NO_DEFAULT_PATH
        )
    FIND_FILE(OGRE_RENDER_GL_DLL RenderSystem_GL3Plus.dll RenderSystem_GL3Plus_d.dll
        HINTS ${Ogre_ROOT}/bin/${CMAKE_BUILD_TYPE}
        NO_DEFAULT_PATH
        )
    FIND_FILE(OGRE_RENDER_D3D11_DLL RenderSystem_Direct3D11.dll RenderSystem_Direct3D11_d.dll
        HINTS ${Ogre_ROOT}/bin/${CMAKE_BUILD_TYPE}
        NO_DEFAULT_PATH
        )
    FIND_FILE(OGRE_RENDER_VULKAN_DLL RenderSystem_Vulkan.dll RenderSystem_Vulkan_d.dll
        HINTS ${Ogre_ROOT}/bin/${CMAKE_BUILD_TYPE}
        NO_DEFAULT_PATH
        )
    FIND_FILE(OGRE_PLUGIN_PARTICLE_DLL Plugin_ParticleFX.dll Plugin_ParticleFX_d.dll
        HINTS ${Ogre_ROOT}/bin/${CMAKE_BUILD_TYPE}
        NO_DEFAULT_PATH
        )
    FIND_FILE(OGRE_AMD_ARGS amd_ags_x64.dll
        HINTS ${Ogre_ROOT}/bin/${CMAKE_BUILD_TYPE}
        NO_DEFAULT_PATH
        )
endif()

if(APPLE)
    FIND_LIBRARY(LIB_RENDERSYSTEM_METAL NAMES RenderSystem_Metal RenderSystem_MetalStatic
        PATHS ${Ogre_ROOT}/lib ${Ogre_ROOT}/lib/${CMAKE_BUILD_TYPE}
        PATH_SUFFIXES a lib
        )

        if("${PLATFORM}" STREQUAL "OS64")
            FIND_LIBRARY(LIB_DEP_FREEIMAGE NAMES FreeImage
                PATHS ${Ogre_ROOT}/iOSDependencies/lib/${CMAKE_BUILD_TYPE}
                PATH_SUFFIXES a
                )
            FIND_LIBRARY(LIB_DEP_ZZIP NAMES zziplib
                PATHS ${Ogre_ROOT}/iOSDependencies/lib/${CMAKE_BUILD_TYPE}
                PATH_SUFFIXES a
                )
        endif()
endif()


#TODO will want to sort out the .so
FIND_LIBRARY(LIB_RENDERSYSTEM_OPENGL NAMES RenderSystem_GL3Plus_d.so RenderSystem_GL3Plus.so
    PATHS ${Ogre_ROOT}/lib/OGRE ${Ogre_ROOT}/lib/${CMAKE_BUILD_TYPE}/opt
    ENV Ogre_ROOT
    )
FIND_LIBRARY(LIB_RENDERSYSTEM_VULKAN NAMES RenderSystem_Vulkan_d.so RenderSystem_Vulkan.so RenderSystem_VulkanStatic_d
    PATHS ${Ogre_ROOT}/lib/OGRE ${Ogre_ROOT}/lib/OGRE ${Ogre_ROOT}/lib/${CMAKE_BUILD_TYPE}/opt
    ENV Ogre_ROOT
    )

#message(FATAL_ERROR ${LIB_PARTICLE_FX})
#message(FATAL_ERROR ${LIB_RENDERSYSTEM_VULKAN})

set(Ogre_LIBRARY "${Ogre_LIBRARY};${LIB_OGRE_MAIN};${LIB_HLMS_PBS};${LIB_HLMS_UNLIT}"
    CACHE STRING "" FORCE)
set(Ogre_INCLUDE_DIR "${Ogre_INCLUDE_DIR};${OGRE_MAIN_INCLUDE};${OGRE_HLMS_PBS_INCLUDE};${OGRE_HLMS_UNLIT_INCLUDE};${OGRE_HLMS_COMMON_INCLUDE};${OGRE_PLUGIN_PARTICLE_INCLUDE};"
    CACHE STRING "" FORCE)

if(NOT APPLE)
    set(Ogre_INCLUDE_DIR "${Ogre_INCLUDE_DIR};${OGRE_PLUGIN_VULKAN_INCLUDE}" CACHE STRING "" FORCE)
endif()

if(APPLE)
    set(Ogre_INCLUDE_DIR "${Ogre_INCLUDE_DIR};${OGRE_RENDER_METAL_INCLUDE}"
        CACHE STRING "" FORCE)
    set(Ogre_LIBRARY "${Ogre_LIBRARY};${LIB_RENDERSYSTEM_METAL};${LIB_PARTICLE_FX}"
        CACHE STRING "" FORCE)
    if("${PLATFORM}" STREQUAL "OS64")
        set(Ogre_LIBRARY "${Ogre_LIBRARY};${LIB_DEP_FREEIMAGE};${LIB_DEP_ZZIP}"
            CACHE STRING "" FORCE)
    endif()
elseif("${PLATFORM}" STREQUAL "android")
    set(Ogre_LIBRARY "${Ogre_LIBRARY};${LIB_DEP_FREEIMAGE};${LIB_DEP_ZZIP};${LIB_DEP_SHADERC};${LIB_RENDERSYSTEM_VULKAN};${LIB_PARTICLE_FX}"
        CACHE STRING "" FORCE)
    set(Ogre_INCLUDE_DIR "${Ogre_INCLUDE_DIR};"
        CACHE STRING "" FORCE)
else()
    set(Ogre_INCLUDE_DIR "${Ogre_INCLUDE_DIR};${OGRE_RENDER_OPENGL_INCLUDE};${OGRE_RENDER_OPENGL_INCLUDE}/GLSL;"
        CACHE STRING "" FORCE)
endif()

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
