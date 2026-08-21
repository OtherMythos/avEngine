#include "ParsedAvSceneUserData.h"

#include "ParsedAvSceneNodeUserData.h"

#include "Scripting/ScriptObjectTypeTags.h"

#include "World/Slot/Recipe/AvScene/AvSceneParsedDataUtils.h"

namespace AV{

    SQObject ParsedAvSceneUserData::sceneObjectDelegateTable;

    void ParsedAvSceneUserData::sceneObjectToUserData(HSQUIRRELVM vm, ParsedSceneFilePtr sceneFile){
        void* pointer = sq_newuserdata(vm, sizeof(ParsedSceneFilePtr));
        //The userdata memory is raw, so the shared pointer is constructed into it by hand and
        //destroyed again by the release hook.
        new (pointer) ParsedSceneFilePtr(sceneFile);

        sq_setreleasehook(vm, -1, SceneObjectReleaseHook);
        sq_pushobject(vm, sceneObjectDelegateTable);
        sq_setdelegate(vm, -2); //This pops the pushed table
        sq_settypetag(vm, -1, AvSceneObjectTypeTag);
    }

    UserDataGetResult ParsedAvSceneUserData::readSceneObjectFromUserData(HSQUIRRELVM vm, SQInteger stackInx, ParsedSceneFilePtr* outScene){
        SQUserPointer pointer, typeTag;
        if(SQ_FAILED(sq_getuserdata(vm, stackInx, &pointer, &typeTag))) return USER_DATA_GET_INCORRECT_TYPE;
        if(typeTag != AvSceneObjectTypeTag){
            outScene->reset();
            return USER_DATA_GET_TYPE_MISMATCH;
        }

        *outScene = *static_cast<ParsedSceneFilePtr*>(pointer);

        return USER_DATA_GET_SUCCESS;
    }

    /**SQFunction
    @name getNodeByTag
    @desc Find the node carrying a tag. Tags are unique within a scene, so at most one node
    can match.
    @param1:String:The tag to search for.
    @returns A parsed scene node object, or null if no node in this scene has that tag.
    */
    SQInteger ParsedAvSceneUserData::getNodeByTag(HSQUIRRELVM vm){
        ParsedSceneFilePtr scene;
        SCRIPT_ASSERT_RESULT(readSceneObjectFromUserData(vm, 1, &scene));

        const SQChar* tag;
        sq_getstring(vm, 2, &tag);

        const int idx = AvSceneParsedDataUtils::findNodeIndexByTag(*scene, tag);
        if(idx < 0){
            sq_pushnull(vm);
            return 1;
        }

        ParsedAvSceneNodeUserData::sceneNodeToUserData(vm, scene, static_cast<uint32>(idx));

        return 1;
    }

    /**SQFunction
    @name hasTag
    @desc Determine whether any node in this scene carries a tag.
    @param1:String:The tag to search for.
    @returns True if a node has that tag.
    */
    SQInteger ParsedAvSceneUserData::hasTag(HSQUIRRELVM vm){
        ParsedSceneFilePtr scene;
        SCRIPT_ASSERT_RESULT(readSceneObjectFromUserData(vm, 1, &scene));

        const SQChar* tag;
        sq_getstring(vm, 2, &tag);

        sq_pushbool(vm, AvSceneParsedDataUtils::findNodeIndexByTag(*scene, tag) >= 0);

        return 1;
    }

    /**SQFunction
    @name getNumRootNodes
    @desc Determine how many nodes sit at the root of this parsed scene.
    @returns An integer count.
    */
    SQInteger ParsedAvSceneUserData::getNumRootNodes(HSQUIRRELVM vm){
        ParsedSceneFilePtr scene;
        SCRIPT_ASSERT_RESULT(readSceneObjectFromUserData(vm, 1, &scene));

        sq_pushinteger(vm, static_cast<SQInteger>(scene->rootIndices.size()));

        return 1;
    }

    /**SQFunction
    @name getRootNode
    @desc Obtain one of the nodes at the root of this parsed scene.
    @param1:Integer:Index of the root node, between 0 and getNumRootNodes().
    @returns A parsed scene node object.
    */
    SQInteger ParsedAvSceneUserData::getRootNode(HSQUIRRELVM vm){
        ParsedSceneFilePtr scene;
        SCRIPT_ASSERT_RESULT(readSceneObjectFromUserData(vm, 1, &scene));

        SQInteger idx;
        sq_getinteger(vm, 2, &idx);
        if(idx < 0 || static_cast<size_t>(idx) >= scene->rootIndices.size()){
            return sq_throwerror(vm, "Root node index out of range.");
        }

        ParsedAvSceneNodeUserData::sceneNodeToUserData(vm, scene, scene->rootIndices[idx]);

        return 1;
    }

    SQInteger ParsedAvSceneUserData::sceneToString(HSQUIRRELVM vm){
        ParsedSceneFilePtr scene;
        SCRIPT_ASSERT_RESULT(readSceneObjectFromUserData(vm, 1, &scene));

        std::string out = "ParsedAvScene(objects: " + std::to_string(scene->data.size()) + ")";
        sq_pushstring(vm, out.c_str(), -1);

        return 1;
    }

    void ParsedAvSceneUserData::setupDelegateTable(HSQUIRRELVM vm){
        sq_newtableex(vm, 6);

        ScriptUtils::addFunction(vm, sceneToString, "_tostring");
        ScriptUtils::addFunction(vm, getNodeByTag, "getNodeByTag", 2, ".s");
        ScriptUtils::addFunction(vm, hasTag, "hasTag", 2, ".s");
        ScriptUtils::addFunction(vm, getNumRootNodes, "getNumRootNodes");
        ScriptUtils::addFunction(vm, getRootNode, "getRootNode", 2, ".i");

        sq_resetobject(&sceneObjectDelegateTable);
        sq_getstackobj(vm, -1, &sceneObjectDelegateTable);
        sq_addref(vm, &sceneObjectDelegateTable);
        sq_pop(vm, 1);
    }

    SQInteger ParsedAvSceneUserData::SceneObjectReleaseHook(SQUserPointer p, SQInteger size){
        static_cast<ParsedSceneFilePtr*>(p)->~ParsedSceneFilePtr();

        return 0;
    }
}
