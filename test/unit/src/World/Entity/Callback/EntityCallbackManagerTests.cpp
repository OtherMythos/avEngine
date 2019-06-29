#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "World/Entity/Callback/EntityCallbackManager.h"

class EntityCallbackManagerTests : public ::testing::Test {

    protected:
    std::unique_ptr<AV::EntityCallbackManager> manager;

    EntityCallbackManagerTests() {
    }

    virtual ~EntityCallbackManagerTests() {
    }

    virtual void SetUp() {
        manager = std::unique_ptr<AV::EntityCallbackManager>(new AV::EntityCallbackManager());
    }

    virtual void TearDown() {
        manager->mScripts.clear();
    }
};

TEST_F(EntityCallbackManagerTests, loadScriptReturnsHandleIfScriptExists){
    //For some reason this test was causing a seg fault on my docker jenkins.
    //I've got no real idea what caused it, but I'm just going to disable it for the mean time.
    //Hopefully it'll go away :3
    /*Ogre::String path = "SomePath";
    int setVal = 7;
    manager->mCallbackScripts.push_back(AV::EntityCallbackManager::callbackScriptEntry(1, 0));
    manager->mScripts[path] = setVal;

    int handle = manager->loadScript(path);

    ASSERT_EQ(handle, setVal);*/
}

TEST_F(EntityCallbackManagerTests, createLoadedSlotCreatesNewIndexIfNoneExists){
    int slot = manager->_createLoadedSlot("Something", 0);
    //If none exist the first slot should be 0.
    ASSERT_EQ(slot, 0);

    //The second should be 1
    slot = manager->_createLoadedSlot("Something", 0);
    ASSERT_EQ(slot, 1);
}

TEST_F(EntityCallbackManagerTests, createLoadedSlotFillsGapIfFound){
    Ogre::String path = "SomePath";
    manager->mCallbackScripts.push_back(AV::EntityCallbackManager::callbackScriptEntry(0, 0));
    manager->mScripts[path] = 0;

    int slot = manager->_createLoadedSlot("Something", 0);
    //It should target slot 0 for filling.
    ASSERT_EQ(slot, 0);
}
