#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include <squirrel.h>
#include <string>

namespace AV{
class ScriptManager;

class Script{
    public:
        Script(HSQUIRRELVM vm);
        Script();
        ~Script();

        bool compileFile(const SQChar *path);
        bool run();
        bool runFunction(const SQChar *entry);
        void release();

        void initialise(HSQUIRRELVM vm);

    private:
        HSQUIRRELVM vm;
        HSQOBJECT obj;

        std::string filePath;

        void _processSquirrelFailure();

        bool available = false;
        bool initialised = false;
    };
}
#endif
