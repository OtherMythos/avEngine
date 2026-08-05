#pragma once

//Test-only subclasses that re-expose protected members of the engine's static singletons
//(promoted from private to protected in their headers specifically for this). Kept as
//subclasses rather than named friends so the production headers stay unaware of test
//names/layout - see AGENTS.md's "Testing private members" section for the rationale.

#include "System/SystemSetup/SystemSettings.h"
#include "System/SystemSetup/UserSettings.h"
#include "World/WorldSingleton.h"

class TestableSystemSettings : public AV::SystemSettings{
public:
    using AV::SystemSettings::_dataPath;
    using AV::SystemSettings::_masterPath;
    using AV::SystemSettings::_worldSlotSize;
    using AV::SystemSettings::mCurrentRenderSystem;
    using AV::SystemSettings::mAvailableRenderSystems;
    using AV::SystemSettings::mUserComponentSettings;
    using AV::SystemSettings::mPhysicsUpdateRate;
    using AV::SystemSettings::mMaxPhysicsStepsPerUpdate;
    using AV::SystemSettings::mScriptWorkersEnabled;
    using AV::SystemSettings::mMaxScriptWorkers;
};

class TestableUserSettings : public AV::UserSettings{
public:
    using AV::UserSettings::mRequestedRenderSystem;
};

class TestableWorldSingleton : public AV::WorldSingleton{
public:
    using AV::WorldSingleton::_world;
    using AV::WorldSingleton::_origin;
    using AV::WorldSingleton::_playerPosition;
    using AV::WorldSingleton::mPlayerLoadRadius;
};
