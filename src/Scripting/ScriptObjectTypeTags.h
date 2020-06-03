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
}
