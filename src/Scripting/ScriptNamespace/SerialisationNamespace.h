#pragma once

#include "ScriptNamespace.h"

namespace AV{
    class SerialisationNamespace : public ScriptNamespace{
    public:
        SerialisationNamespace() {};

        void setupNamespace(HSQUIRRELVM vm);

        void setupConstants(HSQUIRRELVM vm);

    private:
        static SQInteger getAvailableSaves(HSQUIRRELVM vm);
    };
}
