#ifdef DEBUGGING_TOOLS
#include "DeveloperNamespace.h"

#include "Classes/SlotPositionClass.h"
#include "World/WorldSingleton.h"
#include "System/BaseSingleton.h"
#include "World/Developer/DebugDrawer.h"

#include "World/Developer/MeshVisualiser.h"


namespace AV{

    SQInteger DeveloperNamespace::setMeshGroupVisible(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            SQInteger type = 0;
            SQBool visible = false;

            sq_getbool(vm, -1, &visible);
            sq_getinteger(vm, -2, &type);

            MeshVisualiser::MeshGroupType target = (MeshVisualiser::MeshGroupType)type;
            world->getMeshVisualiser()->setMeshGroupVisible(target, visible);
        }
        return 0;
    }

    SQInteger DeveloperNamespace::setRenderQueueForMeshGroup(HSQUIRRELVM vm){
        SCRIPT_CHECK_WORLD();

        {
            SQInteger group = 0;
            sq_getinteger(vm, 2, &group);

            if(group < 0 || group > 100){
                return sq_throwerror(vm, "Group must be in range 0-100.");
            }
            uint8 val = static_cast<uint8>(group);

            world->getMeshVisualiser()->setRenderQueueForMeshes(val);
        }
        return 0;
    }

    SQInteger DeveloperNamespace::drawPoint(HSQUIRRELVM vm){
        SlotPosition pos;
        SCRIPT_CHECK_RESULT(SlotPositionClass::getSlotFromInstance(vm, -1, &pos));

        BaseSingleton::getDebugDrawer()->drawPoint(pos);

        return 0;
    }

    SQInteger DeveloperNamespace::drawSphere(HSQUIRRELVM vm){
        SlotPosition pos;
        SCRIPT_CHECK_RESULT(SlotPositionClass::getSlotFromInstance(vm, -2, &pos));
        SQFloat radius;
        sq_getfloat(vm, -1, &radius);

        BaseSingleton::getDebugDrawer()->drawSphere(pos, radius);

        return 0;
    }

    SQInteger DeveloperNamespace::drawAxis(HSQUIRRELVM vm){
        SQInteger axisInt;
        SlotPosition pos;
        SCRIPT_CHECK_RESULT(SlotPositionClass::getSlotFromInstance(vm, -2, &pos));
        sq_getinteger(vm, -1, &axisInt);

        DebugDrawer::DrawAxis axis;
        switch(axisInt){
            default:
            case 0:
                axis = DebugDrawer::DrawAxis::AXIS_X; break;
            case 1:
                axis = DebugDrawer::DrawAxis::AXIS_Y; break;
            case 2:
                axis = DebugDrawer::DrawAxis::AXIS_Z; break;
        }

        BaseSingleton::getDebugDrawer()->drawAxis(pos, axis);

        return 0;
    }

    SQInteger DeveloperNamespace::drawCircle(HSQUIRRELVM vm){
        SlotPosition pos;
        SCRIPT_CHECK_RESULT(SlotPositionClass::getSlotFromInstance(vm, -2, &pos));
        SQFloat radius;
        sq_getfloat(vm, -1, &radius);


        BaseSingleton::getDebugDrawer()->drawCircle(pos, radius);

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
        /**SQFunction
        @name setRenderQueueForMeshGroup
        @param1:integer: The render queue value between 0 and 100.
        @desc Change the render queue of all objects in the mesh visualiser.
        */
        ScriptUtils::addFunction(vm, setRenderQueueForMeshGroup, "setRenderQueueForMeshGroup", 2, ".i");

        /**SQFunction
        @name drawPoint
        @param1:SlotPosition: The position where the point should be drawn.
        @desc Draw a debug point in 3D space. This function should be called each frame the point should be drawn.
        */
        ScriptUtils::addFunction(vm, drawPoint, "drawPoint", 2, ".u");

        /**SQFunction
        @name drawAxis
        @param1:SlotPosition: The position where the axis should be drawn.
        @param2:Axis: A number representing the axis. 0 is x, 1 is y, 2 is z. Anything else will draw x.
        @desc Draw a straight line from a point representing an axis.
        */
        ScriptUtils::addFunction(vm, drawAxis, "drawAxis", 3, ".ui");

        /**SQFunction
        @name drawCircle
        @param1:SlotPosition: The position where the circle should be drawn.
        @param2:Radius: A float or int representing the drawn circle radius.
        @desc Draw a single circle at a position, with a specified radius, which faces up on the Y axis.
        */
        ScriptUtils::addFunction(vm, drawCircle, "drawCircle", 3, ".un");

        /**SQFunction
        @name drawSphere
        @param1:SlotPosition: The position where the sphere should be drawn.
        @param2:Radius: A float or int representing the drawn sphere radius.
        @desc Draw a sphere comprised of lines, with a specified radius.
        */
        ScriptUtils::addFunction(vm, drawSphere, "drawSphere", 3, ".un");
    }
}
#endif
