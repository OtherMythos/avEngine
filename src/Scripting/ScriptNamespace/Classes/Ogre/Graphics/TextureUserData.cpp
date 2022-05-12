#include "TextureUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "OgreTextureGpu.h"

#include "TextureGpuManagerListener.h"

#include "OgreRoot.h"
#include "OgreTextureGpuManager.h"
#include "OgreRenderSystem.h"

#define CHECK_TEXTURE_VALID(x) if(!_isTexValid(x)) return sq_throwerror(vm, "Texture is invalid");

namespace AV{
    SQObject TextureUserData::textureDelegateTableObject;
    VersionedPtr<Ogre::TextureGpu*> TextureUserData::_data;

    const char* NON_WRITEABLE_TEXTURE = "This texture cannot be modified.";

    AVTextureGpuManagerListener listener;

    bool TextureUserData::_isTexValid(const TextureUserDataContents* content){
        bool result = _data.isIdValid(content->textureId);
        result &= (content->ptr != 0);
        return result;
    }

    SQInteger TextureUserData::textureReleaseHook(SQUserPointer p, SQInteger size){
        TextureUserDataContents* pointer = (TextureUserDataContents*)p;
        _data.removeEntry(pointer->ptr);

        return 0;
    }

    void TextureUserData::textureToUserData(HSQUIRRELVM vm, Ogre::TextureGpu* tex, bool userOwned){
        uint64 texId = _data.storeEntry(tex);
        TextureUserDataContents contents({tex, texId, userOwned});

        TextureUserDataContents* pointer = (TextureUserDataContents*)sq_newuserdata(vm, sizeof(TextureUserDataContents));
        memcpy(pointer, &contents, sizeof(TextureUserDataContents));

        sq_pushobject(vm, textureDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, TextureTypeTag);
        sq_setreleasehook(vm, -1, textureReleaseHook);
    }

    UserDataGetResult TextureUserData::readTextureFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TextureGpu** outObject, bool* userOwned, bool* isValid){
        TextureUserDataContents* objPtr;
        UserDataGetResult result = _readTexturePtrFromUserData(vm, stackInx, &objPtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outObject = objPtr->ptr;
        *userOwned = objPtr->userOwned;
        *isValid = _data.isIdValid(objPtr->textureId);
        return result;
    }

    UserDataGetResult TextureUserData::readTextureFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TextureGpu** outObject, bool* userOwned){
        TextureUserDataContents* objPtr;
        UserDataGetResult result = _readTexturePtrFromUserData(vm, stackInx, &objPtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *outObject = objPtr->ptr;
        *userOwned = objPtr->userOwned;
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

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger TextureUserData::getName(HSQUIRRELVM vm){
        TextureUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readTexturePtrFromUserData(vm, 1, &content));
        CHECK_TEXTURE_VALID(content);

        Ogre::String name = content->ptr->getNameStr();
        sq_pushstring(vm, name.c_str(), -1);

        return 1;
    }

    SQInteger TextureUserData::getWidth(HSQUIRRELVM vm){
        TextureUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readTexturePtrFromUserData(vm, 1, &content));
        sq_pushinteger(vm, content->ptr->getWidth());
        CHECK_TEXTURE_VALID(content);

        return 1;
    }

    SQInteger TextureUserData::getHeight(HSQUIRRELVM vm){
        TextureUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readTexturePtrFromUserData(vm, 1, &content));
        CHECK_TEXTURE_VALID(content);
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
        CHECK_TEXTURE_VALID(content);
        if(!content->userOwned) return sq_throwerror(vm, NON_WRITEABLE_TEXTURE);

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
        if(!content->userOwned) return sq_throwerror(vm, NON_WRITEABLE_TEXTURE);

        //TODO check if the texture is valid.
        content->ptr->scheduleTransitionTo((Ogre::GpuResidency::GpuResidency)transitionType);

        return 0;
    }

    SQInteger TextureUserData::isTextureValid(HSQUIRRELVM vm){
        TextureUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readTexturePtrFromUserData(vm, 1, &content));
        bool result = _isTexValid(content);

        sq_pushbool(vm, result);

        return 1;
    }

    void TextureUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getWidth, "getWidth");
        ScriptUtils::addFunction(vm, getHeight, "getHeight");
        ScriptUtils::addFunction(vm, getName, "getName");
        ScriptUtils::addFunction(vm, setResolution, "setResolution", 3, ".ii");
        ScriptUtils::addFunction(vm, schduleTransitionTo, "scheduleTransitionTo", 2, ".i");

        ScriptUtils::addFunction(vm, isTextureValid, "isValid");

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
        //_data.removeEntry(texture);

        //The texture itself has been deleted, so any references to it need to be invalidated.
        _data.invalidateEntry(texture);
    }

    void TextureUserData::setupListener(){
        Ogre::Root::getSingletonPtr()->getRenderSystem()->getTextureGpuManager()->setTextureGpuManagerListener(&listener);
    }
}
