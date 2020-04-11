#pragma once

namespace AV{
    //Hlms
    static void* macroblockTypeTag = reinterpret_cast<void*>(10);
    static void* datablockTypeTag = reinterpret_cast<void*>(11);

    //InputManager
    static void* ButtonActionHandleTypeTag = reinterpret_cast<void*>(15);
    static void* ActionSetHandleTypeTag = reinterpret_cast<void*>(16);

    //Gui
    static void* WidgetWindow = reinterpret_cast<void*>(20);
}
