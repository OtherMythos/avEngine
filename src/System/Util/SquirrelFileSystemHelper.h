#pragma once

#include "squirrel.h"

namespace AV{

    /**
    Squirrel function implementations similar to sqstdio.
    */
    class SquirrelFileSystemHelper{
    public:
        SquirrelFileSystemHelper() = delete;
        ~SquirrelFileSystemHelper() = delete;

        static SQRESULT sqstd_loadfile(HSQUIRRELVM v, const SQChar *filename, SQBool printerror);
        static SQRESULT sqstd_dofile(HSQUIRRELVM v, const SQChar *filename, SQBool retval, SQBool printerror);
    };
}