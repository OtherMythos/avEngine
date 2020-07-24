#include "SceneNodeUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "MovableObjectUserData.h"

#include "OgreSceneNode.h"
#include "OgreMovableObject.h"
#include "OgreObjectTypes.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"

#include "Scripting/ScriptNamespace/SceneNamespace.h"

namespace AV{

    SQObject SceneNodeUserData::SceneNodeDelegateTableObject;

    SQInteger SceneNodeUserData::setPosition(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));
        Ogre::Vector3 pos;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &pos));

        outNode->setPosition(pos);

        return 0;
    }

    SQInteger SceneNodeUserData::setScale(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));
        Ogre::Vector3 scale;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &scale));

        outNode->setScale(scale);

        return 0;
    }

    SQInteger SceneNodeUserData::createChildSceneNode(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

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

    SQInteger SceneNodeUserData::attachObject(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        Ogre::MovableObject* outObject = 0;
        SCRIPT_CHECK_RESULT(MovableObjectUserData::readMovableObjectFromUserData(vm, 2, &outObject));

        outNode->attachObject(outObject);

        return 0;
    }

    SQInteger SceneNodeUserData::detachObject(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        Ogre::MovableObject* outObject = 0;
        SCRIPT_CHECK_RESULT(MovableObjectUserData::readMovableObjectFromUserData(vm, 2, &outObject));

        outNode->detachObject(outObject);

        return 0;
    }

    SQInteger SceneNodeUserData::getNumChildren(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        sq_pushinteger(vm, outNode->numChildren());

        return 1;
    }

    SQInteger SceneNodeUserData::getNumAttachedObjects(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        sq_pushinteger(vm, outNode->numAttachedObjects());

        return 1;
    }

    SQInteger SceneNodeUserData::getChildByIndex(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        SQInteger idx = 0;
        sq_getinteger(vm, 2, &idx);

        Ogre::SceneNode* childNode = (Ogre::SceneNode*)outNode->getChild(idx);
        sceneNodeToUserData(vm, childNode);

        return 1;
    }

    SQInteger SceneNodeUserData::getAttachedObjectByIndex(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        SQInteger idx = 0;
        sq_getinteger(vm, 2, &idx);

        Ogre::MovableObject* childMovableObject = (Ogre::MovableObject*)outNode->getAttachedObject(idx);
        //Right now I use the listener pointer to reference the movable object type. There doesn't seem to be any other way to do that.
        MovableObjectType targetType = SceneNamespace::determineTypeFromMovableObject(childMovableObject);
        MovableObjectUserData::movableObjectToUserData(vm, childMovableObject, targetType);

        return 1;
    }

    SQInteger SceneNodeUserData::setVisible(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        SQBool visible, cascade;
        visible = cascade = true;
        sq_getbool(vm, 2, &visible);

        SQInteger top = sq_gettop(vm);
        if(top == 3){
            sq_getbool(vm, 3, &cascade);
        }

        outNode->setVisible(visible, cascade);

        return 0;
    }

    SQInteger SceneNodeUserData::setOrientation(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        Ogre::Quaternion outQuat;
        SCRIPT_CHECK_RESULT(QuaternionUserData::readQuaternionFromUserData(vm, 2, &outQuat));

        outNode->setOrientation(outQuat);

        return 0;
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
        ScriptUtils::addFunction(vm, setOrientation, "setOrientation", 2, ".u");

        ScriptUtils::addFunction(vm, createChildSceneNode, "createChildSceneNode", -1, ".i");
        ScriptUtils::addFunction(vm, attachObject, "attachObject", 2, ".u");
        ScriptUtils::addFunction(vm, detachObject, "detachObject", 2, ".u");

        ScriptUtils::addFunction(vm, getNumChildren, "getNumChildren");
        ScriptUtils::addFunction(vm, getNumAttachedObjects, "getNumAttachedObjects");
        ScriptUtils::addFunction(vm, getChildByIndex, "getChild", 2, ".i");
        ScriptUtils::addFunction(vm, getAttachedObjectByIndex, "getAttachedObject", 2, ".i");

        ScriptUtils::addFunction(vm, setVisible, "setVisible", -2, ".bb");

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
