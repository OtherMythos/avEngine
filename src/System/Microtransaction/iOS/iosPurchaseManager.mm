#ifdef ENABLE_MICROTRANSACTIONS
#ifdef TARGET_APPLE_IPHONE

#include "iosPurchaseManager.h"

#import <StoreKit/StoreKit.h>
#include "Event/Events/PurchaseEvent.h"
#include "Event/EventDispatcher.h"

namespace AV {}

//----------------------------------------------------------------------------
//Objective-C delegate that receives StoreKit callbacks and forwards to C++.
//----------------------------------------------------------------------------
@interface iOSPurchaseDelegate : NSObject <SKPaymentTransactionObserver, SKProductsRequestDelegate>
@end

@implementation iOSPurchaseDelegate

//----------------------------------------------------------------------------
//SKPaymentTransactionObserver
//----------------------------------------------------------------------------

- (void)paymentQueue:(SKPaymentQueue*)queue updatedTransactions:(NSArray<SKPaymentTransaction*>*)transactions {
    for(SKPaymentTransaction* transaction in transactions) {
        switch(transaction.transactionState) {
            case SKPaymentTransactionStatePurchased:
            case SKPaymentTransactionStateRestored: {
                AV::PurchaseEventProductPurchased event;
                event.productId = transaction.payment.productIdentifier.UTF8String;
                AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
                [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                break;
            }
            case SKPaymentTransactionStateFailed: {
                AV::PurchaseEventProductFailed event;
                event.productId = transaction.payment.productIdentifier.UTF8String;
                AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
                [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                break;
            }
            case SKPaymentTransactionStatePurchasing:
            case SKPaymentTransactionStateDeferred:
            default:
                break;
        }
    }
}

- (void)paymentQueueRestoreCompletedTransactionsFinished:(SKPaymentQueue*)queue {
    AV::PurchaseEventRestoreCompleted event;
    AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
}

- (void)paymentQueue:(SKPaymentQueue*)queue restoreCompletedTransactionsFailedWithError:(NSError*)error {
    AV::PurchaseEventRestoreFailed event;
    AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
}

//----------------------------------------------------------------------------
//SKProductsRequestDelegate
//----------------------------------------------------------------------------

- (void)productsRequest:(SKProductsRequest*)request didReceiveResponse:(SKProductsResponse*)response {
    NSNumberFormatter* formatter = [[NSNumberFormatter alloc] init];
    formatter.numberStyle = NSNumberFormatterCurrencyStyle;

    for(SKProduct* product in response.products) {
        formatter.locale = product.priceLocale;
        NSString* priceString = [formatter stringFromNumber:product.price];

        AV::PurchaseEventProductInfo event;
        event.productId = product.productIdentifier.UTF8String;
        event.price = priceString ? priceString.UTF8String : "";
        event.title = product.localizedTitle ? product.localizedTitle.UTF8String : "";
        event.description = product.localizedDescription ? product.localizedDescription.UTF8String : "";
        AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
    }

    for(NSString* invalidId in response.invalidProductIdentifiers) {
        AV::PurchaseEventProductInfoFailed event;
        event.productId = invalidId.UTF8String;
        AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
    }
}

- (void)request:(SKRequest*)request didFailWithError:(NSError*)error {
    //Product request failed without a specific product ID available.
    //Fire a failed event with an empty product ID so the script can react.
    AV::PurchaseEventProductInfoFailed event;
    event.productId = "";
    AV::EventDispatcher::transmitEvent(AV::EventType::System, event);
}

@end

//----------------------------------------------------------------------------
//C++ implementation
//----------------------------------------------------------------------------
namespace AV {

    iosPurchaseManager::iosPurchaseManager() {
        iOSPurchaseDelegate* delegate = [[iOSPurchaseDelegate alloc] init];
        mDelegate = (__bridge_retained void*)delegate;
    }

    iosPurchaseManager::~iosPurchaseManager() {
        if(mDelegate) {
            [[SKPaymentQueue defaultQueue] removeTransactionObserver:(__bridge iOSPurchaseDelegate*)mDelegate];
            iOSPurchaseDelegate* delegate = (__bridge_transfer iOSPurchaseDelegate*)mDelegate;
            (void)delegate;
            mDelegate = nullptr;
        }
    }

    void iosPurchaseManager::initialise() {
        iOSPurchaseDelegate* delegate = (__bridge iOSPurchaseDelegate*)mDelegate;
        [[SKPaymentQueue defaultQueue] addTransactionObserver:delegate];
    }

    void iosPurchaseManager::purchaseProduct(const std::string& productId) {
        if(productId.empty()) return;
        if(![SKPaymentQueue canMakePayments]) return;

        NSString* nsProductId = [NSString stringWithUTF8String:productId.c_str()];
        SKMutablePayment* payment = [SKMutablePayment new];
        payment.productIdentifier = nsProductId;
        [[SKPaymentQueue defaultQueue] addPayment:payment];
    }

    void iosPurchaseManager::restorePurchases() {
        [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
    }

    void iosPurchaseManager::queryProductInfo(const std::string& productId) {
        if(productId.empty()) return;

        NSSet<NSString*>* productIds = [NSSet setWithObject:[NSString stringWithUTF8String:productId.c_str()]];
        SKProductsRequest* request = [[SKProductsRequest alloc] initWithProductIdentifiers:productIds];
        request.delegate = (__bridge iOSPurchaseDelegate*)mDelegate;
        [request start];
    }

}

#endif //TARGET_APPLE_IPHONE
#endif //ENABLE_MICROTRANSACTIONS
