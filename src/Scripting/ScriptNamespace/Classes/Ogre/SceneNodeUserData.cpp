#include "SceneNodeUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"

#include "OgreSceneNode.h"

namespace AV{

    SQObject SceneNodeUserData::SceneNodeDelegateTableObject;

    SQInteger SceneNodeUserData::setPosition(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_CHECK_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));
        Ogre::Vector3 pos;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &pos));

        outNode->setPosition(pos);

        return 0;
    }

    SQInteger SceneNodeUserData::setScale(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_CHECK_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));
        Ogre::Vector3 scale;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &scale));

        outNode->setScale(scale);

        return 0;
    }

    SQInteger SceneNodeUserData::createChildSceneNode(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_CHECK_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        SQInteger size = sq_gettop(vm);
        Ogre::SceneMemoryMgrTypes targetType = Ogre::SCENE_DYNAMIC;
        if(size == 2){
            SQInteger sceneNodeType = 0;
            sq_getinteger(vm, 2, &sceneNodeType);
            targetType = static_cast<Ogre::SceneMemoryMgrTypes>(sceneNodeType);
        }

        Ogre::SceneNode* newNode = outNode->createChildSceneNode(targetType);
        SceneNodeUserData::sceneNodeToUserData(vm, newNode);

        return 1;
    }

    void SceneNodeUserData::sceneNodeToUserData(HSQUIRRELVM vm, Ogre::SceneNode* node){
        Ogre::SceneNode** pointer = (Ogre::SceneNode**)sq_newuserdata(vm, sizeof(Ogre::SceneNode*));
        *pointer = node;

        sq_pushobject(vm, SceneNodeDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, SceneNodeTypeTag);
    }

    UserDataGetResult SceneNodeUserData::readSceneNodeFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::SceneNode** outNode){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != SceneNodeTypeTag){
            *outNode = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::SceneNode** p = (Ogre::SceneNode**)pointer;
        *outNode = *p;

        return USER_DATA_GET_SUCCESS;
    }

    void SceneNodeUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtable(vm);

        ScriptUtils::addFunction(vm, setPosition, "setPosition", -2, ".n|unn");
        ScriptUtils::addFunction(vm, setScale, "setScale", -2, ".n|unn");

        ScriptUtils::addFunction(vm, createChildSceneNode, "createChildSceneNode", -1, ".i");

        sq_resetobject(&SceneNodeDelegateTableObject);
        sq_getstackobj(vm, -1, &SceneNodeDelegateTableObject);
        sq_addref(vm, &SceneNodeDelegateTableObject);
        sq_pop(vm, 1);
    }

    void SceneNodeUserData::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_SCENE_DYNAMIC", (SQInteger)Ogre::SCENE_DYNAMIC);
        ScriptUtils::declareConstant(vm, "_SCENE_STATIC", (SQInteger)Ogre::SCENE_STATIC);
    }
}
