#pragma once

#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "Android/OgreAPKFileSystemArchive.h"
#include "Android/OgreAPKZipArchive.h"

#include "sds_fstreamApk.h"

#include "OgreArchiveManager.h"

namespace AV{
    class AndroidSetupHelper{
    public:
        AndroidSetupHelper() = delete;
        ~AndroidSetupHelper() = delete;

        static void setupAndroidAssetManager(){
            JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();

            jobject activity = (jobject)SDL_AndroidGetActivity();

            jclass activity_class = env->GetObjectClass(activity);

            jmethodID activity_class_getAssets = env->GetMethodID(activity_class, "getAssets", "()Landroid/content/res/AssetManager;");
            jobject asset_manager = env->CallObjectMethod(activity, activity_class_getAssets); // activity.getAssets();
            jobject global_asset_manager = env->NewGlobalRef(asset_manager);

            AAssetManager* g_pAssetManager = AAssetManager_fromJava(env, global_asset_manager);

            sds::fstreamApk::ms_assetManager = g_pAssetManager;
            assert(sds::fstreamApk::ms_assetManager);
        }

        static void setupOgreArchiveFactories(){
            Ogre::ArchiveManager::getSingleton().addArchiveFactory( new Ogre::APKFileSystemArchiveFactory(sds::fstreamApk::ms_assetManager) );
            Ogre::ArchiveManager::getSingleton().addArchiveFactory( new Ogre::APKZipArchiveFactory(sds::fstreamApk::ms_assetManager) );
        }
    };
}
