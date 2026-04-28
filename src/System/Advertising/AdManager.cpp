#include "AdManager.h"

#ifdef ENABLE_ADMOB

namespace AV {
    AdManager* AdManager::sInstance = nullptr;

    void AdManager::setInstance(AdManager* mgr) {
        sInstance = mgr;
    }

    AdManager* AdManager::getInstance() {
        return sInstance;
    }
}

#endif //ENABLE_ADMOB
