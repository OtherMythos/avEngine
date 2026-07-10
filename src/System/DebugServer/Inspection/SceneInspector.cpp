#ifdef DEBUG_SERVER

#include "SceneInspector.h"

#include "DebugJsonUtil.h"

#include "System/BaseSingleton.h"

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreMovableObject.h"
#include <Math/Simple/OgreAabb.h>

#include <cstdlib>
#include <vector>

namespace AV{
    //Walk state shared across the recursive serialisation, so a global node budget can be
    //enforced regardless of tree shape.
    struct SceneWalkState{
        int maxDepth;
        int remaining;
        bool truncated;
        rapidjson::Document::AllocatorType& allocator;
    };

    static rapidjson::Value serialiseObject(Ogre::MovableObject* obj, rapidjson::Document::AllocatorType& allocator, bool includeLocalAabb){
        rapidjson::Value o(rapidjson::kObjectType);
        o.AddMember("name", rapidjson::Value(obj->getName().c_str(), allocator), allocator);
        o.AddMember("type", rapidjson::Value(obj->getMovableType().c_str(), allocator), allocator);
        o.AddMember("visible", obj->getVisible(), allocator);
        o.AddMember("castShadows", obj->getCastShadows(), allocator);
        o.AddMember("worldAabb", DebugJsonUtil::aabb(obj->getWorldAabb(), allocator), allocator);
        if(includeLocalAabb){
            o.AddMember("localAabb", DebugJsonUtil::aabb(obj->getLocalAabb(), allocator), allocator);
        }
        return o;
    }

    static rapidjson::Value serialiseNode(Ogre::SceneNode* node, int currentDepth, SceneWalkState& state){
        rapidjson::Document::AllocatorType& allocator = state.allocator;

        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("id", static_cast<uint64_t>(node->getId()), allocator);
        obj.AddMember("pos", DebugJsonUtil::vector3(node->getPosition(), allocator), allocator);
        obj.AddMember("derivedPos", DebugJsonUtil::vector3(node->_getDerivedPositionUpdated(), allocator), allocator);
        obj.AddMember("scale", DebugJsonUtil::vector3(node->getScale(), allocator), allocator);
        obj.AddMember("orient", DebugJsonUtil::quaternion(node->getOrientation(), allocator), allocator);

        const size_t numObjects = node->numAttachedObjects();
        if(numObjects > 0){
            rapidjson::Value objects(rapidjson::kArrayType);
            for(size_t i = 0; i < numObjects; i++){
                objects.PushBack(serialiseObject(node->getAttachedObject(i), allocator, false), allocator);
            }
            obj.AddMember("objects", objects, allocator);
        }

        const size_t numChildren = node->numChildren();
        if(numChildren > 0){
            if(currentDepth >= state.maxDepth){
                //Depth limit reached: report the count so the agent can drill down with root=<id>.
                obj.AddMember("childCount", static_cast<uint64_t>(numChildren), allocator);
                state.truncated = true;
            }else{
                rapidjson::Value children(rapidjson::kArrayType);
                size_t serialised = 0;
                for(size_t i = 0; i < numChildren; i++){
                    if(state.remaining <= 0) break;
                    state.remaining--;
                    Ogre::SceneNode* child = static_cast<Ogre::SceneNode*>(node->getChild(i));
                    children.PushBack(serialiseNode(child, currentDepth + 1, state), allocator);
                    serialised++;
                }
                obj.AddMember("children", children, allocator);
                if(serialised < numChildren){
                    //Node budget exhausted before all children fit.
                    obj.AddMember("childCount", static_cast<uint64_t>(numChildren), allocator);
                    state.truncated = true;
                }
            }
        }

        return obj;
    }

    Ogre::SceneNode* SceneInspector::findById(Ogre::SceneNode* root, uint64_t id){
        if(!root) return nullptr;
        if(static_cast<uint64_t>(root->getId()) == id) return root;
        const size_t numChildren = root->numChildren();
        for(size_t i = 0; i < numChildren; i++){
            Ogre::SceneNode* found = findById(static_cast<Ogre::SceneNode*>(root->getChild(i)), id);
            if(found) return found;
        }
        return nullptr;
    }

    void SceneInspector::writeSceneTree(rapidjson::Document& doc, int& status, const std::string& rootId, int maxDepth, int maxNodes){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        Ogre::SceneManager* sceneMgr = BaseSingleton::getSceneManager();
        if(!sceneMgr){
            status = 503;
            doc.AddMember("error", "scene manager not available", allocator);
            return;
        }

        Ogre::SceneNode* dynamicRoot = sceneMgr->getRootSceneNode(Ogre::SCENE_DYNAMIC);
        Ogre::SceneNode* staticRoot = sceneMgr->getRootSceneNode(Ogre::SCENE_STATIC);

        SceneWalkState state{maxDepth, maxNodes, false, allocator};

        rapidjson::Value nodes(rapidjson::kArrayType);

        if(!rootId.empty()){
            const uint64_t id = std::strtoull(rootId.c_str(), nullptr, 10);
            Ogre::SceneNode* target = findById(dynamicRoot, id);
            if(!target) target = findById(staticRoot, id);
            if(!target){
                status = 404;
                doc.RemoveAllMembers();
                doc.AddMember("error", rapidjson::Value(("no scene node with id " + rootId).c_str(), allocator), allocator);
                return;
            }
            state.remaining--;
            nodes.PushBack(serialiseNode(target, 0, state), allocator);
        }else{
            //Dump from both scene roots.
            if(dynamicRoot && state.remaining > 0){
                state.remaining--;
                rapidjson::Value n = serialiseNode(dynamicRoot, 0, state);
                n.AddMember("memoryType", "dynamic", allocator);
                nodes.PushBack(n, allocator);
            }
            if(staticRoot && state.remaining > 0){
                state.remaining--;
                rapidjson::Value n = serialiseNode(staticRoot, 0, state);
                n.AddMember("memoryType", "static", allocator);
                nodes.PushBack(n, allocator);
            }
        }

        doc.AddMember("truncated", state.truncated, allocator);
        doc.AddMember("nodes", nodes, allocator);
    }

    void SceneInspector::writeNodeDetail(rapidjson::Document& doc, int& status, const std::string& nodeId){
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();

        Ogre::SceneManager* sceneMgr = BaseSingleton::getSceneManager();
        if(!sceneMgr){
            status = 503;
            doc.AddMember("error", "scene manager not available", allocator);
            return;
        }

        const uint64_t id = std::strtoull(nodeId.c_str(), nullptr, 10);
        Ogre::SceneNode* node = findById(sceneMgr->getRootSceneNode(Ogre::SCENE_DYNAMIC), id);
        if(!node) node = findById(sceneMgr->getRootSceneNode(Ogre::SCENE_STATIC), id);
        if(!node){
            status = 404;
            doc.AddMember("error", rapidjson::Value(("no scene node with id " + nodeId).c_str(), allocator), allocator);
            return;
        }

        doc.AddMember("id", static_cast<uint64_t>(node->getId()), allocator);
        doc.AddMember("pos", DebugJsonUtil::vector3(node->getPosition(), allocator), allocator);
        doc.AddMember("derivedPos", DebugJsonUtil::vector3(node->_getDerivedPositionUpdated(), allocator), allocator);
        doc.AddMember("scale", DebugJsonUtil::vector3(node->getScale(), allocator), allocator);
        doc.AddMember("orient", DebugJsonUtil::quaternion(node->getOrientation(), allocator), allocator);

        //Parent chain of ids, nearest parent first, up to the scene root.
        rapidjson::Value parentChain(rapidjson::kArrayType);
        Ogre::Node* parent = node->getParent();
        while(parent){
            parentChain.PushBack(static_cast<uint64_t>(parent->getId()), allocator);
            parent = parent->getParent();
        }
        doc.AddMember("parentChain", parentChain, allocator);

        const size_t numObjects = node->numAttachedObjects();
        rapidjson::Value objects(rapidjson::kArrayType);
        for(size_t i = 0; i < numObjects; i++){
            objects.PushBack(serialiseObject(node->getAttachedObject(i), allocator, true), allocator);
        }
        doc.AddMember("objects", objects, allocator);

        doc.AddMember("childCount", static_cast<uint64_t>(node->numChildren()), allocator);
    }
}

#endif
