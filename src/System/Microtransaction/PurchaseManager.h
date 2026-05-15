#pragma once

#ifdef ENABLE_MICROTRANSACTIONS

#include <string>

namespace AV {
    /**
     Abstract interface for the purchase manager. Platform-specific implementations
     inherit from this class. The null implementation (PurchaseManagerNull) is used
     on platforms where in-app purchasing is not yet implemented.

     Purchase events are proper engine EventIds defined in Event.h:
     - PurchaseProductPurchased
     - PurchaseProductFailed
     - PurchaseRestoreCompleted
     - PurchaseRestoreFailed
     - PurchaseProductInfo
     - PurchaseProductInfoFailed

     Implementations dispatch these events via EventDispatcher::transmitEvent().
     Scripts subscribe using: _event.subscribe(_EVENT_PURCHASE_*, callback, this)
    */
    class PurchaseManager {
    public:
        virtual ~PurchaseManager() = default;

        virtual void initialise() = 0;

        /**
         Initiate a purchase for the given product ID.
         Fires PurchaseProductPurchased on success, PurchaseProductFailed on failure.
        */
        virtual void purchaseProduct(const std::string& productId) = 0;

        /**
         Restore previously completed purchases.
         Fires PurchaseProductPurchased for each restored product,
         then PurchaseRestoreCompleted or PurchaseRestoreFailed when the batch finishes.
        */
        virtual void restorePurchases() = 0;

        /**
         Query localised product info for the given product ID.
         Fires PurchaseProductInfo on success, PurchaseProductInfoFailed on failure.
        */
        virtual void queryProductInfo(const std::string& productId) = 0;

        static void setInstance(PurchaseManager* mgr);
        static PurchaseManager* getInstance();

    private:
        static PurchaseManager* sInstance;
    };
}

#endif //ENABLE_MICROTRANSACTIONS
