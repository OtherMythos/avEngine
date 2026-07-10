#ifdef DEBUG_SERVER

#pragma once

#include <rapidjson/document.h>
#include <string>
#include <cstdint>

namespace Ogre{
    class SceneNode;
}

namespace AV{
    /**
    Serialises the Ogre scene graph into JSON.

    Ogre-Next scene nodes are nameless, but each carries a stable numeric id
    (Ogre::Node::getId()). That id is the identifier used to address a node in the
    'root' query parameter and the /api/scene/node/<id> route.

    Must be run on the main thread (via the MainThreadQueue), as it reads live scene state.
    */
    class SceneInspector{
    public:
        /**
        Dump the scene graph.

        @param doc Document to populate.
        @param status Set to 404 if a requested root id is not found; otherwise left at 200.
        @param rootId Numeric node id to root the dump at. Empty dumps from the dynamic and static roots.
        @param maxDepth How many levels of children to descend.
        @param maxNodes Cap on the total number of nodes serialised (protects agent context budgets).
        */
        static void writeSceneTree(rapidjson::Document& doc, int& status, const std::string& rootId, int maxDepth, int maxNodes);

        /**
        Deep dive on a single node addressed by its numeric id.

        @param status Set to 404 if the id is not found.
        */
        static void writeNodeDetail(rapidjson::Document& doc, int& status, const std::string& nodeId);

    private:
        //Depth-first search for a node with the given id, starting at root.
        static Ogre::SceneNode* findById(Ogre::SceneNode* root, uint64_t id);
    };
}

#endif
