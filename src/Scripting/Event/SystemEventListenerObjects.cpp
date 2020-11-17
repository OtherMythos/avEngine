#include "SystemEventListenerObjects.h"

#include <cassert>

namespace AV{
    SQObject SystemEventListenerObjects::regObj[LISTENER_MAX];

    const RecipeData* SystemEventListenerObjects::mCurrentRecipeData = 0;

    void SystemEventListenerObjects::checkAndClear(ListenerType type){
        if(regObj[type]._type == OT_CLOSURE){
            ScriptVM::dereferenceObject(regObj[type]);
        }
        sq_resetobject(&regObj[type]);
    }

    void SystemEventListenerObjects::clear(){
        for(int i = 0; i < LISTENER_MAX; i++){
            checkAndClear( static_cast<ListenerType>(i) );
        }
    }

    void SystemEventListenerObjects::registerListenerForType(ListenerType type, SQObject obj){
        checkAndClear(type);

        ScriptVM::referenceObject(obj);
        regObj[type] = obj;
    }

    void SystemEventListenerObjects::executeListener(ListenerType type){
        if(regObj[type]._type == OT_NULL) return;
        assert(regObj[type]._type == OT_CLOSURE);

        ScriptVM::callClosure(regObj[type]);
    }
}
