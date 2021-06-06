#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/Util/VersionedPtr.h"

namespace Ogre{
    class CompositorWorkspace;
}

namespace AV{
    class CompositorWorkspaceUserData{
    private:
        struct CompositorWorkspaceUserDataContents{
            Ogre::CompositorWorkspace* ptr;
            uint64 workspaceId;
        };
    public:
        CompositorWorkspaceUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);
        static void setupConstants(HSQUIRRELVM vm);

        static void workspaceToUserData(HSQUIRRELVM vm, Ogre::CompositorWorkspace* workspace);

        static UserDataGetResult readWorkspaceFromUserData(HSQUIRRELVM vm, SQInteger stackInx, Ogre::CompositorWorkspace** workspace);

    private:
        static SQObject compositorWorkspaceDelegateTableObject;
        static VersionedPtr<Ogre::CompositorWorkspace*> _data;

        static SQInteger setEnabled(HSQUIRRELVM vm);

        static UserDataGetResult _readWorkspacePtrFromUserData(HSQUIRRELVM vm, SQInteger stackInx, CompositorWorkspaceUserDataContents** outObject);

    };
}
