#pragma once

#include <squirrel.h>
#include <fstream>

namespace AV{

    /**
    Abstracts a simple file interface for squirrel.
    */
    class FileClass{
    public:
        FileClass() = delete;

        static void setupClass(HSQUIRRELVM vm);

        /**
        The mode a file can be opened with.
        These values are exposed to squirrel as constants.
        */
        enum class FileOpenMode : SQInteger{
            READ = 0,
            WRITE = 1,
            READ_WRITE = 2
        };

    private:
        /**
        Data stored against a squirrel File instance.
        As well as the underlying stream, this tracks whether the file was opened
        with read and/or write access so the relevant api calls can be rejected.
        */
        struct FileClassData{
            std::fstream stream;
            bool readable = true;
            bool writable = true;
        };

        static SQObject classObject;

        static SQInteger fileClassConstructor(HSQUIRRELVM vm);
        static SQInteger openFile(HSQUIRRELVM vm);
        static SQInteger closeFile(HSQUIRRELVM vm);
        static SQInteger writeToFile(HSQUIRRELVM vm);
        static SQInteger writeLineToFile(HSQUIRRELVM vm);

        static SQInteger getLine(HSQUIRRELVM vm);
        static SQInteger getEOF(HSQUIRRELVM vm);

        static SQInteger fileReleaseHook(SQUserPointer p, SQInteger size);
    };
}
