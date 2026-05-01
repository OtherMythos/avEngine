#pragma once

#ifdef ENABLE_MICROTRANSACTIONS
#ifdef TARGET_APPLE_IPHONE

#include "System/Microtransaction/PurchaseManager.h"

//Forward declare Objective-C class to avoid importing ObjC headers here.
#ifdef __OBJC__
@class iOSPurchaseDelegate;
#endif

namespace AV {
    /**
     iOS implementation of PurchaseManager using StoreKit 1 (SKPaymentQueue).
     The Objective-C bridge objects are owned by the delegate helper class
     to keep this header free of Objective-C types.
    */
    class iosPurchaseManager : public PurchaseManager {
    public:
        iosPurchaseManager();
        ~iosPurchaseManager();

        void initialise() override;
        void purchaseProduct(const std::string& productId) override;
        void restorePurchases() override;
        void queryProductInfo(const std::string& productId) override;

    private:
        //Opaque pointer to the Objective-C delegate/observer object.
        void* mDelegate = nullptr;
    };
}

#endif //TARGET_APPLE_IPHONE
#endif //ENABLE_MICROTRANSACTIONS
