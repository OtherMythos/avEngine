#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>
#include <string>

namespace AV{
class ScriptManager;

class Script{
    public:
        Script(HSQUIRRELVM vm);
        ~Script();

        bool compileFile(const SQChar *path);
        bool run();
        bool runFunction(const SQChar *entry);
        void release();

    private:
        HSQUIRRELVM vm;
        HSQOBJECT obj;

        std::string filePath;

        void _processSquirrelFailure();

        bool available = false;
    };
}
#endif
