#pragma once

namespace AV{
    //Hlms
    static void* macroblockTypeTag = reinterpret_cast<void*>(10);
    static void* datablockTypeTag = reinterpret_cast<void*>(11);

    //InputManager
    static void* ButtonActionHandleTypeTag = reinterpret_cast<void*>(15);
    static void* ActionSetHandleTypeTag = reinterpret_cast<void*>(16);

    //Gui
    static void* WidgetWindowTypeTag = reinterpret_cast<void*>(20);
    static void* WidgetButtonTypeTag = reinterpret_cast<void*>(21);
    static void* WidgetLabelTypeTag = reinterpret_cast<void*>(22);
    static void* WidgetEditboxTypeTag = reinterpret_cast<void*>(23);
    static void* WidgetSliderTypeTag = reinterpret_cast<void*>(24);
    static void* WidgetCheckboxTypeTag = reinterpret_cast<void*>(25);
    static void* LayoutLineTypeTag = reinterpret_cast<void*>(30);

    //Basic
    static void* Vector3TypeTag = reinterpret_cast<void*>(40);
    static void* SlotPositionTypeTag = reinterpret_cast<void*>(41);
    static void* QuaternionTypeTag = reinterpret_cast<void*>(42);
    static void* TimerObjectTypeTag = reinterpret_cast<void*>(43);

    //Physics
    static void* CollisionSenderTypeTag = reinterpret_cast<void*>(50);
    static void* CollisionReceiverTypeTag = reinterpret_cast<void*>(51);

    //Entity
    static void* EntityClassTypeTag = reinterpret_cast<void*>(60);

    //Ogre
    static void* SceneNodeTypeTag = reinterpret_cast<void*>(70);
    static void* MovableObjectItemTypeTag = reinterpret_cast<void*>(71);
    static void* MovableObjectLightTypeTag = reinterpret_cast<void*>(72);
    static void* OgreSkeletonTypeTag = reinterpret_cast<void*>(73);
    static void* OgreSkeletonAnimationTypeTag = reinterpret_cast<void*>(74);
    static void* OgreBoneTypeTag = reinterpret_cast<void*>(75);
    static void* OgreAABBTypeTag = reinterpret_cast<void*>(76);
    static void* RayTypeTag = reinterpret_cast<void*>(77);

    //Nav
    static void* NavMeshTypeTag = reinterpret_cast<void*>(90);

}
