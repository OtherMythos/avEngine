#include "PurchaseManager.h"

#ifdef ENABLE_MICROTRANSACTIONS

namespace AV {
    PurchaseManager* PurchaseManager::sInstance = nullptr;

    void PurchaseManager::setInstance(PurchaseManager* mgr) {
        sInstance = mgr;
    }

    PurchaseManager* PurchaseManager::getInstance() {
        return sInstance;
    }
}

#endif //ENABLE_MICROTRANSACTIONS
