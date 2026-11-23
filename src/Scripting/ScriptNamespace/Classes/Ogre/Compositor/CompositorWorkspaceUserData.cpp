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
        //Not asserted to still exist: a removed workspace is reported by its id going invalid.
        *outObject = p;

        return USER_DATA_GET_SUCCESS;
    }

    void CompositorWorkspaceUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, setEnabled, "setEnabled", 2, ".b");
        ScriptUtils::addFunction(vm, isWorkspaceValid, "isValid");
        /**SQFunction
        @name setExecutionMask
        @desc Set which passes of this workspace run. A pass runs when its execution_mask, set in the
        compositor script and 0xFF if left out, shares a bit with this mask. A new workspace starts at 0xFF,
        so every pass runs. Switching passes this way costs nothing, unlike rebuilding the workspace.
        @param1:Integer: The mask, between 0 and 255.
        */
        ScriptUtils::addFunction(vm, setExecutionMask, "setExecutionMask", 2, ".i");
        /**SQFunction
        @name getExecutionMask
        @desc Get the mask set with setExecutionMask.
        @returns The mask, between 0 and 255.
        */
        ScriptUtils::addFunction(vm, getExecutionMask, "getExecutionMask");

        sq_resetobject(&compositorWorkspaceDelegateTableObject);
        sq_getstackobj(vm, -1, &compositorWorkspaceDelegateTableObject);
        sq_addref(vm, &compositorWorkspaceDelegateTableObject);
        sq_pop(vm, 1);

    }

    SQInteger CompositorWorkspaceUserData::setEnabled(HSQUIRRELVM vm){
        CompositorWorkspaceUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readWorkspacePtrFromUserData(vm, 1, &content));
        if(!_data.isIdValid(content->workspaceId)) return sq_throwerror(vm, "Workspace is invalid.");

        SQBool isEnabled;
        sq_getbool(vm, 2, &isEnabled);

        content->ptr->setEnabled(isEnabled);

        return 0;
    }

    SQInteger CompositorWorkspaceUserData::setExecutionMask(HSQUIRRELVM vm){
        CompositorWorkspaceUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readWorkspacePtrFromUserData(vm, 1, &content));
        if(!_data.isIdValid(content->workspaceId)) return sq_throwerror(vm, "Workspace is invalid.");

        SQInteger mask;
        sq_getinteger(vm, 2, &mask);
        if(mask < 0 || mask > 0xFF) return sq_throwerror(vm, "Execution mask must be between 0 and 255.");

        content->ptr->setExecutionMask(static_cast<Ogre::uint8>(mask));

        return 0;
    }

    SQInteger CompositorWorkspaceUserData::getExecutionMask(HSQUIRRELVM vm){
        CompositorWorkspaceUserDataContents* content;
        SCRIPT_ASSERT_RESULT(_readWorkspacePtrFromUserData(vm, 1, &content));
        if(!_data.isIdValid(content->workspaceId)) return sq_throwerror(vm, "Workspace is invalid.");

        sq_pushinteger(vm, content->ptr->getExecutionMask());

        return 1;
    }

    SQInteger CompositorWorkspaceUserData::isWorkspaceValid(HSQUIRRELVM vm){
        CompositorWorkspaceUserDataContents* content;
        SCRIPT_ASSERT_RESULT(CompositorWorkspaceUserData::_readWorkspacePtrFromUserData(vm, 1, &content));
        bool result = _data.isIdValid(content->workspaceId);

        sq_pushbool(vm, result);

        return 1;
    }

    void CompositorWorkspaceUserData::notifyWorkspaceRemoved(Ogre::CompositorWorkspace* workspace){
        _data.invalidateEntry(workspace);
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
