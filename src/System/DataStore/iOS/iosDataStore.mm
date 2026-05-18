#ifdef TARGET_APPLE_IPHONE

#include "iosDataStore.h"

#import <Foundation/Foundation.h>

namespace AV{
    static NSString* toNSString(const std::string& s){
        return [NSString stringWithUTF8String:s.c_str()];
    }

    bool iosDataStore::getInt(const std::string& key, int& outVal){
        NSUserDefaults* ud = [NSUserDefaults standardUserDefaults];
        NSString* nsKey = toNSString(key);
        if(![ud objectForKey:nsKey]) return false;
        outVal = (int)[ud integerForKey:nsKey];
        return true;
    }

    bool iosDataStore::getFloat(const std::string& key, float& outVal){
        NSUserDefaults* ud = [NSUserDefaults standardUserDefaults];
        NSString* nsKey = toNSString(key);
        if(![ud objectForKey:nsKey]) return false;
        outVal = [ud floatForKey:nsKey];
        return true;
    }

    bool iosDataStore::getBool(const std::string& key, bool& outVal){
        NSUserDefaults* ud = [NSUserDefaults standardUserDefaults];
        NSString* nsKey = toNSString(key);
        if(![ud objectForKey:nsKey]) return false;
        outVal = [ud boolForKey:nsKey] == YES;
        return true;
    }

    bool iosDataStore::getString(const std::string& key, std::string& outVal){
        NSUserDefaults* ud = [NSUserDefaults standardUserDefaults];
        NSString* nsKey = toNSString(key);
        NSString* val = [ud stringForKey:nsKey];
        if(!val) return false;
        outVal = val.UTF8String;
        return true;
    }

    void iosDataStore::setInt(const std::string& key, int val){
        [[NSUserDefaults standardUserDefaults] setInteger:val forKey:toNSString(key)];
    }

    void iosDataStore::setFloat(const std::string& key, float val){
        [[NSUserDefaults standardUserDefaults] setFloat:val forKey:toNSString(key)];
    }

    void iosDataStore::setBool(const std::string& key, bool val){
        [[NSUserDefaults standardUserDefaults] setBool:(val ? YES : NO) forKey:toNSString(key)];
    }

    void iosDataStore::setString(const std::string& key, const std::string& val){
        [[NSUserDefaults standardUserDefaults] setObject:toNSString(val) forKey:toNSString(key)];
    }

    void iosDataStore::remove(const std::string& key){
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:toNSString(key)];
    }
}

#endif //TARGET_APPLE_IPHONE
