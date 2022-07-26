#include "SceneNodeComponentNamespace.h"

#include "World/Entity/Logic/SceneNodeComponentLogic.h"
#include "World/Entity/Logic/FundamentalLogic.h"
#include "Scripting/ScriptNamespace/Classes/Entity/EntityUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/SceneNodeUserData.h"

namespace AV{

    SQInteger SceneNodeComponentNamespace::add(HSQUIRRELVM vm){
        Ogre::SceneNode* target;
        SCRIPT_CHECK_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 3, &target));

        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, 2, &id));

        SQBool destroyNodes = false;
        if(sq_gettop(vm) == 4){
            sq_getbool(vm, 4, &destroyNodes);
        }

        SlotPosition entityPos = FundamentalLogic::getPosition(id);
        SceneNodeComponentLogic::add(id, target, entityPos.toOgre(), destroyNodes);

        return 0;
    }

    SQInteger SceneNodeComponentNamespace::remove(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        SceneNodeComponentLogic::remove(id);

        return 0;
    }

    SQInteger SceneNodeComponentNamespace::getNode(HSQUIRRELVM vm){
        eId id;
        SCRIPT_CHECK_RESULT(EntityUserData::readeIDFromUserData(vm, -1, &id));

        Ogre::SceneNode* node = SceneNodeComponentLogic::getSceneNode(id);
        if(!node) return sq_throwerror(vm, "That entity does not have a scene node component.");
        SceneNodeUserData::sceneNodeToUserData(vm, node);

        return 1;
    }

    void SceneNodeComponentNamespace::setupNamespace(HSQUIRRELVM vm){
        sq_pushstring(vm, _SC("sceneNode"), -1);
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, add, "add", -3, ".uub");
        ScriptUtils::addFunction(vm, remove, "remove", 2, ".u");

        ScriptUtils::addFunction(vm, getNode, "getNode", 2, ".u");

        sq_newslot(vm, -3, false);
    }
}
