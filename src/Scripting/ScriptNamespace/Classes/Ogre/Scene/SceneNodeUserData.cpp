#include "SceneNodeUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/ScriptGetterUtils.h"
#include "MovableObjectUserData.h"

#include "OgreSceneNode.h"
#include "OgreParticleSystem.h"
#include "OgreMovableObject.h"
#include "OgreObjectTypes.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/SlotPositionClass.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Scene/TerrainObjectUserData.h"
#include "World/Slot/Chunk/Terrain/TerrainObject.h"
#include "World/Slot/Chunk/Terrain/terra/Terra.h"

#include "System/Util/Scene/ParticleSystemTimeHelper.h"

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
        SQInteger result = ScriptGetterUtils::vector3Read(vm, &pos);
        if(result != 0) return result;

        outNode->setPosition(pos);

        return 0;
    }

    SQInteger SceneNodeUserData::move(HSQUIRRELVM vm){
        CHECK_SCENE_CLEAN()
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));
        Ogre::Vector3 mov;
        SCRIPT_CHECK_RESULT(ScriptGetterUtils::read3FloatsOrVec3(vm, &mov));

        outNode->setPosition(outNode->getPosition() + mov);

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

        {
            TerrainObject* outObject;
            UserDataGetResult result = TerrainObjectUserData::readTerrainObjectFromUserData(vm, 2, &outObject);
            if(result == UserDataGetResult::USER_DATA_GET_SUCCESS){
                outNode->attachObject(outObject->getTerra());
                return 0;
            }
        }

        Ogre::MovableObject* outObject = 0;
        SCRIPT_CHECK_RESULT(MovableObjectUserData::readMovableObjectFromUserData(vm, 2, &outObject));

        { //Extra setup required so particle systems can be paused.
            SQUserPointer type = 0;
            sq_gettypetag(vm, 2, &type);
            if(type == ParticleSystemTypeTag){
                Ogre::ParticleSystem* ps = dynamic_cast<Ogre::ParticleSystem*>(outObject);
                assert(ps);
                assert(outObject->getMovableType() == "ParticleSystem");
                ParticleSystemTimeHelper::notifyParticleSystemAttachment(ps);
            }
        }

        WRAP_OGRE_ERROR(
            outNode->attachObject(outObject);
        )

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

        size_t numNodes = outNode->numChildren();
        if(idx < 0 || idx >= numNodes) return sq_throwerror(vm, "Invalid child id.");

        Ogre::SceneNode* childNode = (Ogre::SceneNode*)outNode->getChild(idx);
        sceneNodeToUserData(vm, childNode);

        return 1;
    }

    SQInteger SceneNodeUserData::getParent(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &outNode));

        Ogre::SceneNode* parent = dynamic_cast<Ogre::SceneNode*>(outNode->getParent());
        sceneNodeToUserData(vm, parent);

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

    SQInteger SceneNodeUserData::getDerivedPositionVec3(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        Vector3UserData::vector3ToUserData(vm, outNode->_getDerivedPositionUpdated());

        return 1;
    }

    SQInteger SceneNodeUserData::getDerivedScale(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        Vector3UserData::vector3ToUserData(vm, outNode->_getDerivedScaleUpdated());

        return 1;
    }

    SQInteger SceneNodeUserData::getDerivedOrientation(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        QuaternionUserData::quaternionToUserData(vm, outNode->_getDerivedOrientationUpdated());

        return 1;
    }

    SQInteger SceneNodeUserData::setDerivedOrientation(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        Ogre::Quaternion quat;
        QuaternionUserData::readQuaternionFromUserData(vm, 2, &quat);

        outNode->_setDerivedOrientation(quat);

        return 1;
    }

    SQInteger SceneNodeUserData::setDerivedPosition(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        Ogre::Vector3 vec;
        Vector3UserData::readVector3FromUserData(vm, 2, &vec);

        outNode->_setDerivedPosition(vec);

        return 1;
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

    SQInteger SceneNodeUserData::getId(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        sq_pushinteger(vm, static_cast<SQInteger>(outNode->getId()));

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

    SQInteger SceneNodeUserData::recursiveDestroyChildren(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        OgreNodeHelper::recursiveDestroyNode(outNode);

        return 0;
    }

    SQInteger SceneNodeUserData::recursiveDestroyAttachedObjects(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        OgreNodeHelper::recursiveDestroyAttachedObjects(outNode);

        return 0;
    }

    SQInteger SceneNodeUserData::lookAt(HSQUIRRELVM vm){
        Ogre::SceneNode* outNode;
        SCRIPT_ASSERT_RESULT(SceneNodeUserData::readSceneNodeFromUserData(vm, 1, &outNode));

        Ogre::Vector3 outVec;
        SCRIPT_CHECK_RESULT(Vector3UserData::readVector3FromUserData(vm, 2, &outVec));

        //TODO In future I might want to make the user able to change the coordinate space.
        outNode->lookAt(outVec, Ogre::Node::TS_WORLD, Ogre::Vector3::UNIT_Z);

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
            //Tag points are also allowed.
            if(typeTag != TagPointTypeTag){
                *outNode = 0;
                return USER_DATA_GET_TYPE_MISMATCH;
            }
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
            ScriptUtils::addFunction(vm, move, "move", -2, ".n|unn"); \
            \
            ScriptUtils::addFunction(vm, getPosition, "getPosition"); \
            ScriptUtils::addFunction(vm, getPositionAsVec3, "getPositionVec3"); \
            ScriptUtils::addFunction(vm, getScale, "getScale"); \
            ScriptUtils::addFunction(vm, getOrientation, "getOrientation"); \
            \
            ScriptUtils::addFunction(vm, getDerivedPositionVec3, "getDerivedPositionVec3"); \
            ScriptUtils::addFunction(vm, getDerivedScale, "getDerivedScale"); \
            ScriptUtils::addFunction(vm, getDerivedOrientation, "getDerivedOrientation"); \
            ScriptUtils::addFunction(vm, setDerivedPosition, "setDerivedPosition", 2, ".u"); \
            ScriptUtils::addFunction(vm, setDerivedOrientation, "setDerivedOrientation", 2, ".u"); \
            \
            ScriptUtils::addFunction(vm, createChildSceneNode, "createChildSceneNode", -1, ".i"); \
            ScriptUtils::addFunction(vm, attachObject, "attachObject", 2, ".u"); \
            ScriptUtils::addFunction(vm, detachObject, "detachObject", 2, ".u"); \
            \
            ScriptUtils::addFunction(vm, getNumChildren, "getNumChildren"); \
            ScriptUtils::addFunction(vm, getNumAttachedObjects, "getNumAttachedObjects"); \
            ScriptUtils::addFunction(vm, getChildByIndex, "getChild", 2, ".i"); \
            ScriptUtils::addFunction(vm, getAttachedObjectByIndex, "getAttachedObject", 2, ".i"); \
            ScriptUtils::addFunction(vm, getParent, "getParent"); \
            ScriptUtils::addFunction(vm, getId, "getId"); \
            \
            ScriptUtils::addFunction(vm, destroyNodeAndChildren, "destroyNodeAndChildren"); \
            ScriptUtils::addFunction(vm, recursiveDestroyChildren, "recursiveDestroyChildren"); \
            ScriptUtils::addFunction(vm, recursiveDestroyAttachedObjects, "recursiveDestroyAttachedObjects"); \
            \
            ScriptUtils::addFunction(vm, setVisible, "setVisible", -2, ".bb"); \
            ScriptUtils::addFunction(vm, translateNode, "translate", 3, ".ui"); \
            \
            ScriptUtils::addFunction(vm, lookAt, "lookAt", 2, ".u"); \
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

            ScriptUtils::addFunction(vm, createChildTagPoint, "createChildTagPoint");

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
