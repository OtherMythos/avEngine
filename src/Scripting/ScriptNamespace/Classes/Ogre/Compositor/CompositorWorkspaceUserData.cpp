#include "CompositorWorkspaceUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "Compositor/OgreCompositorWorkspace.h"

namespace AV{
    SQObject CompositorWorkspaceUserData::compositorWorkspaceDelegateTableObject;
    VersionedPtr<Ogre::CompositorWorkspace*> CompositorWorkspaceUserData::_data;

    void CompositorWorkspaceUserData::workspaceToUserData(HSQUIRRELVM vm, Ogre::CompositorWorkspace* workspace){
        uint64 texId = _data.storeEntry(workspace);
        CompositorWorkspaceUserDataContents contents({workspace, texId});

        CompositorWorkspaceUserDataContents* pointer = (CompositorWorkspaceUserDataContents*)sq_newuserdata(vm, sizeof(CompositorWorkspaceUserDataContents));
        memcpy(pointer, &contents, sizeof(CompositorWorkspaceUserDataContents));

        sq_pushobject(vm, compositorWorkspaceDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, CompositorWorkspaceTypeTag);
    }

    UserDataGetResult CompositorWorkspaceUserData::readWorkspaceFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::CompositorWorkspace** workspace){
        CompositorWorkspaceUserDataContents* objPtr;
        UserDataGetResult result = _readWorkspacePtrFromUserData(vm, stackInx, &objPtr);
        if(result != USER_DATA_GET_SUCCESS) return result;

        *workspace = objPtr->ptr;
        return result;
    }

    UserDataGetResult CompositorWorkspaceUserData::_readWorkspacePtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, CompositorWorkspaceUserDataContents** outObject){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != CompositorWorkspaceTypeTag){
            *outObject = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        CompositorWorkspaceUserDataContents* p = static_cast<CompositorWorkspaceUserDataContents*>(pointer);
        *outObject = p;
        assert( _data.doesPtrExist(p->ptr) );

        return USER_DATA_GET_SUCCESS;
    }

    void CompositorWorkspaceUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, setEnabled, "setEnabled", 2, ".b");

        sq_resetobject(&compositorWorkspaceDelegateTableObject);
        sq_getstackobj(vm, -1, &compositorWorkspaceDelegateTableObject);
        sq_addref(vm, &compositorWorkspaceDelegateTableObject);
        sq_pop(vm, 1);

    }

    SQInteger CompositorWorkspaceUserData::setEnabled(HSQUIRRELVM vm){
        CompositorWorkspaceUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readWorkspacePtrFromUserData(vm, 1, &content));

        SQBool isEnabled;
        sq_getbool(vm, 2, &isEnabled);

        content->ptr->setEnabled(isEnabled);

        return 0;
    }

    void CompositorWorkspaceUserData::setupConstants(HSQUIRRELVM vm){
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
}
