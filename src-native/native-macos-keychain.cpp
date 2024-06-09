//
//  native-macos-keychain.cpp
//
//  Created by H3D- on 6/7/24.
//

#include "native-macos-keychain.hpp"

#include <napi.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <string.h>

/**
 * Sets a keychain Generic Password value.
 */
Napi::Value setGenericPassword(const Napi::CallbackInfo& info) {
    
    Napi::Env env = info.Env();
    bool isServiceSet = info.Length() > 2;
    long querySize = isServiceSet ? 3 : 2;

    // --- Get passed arguments and validate ------------------------------------------------------------------
    if (info.Length() < 2) {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsString() || !info[1].IsString() || (isServiceSet && !info[2].IsString()) ) {
        Napi::TypeError::New(env, "Wrong argument type(s)").ThrowAsJavaScriptException();
        return env.Null();
    }



    std::string key     = info[0].As<Napi::String>().Utf8Value();
    std::string value   = info[1].As<Napi::String>().Utf8Value();
    std::string service;
    if(isServiceSet) { service = info[2].As<Napi::String>().Utf8Value(); }
    
    // --- Lookup Query Dictionary values --------------------------------------------------------------------
    CFStringRef queryKeys[3];
    queryKeys[0] = kSecClass;
    queryKeys[1] = kSecAttrAccount;
    queryKeys[2] = kSecAttrService;

    CFTypeRef queryValues[3];
    queryValues[0] = kSecClassGenericPassword;
    queryValues[1] = CFStringCreateWithCString(kCFAllocatorDefault, key.data(), kCFStringEncodingUTF8);
    queryValues[2] = isServiceSet ? CFStringCreateWithCString(kCFAllocatorDefault, service.data(), kCFStringEncodingUTF8) : NULL;

    
    CFDictionaryRef queryDR;
    queryDR = CFDictionaryCreate(kCFAllocatorDefault, (const void**) queryKeys, (const void**) queryValues, querySize, NULL, NULL);
    
    // Lookup Item
    OSStatus queryResult = SecItemCopyMatching(queryDR, NULL);
    
    // Process based on lookup result
    switch(queryResult) {
            
        // Found existing value: Will update
        case errSecSuccess : {
            // Build value Dictionary
            CFStringRef updateItemKeys[1];
            CFTypeRef   updateItemValues[1];

            updateItemKeys[0]   = kSecValueData;
            updateItemValues[0] = CFDataCreate(kCFAllocatorDefault, (UInt8 *)value.data(), (CFIndex)value.size());

            CFDictionaryRef updateValuesDR;
            updateValuesDR = CFDictionaryCreate(kCFAllocatorDefault, (const void**) updateItemKeys, (const void**) updateItemValues, 1, NULL, NULL);

            // Update key
            OSStatus updateResult = SecItemUpdate(queryDR, updateValuesDR);
            CFRelease(queryDR);
            CFRelease(updateValuesDR);

            if(updateResult != errSecSuccess) {
                Napi::Error::New(env, "Update Error (" + std::to_string(updateResult) + ")").ThrowAsJavaScriptException();
                Napi::Boolean::New(env, false);
            }

        }
        break;
        
        // Item not found, will add
        case errSecItemNotFound : {
            CFStringRef createItemKeys[4];
            createItemKeys[0] = kSecClass;
            createItemKeys[1] = kSecAttrAccount;
            createItemKeys[2] = kSecValueData;
            createItemKeys[3] = kSecAttrService;

            CFTypeRef createItemValues[4];
            createItemValues[0] = kSecClassGenericPassword;
            createItemValues[1] = CFStringCreateWithCString(kCFAllocatorDefault, key.data(), kCFStringEncodingUTF8);
            createItemValues[2] = CFStringCreateWithCString(kCFAllocatorDefault, value.data(), kCFStringEncodingUTF8);
            createItemValues[3] = isServiceSet ? CFStringCreateWithCString(kCFAllocatorDefault, service.data(), kCFStringEncodingUTF8) : NULL;
            
            CFDictionaryRef createDR;
            createDR = CFDictionaryCreate(kCFAllocatorDefault, (const void**) createItemKeys, (const void**) createItemValues, querySize + 1, NULL, NULL);

            OSStatus createResult = SecItemAdd(createDR, NULL);
            CFRelease(queryDR);
            CFRelease(createDR);

            if(createResult != errSecSuccess) {
                Napi::Error::New(env, "Creation Error (" + std::to_string(createResult) + ")").ThrowAsJavaScriptException();
                return Napi::Boolean::New(env, false);
            }
        }
        break;
        

        // Treat all other responses as an error
        default: {
            CFRelease(queryDR);

            Napi::Error::New(env, "Lookup error (" + std::to_string(queryResult) + ")").ThrowAsJavaScriptException();
            return Napi::Boolean::New(env, false);
        }
        break;
    }
  
    return Napi::Boolean::New(env, true);
}

/**
 * Returns a keychain Generic Password value.
 */
Napi::Value getGenericPassword(const Napi::CallbackInfo& info) {

    Napi::Env env = info.Env();
    bool isServiceSet = info.Length() > 1;
    long querySize = isServiceSet ? 4 : 3;

    if (info.Length() < 1) {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsString() || (isServiceSet &&!info[1].IsString()) ) {
        Napi::TypeError::New(env, "Wrong argument type(s)").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    std::string key     = info[0].As<Napi::String>().Utf8Value();
    std::string service;
    if(isServiceSet) { service = info[1].As<Napi::String>().Utf8Value(); }
    
    // Build query
    CFStringRef queryKeys[4];
    queryKeys[0] = kSecClass;
    queryKeys[1] = kSecAttrAccount;
    queryKeys[2] = kSecReturnData;
    queryKeys[3] = kSecAttrService;

    CFTypeRef queryValues[4];
    queryValues[0] = kSecClassGenericPassword;
    queryValues[1] = CFStringCreateWithCString(kCFAllocatorDefault, key.data(), kCFStringEncodingUTF8);
    queryValues[2] = kCFBooleanTrue;
    queryValues[3] = isServiceSet ? CFStringCreateWithCString(kCFAllocatorDefault, service.data(), kCFStringEncodingUTF8) : NULL;
    
    CFDictionaryRef queryDR;
    queryDR = CFDictionaryCreate(kCFAllocatorDefault, (const void**) queryKeys, (const void**) queryValues, querySize, NULL, NULL);
    

    // Lookup item and value
    CFDataRef item;
    OSStatus queryResult = SecItemCopyMatching(queryDR, (CFTypeRef *)&item);

    if(queryResult == errSecItemNotFound) {
        CFRelease(queryDR);
        if(item) CFRelease(item);
        return env.Null();
    }
    
    if(queryResult != errSecSuccess) {
        CFRelease(queryDR);
        if(item) CFRelease(item);
        Napi::Error::New(env, "Lookup error (" + std::to_string(queryResult) + ")").ThrowAsJavaScriptException();
        return env.Null();
    }

    if(!item) {
        CFRelease(queryDR);
        Napi::Error::New(env, "No data returned (" + std::to_string(queryResult) + ")").ThrowAsJavaScriptException();
        return env.Null();
    }

    // Pull data value
    CFIndex length = CFDataGetLength((CFDataRef)item);
    const UInt8 *dataPtr = CFDataGetBytePtr((CFDataRef)(CFDataRef)item);

    // Cleanup
    CFRelease(item);
    CFRelease(queryDR);

    // Return value
    return Napi::String::New(env, (const char*)dataPtr, length);
}

/**
 * Removes a keychain Generic Password item.
 */
Napi::Value removeGenericPassword(const Napi::CallbackInfo& info) {

    Napi::Env env = info.Env();
    bool isServiceSet = info.Length() > 1;
    long querySize = isServiceSet ? 3 : 2;

    // --- Get passed arguments and validate ------------------------------------------------------------------
    if (info.Length() < 1) {
        Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (!info[0].IsString() || (isServiceSet && !info[1].IsString()) ) {
        Napi::TypeError::New(env, "Wrong argument type(s)").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string key     = info[0].As<Napi::String>().Utf8Value();
    std::string service;
    if(isServiceSet) { service = info[1].As<Napi::String>().Utf8Value(); }

    // Build delete query
    CFStringRef queryKeys[3];
    queryKeys[0] = kSecClass;
    queryKeys[1] = kSecAttrAccount;
    queryKeys[2] = kSecAttrService;

    CFTypeRef queryValues[3];
    queryValues[0] = kSecClassGenericPassword;
    queryValues[1] = CFStringCreateWithCString(kCFAllocatorDefault, key.data(), kCFStringEncodingUTF8);
    queryValues[2] = isServiceSet ? CFStringCreateWithCString(kCFAllocatorDefault, service.data(), kCFStringEncodingUTF8) : NULL;

    CFDictionaryRef queryDR;
    queryDR = CFDictionaryCreate(kCFAllocatorDefault, (const void**) queryKeys, (const void**) queryValues, querySize, NULL, NULL);

    // Remove item
    OSStatus deleteResult = SecItemDelete(queryDR);
    CFRelease(queryDR);

    // Success response
    if(deleteResult == errSecSuccess) {
        return Napi::Boolean::New(env, true);
    }

    // Not found response
    if(deleteResult == errSecItemNotFound) {
        return Napi::Boolean::New(env, false);
    }

    // Error response
    Napi::Error::New(env, "No data returned (" + std::to_string(deleteResult) + ")").ThrowAsJavaScriptException();
    return env.Null();
}


/**
 * Node Initialization Method
 */
Napi::Object init(Napi::Env env, Napi::Object exports) {
    // Registration: Set Generic Password
    exports.Set(Napi::String::New(env, "setGenericPassword"), Napi::Function::New(env, setGenericPassword));

    // Registration: Get Generic Password
    exports.Set(Napi::String::New(env, "getGenericPassword"), Napi::Function::New(env, getGenericPassword));

    // Registration: Remove Generic Password
    exports.Set(Napi::String::New(env, "removeGenericPassword"), Napi::Function::New(env, removeGenericPassword));

    // Return exports
    return exports;
};


// Initialize Module
NODE_API_MODULE(ext-mac-keychain, init);
