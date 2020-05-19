#include "DeveloperNamespace.h"

#include "Classes/SlotPositionClass.h"
#include "Classes/EntityClass/EntityClass.h"

#include "World/WorldSingleton.h"
#include "World/Entity/EntityManager.h"
#include "World/Entity/Tracker/EntityTracker.h"

#include "World/Developer/MeshVisualiser.h"


namespace AV{

    SQInteger DeveloperNamespace::setMeshGroupVisible(HSQUIRRELVM vm){
        World *world = WorldSingleton::getWorld();
        if(world){
            SQInteger type = 0;
            SQBool visible = false;

            sq_getbool(vm, -1, &visible);
            sq_getinteger(vm, -2, &type);

            MeshVisualiser::MeshGroupType target = (MeshVisualiser::MeshGroupType)type;
            world->getMeshVisualiser()->setMeshGroupVisible(target, visible);
        }
        return 0;
    }

    /**SQNamespace
    @name _developer
    @desc Functionality specifically for developer purposes. This includes things such as drawing meshes, lines, and text. This namespace is only available if DEBUGGING_TOOLS is enabled.
    */
    void DeveloperNamespace::setupNamespace(HSQUIRRELVM vm){
        /**SQFunction
        @name setMeshGroupVisible
        @param1:meshType: The mesh type to make visible. 1 - physics chunks.
        @param2:visible: A boolean of whether or not this shape should be visible.
        @desc Change the visibility of one of the mesh type catagories.
        */
        ScriptUtils::addFunction(vm, setMeshGroupVisible, "setMeshGroupVisible", 3, ".ib");
    }
}
