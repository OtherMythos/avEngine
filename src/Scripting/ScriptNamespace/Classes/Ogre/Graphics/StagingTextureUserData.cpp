#include "StagingTextureUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "OgreStagingTexture.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/GPUProgramUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/GPUProgramParametersUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureBoxUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"

namespace AV{

    SQObject StagingTextureUserData::StagingTextureDelegateTableObject;

    void StagingTextureUserData::StagingTextureToUserData(HSQUIRRELVM vm, Ogre::StagingTexture* program){
        Ogre::StagingTexture** pointer = (Ogre::StagingTexture**)sq_newuserdata(vm, sizeof(Ogre::StagingTexture*));
        memset(pointer, 0, sizeof(Ogre::StagingTexture*));
        *pointer = program;

        sq_pushobject(vm, StagingTextureDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, StagingTextureTypeTag);
        sq_setreleasehook(vm, -1, StagingTextureObjectReleaseHook);
    }

    UserDataGetResult StagingTextureUserData::readStagingTextureFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::StagingTexture** outProg){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != StagingTextureTypeTag){
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outProg = *((Ogre::StagingTexture**)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger StagingTextureUserData::StagingTextureObjectReleaseHook(SQUserPointer p, SQInteger size){
        Ogre::StagingTexture** ptr = static_cast<Ogre::StagingTexture**>(p);

        return 0;
    }

    SQInteger StagingTextureUserData::startMapRegion(HSQUIRRELVM vm){
        Ogre::StagingTexture* tex;
        SCRIPT_ASSERT_RESULT(readStagingTextureFromUserData(vm, 1, &tex));

        tex->startMapRegion();

        return 0;
    }

    SQInteger StagingTextureUserData::stopMapRegion(HSQUIRRELVM vm){
        Ogre::StagingTexture* tex;
        SCRIPT_ASSERT_RESULT(readStagingTextureFromUserData(vm, 1, &tex));

        tex->stopMapRegion();

        return 0;
    }

    SQInteger StagingTextureUserData::mapRegion(HSQUIRRELVM vm){
        Ogre::StagingTexture* tex;
        SCRIPT_ASSERT_RESULT(readStagingTextureFromUserData(vm, 1, &tex));

        SQInteger width, height, depth, slices;
        SQInteger pixelFormat;

        sq_getinteger(vm, 2, &width);
        sq_getinteger(vm, 3, &height);
        sq_getinteger(vm, 4, &depth);
        sq_getinteger(vm, 5, &slices);
        sq_getinteger(vm, 6, &pixelFormat);

        Ogre::uint32 w = static_cast<Ogre::uint32>(width);
        Ogre::uint32 h = static_cast<Ogre::uint32>(height);
        Ogre::uint32 d = static_cast<Ogre::uint32>(depth);
        Ogre::uint32 s = static_cast<Ogre::uint32>(slices);

        if(pixelFormat <= 0 || pixelFormat >= Ogre::PFG_COUNT){
            return sq_throwerror(vm, "Invalid pixel format provided");
        }
        if(depth <= 0){
            return sq_throwerror(vm, "Depth must be greater than 0.");
        }
        if(slices <= 0){
            return sq_throwerror(vm, "Slices must be greater than 0.");
        }
        Ogre::PixelFormatGpu format = static_cast<Ogre::PixelFormatGpu>(pixelFormat);

        Ogre::TextureBox texBox = tex->mapRegion(w, h, d, s, format);
        TextureBoxUserData::TextureBoxToUserData(vm, &texBox);

        return 1;
    }

    SQInteger StagingTextureUserData::upload(HSQUIRRELVM vm){
        Ogre::StagingTexture* tex;
        SCRIPT_ASSERT_RESULT(readStagingTextureFromUserData(vm, 1, &tex));

        Ogre::TextureBox* texBox;
        SCRIPT_CHECK_RESULT(TextureBoxUserData::readTextureBoxFromUserData(vm, 2, &texBox));

        Ogre::TextureGpu* texGPU;
        bool userOwned;
        SCRIPT_CHECK_RESULT(TextureUserData::readTextureFromUserData(vm, 3, &texGPU, &userOwned));

        SQInteger mipLevel;
        sq_getinteger(vm, 4, &mipLevel);
        Ogre::uint8 outMip = static_cast<Ogre::uint8>(mipLevel);

        WRAP_OGRE_ERROR(
            tex->upload(*texBox, texGPU, outMip);
        )

        return 0;
    }

    void StagingTextureUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, startMapRegion, "startMapRegion");
        ScriptUtils::addFunction(vm, stopMapRegion, "stopMapRegion");
        /**SQFunction
        @name getStagingTexture
        @desc Get a staging texture, which can be used to upload texture data from the CPU to GPU.
        @param1:Integer: width
        @param2:Integer: height
        @param3:Integer: depth
        @param4:Integer: slices
        @param5:PixelFormat: Pixel format
        */
        ScriptUtils::addFunction(vm, mapRegion, "mapRegion", 6, ".iiiii");

        ScriptUtils::addFunction(vm, upload, "upload", 4, ".uui");

        sq_resetobject(&StagingTextureDelegateTableObject);
        sq_getstackobj(vm, -1, &StagingTextureDelegateTableObject);
        sq_addref(vm, &StagingTextureDelegateTableObject);
        sq_pop(vm, 1);
    }
}
