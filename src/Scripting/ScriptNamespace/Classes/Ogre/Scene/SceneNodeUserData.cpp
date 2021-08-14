#include "SceneNodeUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "MovableObjectUserData.h"

#include "OgreSceneNode.h"
#include "OgreMovableObject.h"
#include "OgreObjectTypes.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"

#include "Animation/OgreTagPoint.h"

#include "System/Util/OgreNodeHelper.h"

#include "Scripting/ScriptNamespace/SceneNamespace.h"
#include "System/EngineFlags.h"

namespace AV{

    SQObject SceneNodeUserData::SceneNodeDelegateTableObject;
    SQObject SceneNodeUserData::TagPointDelegateTableObject;

    Ogre::Node::TransformSpace _getTransformFromInt(SQInteger i){
        switch(i){
            default:
            case 0: return Ogre::Node::TS_LOCAL;
            case 1: return Ogre::Node::TS_PARENT;
            case 2: return Ogre::Node::TS_WORLD;
        }
    }

    SQInteger SceneNodeUserData::setPosition(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));
        Ogre::Vector3 pos;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &pos));

        outNode->setPosition(pos);

        return 0;
    }

    SQInteger SceneNodeUserData::setScale(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));
        Ogre::Vector3 scale;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &scale));

        outNode->setScale(scale);

        return 0;
    }

    SQInteger SceneNodeUserData::createChildSceneNode(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
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
        CHECK_SCENE_CLEAN()
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        Ogre::MovableObject* outObject = 0;
        SCRIPT_CHECK_RESULT(MovableObjectUserData::readMovableObjectFromUserData(vm, 2, &outObject));

        outNode->attachObject(outObject);

        return 0;
    }

    SQInteger SceneNodeUserData::detachObject(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
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
        CHECK_SCENE_CLEAN()
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
        CHECK_SCENE_CLEAN()
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        Ogre::Quaternion outQuat;
        SCRIPT_CHECK_RESULT(QuaternionUserData::readQuaternionFromUserData(vm, 2, &outQuat));

        outNode->setOrientation(outQuat);

        return 0;
    }

    SQInteger SceneNodeUserData::translateNode(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        Ogre::Vector3 outVec;
        SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 2, &outVec));

        SQInteger outI;
        sq_getinteger(vm, 3, &outI);
        Ogre::Node::TransformSpace ts = _getTransformFromInt(outI);

        outNode->translate(outVec, ts);

        return 0;
    }

    enum class YawRollPitch{
        Yaw,
        Roll,
        Pitch
    };
    SQInteger _nodeYawRollPitch(HSQUIRRELVM vm, YawRollPitch target){
        CHECK_SCENE_CLEAN()
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        SQFloat outF;
        sq_getfloat(vm, 2, &outF);

        SQInteger outI;
        sq_getinteger(vm, 3, &outI);
        Ogre::Node::TransformSpace ts = _getTransformFromInt(outI);

        switch(target){
            case YawRollPitch::Yaw: outNode->yaw(Ogre::Degree(outF), ts); break;
            case YawRollPitch::Roll: outNode->roll(Ogre::Degree(outF), ts); break;
            case YawRollPitch::Pitch: outNode->pitch(Ogre::Degree(outF), ts); break;
            default: assert(false); break;
        }

        return 0;
    }

    SQInteger SceneNodeUserData::nodeYaw(HSQUIRRELVM vm){
        return _nodeYawRollPitch(vm, YawRollPitch::Yaw);
    }

    SQInteger SceneNodeUserData::nodeRoll(HSQUIRRELVM vm){
        return _nodeYawRollPitch(vm, YawRollPitch::Roll);
    }

    SQInteger SceneNodeUserData::nodePitch(HSQUIRRELVM vm){
        return _nodeYawRollPitch(vm, YawRollPitch::Pitch);
    }

    SQInteger SceneNodeUserData::getPosition(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        SlotPositionClass::createNewInstance(vm, SlotPosition(outNode->getPosition()));

        return 1;
    }

    SQInteger SceneNodeUserData::getPositionAsVec3(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        Vector3UserData::vector3ToUserData(vm, outNode->getPosition());

        return 1;
    }

    SQInteger SceneNodeUserData::getScale(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        Vector3UserData::vector3ToUserData(vm, outNode->getScale());

        return 1;
    }

    SQInteger SceneNodeUserData::getOrientation(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        QuaternionUserData::quaternionToUserData(vm, outNode->getOrientation());

        return 1;
    }

    SQInteger SceneNodeUserData::destroyNodeAndChildren(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        OgreNodeHelper::destroyNodeAndChildren(outNode);

        return 0;
    }


    //TagPoint
    SQInteger SceneNodeUserData::createChildTagPoint(HSQUIRRELVM vm){
        Ogre::TagPoint* outTagPoint;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readTagPointFromUserData(vm, 1, &outTagPoint));

        Ogre::TagPoint* newTagPoint = outTagPoint->createChildTagPoint();

        SceneNodeUserData::tagPointToUserData(vm, newTagPoint);

        return 1;
    }

    void SceneNodeUserData::tagPointToUserData(HSQUIRRELVM vm, Ogre::TagPoint* node){
        Ogre::TagPoint** pointer = (Ogre::TagPoint**)sq_newuserdata(vm, sizeof(Ogre::TagPoint*));
        *pointer = node;

        sq_pushobject(vm, TagPointDelegateTableObject);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, TagPointTypeTag);
    }

    UserDataGetResult SceneNodeUserData::readTagPointFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::TagPoint** outNode){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != TagPointTypeTag){
            *outNode = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        Ogre::TagPoint** p = (Ogre::TagPoint**)pointer;
        *outNode = *p;

        return USER_DATA_GET_SUCCESS;
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

        #define BASIC_NODE_FUNCTIONS \
            ScriptUtils::addFunction(vm, setPosition, "setPosition", -2, ".n|unn"); \
            ScriptUtils::addFunction(vm, setScale, "setScale", -2, ".n|unn"); \
            ScriptUtils::addFunction(vm, setOrientation, "setOrientation", 2, ".u"); \
            \
            ScriptUtils::addFunction(vm, getPosition, "getPosition"); \
            ScriptUtils::addFunction(vm, getPositionAsVec3, "getPositionVec3"); \
            ScriptUtils::addFunction(vm, getScale, "getScale"); \
            ScriptUtils::addFunction(vm, getOrientation, "getOrientation"); \
            \
            ScriptUtils::addFunction(vm, createChildSceneNode, "createChildSceneNode", -1, ".i"); \
            ScriptUtils::addFunction(vm, attachObject, "attachObject", 2, ".u"); \
            ScriptUtils::addFunction(vm, detachObject, "detachObject", 2, ".u"); \
            \
            ScriptUtils::addFunction(vm, getNumChildren, "getNumChildren"); \
            ScriptUtils::addFunction(vm, getNumAttachedObjects, "getNumAttachedObjects"); \
            ScriptUtils::addFunction(vm, getChildByIndex, "getChild", 2, ".i"); \
            ScriptUtils::addFunction(vm, getAttachedObjectByIndex, "getAttachedObject", 2, ".i"); \
            \
            ScriptUtils::addFunction(vm, destroyNodeAndChildren, "destroyNodeAndChildren"); \
            \
            ScriptUtils::addFunction(vm, setVisible, "setVisible", -2, ".bb"); \
            ScriptUtils::addFunction(vm, translateNode, "translate", 3, ".ui"); \
            \
            ScriptUtils::addFunction(vm, nodeYaw, "yaw", 3, ".ni"); \
            ScriptUtils::addFunction(vm, nodeRoll, "roll", 3, ".ni"); \
            ScriptUtils::addFunction(vm, nodePitch, "pitch", 3, ".ni");

        { //Basic scene node
            sq_newtable(vm);

            BASIC_NODE_FUNCTIONS

            sq_resetobject(&SceneNodeDelegateTableObject);
            sq_getstackobj(vm, -1, &SceneNodeDelegateTableObject);
            sq_addref(vm, &SceneNodeDelegateTableObject);
            sq_pop(vm, 1);
        }

        { //Tag point node
            sq_newtable(vm);

            BASIC_NODE_FUNCTIONS

            ScriptUtils::addFunction(vm, nodePitch, "pitch", 3, ".ni");

            sq_resetobject(&TagPointDelegateTableObject);
            sq_getstackobj(vm, -1, &TagPointDelegateTableObject);
            sq_addref(vm, &TagPointDelegateTableObject);
            sq_pop(vm, 1);
        }
    }

    void SceneNodeUserData::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_SCENE_DYNAMIC", (SQInteger)Ogre::SCENE_DYNAMIC);
        ScriptUtils::declareConstant(vm, "_SCENE_STATIC", (SQInteger)Ogre::SCENE_STATIC);

        ScriptUtils::declareConstant(vm, "_NODE_TRANSFORM_LOCAL", (SQInteger)Ogre::Node::TS_LOCAL);
        ScriptUtils::declareConstant(vm, "_NODE_TRANSFORM_PARENT", (SQInteger)Ogre::Node::TS_PARENT);
        ScriptUtils::declareConstant(vm, "_NODE_TRANSFORM_WORLD", (SQInteger)Ogre::Node::TS_WORLD);
    }
}
