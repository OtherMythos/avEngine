#pragma once

#include "System/SystemSetup/SystemSettings.h"

#include <filesystem>

namespace AV{

    static const std::string resHeader = "res://";
    static const std::string userHeader = "user://";

    /**
    Take a res path and convert it into an absolute path.
    A res path begins with res://, where res represents the data directory specified in the avSetup.cfg file.
    */
    static void formatResToPath(const std::string& path, std::string& outPath){
        outPath = path;
        if(path.rfind(resHeader, 0) == 0) { //Has the res header at the beginning.
            outPath.replace(0, 6, SystemSettings::getDataPath());
            return;
        }
        if(path.rfind(userHeader, 0) == 0) {
            outPath.replace(0, 7, SystemSettings::getUserDirectoryPath());
            return;
        }
    }

    static bool fileExists(const std::string& path){
        return std::filesystem::exists(path);
    }
}
