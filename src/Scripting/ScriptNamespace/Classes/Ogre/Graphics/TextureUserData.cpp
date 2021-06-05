#include "TextureUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "OgreTextureGpu.h"

#include "TextureGpuManagerListener.h"

#include "OgreRoot.h"
#include "OgreTextureGpuManager.h"
#include "OgreRenderSystem.h"

namespace AV{
    SQObject TextureUserData::textureDelegateTableObject;
    VersionedDataPool<uint32> TextureUserData::textureDataPool;
    std::map<Ogre::TextureGpu*, uint64> TextureUserData::existingTextures;

    AVTextureGpuManagerListener listener;

    void TextureUserData::textureToUserData(HSQUIRRELVM vm, Ogre::TextureGpu* tex, bool userOwned){
        uint64 texId = 0;
        if(existingTextures.find(tex) == existingTextures.end()){
            //Push to the textures map.
            //Count the number of objects which reference this pointer.
            texId = textureDataPool.storeEntry(1);
            existingTextures[tex] = texId;
        }else{
            //Just increment the counter.
            (textureDataPool.getEntry(1))++;
        }
        TextureUserDataContents contents({tex, texId, userOwned});

        TextureUserDataContents* pointer = (TextureUserDataContents*)sq_newuserdata(vm, sizeof(TextureUserDataContents));
        memcpy(pointer, &contents, sizeof(TextureUserDataContents));

        sq_pushobject(vm, textureDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, TextureTypeTag);
    }

    UserDataGetResult TextureUserData::readTextureFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TextureGpu** outObject){
        TextureUserDataContents* objPtr;
        UserDataGetResult result = _readTexturePtrFromUserData(vm, stackInx, &objPtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outObject = objPtr->ptr;
        return result;
    }

    UserDataGetResult TextureUserData::_readTexturePtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, TextureUserDataContents** outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != TextureTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        TextureUserDataContents* p = static_cast<TextureUserDataContents*>(pointer);
        *outObject = p;
        assert( existingTextures.find(p->ptr) != existingTextures.end() );

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger TextureUserData::getWidth(HSQUIRRELVM vm){
        TextureUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readTexturePtrFromUserData(vm, 1, &content));
        sq_pushinteger(vm, content->ptr->getWidth());

        return 1;
    }

    SQInteger TextureUserData::getHeight(HSQUIRRELVM vm){
        TextureUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readTexturePtrFromUserData(vm, 1, &content));
        sq_pushinteger(vm, content->ptr->getHeight());

        return 1;
    }

    SQInteger TextureUserData::setResolution(HSQUIRRELVM vm){
        SQInteger width, height;
        sq_getinteger(vm, 2, &width);
        sq_getinteger(vm, 3, &height);

        if(width < 0 || height < 0) sq_throwerror(vm, "resolution cannot be negative.");

        TextureUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readTexturePtrFromUserData(vm, 1, &content));

        //TODO check if the texture is valid.
        content->ptr->setResolution(width, height);

        return 0;
    }

    SQInteger TextureUserData::schduleTransitionTo(HSQUIRRELVM vm){
        SQInteger transitionType;
        sq_getinteger(vm, 2, &transitionType);

        if(transitionType < 0 || transitionType > 2) sq_throwerror(vm, "Invalid transition type.");

        TextureUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readTexturePtrFromUserData(vm, 1, &content));

        //TODO check if the texture is valid.
        content->ptr->scheduleTransitionTo((Ogre::GpuResidency::GpuResidency)transitionType);

        return 0;
    }

    void TextureUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getWidth, "getWidth");
        ScriptUtils::addFunction(vm, getHeight, "getHeight");
        ScriptUtils::addFunction(vm, setResolution, "setResolution", 3, ".ii");
        ScriptUtils::addFunction(vm, schduleTransitionTo, "scheduleTransitionTo", 2, ".i");

        sq_resetobject(&textureDelegateTableObject);
        sq_getstackobj(vm, -1, &textureDelegateTableObject);
        sq_addref(vm, &textureDelegateTableObject);
        sq_pop(vm, 1);

    }

    void TextureUserData::setupConstants(HSQUIRRELVM vm){
        /**SQConstant
        @name _GPU_RESIDENCY_ON_STORAGE
        @desc Texture is on storage (i.e. sourced from disk, from listener) A 4x4 blank texture will be shown if user attempts to use this Texture.
        No memory is consumed.
        While in this state, many settings may not be trusted (width, height, etc) as nothing is loaded.
        */
        ScriptUtils::declareConstant(vm, "_GPU_RESIDENCY_ON_STORAGE", (SQInteger)Ogre::GpuResidency::OnStorage);
        /**SQConstant
        @name _GPU_RESIDENCY_ON_SYSTEM_RAM
        @desc Texture is on System RAM.
        If the texture is fully not resident, a 4x4 blank texture will be shown if user attempts to use this Texture.
        */
        ScriptUtils::declareConstant(vm, "_GPU_RESIDENCY_ON_SYSTEM_RAM", (SQInteger)Ogre::GpuResidency::OnSystemRam);
        /**SQConstant
        @name _GPU_RESIDENCY_RESIDENT
        @desc VRAM and other GPU resources have been allocated for this resource.
        */
       ScriptUtils::declareConstant(vm, "_GPU_RESIDENCY_RESIDENT", (SQInteger)Ogre::GpuResidency::Resident);
    }

    void TextureUserData::_notifyTextureDeleted(Ogre::TextureGpu* texture){
        //TODO OPTIMISATION Storing the tracked data in the texture somehow would mean I don't have to do this search each time.
        auto it = existingTextures.find(texture);
        //Nothing to do.
        if(it == existingTextures.end()) return;

        uint64 idx = it->second;
        bool removed = textureDataPool.removeEntry(idx);
        //There should be something which gets removed if the texture was in the map.
        assert(removed);
        existingTextures.erase(it);
    }

    void TextureUserData::setupListener(){
        Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager()->setTextureGpuManagerListener(&listener);
    }
}
