#pragma once

#include <squirrel.h>

#include "System/SystemSetup/SystemSettings.h"

namespace AV{

    static const std::string resHeader = "res://";
    static const std::string userHeader = "user://";
    static const std::string scriptHeader = "script://";

    /**
    Take a res path and convert it into an absolute path.
    A res path begins with res://, where res represents the data directory specified in the avSetup.cfg file.
    */
    void formatResToPath(const std::string& path, std::string& outPath);

    /**
    As above, resolving script:// against an explicit vm rather than the main one.

    script:// means "the directory of the script which is currently executing", which can only be
    answered by the vm actually running that script. Anything which may execute in a second vm -
    i.e. a script worker - must pass its own vm here, since reading the main vm from another
    thread would race it. Passing null resolves against the main vm, which is what
    formatResToPath does.
    */
    void formatResToPathVM(HSQUIRRELVM vm, const std::string& path, std::string& outPath);

    bool fileExists(const std::string& path);
}
