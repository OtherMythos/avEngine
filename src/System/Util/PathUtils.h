#pragma once

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
    bool fileExists(const std::string& path);
}
