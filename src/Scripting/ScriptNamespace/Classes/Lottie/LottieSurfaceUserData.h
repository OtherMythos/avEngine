#pragma once

#include "Scripting/ScriptNamespace/ScriptUtils.h"
#include "System/EnginePrerequisites.h"

namespace rlottie{
    class Surface;
}

namespace AV{

    class LottieSurfaceWrapper{
        public:
        std::shared_ptr<rlottie::Surface> surface;
        std::shared_ptr<AV::uint32> buffer;

        ~LottieSurfaceWrapper(){
            surface.reset();
            buffer.reset();
        }
    };

    typedef std::shared_ptr<LottieSurfaceWrapper> LottieSurfacePtr;

    class LottieSurfaceUserData{
    public:
        LottieSurfaceUserData() = delete;

        static void setupDelegateTable(HSQUIRRELVM vm);

        static void LottieSurfaceToUserData(HSQUIRRELVM vm, LottieSurfacePtr animation);

        static UserDataGetResult readLottieSurfaceFromUserData(HSQUIRRELVM vm, SQInteger stackInx, LottieSurfacePtr* outObject);

    private:

        static SQObject LottieSurfaceDelegateTableObject;

        static SQInteger setDrawRegion(HSQUIRRELVM vm);
        static SQInteger uploadToTextureBox(HSQUIRRELVM vm);
        static SQInteger LottieSurfaceToString(HSQUIRRELVM vm);

        static SQInteger sqReleaseHook(SQUserPointer p, SQInteger size);

    };
}
