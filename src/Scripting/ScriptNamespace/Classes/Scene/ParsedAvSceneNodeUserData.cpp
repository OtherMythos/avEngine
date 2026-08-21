#include "ParsedAvSceneNodeUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"
#include "Scripting/ScriptNamespace/Classes/Vector3UserData.h"
#include "Scripting/ScriptNamespace/Classes/QuaternionUserData.h"

#include "World/Slot/Recipe/AvScene/AvSceneFileParser.h"
#include "World/Slot/Recipe/AvScene/AvSceneParsedDataUtils.h"

namespace AV{

    SQObject ParsedAvSceneNodeUserData::sceneNodeDelegateTable;

    namespace{
        //Push the string at an index into the scene's string table, or null if there isn't one.
        void _pushStringOrNull(HSQUIRRELVM vm, const ParsedSceneFile& scene, int stringIdx){
            if(stringIdx < 0 || static_cast<size_t>(stringIdx) >= scene.strings.size()){
                sq_pushnull(vm);
                return;
            }
            sq_pushstring(vm, scene.strings[stringIdx].c_str(), -1);
        }

        bool _isUserType(SceneObjectType t){
            return t >= SceneObjectType::User0 && t <= SceneObjectType::User6;
        }
    }

    void ParsedAvSceneNodeUserData::sceneNodeToUserData(HSQUIRRELVM vm, ParsedSceneFilePtr scene, uint32 index){
        void* pointer = sq_newuserdata(vm, sizeof(ParsedSceneNodeData));
        //The userdata memory is raw, so the shared pointer inside is constructed into it by
        //hand and destroyed again by the release hook.
        new (pointer) ParsedSceneNodeData{scene, index};

        sq_setreleasehook(vm, -1, nodeReleaseHook);
        sq_pushobject(vm, sceneNodeDelegateTable);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, AvSceneNodeObjectTypeTag);
    }

    UserDataGetResult ParsedAvSceneNodeUserData::readSceneNodeFromUserData(HSQUIRRELVM vm, SQInteger stackInx, ParsedSceneNodeData** outData){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != AvSceneNodeObjectTypeTag){
            *outData = 0;
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outData = static_cast<ParsedSceneNodeData*>(pointer);

        return USER_DATA_GET_SUCCESS;
    }

    /**SQFunction
    @name getName
    @desc Determine the name this node was given in the scene file.
    @returns A string, or null if the node has no name.
    */
    SQInteger ParsedAvSceneNodeUserData::getName(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        _pushStringOrNull(vm, *data->scene, data->scene->data[data->index].name);

        return 1;
    }

    /**SQFunction
    @name getTag
    @desc Determine the tag this node was given in the scene file.
    @returns A string, or null if the node has no tag.
    */
    SQInteger ParsedAvSceneNodeUserData::getTag(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        _pushStringOrNull(vm, *data->scene, data->scene->data[data->index].tag);

        return 1;
    }

    /**SQFunction
    @name getType
    @desc Determine what sort of object this node describes.
    @returns One of the _AV_SCENE_NODE_* constants.
    */
    SQInteger ParsedAvSceneNodeUserData::getType(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        sq_pushinteger(vm, static_cast<SQInteger>(data->scene->data[data->index].type));

        return 1;
    }

    /**SQFunction
    @name getMeshName
    @desc Determine the mesh this node references.
    @returns A string, or null if this node is not a mesh.
    */
    SQInteger ParsedAvSceneNodeUserData::getMeshName(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        const SceneObjectData& d = data->scene->data[data->index];
        if(d.type != SceneObjectType::Mesh){
            sq_pushnull(vm);
            return 1;
        }
        _pushStringOrNull(vm, *data->scene, static_cast<int>(d.idx));

        return 1;
    }

    /**SQFunction
    @name getUserValue
    @desc Determine the value attribute of a user node.
    @returns A string, or null if this node is not a user node.
    */
    SQInteger ParsedAvSceneNodeUserData::getUserValue(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        const SceneObjectData& d = data->scene->data[data->index];
        if(!_isUserType(d.type)){
            sq_pushnull(vm);
            return 1;
        }
        _pushStringOrNull(vm, *data->scene, static_cast<int>(d.idx));

        return 1;
    }

    /**SQFunction
    @name getAnimIdx
    @desc Determine the animation index this node was tagged with in the scene file.
    @returns An integer, or null if the node has no animation index.
    */
    SQInteger ParsedAvSceneNodeUserData::getAnimIdx(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        const uint8 animIdx = data->scene->data[data->index].animIdx;
        if(animIdx == AVSceneFileParserInterface::NONE_ANIM_IDX) sq_pushnull(vm);
        else sq_pushinteger(vm, static_cast<SQInteger>(animIdx));

        return 1;
    }

    /**SQFunction
    @name getPosition
    @desc Determine the position of this node relative to its parent.
    @returns A Vec3.
    */
    SQInteger ParsedAvSceneNodeUserData::getPosition(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        Vector3UserData::vector3ToUserData(vm, data->scene->data[data->index].pos);

        return 1;
    }

    /**SQFunction
    @name getScale
    @desc Determine the scale of this node relative to its parent.
    @returns A Vec3.
    */
    SQInteger ParsedAvSceneNodeUserData::getScale(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        Vector3UserData::vector3ToUserData(vm, data->scene->data[data->index].scale);

        return 1;
    }

    /**SQFunction
    @name getOrientation
    @desc Determine the orientation of this node relative to its parent.
    @returns A Quat.
    */
    SQInteger ParsedAvSceneNodeUserData::getOrientation(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        QuaternionUserData::quaternionToUserData(vm, data->scene->data[data->index].orientation);

        return 1;
    }

    /**SQFunction
    @name getDerivedPosition
    @desc Determine the position of this node with the transforms of its parents applied.
    @returns A Vec3.
    */
    SQInteger ParsedAvSceneNodeUserData::getDerivedPosition(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        Ogre::Vector3 pos;
        AvSceneParsedDataUtils::computeDerivedTransform(*data->scene, data->index, &pos, 0, 0);
        Vector3UserData::vector3ToUserData(vm, pos);

        return 1;
    }

    /**SQFunction
    @name getDerivedScale
    @desc Determine the scale of this node with the transforms of its parents applied.
    @returns A Vec3.
    */
    SQInteger ParsedAvSceneNodeUserData::getDerivedScale(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        Ogre::Vector3 scale;
        AvSceneParsedDataUtils::computeDerivedTransform(*data->scene, data->index, 0, &scale, 0);
        Vector3UserData::vector3ToUserData(vm, scale);

        return 1;
    }

    /**SQFunction
    @name getDerivedOrientation
    @desc Determine the orientation of this node with the transforms of its parents applied.
    @returns A Quat.
    */
    SQInteger ParsedAvSceneNodeUserData::getDerivedOrientation(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        Ogre::Quaternion orientation;
        AvSceneParsedDataUtils::computeDerivedTransform(*data->scene, data->index, 0, 0, &orientation);
        QuaternionUserData::quaternionToUserData(vm, orientation);

        return 1;
    }

    /**SQFunction
    @name getParent
    @desc Obtain the node this node is a child of.
    @returns A parsed scene node object, or null if this node sits at the root of the scene.
    */
    SQInteger ParsedAvSceneNodeUserData::getParent(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        const int parent = data->scene->data[data->index].parent;
        if(parent < 0){
            sq_pushnull(vm);
            return 1;
        }

        //The shared pointer has to be copied out before the new userdata is pushed, as pushing
        //can collect the node this was read from and with it the blob data points into.
        ParsedSceneFilePtr scene = data->scene;
        sceneNodeToUserData(vm, scene, static_cast<uint32>(parent));

        return 1;
    }

    /**SQFunction
    @name getNumChildren
    @desc Determine how many children this node has.
    @returns An integer count.
    */
    SQInteger ParsedAvSceneNodeUserData::getNumChildren(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        sq_pushinteger(vm, static_cast<SQInteger>(data->scene->childIndices[data->index].size()));

        return 1;
    }

    /**SQFunction
    @name getChild
    @desc Obtain one of this node's children.
    @param1:Integer:Index of the child, between 0 and getNumChildren().
    @returns A parsed scene node object.
    */
    SQInteger ParsedAvSceneNodeUserData::getChild(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        SQInteger idx;
        sq_getinteger(vm, 2, &idx);

        const std::vector<uint32>& children = data->scene->childIndices[data->index];
        if(idx < 0 || static_cast<size_t>(idx) >= children.size()){
            return sq_throwerror(vm, "Child index out of range.");
        }

        ParsedSceneFilePtr scene = data->scene;
        sceneNodeToUserData(vm, scene, children[idx]);

        return 1;
    }

    SQInteger ParsedAvSceneNodeUserData::nodeToString(HSQUIRRELVM vm){
        ParsedSceneNodeData* data;
        SCRIPT_ASSERT_RESULT(readSceneNodeFromUserData(vm, 1, &data));

        const SceneObjectData& d = data->scene->data[data->index];
        std::string name = (d.name >= 0 && static_cast<size_t>(d.name) < data->scene->strings.size())
            ? data->scene->strings[d.name] : "unnamed";

        std::string out = "ParsedAvSceneNode(" + name + ", index: " + std::to_string(data->index) + ")";
        sq_pushstring(vm, out.c_str(), -1);

        return 1;
    }

    void ParsedAvSceneNodeUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 17);

        ScriptUtils::addFunction(vm, nodeToString, "_tostring");
        ScriptUtils::addFunction(vm, getName, "getName");
        ScriptUtils::addFunction(vm, getTag, "getTag");
        ScriptUtils::addFunction(vm, getType, "getType");
        ScriptUtils::addFunction(vm, getMeshName, "getMeshName");
        ScriptUtils::addFunction(vm, getUserValue, "getUserValue");
        ScriptUtils::addFunction(vm, getAnimIdx, "getAnimIdx");
        ScriptUtils::addFunction(vm, getPosition, "getPosition");
        ScriptUtils::addFunction(vm, getScale, "getScale");
        ScriptUtils::addFunction(vm, getOrientation, "getOrientation");
        ScriptUtils::addFunction(vm, getDerivedPosition, "getDerivedPosition");
        ScriptUtils::addFunction(vm, getDerivedScale, "getDerivedScale");
        ScriptUtils::addFunction(vm, getDerivedOrientation, "getDerivedOrientation");
        ScriptUtils::addFunction(vm, getParent, "getParent");
        ScriptUtils::addFunction(vm, getNumChildren, "getNumChildren");
        ScriptUtils::addFunction(vm, getChild, "getChild", 2, ".i");

        sq_resetobject(&sceneNodeDelegateTable);
        sq_getstackobj(vm, -1, &sceneNodeDelegateTable);
        sq_addref(vm, &sceneNodeDelegateTable);
        sq_pop(vm, 1);
    }

    void ParsedAvSceneNodeUserData::setupConstants(HSQUIRRELVM vm){
        ScriptUtils::declareConstant(vm, "_AV_SCENE_NODE_EMPTY", (SQInteger)SceneObjectType::Empty);
        ScriptUtils::declareConstant(vm, "_AV_SCENE_NODE_MESH", (SQInteger)SceneObjectType::Mesh);
        ScriptUtils::declareConstant(vm, "_AV_SCENE_NODE_USER0", (SQInteger)SceneObjectType::User0);
        ScriptUtils::declareConstant(vm, "_AV_SCENE_NODE_USER1", (SQInteger)SceneObjectType::User1);
        ScriptUtils::declareConstant(vm, "_AV_SCENE_NODE_USER2", (SQInteger)SceneObjectType::User2);
        ScriptUtils::declareConstant(vm, "_AV_SCENE_NODE_USER3", (SQInteger)SceneObjectType::User3);
        ScriptUtils::declareConstant(vm, "_AV_SCENE_NODE_USER4", (SQInteger)SceneObjectType::User4);
        ScriptUtils::declareConstant(vm, "_AV_SCENE_NODE_USER5", (SQInteger)SceneObjectType::User5);
        ScriptUtils::declareConstant(vm, "_AV_SCENE_NODE_USER6", (SQInteger)SceneObjectType::User6);
    }

    SQInteger ParsedAvSceneNodeUserData::nodeReleaseHook(SQUserPointer p, SQInteger size){
        static_cast<ParsedSceneNodeData*>(p)->~ParsedSceneNodeData();

        return 0;
    }
}
