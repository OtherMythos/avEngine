#include "CompositorNamespace.h"

#include "Ogre.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorManager2.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/MovableObjectUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"

#include "Scripting/ScriptNamespace/Classes/Ogre/Compositor/CompositorWorkspaceUserData.h"

namespace AV{

    Ogre::SceneManager* CompositorNamespace::_scene = 0;

    SQInteger _parseWorkspaceInputs(Ogre::CompositorChannelVec& vec, HSQUIRRELVM vm, SQInteger idx){
        bool firstEntry = true;
        sq_pushnull(vm);
        while(SQ_SUCCEEDED(sq_next(vm, idx))){
            SQObjectType objectType = sq_gettype(vm, -1);
            if(objectType != OT_USERDATA){
                sq_pop(vm, 3);
                return sq_throwerror(vm, "Invalid type");
            }
            SQUserPointer typeTag;
            sq_gettypetag(vm, -1, &typeTag);
            if(typeTag != TextureTypeTag){
                sq_pop(vm, 3);
                return sq_throwerror(vm, "Invalid type");
            }

            Ogre::TextureGpu* outTex;
            bool userOwned;
            TextureUserData::readTextureFromUserData(vm, -1, &outTex, &userOwned);
            assert(outTex);
            vec.push_back(outTex);

            firstEntry = false;

            sq_pop(vm, 2);
        }
        sq_pop(vm, 1);

        return 0;
    }

    SQInteger CompositorNamespace::addWorkspace(HSQUIRRELVM vm){
        Ogre::CompositorChannelVec externalChannels;
        SQInteger result = _parseWorkspaceInputs(externalChannels, vm, 2);
        if(result != 0) return result;

        Ogre::MovableObject* outObject = 0;
        SCRIPT_CHECK_RESULT(MovableObjectUserData::readMovableObjectFromUserData(vm, 3, &outObject, MovableObjectType::Camera));
        Ogre::Camera* camera = dynamic_cast<Ogre::Camera*>(outObject);
        if(!camera->isAttached()){
            return sq_throwerror(vm, "Provided camera is not attached to a scene node.");
        }

        const SQChar* workspaceName;
        sq_getstring(vm, 4, &workspaceName);

        SQBool isEnabled = false;
        sq_getbool(vm, 5, &isEnabled);

        Ogre::CompositorManager2 *compositorManager = Ogre::Root::getSingletonPtr()->getCompositorManager2();

        Ogre::CompositorWorkspace* w = 0;
        WRAP_OGRE_ERROR(
             w = compositorManager->addWorkspace( _scene, externalChannels, camera,
                workspaceName, isEnabled);
        )

        CompositorWorkspaceUserData::workspaceToUserData(vm, w);

        return 1;
    }

    SQInteger CompositorNamespace::removeWorkspace(HSQUIRRELVM vm){
        Ogre::CompositorWorkspace* workspace = 0;
        SCRIPT_CHECK_RESULT(CompositorWorkspaceUserData::readWorkspaceFromUserData(vm, 2, &workspace));
        assert(workspace);

        Ogre::CompositorManager2 *compositorManager = Ogre::Root::getSingletonPtr()->getCompositorManager2();
        WRAP_OGRE_ERROR(compositorManager->removeWorkspace(workspace);)

        CompositorWorkspaceUserData::notifyWorkspaceRemoved(workspace);

        return 0;
    }

    /**SQNamespace
    @name _compositor
    @desc Namespace to perform compositor tasks.
    */
    void CompositorNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name addWorkspace
        @desc Add a workspace based on a loaded definition.
        @param1:Array: An array containing the renderable inputs and output to give to the workspace.
        @param2:Camera: The camera to use for the workspace.
        @param3:String: The name of the workspace definition to use.
        @param3:bool: Whether or not this workspace is automatically enabled.
        */
        ScriptUtils::addFunction(vm, addWorkspace, "addWorkspace", 5, ".ausb");
        /**SQFunction
        @name removeWorkspace
        @desc Remove a workspace. This will invalidate it if any references exist.
        @param1:CompositorWorkspace: The workspace to remove.
        */
        ScriptUtils::addFunction(vm, removeWorkspace, "removeWorkspace", 2, ".u");
    }
}
