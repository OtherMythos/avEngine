#include "SceneNodeComponentNamespace.h"

#include "World/Entity/Logic/SceneNodeComponentLogic.h"
#include "Scripting/ScriptNamespace/Classes/EntityClass/EntityClass.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/SceneNodeUserData.h"

namespace AV{

    SQInteger SceneNodeComponentNamespace::add(HSQUIRRELVM vm){
        Ogre::SceneNode* target;
        SCRIPT_CHECK_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, -1, &target));

        eId id;
        SCRIPT_ASSERT_RESULT(EntityClass::getEID(vm, -2, &id));

        SceneNodeComponentLogic::add(id, target);

        return 0;
    }

    SQInteger SceneNodeComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_ASSERT_RESULT(EntityClass::getEID(vm, -1, &id));

        SceneNodeComponentLogic::remove(id);

        return 0;
    }

    SQInteger SceneNodeComponentNamespace::getNode(HSQUIRRELVM vm){
        eId id;
        SCRIPT_ASSERT_RESULT(EntityClass::getEID(vm, -1, &id));

        Ogre::SceneNode* node = SceneNodeComponentLogic::getSceneNode(id);
        if(!node) return sq_throwerror(vm, "That entity does not have a scene node component.");
        SceneNodeUserData::sceneNodeToUserData(vm, node);

        return 1;
    }

    void SceneNodeComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("sceneNode"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", 3, ".xu");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".x");

        ScriptUtils::addFunction(vm, getNode, "getNode");

        sq_newslot(vm, -3, false);
    }
}
