#include "gtest/gtest.h"

#include "OgreTextureGpu.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureGpuManagerListener.h"
#include "Scripting/ScriptNamespace/Classes/Ogre/Graphics/TextureUserData.h"

#include <memory>

TEST(TextureUserDataTests, ReleasingStaleHandlePreservesTextureAtReusedAddress){
    std::unique_ptr<SQVM, decltype(&sq_close)> vm(sq_open(128), sq_close);
    AV::TextureUserData::setupDelegateTable(vm.get());

    //Use the same address for both texture lifetimes without relying on GPU allocation.
    alignas(Ogre::TextureGpu) unsigned char storage;
    auto* address = reinterpret_cast<Ogre::TextureGpu*>(&storage);
    AV::TextureUserData::textureToUserData(vm.get(), address, false);

    AV::AVTextureGpuManagerListener listener;
    listener.notifyTextureChanged(address, Ogre::TextureGpuListener::Reason::Deleted, nullptr);
    AV::TextureUserData::textureToUserData(vm.get(), address, false);

    Ogre::TextureGpu* texture;
    bool userOwned, valid;
    ASSERT_EQ(AV::TextureUserData::readTextureFromUserData(vm.get(), 1, &texture, &userOwned, &valid),
              AV::USER_DATA_GET_SUCCESS);
    EXPECT_FALSE(valid);

    sq_remove(vm.get(), 1);
    ASSERT_EQ(AV::TextureUserData::readTextureFromUserData(vm.get(), 1, &texture, &userOwned, &valid),
              AV::USER_DATA_GET_SUCCESS);
    EXPECT_TRUE(valid);
}
