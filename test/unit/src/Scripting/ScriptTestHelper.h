#pragma once

#include <squirrel.h>
#include <string>

class ScriptTestHelper{
public:
    ScriptTestHelper() {};
    ~ScriptTestHelper() {};

    static void initialise();

    static void executeString(const std::string&s);
    static bool executeStringInt(const std::string&s, int* i);
    static bool executeStringBool(const std::string&s, bool* i);

private:
    static HSQUIRRELVM mSqvm;

    static bool _executeString(const std::string& s);
    static void _processSquirrelFailure(HSQUIRRELVM vm);
};
