#pragma once

namespace AV{
    //Hlms
    static void* macroblockTypeTag = reinterpret_cast<void*>(10);
    static void* datablockTypeTag = reinterpret_cast<void*>(11);
    static void* blendblockTypeTag = reinterpret_cast<void*>(12);

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
    static void* WidgetSpinnerTypeTag = reinterpret_cast<void*>(28);

    static void* LayoutLineTypeTag = reinterpret_cast<void*>(30);
    static void* LayoutTableTypeTag = reinterpret_cast<void*>(31);

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
    static void* EntityUserDataTypeTag = reinterpret_cast<void*>(60);

    //Ogre
    static void* SceneNodeTypeTag = reinterpret_cast<void*>(70);
    static void* TagPointTypeTag = reinterpret_cast<void*>(71);
        //Movable objects
        static void* MovableObjectItemTypeTag = reinterpret_cast<void*>(72);
        static void* MovableObjectLightTypeTag = reinterpret_cast<void*>(73);
        static void* ParticleSystemTypeTag = reinterpret_cast<void*>(74);
        static void* CameraTypeTag = reinterpret_cast<void*>(75);
    static void* OgreSkeletonTypeTag = reinterpret_cast<void*>(80);
    static void* OgreSkeletonAnimationTypeTag = reinterpret_cast<void*>(81);
    static void* OgreBoneTypeTag = reinterpret_cast<void*>(82);
    static void* OgreAABBTypeTag = reinterpret_cast<void*>(83);
    static void* RayTypeTag = reinterpret_cast<void*>(84);
    static void* TextureTypeTag = reinterpret_cast<void*>(85);
    static void* CompositorWorkspaceTypeTag = reinterpret_cast<void*>(86);
    static void* ColourValueTypeTag = reinterpret_cast<void*>(87);
    static void* GPUProgramTypeTag = reinterpret_cast<void*>(88);
    static void* MaterialTypeTag = reinterpret_cast<void*>(89);
    static void* GPUProgramParametersTypeTag = reinterpret_cast<void*>(90);
    static void* PlaneTypeTag = reinterpret_cast<void*>(91);
    static void* MeshTypeTag = reinterpret_cast<void*>(92);
    static void* SubMeshTypeTag = reinterpret_cast<void*>(93);
    static void* VertexArrayObjectTypeTag = reinterpret_cast<void*>(94);
    static void* OgreBufferObjectTypeTag = reinterpret_cast<void*>(95);
    static void* VertexElementVecTypeTag = reinterpret_cast<void*>(96);
    static void* TextureBoxTypeTag = reinterpret_cast<void*>(97);
    static void* StagingTextureTypeTag = reinterpret_cast<void*>(98);


    //Nav
    static void* NavMeshTypeTag = reinterpret_cast<void*>(100);
    static void* NavMeshQueryTypeTag = reinterpret_cast<void*>(101);

    //Animation
    static void* AnimationInfoTypeTag = reinterpret_cast<void*>(110);
    static void* AnimationInstanceTypeTag = reinterpret_cast<void*>(111);

    //AvScene
    static void* AvSceneObjectTypeTag = reinterpret_cast<void*>(120);

    //Audio
    static void* AudioSourceTypeTag = reinterpret_cast<void*>(130);
    static void* AudioBufferTypeTag = reinterpret_cast<void*>(131);
}
