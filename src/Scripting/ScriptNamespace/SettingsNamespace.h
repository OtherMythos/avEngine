#pragma once

#include "ScriptNamespace.h"

namespace AV{
    class SettingsNamespace : public ScriptNamespace{
    public:
        SettingsNamespace() {};
        
        void setupNamespace(HSQUIRRELVM vm);
        
        void setupConstants(HSQUIRRELVM vm);
        
    private:
        static SQInteger getDataDirectory(HSQUIRRELVM vm);
        static SQInteger getMasterDirectory(HSQUIRRELVM vm);
        static SQInteger getWorldSlotSize(HSQUIRRELVM vm);
        static SQInteger getCurrentRenderSystem(HSQUIRRELVM vm);
        
        static SQInteger getSaveDirectoryViable(HSQUIRRELVM vm);
    };
}

