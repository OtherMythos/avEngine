#include "LottieSurfaceUserData.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/StagingTextureUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureBoxUserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptObjectTypeTags.h"

#include "OgreStagingTexture.h"
#include "OgreTextureBox.h"

#include "rlottie.h"

namespace AV{
    SQObject LottieSurfaceUserData::LottieSurfaceDelegateTableObject;

    void LottieSurfaceUserData::LottieSurfaceToUserData(HSQUIRRELVM vm, LottieSurfacePtr animation){
        LottieSurfacePtr* pointer = (LottieSurfacePtr*)sq_newuserdata(vm, sizeof(LottieSurfacePtr));
        memset(pointer, 0, sizeof(LottieSurfacePtr));
        *pointer = animation;

        sq_settypetag(vm, -1, LottieSurfaceTypeTag);
        sq_setreleasehook(vm, -1, sqReleaseHook);
        sq_pushobject(vm, LottieSurfaceDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
    }

    UserDataGetResult LottieSurfaceUserData::readLottieSurfaceFromUserData(HSQUIRRELVM vm, SQInteger stackInx, LottieSurfacePtr* outAnim){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != LottieSurfaceTypeTag){
            outAnim->reset();
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outAnim = *((LottieSurfacePtr*)pointer);

        return USER_DATA_GET_SUCCESS;
    }

    SQInteger LottieSurfaceUserData::setDrawRegion(HSQUIRRELVM vm){
        LottieSurfacePtr ptr;
        SCRIPT_ASSERT_RESULT(readLottieSurfaceFromUserData(vm, 1, &ptr));

        return 0;
    }

    SQInteger LottieSurfaceUserData::uploadToTextureBox(HSQUIRRELVM vm){
        LottieSurfacePtr surfacePtr;
        SCRIPT_CHECK_RESULT(LottieSurfaceUserData::readLottieSurfaceFromUserData(vm, 1, &surfacePtr));

        Ogre::TextureBox* texBox;
        //StagingTextureUserData::readStagingTextureFromUserData(vm, 2, &tex);
        TextureBoxUserData::readTextureBoxFromUserData(vm, 2, &texBox);

        //Write the values over.
        AV::uint32* pDest = static_cast<AV::uint32*>(texBox->at(0, 0, 0));
        AV::uint32* surfaceBuffer = surfacePtr->buffer.get();
        for(size_t i = 0; i < texBox->width * texBox->height; i++){
            //Swap the R and A values.
            AV::uint32 val = *surfaceBuffer;
            AV::uint8* idx = reinterpret_cast<AV::uint8*>(&val);
            *(idx+2) = *surfaceBuffer & 0xFF;
            *(idx) = (*surfaceBuffer >> 16) & 0xFF;
            *pDest = val;
            pDest++;
            surfaceBuffer++;
        }

        return 0;
    }

    SQInteger LottieSurfaceUserData::sqReleaseHook(SQUserPointer p, SQInteger size){
        LottieSurfacePtr* ptr = static_cast<LottieSurfacePtr*>(p);
        ptr->reset();

        return 0;
    }

    SQInteger LottieSurfaceUserData::LottieSurfaceToString(HSQUIRRELVM vm){
        LottieSurfacePtr ptr;
        SCRIPT_ASSERT_RESULT(readLottieSurfaceFromUserData(vm, 1, &ptr));

        std::ostringstream stream;
        stream << "LottieSurface (\"" << ptr.get() << "\")";
        sq_pushstring(vm, _SC(stream.str().c_str()), -1);

        return 1;
    }

    void LottieSurfaceUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, setDrawRegion, "setDrawRegion");
        ScriptUtils::addFunction(vm, uploadToTextureBox, "uploadToTextureBox", 2, ".u");
        ScriptUtils::addFunction(vm, LottieSurfaceToString, "_tostring");

        sq_resetobject(&LottieSurfaceDelegateTableObject);
        sq_getstackobj(vm, -1, &LottieSurfaceDelegateTableObject);
        sq_addref(vm, &LottieSurfaceDelegateTableObject);
        sq_pop(vm, 1);
    }
}
