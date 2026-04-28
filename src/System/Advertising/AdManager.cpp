#include "AdManager.h"

#ifdef ENABLE_ADMOB

#include "Scripting/Event/ScriptEventManager.h"

#include <squirrel.h>

namespace AV {
    AdManager* AdManager::sInstance = nullptr;
    ScriptEventManager* AdManager::mScriptEventManager = nullptr;

    void AdManager::setInstance(AdManager* mgr) {
        sInstance = mgr;
    }

    AdManager* AdManager::getInstance() {
        return sInstance;
    }

    void AdManager::notifyAdEvent(int eventId) {
        if(!mScriptEventManager) return;

        SQObject nullData;
        sq_resetobject(&nullData);
        mScriptEventManager->transmitEvent(eventId, nullData);
    }
}

#endif //ENABLE_ADMOB
