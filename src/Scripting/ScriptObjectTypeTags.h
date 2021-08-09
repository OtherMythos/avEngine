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
    static void* WidgetPanelTypeTag = reinterpret_cast<void*>(26);
    static void* WidgetAnimatedLabelTypeTag = reinterpret_cast<void*>(27);
    static void* LayoutLineTypeTag = reinterpret_cast<void*>(30);

    //Basic
    static void* Vector3TypeTag = reinterpret_cast<void*>(40);
    static void* Vector2TypeTag = reinterpret_cast<void*>(41);
    static void* SlotPositionTypeTag = reinterpret_cast<void*>(42);
    static void* QuaternionTypeTag = reinterpret_cast<void*>(43);
    static void* TimerObjectTypeTag = reinterpret_cast<void*>(44);

    //Physics
    static void* CollisionSenderTypeTag = reinterpret_cast<void*>(50);
    static void* CollisionReceiverTypeTag = reinterpret_cast<void*>(51);
    static void* DynamicsConstructionInfoTypeTag = reinterpret_cast<void*>(52);

    //Entity
    static void* EntityClassTypeTag = reinterpret_cast<void*>(60);

    //Ogre
    static void* SceneNodeTypeTag = reinterpret_cast<void*>(70);
        //Movable objects
        static void* MovableObjectItemTypeTag = reinterpret_cast<void*>(71);
        static void* MovableObjectLightTypeTag = reinterpret_cast<void*>(72);
        static void* ParticleSystemTypeTag = reinterpret_cast<void*>(73);
        static void* CameraTypeTag = reinterpret_cast<void*>(74);
    static void* OgreSkeletonTypeTag = reinterpret_cast<void*>(80);
    static void* OgreSkeletonAnimationTypeTag = reinterpret_cast<void*>(81);
    static void* OgreBoneTypeTag = reinterpret_cast<void*>(82);
    static void* OgreAABBTypeTag = reinterpret_cast<void*>(83);
    static void* RayTypeTag = reinterpret_cast<void*>(84);
    static void* TextureTypeTag = reinterpret_cast<void*>(85);
    static void* CompositorWorkspaceTypeTag = reinterpret_cast<void*>(86);

    //Nav
    static void* NavMeshTypeTag = reinterpret_cast<void*>(90);
    static void* NavMeshQueryTypeTag = reinterpret_cast<void*>(91);

    //Animation
    static void* AnimationInfoTypeTag = reinterpret_cast<void*>(100);
    static void* AnimationInstanceTypeTag = reinterpret_cast<void*>(101);

}
