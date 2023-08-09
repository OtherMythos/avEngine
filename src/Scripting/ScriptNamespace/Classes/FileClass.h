#pragma once

#include <squirrel.h>

namespace AV{

    /**
    Abstracts a simple file interface for squirrel.
    */
    class FileClass{
    public:
        FileClass() = delete;

        static void setupClass(HSQUIRRELVM vm);

    private:
        static SQObject classObject;

        static SQInteger fileClassConstructor(HSQUIRRELVM vm);
        static SQInteger openFile(HSQUIRRELVM vm);
        static SQInteger closeFile(HSQUIRRELVM vm);
        static SQInteger writeToFile(HSQUIRRELVM vm);

        static SQInteger getLine(HSQUIRRELVM vm);
        static SQInteger getEOF(HSQUIRRELVM vm);

        static SQInteger fileReleaseHook(SQUserPointer p, SQInteger size);
    };
}
