#pragma once

#include "World/Slot/Recipe/RecipeData.h"
#include "Scripting/ScriptVM.h"

namespace AV{

    /**
    Keeps track of important squirrel closure objects which are called as part of system operation.
    */
    class SystemEventListenerObjects{
    public:
        SystemEventListenerObjects() = delete;
        ~SystemEventListenerObjects() = delete;

        enum ListenerType{
            CHUNK,
            LISTENER_MAX
        };

        /**
        Register a closure to listen to an event type.
        Either closure or null can be passed here.
        Passing null will cause the event to be unset.
        */
        static void registerListenerForType(ListenerType type, SQObject obj);

        static void executeListener(ListenerType type);

        static void clear();

        static const RecipeData* mCurrentRecipeData;

    private:
        static SQObject regObj[LISTENER_MAX];

        static void checkAndClear(ListenerType type);
    };
}
