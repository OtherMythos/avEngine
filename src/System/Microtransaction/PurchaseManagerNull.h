#pragma once

#include "PurchaseManager.h"

#ifdef ENABLE_MICROTRANSACTIONS

namespace AV {
    /**
     No-op PurchaseManager used on platforms where in-app purchasing is not yet
     implemented, or as a safe fallback when no platform implementation is available.
    */
    class PurchaseManagerNull : public PurchaseManager {
    public:
        void initialise() override {}
        void purchaseProduct(const std::string& productId) override {}
        void restorePurchases() override {}
        void queryProductInfo(const std::string& productId) override {}
    };
}

#endif //ENABLE_MICROTRANSACTIONS
