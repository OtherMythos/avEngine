#include "TextureUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "OgreTextureGpu.h"

namespace AV{
    SQObject TextureUserData::textureDelegateTableObject;
    VersionedDataPool<uint32> TextureUserData::textureDataPool;
    std::map<Ogre::TextureGpu*, uint64> TextureUserData::existingTextures;

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

    void TextureUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, getWidth, "getWidth");
        ScriptUtils::addFunction(vm, getHeight, "getHeight");

        sq_resetobject(&textureDelegateTableObject);
        sq_getstackobj(vm, -1, &textureDelegateTableObject);
        sq_addref(vm, &textureDelegateTableObject);
        sq_pop(vm, 1);

    }
}
